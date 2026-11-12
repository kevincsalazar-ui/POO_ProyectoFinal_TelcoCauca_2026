#include "Socket.h"
#include "DispositivoRed.h"
#include "Excepciones.h"
#include "Util.h"
#include <cstring>

namespace {

#ifdef _WIN32
int codigoError() { return WSAGetLastError(); }
bool esTimeout(int c) { return c == WSAETIMEDOUT || c == WSAEWOULDBLOCK; }
bool esReinicioUdp(int c) { return c == WSAECONNRESET; }  // ICMP "puerto inalcanzable" previo
bool enProgreso(int c) { return c == WSAEWOULDBLOCK; }
SocketHandle invalido() { return INVALID_SOCKET; }
void cerrarHandle(SocketHandle h) { closesocket(h); }
const int FLAGS_ENVIO = 0;
#else
int codigoError() { return errno; }
bool esTimeout(int c) { return c == EAGAIN || c == EWOULDBLOCK || c == ETIMEDOUT; }
bool esReinicioUdp(int c) { return c == ECONNREFUSED; }
bool enProgreso(int c) { return c == EINPROGRESS; }
SocketHandle invalido() { return -1; }
void cerrarHandle(SocketHandle h) { ::close(h); }
const int FLAGS_ENVIO = MSG_NOSIGNAL;  // evita SIGPIPE al escribir en una conexion cerrada
#endif

[[noreturn]] void fallo(TipoSocket tipo, const std::string& operacion, int codigo) {
    const std::string mensaje = operacion + " fallo (codigo " + aTexto(codigo) + ")";
    if (tipo == TipoSocket::TCP) throw ExcepcionRedTCP(mensaje);
    throw ExcepcionRedUDP(mensaje);
}

[[noreturn]] void falloTexto(TipoSocket tipo, const std::string& mensaje) {
    if (tipo == TipoSocket::TCP) throw ExcepcionRedTCP(mensaje);
    throw ExcepcionRedUDP(mensaje);
}

sockaddr_in direccion(const std::string& ip, std::uint16_t puerto) {
    if (!DispositivoRed::ipValida(ip)) throw ExcepcionNodoRed("Direccion IPv4 invalida: " + ip);
    sockaddr_in a;
    std::memset(&a, 0, sizeof(a));
    a.sin_family = AF_INET;
    a.sin_port = htons(puerto);
    a.sin_addr.s_addr = inet_addr(ip.c_str());
    return a;
}

std::string textoIp(const sockaddr_in& a) {
    const unsigned char* b = reinterpret_cast<const unsigned char*>(&a.sin_addr.s_addr);
    return aTexto(static_cast<int>(b[0])) + "." + aTexto(static_cast<int>(b[1])) + "." +
           aTexto(static_cast<int>(b[2])) + "." + aTexto(static_cast<int>(b[3]));
}

}

SistemaRed::SistemaRed() {
#ifdef _WIN32
    WSADATA datos;
    if (WSAStartup(MAKEWORD(2, 2), &datos) != 0) throw ExcepcionRedTCP("No se pudo iniciar Winsock (WSAStartup)");
#endif
}

SistemaRed::~SistemaRed() {
#ifdef _WIN32
    WSACleanup();
#endif
}

bool Socket::valido() const { return handle != invalido(); }

Socket::Socket() : handle(invalido()), tipo(TipoSocket::TCP) {}

Socket::Socket(TipoSocket t) : handle(invalido()), tipo(t) {
    handle = ::socket(AF_INET, t == TipoSocket::TCP ? SOCK_STREAM : SOCK_DGRAM,
                      t == TipoSocket::TCP ? IPPROTO_TCP : IPPROTO_UDP);
    if (!valido()) fallo(t, "socket()", codigoError());
}

Socket::Socket(SocketHandle existente, TipoSocket t) : handle(existente), tipo(t) {}

Socket::Socket(Socket&& otro) noexcept : handle(otro.handle), tipo(otro.tipo) { otro.handle = invalido(); }

Socket& Socket::operator=(Socket&& otro) noexcept {
    if (this != &otro) {
        cerrar();
        handle = otro.handle;
        tipo = otro.tipo;
        otro.handle = invalido();
    }
    return *this;
}

Socket::~Socket() { cerrar(); }

void Socket::cerrar() {
    if (valido()) {
        cerrarHandle(handle);
        handle = invalido();
    }
}

