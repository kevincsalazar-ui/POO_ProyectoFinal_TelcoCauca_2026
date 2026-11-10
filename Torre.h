#ifndef TORRE_H
#define TORRE_H
#include <string>
#include <vector>

class Antena;

class Torre {
    int idTorre;
    std::string ubicacion;
    double alturaMetros;
    std::vector<Antena*> listaAntenas;
public:
    Torre(int id, const std::string& sitio, double altura);
    ~Torre() = default;
    int getIdTorre() const { return idTorre; }
    const std::string& getUbicacion() const { return ubicacion; }
    double getAlturaMetros() const { return alturaMetros; }
    const std::vector<Antena*>& getListaAntenas() const { return listaAntenas; }
    void setIdTorre(int id) { idTorre = id; }
    void setUbicacion(const std::string& sitio) { ubicacion = sitio; }
    void setAlturaMetros(double altura);
    void agregarAntena(Antena* antena);
    void mostrarInformacion() const;
};
#endif
