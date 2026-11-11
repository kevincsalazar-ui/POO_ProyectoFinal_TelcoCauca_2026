#ifndef EVENTOS_H
#define EVENTOS_H
#include "Registro.h"
#include <vector>

// Agenda de eventos de Ingenieria Electronica y Telecomunicaciones proyectados para 2027.
std::vector<EventoIngenieria> eventosProyectados2027();

// Tarea del hilo independiente: registra los eventos en el Registro compartido.
void tareaEventos2027(Registro& registro);

#endif
