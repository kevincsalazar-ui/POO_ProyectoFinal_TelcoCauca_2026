#ifndef TECNICO_H
#define TECNICO_H
#include <string>

class Antena;  // declaracion adelantada: Tecnico y Antena se conocen mutuamente

// Tecnico responsable de mantenimiento. Asociacion 1:1 con Antena (ver Antena::setResponsable).
class Tecnico {
    int idTecnico;
    std::string nombre;
    std::string especialidad;
    Antena* antenaAsignada;  // NO propietario: la antena existe por su cuenta
    friend class Antena;     // Antena mantiene este puntero para imponer la regla 1:1

public:
    Tecnico(int id, const std::string& nom, const std::string& esp);
    ~Tecnico();
    Tecnico(const Tecnico&) = delete;             // copiar romperia la asociacion 1:1
    Tecnico& operator=(const Tecnico&) = delete;

    int getIdTecnico() const { return idTecnico; }
    const std::string& getNombre() const { return nombre; }
    const std::string& getEspecialidad() const { return especialidad; }
    const Antena* getAntenaAsignada() const { return antenaAsignada; }

    void setNombre(const std::string& nom);
    void setEspecialidad(const std::string& esp);

    void mostrarInformacion() const;
};

#endif
