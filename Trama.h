#ifndef TRAMA_H
#define TRAMA_H
#include <string>

struct Trama {
    std::string clase;
    std::string origen;
    std::string ip;
    double valor;

    std::string serializar() const;
    static Trama deserializar(const std::string& texto);
};
#endif
