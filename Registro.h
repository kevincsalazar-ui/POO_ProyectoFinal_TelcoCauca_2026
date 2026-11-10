#ifndef REGISTRO_H
#define REGISTRO_H

#include <string>
#include <vector>
#include "Hilos.h"
#include "Dispositivos.h"
#include "EventoIngenieria2027.h"


struct Registro {
    std::mutex mutex;
    std::vector<Trama> tramasRecibidas;
    int exitosas = 0;
    int errores = 0;
    std::vector<EventoIngenieria2027> eventos;
};

void mostrarMensaje(const std::string& mensaje, Registro& registro);
void registrarError(const std::string& mensaje, Registro& registro);
void tareaTransmisionHilo(DispositivoComunicacion* dispositivo, Registro& registro);
void tareaEventos2027(Registro& registro);

#endif
