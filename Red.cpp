#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Red.h"
#include "GrupoHilos.h"
#include "NodoRedInteligente.h"
#include <functional>
#include <iostream>
#include <utility>

namespace {
class RedWindows {
public:
    RedWindows() {
        WSADATA datos;
        if (WSAStartup(MAKEWORD(2, 2), &datos) != 0)
            throw ExcepcionNodoRed("No se pudo iniciar Winsock");
    }
    ~RedWindows() { WSACleanup(); }
};

void fallo(const std::string& operacion) {
    throw ExcepcionNodoRed(operacion + " fallo (Winsock " + std::to_string(WSAGetLastError()) + ")");
}

class Conexion {
    SOCKET descriptor;
public:
    explicit Conexion(SOCKET s) : descriptor(s) {
        if (s == INVALID_SOCKET) fallo("socket/accept");
    }
    Conexion(const Conexion&) = delete;
    Conexion& operator=(const Conexion&) = delete;
    Conexion(Conexion&& otra) noexcept : descriptor(otra.descriptor) {
        otra.descriptor = INVALID_SOCKET;
    }
    ~Conexion() { if (descriptor != INVALID_SOCKET) closesocket(descriptor); }
    SOCKET get() const { return descriptor; }
};

sockaddr_in direccion(const std::string& ip, unsigned short puerto) {
    sockaddr_in dir = {};
    dir.sin_family = AF_INET;
    dir.sin_port = htons(puerto);
    if (inet_pton(AF_INET, ip.c_str(), &dir.sin_addr) != 1)
        throw ExcepcionNodoRed("Direccion IPv4 invalida: " + ip);
    return dir;
}

void configurarTiempo(SOCKET s) {
    DWORD ms = 3500;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms)) ||
        setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&ms), sizeof(ms)))
        fallo("setsockopt");
}

Conexion enlazar(const std::string& ip, unsigned short puerto, int tipo) {
    Conexion s(socket(AF_INET, tipo, 0));
    const sockaddr_in dir = direccion(ip, puerto);
    if (bind(s.get(), reinterpret_cast<const sockaddr*>(&dir), sizeof(dir)) == SOCKET_ERROR)
        fallo("bind");
    configurarTiempo(s.get());
    if (tipo == SOCK_STREAM && listen(s.get(), SOMAXCONN) == SOCKET_ERROR)
        fallo("listen");
    return s;
}

unsigned short puertoLocal(SOCKET s) {
    sockaddr_in dir = {};
    int tam = sizeof(dir);
    if (getsockname(s, reinterpret_cast<sockaddr*>(&dir), &tam) == SOCKET_ERROR)
        fallo("getsockname");
    return ntohs(dir.sin_port);
}

bool listo(SOCKET s, long segundos, bool escritura = false) {
    fd_set conjunto;
    FD_ZERO(&conjunto);
    FD_SET(s, &conjunto);
    timeval tiempo = {segundos, 0};
    const int n = select(0, escritura ? nullptr : &conjunto,
                         escritura ? &conjunto : nullptr, nullptr, &tiempo);
    if (n == SOCKET_ERROR) fallo("select");
    return n > 0;
}

void conectar(SOCKET s, const sockaddr_in& dir) {
    u_long modo = 1;
    if (ioctlsocket(s, FIONBIO, &modo)) fallo("ioctlsocket");
    if (connect(s, reinterpret_cast<const sockaddr*>(&dir), sizeof(dir)) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) fallo("connect");
        fd_set escritura, errores;
        FD_ZERO(&escritura);
        FD_ZERO(&errores);
        FD_SET(s, &escritura);
        FD_SET(s, &errores);
        timeval tiempo = {3, 0};
        const int n = select(0, nullptr, &escritura, &errores, &tiempo);
        if (n == SOCKET_ERROR) fallo("select connect");
        if (n == 0) throw ExcepcionNodoRed("Tiempo de conexion agotado");
        int error = 0;
        int tam = sizeof(error);
        if (getsockopt(s, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &tam))
            fallo("getsockopt");
        if (error || FD_ISSET(s, &errores))
            throw ExcepcionNodoRed("connect fallo (Winsock " + std::to_string(error) + ")");
    }
    modo = 0;
    if (ioctlsocket(s, FIONBIO, &modo)) fallo("ioctlsocket");
    configurarTiempo(s);
}

