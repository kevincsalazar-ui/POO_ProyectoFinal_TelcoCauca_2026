#include "DispositivoRed.h"
#include "Excepciones.h"
#include <cctype>

bool DispositivoRed::ipValida(const std::string& ip) {
    int partes = 0;
    std::size_t i = 0;
    while (i <= ip.size()) {
        std::size_t inicio = i;
        int valor = 0;
        while (i < ip.size() && std::isdigit(static_cast<unsigned char>(ip[i]))) {
            valor = valor * 10 + (ip[i] - '0');
            if (valor > 255) return false;
            ++i;
        }
        if (i == inicio || i - inicio > 3) return false;
        // Formato decimal canonico: evita que inet_addr interprete octetos como octal.
        if (i - inicio > 1 && ip[inicio] == '0') return false;
        ++partes;
        if (i == ip.size()) break;
        if (ip[i] != '.') return false;
        ++i;
        if (i == ip.size()) return false;
    }
    return partes == 4;
}

bool DispositivoRed::macValida(const std::string& mac) {
    if (mac.size() != 17) return false;
    for (std::size_t i = 0; i < mac.size(); ++i) {
        if (i % 3 == 2) {
            if (mac[i] != ':') return false;
        } else if (!std::isxdigit(static_cast<unsigned char>(mac[i]))) {
            return false;
        }
    }
    return true;
}

DispositivoRed::DispositivoRed(const std::string& ip, const std::string& mac, bool activo)
    : direccionIP(ip), direccionMAC(mac), estadoOperativo(activo) {
    if (!ipValida(ip)) throw ExcepcionNodoRed("Direccion IPv4 invalida: " + ip);
    if (!macValida(mac)) throw ExcepcionNodoRed("Direccion MAC invalida: " + mac);
}

void DispositivoRed::setDireccionIP(const std::string& ip) {
    if (!ipValida(ip)) throw ExcepcionNodoRed("Direccion IPv4 invalida: " + ip);
    direccionIP = ip;
}

void DispositivoRed::setDireccionMAC(const std::string& mac) {
    if (!macValida(mac)) throw ExcepcionNodoRed("Direccion MAC invalida: " + mac);
    direccionMAC = mac;
}
