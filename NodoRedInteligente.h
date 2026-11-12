#ifndef NODOREDINTELIGENTE_H
#define NODOREDINTELIGENTE_H
#include "DispositivoRed.h"
#include "SensorIoT.h"
#include <ostream>

// Herencia multiple: un nodo ES un dispositivo de red Y ES un sensor IoT.
class NodoRedInteligente : public DispositivoRed, public SensorIoT {
    double cargaProcesamiento;  // porcentaje o unidades de carga, >= 0

public:
    NodoRedInteligente(const std::string& ip, const std::string& mac, bool activo,
                       const std::string& tipo, double lectura, double carga);

    double getCargaProcesamiento() const { return cargaProcesamiento; }
    void setCargaProcesamiento(double carga);

    double operator+(const NodoRedInteligente& otro) const;

    // Funcion amiga: necesita leer cargaProcesamiento (privado) y devuelve el flujo para encadenar.
    friend std::ostream& operator<<(std::ostream& salida, const NodoRedInteligente& nodo);
};

#endif