void enviarLinea(SOCKET s, const std::string& texto) {
    const std::string linea = texto + '\n';
    std::size_t offset = 0;
    while (offset < linea.size()) {
        const int n = send(s, linea.data() + offset, static_cast<int>(linea.size() - offset), 0);
        if (n == SOCKET_ERROR) fallo("send");
        if (n == 0) throw ExcepcionNodoRed("Conexion cerrada durante send");
        offset += static_cast<std::size_t>(n);
    }
}

std::string recibirLinea(SOCKET s) {
    std::string linea;
    char bloque[256];
    while (linea.size() < 1024) {
        const int n = recv(s, bloque, sizeof(bloque), 0);
        if (n == SOCKET_ERROR) fallo("recv");
        if (n == 0) throw ExcepcionNodoRed("Cliente desconectado antes de completar la trama");
        const std::string texto(bloque, n);
        const auto fin = texto.find('\n');
        linea += texto.substr(0, fin);
        if (linea.size() > 1024) break;
        if (fin != std::string::npos) return linea;
    }
    throw ExcepcionNodoRed("Trama TCP demasiado larga");
}

void registrarTrama(const Trama& trama, const std::string& protocolo, Registro& registro) {
    std::lock_guard<std::mutex> bloqueo(registro.mutex);
    registro.tramasRecibidas.push_back(trama);
    std::cout << "[Hilo " << std::this_thread::get_id() << "] " << protocolo
              << " recibido: " << trama.serializar() << '\n';
}

void atenderCliente(Conexion cliente, Registro& registro) {
    try {
        configurarTiempo(cliente.get());
        const Trama trama = Trama::deserializar(recibirLinea(cliente.get()));
        if (trama.clase != "ESTADO") throw ExcepcionNodoRed("Se esperaba estado por TCP");
        registrarTrama(trama, "TCP", registro);
        enviarLinea(cliente.get(), "OK TCP");
    } catch (const ExcepcionNodoRed& e) {
        registrarError(std::string("Servidor TCP: ") + e.what(), registro);
        try { enviarLinea(cliente.get(), "ERROR TCP"); } catch (const std::exception&) {}
    } catch (const std::exception& e) {
        registrarError(e.what(), registro);
    }
}

void aceptarClientes(SOCKET escucha, int cantidad, Registro& registro) {
    GrupoHilos clientes(cantidad);
    try {
        for (int i = 0; i < cantidad; ++i) {
            if (!listo(escucha, 20)) throw ExcepcionNodoRed("Tiempo de espera de clientes agotado");
            Conexion cliente(accept(escucha, nullptr, nullptr));
            clientes.agregar(atenderCliente, std::move(cliente), std::ref(registro));
        }
    } catch (const std::exception& e) {
        registrarError(e.what(), registro);
    }
}

void recibirTelemetria(SOCKET s, Registro& registro) {
    try {
        if (!listo(s, 20)) throw ExcepcionNodoRed("Tiempo de espera de telemetria agotado");
        char datos[1024];
        sockaddr_in remitente = {};
        int tam = sizeof(remitente);
        const int n = recvfrom(s, datos, sizeof(datos), 0, reinterpret_cast<sockaddr*>(&remitente), &tam);
        if (n == SOCKET_ERROR) fallo("recvfrom");
        const Trama trama = Trama::deserializar(std::string(datos, n));
        if (trama.clase != "TEMPERATURA") throw ExcepcionNodoRed("Se esperaba telemetria UDP");
        registrarTrama(trama, "UDP", registro);
        const char respuesta[] = "OK UDP";
        if (sendto(s, respuesta, sizeof(respuesta)-1, 0,
                   reinterpret_cast<const sockaddr*>(&remitente), tam) == SOCKET_ERROR)
            fallo("sendto");
    } catch (const std::exception& e) {
        registrarError(std::string("Servidor UDP: ") + e.what(), registro);
    }
}