void Socket::bloqueante(bool si) {
#ifdef _WIN32
    u_long modo = si ? 0 : 1;
    if (ioctlsocket(handle, FIONBIO, &modo) != 0) fallo(tipo, "ioctlsocket(FIONBIO)", codigoError());
#else
    const int flags = fcntl(handle, F_GETFL, 0);
    if (flags < 0) fallo(tipo, "fcntl(F_GETFL)", codigoError());
    if (fcntl(handle, F_SETFL, si ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK)) < 0)
        fallo(tipo, "fcntl(F_SETFL)", codigoError());
#endif
}

void Socket::fijarTimeout(int ms) {
    if (ms <= 0) falloTexto(tipo, "El timeout debe ser positivo");
#ifdef _WIN32
    const DWORD t = static_cast<DWORD>(ms);
    if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&t), sizeof(t)) != 0)
        fallo(tipo, "setsockopt(SO_RCVTIMEO)", codigoError());
    if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&t), sizeof(t)) != 0)
        fallo(tipo, "setsockopt(SO_SNDTIMEO)", codigoError());
#else
    timeval t;
    t.tv_sec = ms / 1000;
    t.tv_usec = (ms % 1000) * 1000;
    if (setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) != 0)
        fallo(tipo, "setsockopt(SO_RCVTIMEO)", codigoError());
    if (setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) != 0)
        fallo(tipo, "setsockopt(SO_SNDTIMEO)", codigoError());
#endif
}

void Socket::enlazar(const std::string& ip, std::uint16_t puerto) {
#ifndef _WIN32
  
    if (tipo == TipoSocket::TCP) {
        int uno = 1;
        if (setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &uno, sizeof(uno)) != 0)
            fallo(tipo, "setsockopt(SO_REUSEADDR)", codigoError());
    }
#endif
    const sockaddr_in a = direccion(ip, puerto);
    if (::bind(handle, reinterpret_cast<const sockaddr*>(&a), sizeof(a)) != 0) {
        const int codigo = codigoError();
        falloTexto(tipo, "bind() en " + ip + ":" + aTexto(puerto) + " fallo (puerto ocupado o sin permiso, codigo " +
                             aTexto(codigo) + ")");
    }
}

void Socket::escuchar(int cola) {
    if (::listen(handle, cola) != 0) fallo(tipo, "listen()", codigoError());
}

bool Socket::esperarLectura(int ms) {
    if (!valido() || ms < 0) falloTexto(tipo, "select(): socket invalido o espera negativa");
#ifndef _WIN32
    if (handle >= FD_SETSIZE) falloTexto(tipo, "select(): descriptor fuera de FD_SETSIZE");
#endif
    fd_set lectura;
    FD_ZERO(&lectura);
    FD_SET(handle, &lectura);
    timeval t;
    t.tv_sec = ms / 1000;
    t.tv_usec = (ms % 1000) * 1000;
    const int r = ::select(static_cast<int>(handle) + 1, &lectura, nullptr, nullptr, &t);
    if (r < 0) fallo(tipo, "select()", codigoError());
    return r > 0;
}

Socket Socket::aceptar() {
    const SocketHandle nuevo = ::accept(handle, nullptr, nullptr);
    if (nuevo == invalido()) fallo(tipo, "accept()", codigoError());
    return Socket(nuevo, tipo);
}

std::uint16_t Socket::puertoLocal() const {
    sockaddr_in a;
    TamanoDireccion tam = sizeof(a);
    if (::getsockname(handle, reinterpret_cast<sockaddr*>(&a), &tam) != 0) fallo(tipo, "getsockname()", codigoError());
    return ntohs(a.sin_port);
}

void Socket::conectar(const std::string& ip, std::uint16_t puerto, int timeoutMs) {
    if (!valido() || timeoutMs <= 0) falloTexto(tipo, "connect(): socket invalido o timeout no positivo");
#ifndef _WIN32
    if (handle >= FD_SETSIZE) falloTexto(tipo, "connect(): descriptor fuera de FD_SETSIZE");
#endif
    const sockaddr_in a = direccion(ip, puerto);
    try {
        bloqueante(false);  // connect no bloqueante + select = limite de tiempo
        if (::connect(handle, reinterpret_cast<const sockaddr*>(&a), sizeof(a)) != 0) {
            const int codigo = codigoError();
            if (!enProgreso(codigo)) fallo(tipo, "connect() a " + ip + ":" + aTexto(puerto), codigo);
            fd_set escritura, error;
            FD_ZERO(&escritura);
            FD_ZERO(&error);
            FD_SET(handle, &escritura);
            FD_SET(handle, &error);
            timeval t;
            t.tv_sec = timeoutMs / 1000;
            t.tv_usec = (timeoutMs % 1000) * 1000;
            const int r = ::select(static_cast<int>(handle) + 1, nullptr, &escritura, &error, &t);
            if (r < 0) fallo(tipo, "select(connect)", codigoError());
            if (r == 0) falloTexto(tipo, "connect(): tiempo de espera agotado");
            int err = 0;
            TamanoDireccion tam = sizeof(err);
            if (getsockopt(handle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &tam) != 0)
                fallo(tipo, "getsockopt(SO_ERROR)", codigoError());
            if (err != 0) fallo(tipo, "connect() rechazado", err);
            if (FD_ISSET(handle, &error) || !FD_ISSET(handle, &escritura))
                falloTexto(tipo, "connect(): estado de conexion inesperado");
        }
        bloqueante(true);
    } catch (...) {
        // No dejar un socket de conexion fallida en un modo de bloqueo desconocido.
        cerrar();
        throw;
    }
}

