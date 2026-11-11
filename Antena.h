#ifndef ANTENA_H
#define ANTENA_H
#include <string>

class Tecnico;

// Antena de una torre. Guarda un puntero NO propietario al tecnico responsable.
class Antena {
    int idAntena;
    double frecuenciaMHz;
    std::string tecnologia;  // "4G" o "5G"
    Tecnico* responsableMantenimiento;
    friend class Tecnico;

public:
    Antena(int id, double frecuencia, const std::string& tec, Tecnico* responsable = nullptr);
    ~Antena();
    Antena(const Antena&) = delete;
    Antena& operator=(const Antena&) = delete;

    int getIdAntena() const { return idAntena; }
    double getFrecuenciaMHz() const { return frecuenciaMHz; }
    const std::string& getTecnologia() const { return tecnologia; }
    const Tecnico* getResponsable() const { return responsableMantenimiento; }

    void setFrecuenciaMHz(double frecuencia);
    void setTecnologia(const std::string& tec);
    // Asociacion 1:1: lanza std::invalid_argument si el tecnico ya atiende otra antena.
    void setResponsable(Tecnico* tecnico);

    void mostrarInformacion() const;
};

#endif
