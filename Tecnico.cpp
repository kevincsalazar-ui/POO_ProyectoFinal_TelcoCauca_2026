#include "Tecnico.h"
#include "Antena.h"
#include <iostream>
#include <stdexcept>

Tecnico::Tecnico(int id, const std::string& nom, const std::string& esp)
    : idTecnico(id), nombre(nom), especialidad(esp), antenaAsignada(nullptr) {
    if (id <= 0) throw std::invalid_argument("idTecnico debe ser positivo");
    if (nom.empty()) throw std::invalid_argument("El nombre del tecnico no puede estar vacio");
    if (esp.empty()) throw std::invalid_argument("La especialidad no puede estar vacia");
}

Tecnico::~Tecnico() {
    // Si el tecnico muere antes que la antena, la antena no debe quedar con un puntero colgante.
    if (antenaAsignada != nullptr) antenaAsignada->responsableMantenimiento = nullptr;
}

void Tecnico::setNombre(const std::string& nom) {
    if (nom.empty()) throw std::invalid_argument("El nombre del tecnico no puede estar vacio");
    nombre = nom;
}

void Tecnico::setEspecialidad(const std::string& esp) {
    if (esp.empty()) throw std::invalid_argument("La especialidad no puede estar vacia");
    especialidad = esp;
}

void Tecnico::mostrarInformacion() const {
    std::cout << "Tecnico #" << idTecnico << ": " << nombre << " (" << especialidad << ")" << std::endl;
}
