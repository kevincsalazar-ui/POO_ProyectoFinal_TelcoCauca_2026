#ifndef SOCKET_H
#define SOCKET_H
#include "PlataformaRed.h"
#include <cstddef>
#include <cstdint>
#include <string>

// RAII de Winsock: WSAStartup al crearse, WSACleanup al destruirse (no hace nada en Linux).
class SistemaRed {
public:
    SistemaRed();
    ~SistemaRed();
    SistemaRed(const SistemaRed&) = delete;
    SistemaRed& operator=(const SistemaRed&) = delete;
};

enum class TipoSocket { TCP, UDP };
enum class EstadoRecepcion { OK, CERRADO, TIMEOUT, DESCARTADO };

// Envoltorio RAII de un socket: el destructor cierra el descriptor (un solo duenio, solo se mueve).
// Los errores se lanzan como ExcepcionRedTCP o ExcepcionRedUDP segun el tipo del socket.
class Socket {
    SocketHandle handle;
    TipoSocket tipo;

    bool valido() const;
    void bloqueante(bool si);

public:
    Socket();                       // sin descriptor
    explicit Socket(TipoSocket t);  // socket()
    Socket(SocketHandle existente, TipoSocket t);
    Socket(Socket&& otro) noexcept;
    Socket& operator=(Socket&& otro) noexcept;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    ~Socket();

    void cerrar();
    bool abierto() const { return valido(); }

    // Servidor
    void enlazar(const std::string& ip, std::uint16_t puerto);  // bind()
    void escuchar(int cola = 16);                                // listen()
    bool esperarLectura(int milisegundos);                       // select(): hay algo que leer/aceptar?
    Socket aceptar();                                            // accept()
    std::uint16_t puertoLocal() const;                           // getsockname()

    // Cliente
    void conectar(const std::string& ip, std::uint16_t puerto, int timeoutMs);  // connect() con limite de tiempo

    // TCP
    void enviarTodo(const std::uint8_t* datos, std::size_t n);  // repite send() hasta enviar todo
    EstadoRecepcion recibirExacto(std::uint8_t* datos, std::size_t n);  // repite recv() hasta n bytes

    // UDP: DESCARTADO significa datagrama consumido pero truncado; no interpretar el buffer.
    void enviarA(const std::string& ip, std::uint16_t puerto, const std::uint8_t* datos, std::size_t n);  // sendto()
    EstadoRecepcion recibirDe(std::uint8_t* datos, std::size_t max, std::size_t& recibidos, std::string& ipOrigen,
                              std::uint16_t& puertoOrigen);  // recvfrom()

    void fijarTimeout(int milisegundos);  // SO_RCVTIMEO y SO_SNDTIMEO
};

#endif
