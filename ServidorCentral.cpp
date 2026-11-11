#include "ServidorCentral.h"
#include "Excepciones.h"
#include "Util.h"
#include <utility>

namespace {
const int TICK_MS = 200;  // los sockets despiertan cada 200 ms para revisar si el servidor debe detenerse
}

ServidorCentral::ServidorCentral(Registro& reg, int timeoutInactividad)
    : registro(reg), timeoutInactividadMs(timeoutInactividad), activo(false), aceptadasTcp(0), aceptadasUdp(0),
      rechazadas(0), clientesAtendidos(0) {}

ServidorCentral::~ServidorCentral() { detener(); }

void ServidorCentral::iniciar(const std::string& ip, std::uint16_t puertoTcp, std::uint16_t puertoUdp) {
    if (activo.load()) throw ExcepcionRedTCP("El servidor ya esta iniciado");
    Socket nuevoTcp(TipoSocket::TCP);  // socket()
    nuevoTcp.enlazar(ip, puertoTcp);   // bind()   (lanza si el puerto esta ocupado)
    nuevoTcp.escuchar();               // listen()
    Socket nuevoUdp(TipoSocket::UDP);  // socket()
    nuevoUdp.enlazar(ip, puertoUdp);   // bind()
    nuevoUdp.fijarTimeout(TICK_MS);    // recvfrom() no se queda bloqueado para siempre
    tcp = std::move(nuevoTcp);
    udp = std::move(nuevoUdp);
    activo = true;
    try {
        hiloAceptador = std::thread(&ServidorCentral::bucleAceptar, this);
        hiloUdp = std::thread(&ServidorCentral::bucleUdp, this);
    } catch (...) {
        detener();  // revierte un inicio parcial antes de propagar el fallo de recursos
        throw;
    }
}

void ServidorCentral::detener() {
    activo = false;
    if (hiloAceptador.joinable()) hiloAceptador.join();  // primero el aceptador: ya no nacen clientes nuevos
    if (hiloUdp.joinable()) hiloUdp.join();
    std::vector<std::thread> pendientes;
    {
        std::lock_guard<std::mutex> bloqueo(mtxClientes);
        pendientes.swap(hilosClientes);
    }
    for (std::size_t i = 0; i < pendientes.size(); ++i)
        if (pendientes[i].joinable()) pendientes[i].join();
    tcp.cerrar();
    udp.cerrar();
}

void ServidorCentral::bucleAceptar() {
    try {
        while (activo.load()) {
            if (!tcp.esperarLectura(TICK_MS)) continue;  // sin conexiones nuevas: vuelve a revisar 'activo'
            Socket cliente = tcp.aceptar();              // accept(): devuelve un socket NUEVO por cliente
            std::lock_guard<std::mutex> bloqueo(mtxClientes);
            // Construccion directa: evita un temporal joinable si el vector no puede crecer.
            hilosClientes.emplace_back(&ServidorCentral::atenderCliente, this, std::move(cliente));
        }
    } catch (const std::exception& e) {
        if (activo.load()) registro.registrarError(std::string("Servidor TCP (accept): ") + e.what());
    } catch (...) {
        registro.registrarError("Servidor TCP (accept): fallo desconocido");
    }
}

Protocolo::CodigoRespuesta ServidorCentral::procesar(const std::uint8_t* datos, std::size_t n, const char* canal,
                                                     std::uint32_t& secuencia) {
    secuencia = Protocolo::secuenciaDe(datos, n);
    try {
        const Protocolo::Trama trama = Protocolo::deserializar(datos, n);  // valida todo
        registro.agregarTrama(std::string(canal) + " " + Protocolo::describir(trama));
        return Protocolo::ACK;
    } catch (const ExcepcionNodoRed& e) {
        ++rechazadas;
        registro.registrarError(std::string("Servidor ") + canal + ": trama rechazada (" + e.what() + ")");
        return Protocolo::NACK;
    }
}

void ServidorCentral::atenderCliente(Socket cliente) {
    ++clientesAtendidos;
    try {
        cliente.fijarTimeout(TICK_MS);
        int inactivoMs = 0;
        while (activo.load()) {
            std::uint8_t buffer[Protocolo::TAM_TRAMA];
            const EstadoRecepcion estado = cliente.recibirExacto(buffer, sizeof(buffer));
            if (estado == EstadoRecepcion::CERRADO) {  // recv() devolvio 0: el cliente se desconecto
                registro.imprimir("Servidor TCP: cliente desconectado");
                break;
            }
            if (estado == EstadoRecepcion::TIMEOUT) {
                inactivoMs += TICK_MS;
                if (inactivoMs >= timeoutInactividadMs) {
                    registro.registrarError("Servidor TCP: cliente inactivo, se cierra la conexion");
                    break;
                }
                continue;
            }
            inactivoMs = 0;
            std::uint32_t secuencia = 0;
            const Protocolo::CodigoRespuesta codigo = procesar(buffer, sizeof(buffer), "TCP", secuencia);
            if (codigo == Protocolo::ACK) ++aceptadasTcp;
            const std::vector<std::uint8_t> respuesta = Protocolo::serializarRespuesta(codigo, secuencia);
            cliente.enviarTodo(&respuesta[0], respuesta.size());
        }
    } catch (const std::exception& e) {  // el fallo de UN cliente no tumba el servidor
        registro.registrarError(std::string("Servidor TCP: ") + e.what());
    } catch (...) {
        registro.registrarError("Servidor TCP: fallo desconocido atendiendo un cliente");
    }
    // 'cliente' sale de alcance: su destructor ejecuta closesocket()/close() (cierre explicito y seguro)
}

void ServidorCentral::bucleUdp() {
    try {
        while (activo.load()) {
            std::uint8_t buffer[512];
            std::size_t recibidos = 0;
            std::string ipOrigen;
            std::uint16_t puertoOrigen = 0;
            const EstadoRecepcion estado = udp.recibirDe(buffer, sizeof(buffer), recibidos, ipOrigen, puertoOrigen);
            if (estado == EstadoRecepcion::TIMEOUT) continue;
            if (estado == EstadoRecepcion::DESCARTADO) {
                ++rechazadas;
                registro.registrarError("Servidor UDP: datagrama truncado descartado (supera 512 bytes)");
                continue;  // sin ACK/NACK: no se interpreta un mensaje incompleto
            }
            std::uint32_t secuencia = 0;
            const Protocolo::CodigoRespuesta codigo = procesar(buffer, recibidos, "UDP", secuencia);
            if (codigo == Protocolo::ACK) ++aceptadasUdp;
            try {
                const std::vector<std::uint8_t> respuesta = Protocolo::serializarRespuesta(codigo, secuencia);
                udp.enviarA(ipOrigen, puertoOrigen, &respuesta[0], respuesta.size());
            } catch (const std::exception& e) {
                registro.registrarError(std::string("Servidor UDP (respuesta): ") + e.what());
            }
        }
    } catch (const std::exception& e) {
        if (activo.load()) registro.registrarError(std::string("Servidor UDP: ") + e.what());
    } catch (...) {
        registro.registrarError("Servidor UDP: fallo desconocido");
    }
}
