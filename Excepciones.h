#ifndef EXCEPCIONES_H
#define EXCEPCIONES_H
// Jerarquia de excepciones personalizadas. Todas derivan de std::runtime_error,
// de modo que un catch (const std::exception&) las captura de forma polimorfica.
#include <stdexcept>
#include <string>

// Datos de un nodo/trama invalidos: IP incorrecta, lectura fuera de rango, trama corrupta.
class ExcepcionNodoRed : public std::runtime_error {
public:
    explicit ExcepcionNodoRed(const std::string& mensaje) : std::runtime_error(mensaje) {}
};

// Fallos de la capa TCP: bind ocupado, connect rechazado, cierre a mitad de trama, timeout.
class ExcepcionRedTCP : public std::runtime_error {
public:
    explicit ExcepcionRedTCP(const std::string& mensaje) : std::runtime_error(mensaje) {}
};

// Fallos de la capa UDP: sendto/recvfrom, sin respuesta dentro del tiempo de espera.
class ExcepcionRedUDP : public std::runtime_error {
public:
    explicit ExcepcionRedUDP(const std::string& mensaje) : std::runtime_error(mensaje) {}
};

#endif
