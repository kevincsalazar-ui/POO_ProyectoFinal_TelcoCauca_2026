#include "Torre.h"
#include "Antena.h"
#include <algorithm>

Torre::Torre(int id, const std::string& sitio, double altura)
    : idTorre(id), ubicacion(sitio) {
    setAlturaMetros(altura);
}

void Torre::setAlturaMetros(double altura) {
    if (!std::isfinite(altura) || altura <= 0) throw std::invalid_argument("Altura invalida");
    alturaMetros = altura;
}

void Torre::agregarAntena(Antena* antena) {
    if (!antena) throw std::invalid_argument("Antena nula");
    if (std::find(listaAntenas.begin(), listaAntenas.end(), antena) == listaAntenas.end())
        listaAntenas.push_back(antena);
}

void Torre::mostrarInformacion() const {
    std::cout << "Torre " << idTorre << " | " << ubicacion << " | " << alturaMetros << " m\n";
    for (const auto* antena : listaAntenas) antena->mostrarInformacion();
}
