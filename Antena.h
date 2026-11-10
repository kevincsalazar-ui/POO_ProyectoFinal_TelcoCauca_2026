#ifndef ANTENA_H
#define ANTENA_H
#include "Tecnico.h"
#include <cmath>
#include <stdexcept>

class Antena {
    int idAntena;
    double frecuenciaMHz;
    std::string tecnologia;
    Tecnico* responsableMantenimiento;
public:
    Antena(int id, double frecuencia, const std::string& tipo, Tecnico* tecnico)
        : idAntena(id), responsableMantenimiento(tecnico) {
        setFrecuenciaMHz(frecuencia);
        setTecnologia(tipo);
    }
    int getIdAntena() const { return idAntena; }
    double getFrecuenciaMHz() const { return frecuenciaMHz; }
    const std::string& getTecnologia() const { return tecnologia; }
    Tecnico* getResponsableMantenimiento() const { return responsableMantenimiento; }
    void setIdAntena(int id) { idAntena = id; }
    void setFrecuenciaMHz(double f) {
        if (!std::isfinite(f) || f <= 0) throw std::invalid_argument("Frecuencia invalida");
        frecuenciaMHz = f;
    }
    void setTecnologia(const std::string& tipo) {
        if (tipo != "4G" && tipo != "5G") throw std::invalid_argument("Tecnologia invalida");
        tecnologia = tipo;
    }
    void setResponsableMantenimiento(Tecnico* t) { responsableMantenimiento = t; }
    void mostrarInformacion() const {
        std::cout << "Antena " << idAntena << " | " << tecnologia << " | " << frecuenciaMHz << " MHz\n";
        if (responsableMantenimiento) responsableMantenimiento->mostrarInformacion();
    }
};
#endif
