#ifndef NODOREDINTELIGENTE_H
#define NODOREDINTELIGENTE_H
#include <string>
#include <stdexcept>
#include <ostream>

class ExcepcionNodoRed : public std::runtime_error {
public:
    explicit ExcepcionNodoRed(const std::string& mensaje) : std::runtime_error(mensaje) {}
};

bool ipValida(const std::string& ip);

class DispositivoRed {
    std::string direccionIP;
    std::string direccionMAC;
    bool estadoOperativo;
public:
    DispositivoRed(const std::string& ip, const std::string& mac, bool activo);
    const std::string& getDireccionIP() const { return direccionIP; }
    const std::string& getDireccionMAC() const { return direccionMAC; }
    bool getEstadoOperativo() const { return estadoOperativo; }
};

class SensorIoT {
    std::string tipoSensor;
    double valorLectura;
public:
    SensorIoT(const std::string& tipo, double lectura);
    const std::string& getTipoSensor() const { return tipoSensor; }
    double getValorLectura() const { return valorLectura; }
    void setValorLectura(double lectura);
};

class NodoRedInteligente : public DispositivoRed, public SensorIoT {
    double cargaProcesamiento;
public:
    NodoRedInteligente(const std::string& ip, const std::string& mac, bool activo,
                       const std::string& tipo, double lectura, double carga);
    double operator+(const NodoRedInteligente& otro) const;
    friend std::ostream& operator<<(std::ostream& salida, const NodoRedInteligente& nodo);
};
#endif