void clienteTcp(const std::string& ip, unsigned short puerto, const std::string& trama,
                Registro& registro) {
    Conexion s(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    conectar(s.get(), direccion(ip, puerto));
    enviarLinea(s.get(), trama);
    const std::string respuesta = recibirLinea(s.get());
    if (respuesta != "OK TCP") throw ExcepcionNodoRed("Respuesta TCP: " + respuesta);
    mostrarMensaje("Cliente recibio " + respuesta, registro);
}

void clienteUdp(const std::string& ip, unsigned short puerto, Registro& registro) {
    Conexion s(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    configurarTiempo(s.get());
    const sockaddr_in destino = direccion(ip, puerto);
    const NodoRedInteligente nodo("127.0.0.1", "02:00:00:00:00:03", true, "Temperatura", 24.5, 7);
    const std::string datos = Trama{"TEMPERATURA", "SENSOR-01", nodo.getDireccionIP(),
                                   nodo.getValorLectura()}.serializar();
    if (sendto(s.get(), datos.data(), static_cast<int>(datos.size()), 0,
               reinterpret_cast<const sockaddr*>(&destino), sizeof(destino)) == SOCKET_ERROR)
        fallo("sendto");
    char respuesta[32];
    sockaddr_in remitente = {};
    int tam = sizeof(remitente);
    const int n = recvfrom(s.get(), respuesta, sizeof(respuesta), 0,
                          reinterpret_cast<sockaddr*>(&remitente), &tam);
    if (n == SOCKET_ERROR) fallo("recvfrom");
    if (remitente.sin_addr.s_addr != destino.sin_addr.s_addr ||
        remitente.sin_port != destino.sin_port || std::string(respuesta, n) != "OK UDP")
        throw ExcepcionNodoRed("Respuesta UDP invalida");
    mostrarMensaje("Sensor recibio OK UDP", registro);
}

void probarCliente(std::string ip, unsigned short puerto, std::string datos, Registro& registro) {
    try { clienteTcp(ip, puerto, datos, registro); }
    catch (const std::exception& e) { registrarError(e.what(), registro); }
}

void probarUdp(std::string ip, unsigned short puerto, Registro& registro) {
    try { clienteUdp(ip, puerto, registro); }
    catch (const std::exception& e) { registrarError(e.what(), registro); }
}
}

void ServidorCentral::atender(const std::string& ip, unsigned short puerto, int clientes,
                             Registro& registro) {
    RedWindows red;
    Conexion tcp = enlazar(ip, puerto, SOCK_STREAM);
    Conexion udp = enlazar(ip, puerto, SOCK_DGRAM);
    mostrarMensaje("Servidor listo en " + ip + ":" + std::to_string(puerto) + " TCP/UDP", registro);
    GrupoHilos hilos(1);
    hilos.agregar(recibirTelemetria, udp.get(), std::ref(registro));
    aceptarClientes(tcp.get(), clientes, registro);
}

void enviarEstado(const std::string& servidor, unsigned short puerto,
                  const DispositivoComunicacion& dispositivo, Registro& registro) {
    RedWindows red;
    clienteTcp(servidor, puerto, dispositivo.serializar(), registro);
}

void enviarTelemetria(const std::string& servidor, unsigned short puerto, Registro& registro) {
    RedWindows red;
    clienteUdp(servidor, puerto, registro);
}

void demostrarRed(Registro& registro) {
    RedWindows red;
    const std::string local = "127.0.0.1";
    Conexion tcp = enlazar(local, 0, SOCK_STREAM);
    Conexion udp = enlazar(local, 0, SOCK_DGRAM);
    const auto puertoTcp = puertoLocal(tcp.get());
    const auto puertoUdp = puertoLocal(udp.get());
    RouterWiFi router("RTR-01", 12);
    GatewayLoRa gateway("GW-01", 868.1f);

    GrupoHilos hilos(6);
    hilos.agregar(aceptarClientes, tcp.get(), 4, std::ref(registro));
    hilos.agregar(recibirTelemetria, udp.get(), std::ref(registro));
    hilos.agregar(probarCliente, local, puertoTcp, router.serializar(), std::ref(registro));
    hilos.agregar(probarCliente, local, puertoTcp, gateway.serializar(), std::ref(registro));
    hilos.agregar(probarUdp, local, puertoUdp, std::ref(registro));
    hilos.agregar(probarCliente, local, puertoTcp, "TRAMA INVALIDA", std::ref(registro));
    {
        Conexion desconexion(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
        conectar(desconexion.get(), direccion(local, puertoTcp));
    }
    hilos.esperar();

    if (registro.tramasRecibidas.size() != 3 || registro.errores != 3)
        throw ExcepcionNodoRed("La prueba de red no produjo los resultados esperados");

    Conexion sinEscucha(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    const sockaddr_in dir = direccion(local, 0);
    if (bind(sinEscucha.get(), reinterpret_cast<const sockaddr*>(&dir), sizeof(dir)))
        fallo("bind de prueba");
    try {
        clienteTcp(local, puertoLocal(sinEscucha.get()), router.serializar(), registro);
        throw std::logic_error("Se esperaba rechazo de conexion");
    } catch (const ExcepcionNodoRed& e) {
        mostrarMensaje(std::string("Fallo de conexion controlado: ") + e.what(), registro);
    }
    mostrarMensaje("Red verificada: 2 tramas TCP y 1 UDP, trama invalida y desconexion controladas", registro);
}
