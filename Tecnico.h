#ifndef TECNICO_H
#define TECNICO_H
#include <string>
#include <iostream>

class Tecnico {
    int idTecnico;
    std::string nombre;
    std::string especialidad;
public:
    Tecnico(int id, const std::string& n, const std::string& e)
        : idTecnico(id), nombre(n), especialidad(e) {}
    int getIdTecnico() const { return idTecnico; }
    const std::string& getNombre() const { return nombre; }
    const std::string& getEspecialidad() const { return especialidad; }
    void setIdTecnico(int id) { idTecnico = id; }
    void setNombre(const std::string& n) { nombre = n; }
    void setEspecialidad(const std::string& e) { especialidad = e; }
    void mostrarInformacion() const {
        std::cout << "Tecnico " << idTecnico << " | " << nombre << " | " << especialidad << '\n';
    }
};
#endif
