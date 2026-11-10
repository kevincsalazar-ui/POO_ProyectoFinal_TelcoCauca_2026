#ifndef RED_H
#define RED_H
#include "Registro.h"

class ServidorCentral {
public:
    void atender(const std::string& ip, unsigned short puerto, int clientes, Registro& registro);
};

void enviarEstado(const std::string& servidor, unsigned short puerto,
                  const DispositivoComunicacion& dispositivo, Registro& registro);
void enviarTelemetria(const std::string& servidor, unsigned short puerto, Registro& registro);
void demostrarRed(Registro& registro);
#endif
