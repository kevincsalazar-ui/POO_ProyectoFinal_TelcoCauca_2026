#ifndef UTIL_H
#define UTIL_H
// Utilidades minimas. aTexto evita std::to_string (problemas en algunos MinGW antiguos).
#include <sstream>
#include <string>
#include <stdexcept>

template <class T>
std::string aTexto(const T& valor) {
    std::ostringstream salida;
    salida << valor;
    return salida.str();
}

// Entero decimal completo; verifica el limite ANTES de multiplicar para evitar overflow.
inline int enteroDecimal(const std::string& texto, int minimo, int maximo, const char* nombre) {
    if (texto.empty()) throw std::invalid_argument(std::string(nombre) + ": valor vacio");
    int valor = 0;
    for (std::size_t i = 0; i < texto.size(); ++i) {
        const char c = texto[i];
        if (c < '0' || c > '9') throw std::invalid_argument(std::string(nombre) + ": use solo digitos decimales");
        const int digito = c - '0';
        if (digito > maximo || valor > (maximo - digito) / 10)
            throw std::invalid_argument(std::string(nombre) + ": fuera de rango");
        valor = valor * 10 + digito;
    }
    if (valor < minimo) throw std::invalid_argument(std::string(nombre) + ": fuera de rango");
    return valor;
}

#endif
