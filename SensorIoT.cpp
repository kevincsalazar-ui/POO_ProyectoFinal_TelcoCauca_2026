#include "SensorIoT.h"
#include "Excepciones.h"
#include <cmath>

void SensorIoT::validarLectura(double lectura) {
    if (!std::isfinite(lectura) || lectura < lecturaMinima() || lectura > lecturaMaxima())
        throw ExcepcionNodoRed("Lectura fuera del rango de temperatura [-40, 85] C");
}

SensorIoT::SensorIoT(const std::string& tipo, double lectura) : tipoSensor(tipo), valorLectura(0) {
    if (tipo.empty()) throw ExcepcionNodoRed("El tipo de sensor no puede estar vacio");
    validarLectura(lectura);
    valorLectura = lectura;
}

void SensorIoT::setValorLectura(double lectura) {
    validarLectura(lectura);  // si falla, valorLectura conserva el valor anterior
    valorLectura = lectura;
}
