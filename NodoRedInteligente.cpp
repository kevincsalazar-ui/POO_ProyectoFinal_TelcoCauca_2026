#include "NodoRedInteligente.h"
#include <cmath>
#include <sstream>

bool ipValida(const std::string& ip) {
    std::istringstream entrada(ip);
    std::string parte;
    int total = 0;
    while (std::getline(entrada, parte, '.')) {
        if (parte.empty() || parte.size() > 3 || parte.find_first_not_of("0123456789") != std::string::npos)
            return false;
        if (std::stoi(parte) > 255) return false;
        ++total;
    }
    return total == 4 && ip.back() != '.';
}

DispositivoRed::DispositivoRed(const std::string& ip, const std::string& mac, bool activo)
    : direccionIP(ip), direccionMAC(mac), estadoOperativo(activo) {
    if (!ipValida(ip)) throw ExcepcionNodoRed("Direccion IPv4 invalida: " + ip);
}

SensorIoT::SensorIoT(const std::string& tipo, double lectura) : tipoSensor(tipo) {
    setValorLectura(lectura);
}

void SensorIoT::setValorLectura(double lectura) {
    if (!std::isfinite(lectura) || lectura < -40 || lectura > 85)
        throw ExcepcionNodoRed("Lectura fuera del rango de temperatura [-40, 85] C");
    valorLectura = lectura;
}

NodoRedInteligente::NodoRedInteligente(const std::string& ip, const std::string& mac, bool activo,
                                       const std::string& tipo, double lectura, double carga)
    : DispositivoRed(ip, mac, activo), SensorIoT(tipo, lectura), cargaProcesamiento(carga) {
    if (!std::isfinite(carga) || carga < 0) throw ExcepcionNodoRed("Carga invalida");
}

double NodoRedInteligente::operator+(const NodoRedInteligente& otro) const {
    return cargaProcesamiento + otro.cargaProcesamiento;
}

std::ostream& operator<<(std::ostream& salida, const NodoRedInteligente& nodo) {
    return salida << nodo.getDireccionIP() << " | " << nodo.getDireccionMAC() << " | "
                  << (nodo.getEstadoOperativo() ? "activo" : "inactivo") << " | "
                  << nodo.getTipoSensor() << ' ' << nodo.getValorLectura()
                  << " C | carga " << nodo.cargaProcesamiento;
}
