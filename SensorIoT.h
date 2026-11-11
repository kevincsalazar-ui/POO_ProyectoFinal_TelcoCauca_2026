#ifndef SENSORIOT_H
#define SENSORIOT_H
#include <string>

// Segunda clase base de NodoRedInteligente: tipo de sensor y ultima lectura.
class SensorIoT {
    std::string tipoSensor;
    double valorLectura;

public:
    SensorIoT(const std::string& tipo, double lectura);
    virtual ~SensorIoT() = default;

    // Rango fisico aceptado para las lecturas (grados C): [-40, 85].
    static double lecturaMinima() { return -40.0; }
    static double lecturaMaxima() { return 85.0; }
    static void validarLectura(double lectura);  // lanza ExcepcionNodoRed si no es finita o esta fuera de rango

    const std::string& getTipoSensor() const { return tipoSensor; }
    double getValorLectura() const { return valorLectura; }
    void setValorLectura(double lectura);  // valida ANTES de asignar
};

#endif
