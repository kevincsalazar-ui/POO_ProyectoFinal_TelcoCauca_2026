#include "NodoRedInteligente.h"
#include "Excepciones.h"
#include <cmath>
#include <iomanip>
#include <sstream>

// Los constructores base se invocan explicitamente en la lista de inicializacion,
// en el orden en que las bases se declaran: primero DispositivoRed, luego SensorIoT.
NodoRedInteligente::NodoRedInteligente(const std::string& ip, const std::string& mac, bool activo,
                                       const std::string& tipo, double lectura, double carga)
    : DispositivoRed(ip, mac, activo), SensorIoT(tipo, lectura), cargaProcesamiento(carga) {
    if (!std::isfinite(carga) || carga < 0) throw ExcepcionNodoRed("Carga de procesamiento invalida");
}

void NodoRedInteligente::setCargaProcesamiento(double carga) {
    if (!std::isfinite(carga) || carga < 0) throw ExcepcionNodoRed("Carga de procesamiento invalida");
    cargaProcesamiento = carga;
}

double NodoRedInteligente::operator+(const NodoRedInteligente& otro) const {
    return cargaProcesamiento + otro.cargaProcesamiento;
}

std::ostream& operator<<(std::ostream& salida, const NodoRedInteligente& nodo) {
    std::ostringstream texto;  // formato local: no altera las banderas del flujo de salida
    texto << std::fixed << std::setprecision(1);
    texto << "Nodo[ip=" << nodo.getDireccionIP() << ", mac=" << nodo.getDireccionMAC()
          << ", operativo=" << (nodo.getEstadoOperativo() ? "SI" : "NO")
          << ", sensor=" << nodo.getTipoSensor() << ", lectura=" << nodo.getValorLectura()
          << ", carga=" << nodo.cargaProcesamiento << "]";
    return salida << texto.str();
}
