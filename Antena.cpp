#include "Antena.h"
#include "Tecnico.h"
#include <iostream>
#include <stdexcept>
#include <cmath>

Antena::Antena(int id, double frecuencia, const std::string& tec, Tecnico* responsable)
    : idAntena(id), frecuenciaMHz(0), tecnologia(), responsableMantenimiento(nullptr) {
    if (id <= 0) throw std::invalid_argument("idAntena debe ser positivo");
    setFrecuenciaMHz(frecuencia);
    setTecnologia(tec);
    setResponsable(responsable);
}

Antena::~Antena() {
    if (responsableMantenimiento != nullptr) responsableMantenimiento->antenaAsignada = nullptr;
}

void Antena::setFrecuenciaMHz(double frecuencia) {
    if (!std::isfinite(frecuencia) || !(frecuencia > 0.0)) throw std::invalid_argument("La frecuencia debe ser finita y mayor que cero");
    frecuenciaMHz = frecuencia;
}

void Antena::setTecnologia(const std::string& tec) {
    if (tec != "4G" && tec != "5G") throw std::invalid_argument("Tecnologia invalida (use 4G o 5G)");
    tecnologia = tec;
}

void Antena::setResponsable(Tecnico* tecnico) {
    if (tecnico == responsableMantenimiento) return;
    if (tecnico != nullptr && tecnico->antenaAsignada != nullptr && tecnico->antenaAsignada != this)
        throw std::invalid_argument("El tecnico ya es responsable de otra antena (asociacion 1:1)");
    if (responsableMantenimiento != nullptr) responsableMantenimiento->antenaAsignada = nullptr;
    responsableMantenimiento = tecnico;
    if (tecnico != nullptr) tecnico->antenaAsignada = this;
}

void Antena::mostrarInformacion() const {
    std::cout << "Antena #" << idAntena << " " << tecnologia << " " << frecuenciaMHz << " MHz - responsable: ";
    if (responsableMantenimiento != nullptr) std::cout << responsableMantenimiento->getNombre();
    else std::cout << "(sin asignar)";
    std::cout << std::endl;
}
