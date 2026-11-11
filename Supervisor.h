#ifndef SUPERVISOR_H
#define SUPERVISOR_H
#include "DispositivoComunicacion.h"
#include "Registro.h"

// Supervisa un dispositivo. Se ejecuta como funcion miembro en un hilo:
//   std::thread t(&Supervisor::supervisar, &supervisor, dispositivo, std::ref(registro));
class Supervisor {
public:
    // Captura fallos del trabajo; registrar el error tambien puede fallar por falta de recursos.
    // Una excepcion que escape del hilo provoca std::terminate: no hay recuperacion universal.
    void supervisar(DispositivoComunicacion* dispositivo, Registro& registro) const;
};

#endif
