#include "Clientes.h"
#include "Dispositivos.h"
#include "Excepciones.h"
#include "NodoRedInteligente.h"
#include "Socket.h"
#include "Util.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace {

void pausar(int ms) {
    if (ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

Protocolo::CodigoRespuesta contar(const std::uint8_t* datos, std::size_t n, std::uint32_t esperada, ResultadoCliente& r, bool udp) {
    Protocolo::CodigoRespuesta codigo = Protocolo::ACK;
    std::uint32_t secuencia = 0;
    if (!Protocolo::interpretarRespuesta(datos, n, codigo, secuencia)) {
        if (udp) throw ExcepcionRedUDP("Respuesta del servidor con formato invalido");
        throw ExcepcionRedTCP("Respuesta del servidor con formato invalido");
    }
    if (secuencia != esperada) {
        if (udp) throw ExcepcionRedUDP("La respuesta no corresponde a la trama enviada");
        throw ExcepcionRedTCP("La respuesta no corresponde a la trama enviada");
    }
    if (codigo == Protocolo::ACK) ++r.ack;
    else ++r.nack;
    return codigo;
}

}  // namespace

ResultadoCliente clienteTcp(const std::string& ip, std::uint16_t puerto, const std::vector<Protocolo::Trama>& tramas,
                            Registro& registro, int pausaMs, int timeoutMs) {
    ResultadoCliente resultado;
    Socket s(TipoSocket::TCP);
    s.fijarTimeout(timeoutMs);
    s.conectar(ip, puerto, timeoutMs);  // connect()
    for (std::size_t i = 0; i < tramas.size(); ++i) {
        const std::vector<std::uint8_t> bytes = Protocolo::serializar(tramas[i]);
        s.enviarTodo(&bytes[0], bytes.size());  // send()
        ++resultado.enviadas;
        std::uint8_t respuesta[Protocolo::TAM_RESPUESTA];
        const EstadoRecepcion estado = s.recibirExacto(respuesta, sizeof(respuesta));  // recv()
        if (estado == EstadoRecepcion::TIMEOUT) throw ExcepcionRedTCP("El servidor no respondio a tiempo");
        if (estado == EstadoRecepcion::CERRADO) throw ExcepcionRedTCP("El servidor cerro la conexion");
        const Protocolo::CodigoRespuesta codigo = contar(respuesta, sizeof(respuesta), tramas[i].secuencia, resultado, false);
        registro.imprimir("Cliente TCP " + tramas[i].origen + ": " + Protocolo::describir(tramas[i]) + " -> " +
                          (codigo == Protocolo::ACK ? "ACK" : "NACK"));
        pausar(pausaMs);
    }
    return resultado;
}

ResultadoCliente clienteUdp(const std::string& ip, std::uint16_t puerto, const std::vector<Protocolo::Trama>& tramas,
                            Registro& registro, int pausaMs, int timeoutMs) {
    ResultadoCliente resultado;
    Socket s(TipoSocket::UDP);
    s.fijarTimeout(timeoutMs);
    for (std::size_t i = 0; i < tramas.size(); ++i) {
        const std::vector<std::uint8_t> bytes = Protocolo::serializar(tramas[i]);
        s.enviarA(ip, puerto, &bytes[0], bytes.size());  // sendto()
        ++resultado.enviadas;
        std::uint8_t respuesta[64];
        std::size_t recibidos = 0;
        std::string ipOrigen;
        std::uint16_t puertoOrigen = 0;
        const EstadoRecepcion estado = s.recibirDe(respuesta, sizeof(respuesta), recibidos, ipOrigen, puertoOrigen);
        if (estado == EstadoRecepcion::TIMEOUT)
            throw ExcepcionRedUDP("Sin respuesta UDP dentro del tiempo de espera");
        if (estado != EstadoRecepcion::OK) throw ExcepcionRedUDP("Respuesta UDP truncada descartada");
        // Comparar extremo completo antes de contar ACK. Esto no es autenticacion criptografica.
        if (ipOrigen != ip || puertoOrigen != puerto)
            throw ExcepcionRedUDP("Respuesta UDP de un origen distinto del servidor esperado");
        const Protocolo::CodigoRespuesta codigo = contar(respuesta, recibidos, tramas[i].secuencia, resultado, true);
        registro.imprimir("Cliente UDP " + tramas[i].origen + ": " + Protocolo::describir(tramas[i]) + " -> " +
                          (codigo == Protocolo::ACK ? "ACK" : "NACK"));
        pausar(pausaMs);
    }
    return resultado;
}

int ejecutarRol(const std::string& rol, const std::string& ip, std::uint16_t puerto) {
    Registro registro;
    std::vector<Protocolo::Trama> tramas;
    try {
        if (rol == "router") {
            RouterWiFi router("RTR-01", 4, ip);
            for (std::uint32_t i = 1; i <= 3; ++i) tramas.push_back(router.aTrama(i));
        } else if (rol == "gateway") {
            GatewayLoRa gateway("GW-01", 9, ip);
            for (std::uint32_t i = 1; i <= 3; ++i) tramas.push_back(gateway.aTrama(i));
        } else if (rol == "sensor") {
            for (std::uint32_t i = 1; i <= 3; ++i) {
                NodoRedInteligente nodo(ip, "02:00:00:00:00:03", true, "Temperatura", 23.5 + i, 7);
                tramas.push_back(Protocolo::crearTemperatura("SENSOR-01", nodo.getDireccionIP(), i, nodo.getValorLectura()));
            }
        } else {
            std::cout << "Rol desconocido: " << rol << std::endl;
            return 2;
        }
        ResultadoCliente r = (rol == "sensor") ? clienteUdp(ip, puerto, tramas, registro, 400)
                                               : clienteTcp(ip, puerto, tramas, registro, 400);
        registro.imprimir("Resumen " + rol + ": enviadas=" + aTexto(r.enviadas) + " ACK=" + aTexto(r.ack) +
                          " NACK=" + aTexto(r.nack));
        return (r.ack == r.enviadas) ? 0 : 1;
    } catch (const std::exception& e) {
        registro.registrarError("Cliente " + rol + ": " + e.what());
        return 1;
    }
}
