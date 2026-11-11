#ifndef TORRE_H
#define TORRE_H
#include <cstddef>
#include <string>
#include <vector>

class Antena;  // declaracion adelantada: basta para declarar punteros Antena*

// Torre con N antenas (asociacion 1:N, agregacion).
// Propiedad: la Torre NO es duena de las antenas. Quien crea la antena la destruye;
// debe vivir mientras la torre la use (en la demo se declaran antes que la torre).
class Torre {
    int idTorre;
    std::string ubicacion;
    double alturaMetros;
    std::vector<Antena*> listaAntenas;

public:
    Torre(int id, const std::string& sitio, double altura);

    int getIdTorre() const { return idTorre; }
    const std::string& getUbicacion() const { return ubicacion; }
    double getAlturaMetros() const { return alturaMetros; }
    std::size_t cantidadAntenas() const { return listaAntenas.size(); }

    void setUbicacion(const std::string& sitio);
    void setAlturaMetros(double altura);

    void agregarAntena(Antena* antena);  // nullptr o repetida -> std::invalid_argument
    bool quitarAntena(const Antena* antena);

    void mostrarInformacion() const;
};

#endif