void Socket::enviarTodo(const std::uint8_t* datos, std::size_t n) {
    std::size_t total = 0;
    while (total < n) {
        const int r = ::send(handle, reinterpret_cast<const char*>(datos + total), static_cast<int>(n - total), FLAGS_ENVIO);
        if (r <= 0) fallo(tipo, "send()", codigoError());
        total += static_cast<std::size_t>(r);
    }
}

EstadoRecepcion Socket::recibirExacto(std::uint8_t* datos, std::size_t n) {
    std::size_t total = 0;
    while (total < n) {
        const int r = ::recv(handle, reinterpret_cast<char*>(datos + total), static_cast<int>(n - total), 0);
        if (r > 0) {
            total += static_cast<std::size_t>(r);
        } else if (r == 0) {  // el otro extremo cerro ordenadamente
            if (total == 0) return EstadoRecepcion::CERRADO;
            falloTexto(tipo, "Conexion cerrada a mitad de trama (" + aTexto(total) + " de " + aTexto(n) + " bytes)");
        } else {
            const int codigo = codigoError();
            if (esTimeout(codigo)) {
                if (total == 0) return EstadoRecepcion::TIMEOUT;
                falloTexto(tipo, "Tiempo agotado a mitad de trama (" + aTexto(total) + " de " + aTexto(n) + " bytes)");
            }
            fallo(tipo, "recv()", codigo);
        }
    }
    return EstadoRecepcion::OK;
}

void Socket::enviarA(const std::string& ip, std::uint16_t puerto, const std::uint8_t* datos, std::size_t n) {
    const sockaddr_in a = direccion(ip, puerto);
    const int r = ::sendto(handle, reinterpret_cast<const char*>(datos), static_cast<int>(n), 0,
                           reinterpret_cast<const sockaddr*>(&a), sizeof(a));
    if (r < 0) fallo(tipo, "sendto()", codigoError());
    if (static_cast<std::size_t>(r) != n) falloTexto(tipo, "sendto(): datagrama incompleto");
}

EstadoRecepcion Socket::recibirDe(std::uint8_t* datos, std::size_t max, std::size_t& recibidos, std::string& ipOrigen,
                                  std::uint16_t& puertoOrigen) {
    recibidos = 0;
    ipOrigen.clear();
    puertoOrigen = 0;
    sockaddr_in origen;
    std::memset(&origen, 0, sizeof(origen));
    TamanoDireccion tam = sizeof(origen);
#ifdef _WIN32
    const int r = ::recvfrom(handle, reinterpret_cast<char*>(datos), static_cast<int>(max), 0,
                             reinterpret_cast<sockaddr*>(&origen), &tam);
#else
    // recvmsg informa truncamiento tambien si los primeros bytes parecen una trama valida.
    iovec fragmento;
    fragmento.iov_base = datos;
    fragmento.iov_len = max;
    msghdr mensaje;
    std::memset(&mensaje, 0, sizeof(mensaje));
    mensaje.msg_name = &origen;
    mensaje.msg_namelen = tam;
    mensaje.msg_iov = &fragmento;
    mensaje.msg_iovlen = 1;
    const int r = static_cast<int>(::recvmsg(handle, &mensaje, 0));
    if (r >= 0 && (mensaje.msg_flags & MSG_TRUNC)) return EstadoRecepcion::DESCARTADO;
#endif
    if (r < 0) {
        const int codigo = codigoError();
#ifdef _WIN32
        
        if (codigo == WSAEMSGSIZE) return EstadoRecepcion::DESCARTADO;
#endif
        if (esTimeout(codigo) || esReinicioUdp(codigo)) return EstadoRecepcion::TIMEOUT;
        fallo(tipo, "recvfrom()", codigo);
    }
    recibidos = static_cast<std::size_t>(r);
    ipOrigen = textoIp(origen);
    puertoOrigen = ntohs(origen.sin_port);
    return EstadoRecepcion::OK;
}
