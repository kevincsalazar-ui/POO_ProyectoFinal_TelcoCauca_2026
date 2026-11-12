#include "Torre.h"
#include "Antena.h"  
#include "Tecnico.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cmath>

Torre::Torre(int id, const std::string& sitio, double altura)
    : idTorre(id), ubicacion(sitio), alturaMetros(altura) {
    if (id <= 0) throw std::invalid_argument("idTorre debe ser positivo");
    if (sitio.empty()) throw std::invalid_argument("La ubicacion no puede estar vacia");
    if (!std::isfinite(altura) || !(altura > 0.0)) throw std::invalid_argument("La altura debe ser finita y mayor que cero");
}

void Torre::setUbicacion(const std::string& sitio) {
    if (sitio.empty()) throw std::invalid_argument("La ubicacion no puede estar vacia");
    ubicacion = sitio;
}

void Torre::setAlturaMetros(double altura) {
    if (!std::isfinite(altura) || !(altura > 0.0)) throw std::invalid_argument("La altura debe ser finita y mayor que cero");
    alturaMetros = altura;
}

void Torre::agregarAntena(Antena* antena) {
    if (antena == nullptr) throw std::invalid_argument("No se puede agregar una antena nula");
    if (std::find(listaAntenas.begin(), listaAntenas.end(), antena) != listaAntenas.end())
        throw std::invalid_argument("La antena ya pertenece a la torre");
    listaAntenas.push_back(antena);
}

bool Torre::quitarAntena(const Antena* antena) {
    for (std::vector<Antena*>::iterator it = listaAntenas.begin(); it != listaAntenas.end(); ++it) {
        if (*it == antena) {
            listaAntenas.erase(it);
            return true;
        }
    }
    return false;
}

void Torre::mostrarInformacion() const {
    std::cout << "Torre #" << idTorre << " en " << ubicacion << " (" << alturaMetros << " m), "
              << listaAntenas.size() << " antena(s)" << std::endl;
    for (std::size_t i = 0; i < listaAntenas.size(); ++i) {
        std::cout << "  ";
        listaAntenas[i]->mostrarInformacion();
        const Tecnico* t = listaAntenas[i]->getResponsable();
        if (t != nullptr) {
            std::cout << "    ";
            t->mostrarInformacion();
        }
    }
}
