#include "Trama.h"
#include "NodoRedInteligente.h"
#include <cmath>
#include <locale>
#include <sstream>

std::string Trama::serializar() const {
    if ((clase != "ESTADO" && clase != "TEMPERATURA") || origen.empty() ||
        origen.find_first_of("|\r\n") != std::string::npos ||
        !ipValida(ip) || !std::isfinite(valor))
        throw ExcepcionNodoRed("Datos de trama invalidos");
    std::ostringstream salida;
    salida.imbue(std::locale::classic());
    salida.precision(17);
    salida << "TC1|" << clase << '|' << origen << '|' << ip << '|' << valor;
    return salida.str();
}

Trama Trama::deserializar(const std::string& texto) {
    std::istringstream entrada(texto);
    std::string version, numero;
    Trama trama;
    if (!std::getline(entrada, version, '|') || version != "TC1" ||
        !std::getline(entrada, trama.clase, '|') || !std::getline(entrada, trama.origen, '|') ||
        !std::getline(entrada, trama.ip, '|') || !std::getline(entrada, numero))
        throw ExcepcionNodoRed("Formato de trama invalido");
    std::istringstream lectura(numero);
    lectura.imbue(std::locale::classic());
    if (!(lectura >> trama.valor) || lectura.peek() != std::char_traits<char>::eof())
        throw ExcepcionNodoRed("Valor numerico invalido");
    trama.serializar();
    if (trama.clase == "TEMPERATURA") SensorIoT("Temperatura", trama.valor);
    return trama;
}
