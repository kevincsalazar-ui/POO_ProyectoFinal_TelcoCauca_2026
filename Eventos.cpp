#include "Eventos.h"
#include <chrono>
#include <stdexcept>
#include <thread>

std::vector<EventoIngenieria> eventosProyectados2027() {
    std::vector<EventoIngenieria> agenda;
    agenda.push_back(EventoIngenieria{"IEEE Radio and Wireless Week (RWW) 2027", "17 al 20 de enero de 2027", "Tampa, Estados Unidos", "Radio, antenas y sistemas inalambricos"});
    agenda.push_back(EventoIngenieria{"IEEE ICC 2027", "30 de mayo al 3 de junio de 2027", "Washington D.C., Estados Unidos", "Comunicaciones y redes"});
    agenda.push_back(EventoIngenieria{"IEEE GLOBECOM 2027", "6 al 10 de diciembre de 2027 (por confirmar)", "Abu Dabi, Emiratos Arabes (por confirmar)", "Comunicaciones globales e industria"});
    agenda.push_back(EventoIngenieria{"EuCNC & 6G Summit 2027", "Fecha por confirmar", "Europa (sede por confirmar)", "Redes 5G/6G y antenas"});
    return agenda;
}

void tareaEventos2027(Registro& registro) {
    try {
        const std::vector<EventoIngenieria> agenda = eventosProyectados2027();
        for (std::size_t i = 0; i < agenda.size(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));  // simula el procesamiento (fuera del bloqueo)
            registro.agregarEvento(agenda[i]);                           // el Registro toma el lock_guard
        }
    } catch (const std::exception& e) {
        registro.registrarError(std::string("Hilo de eventos 2027: ") + e.what());
    } catch (...) {
        registro.registrarError("Hilo de eventos 2027: fallo desconocido");
    }
}
