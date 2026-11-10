#include "Registro.h"
#include <iostream>
#include <chrono>
#include <stdexcept>

void tareaEventos2027(Registro& registro) {
    try {
        std::vector<EventoIngenieria2027> agenda = {
            {"Andina Link Smart Cities Expo 2027",
             "8 al 11 de marzo de 2027", "Cartagena, Colombia",
             "Telecomunicaciones, conectividad y ciudades inteligentes"},
            {"IEEE PES GT&D Latinoamerica 2027",
             "8 al 10 de marzo de 2027", "Medellin, Colombia",
             "Sistemas electricos, energia y tecnologias de infraestructura"},
            {"IEEE ACSOS 2027",
             "27 de septiembre al 1 de octubre de 2027", "Bogota, Colombia",
             "Computacion autonomica y sistemas autoorganizados"},
            {"Cambiemos el Chip Fest 2027",
             "Marzo de 2027, dia por confirmar", "Cali, Colombia",
             "Electronica, semiconductores y sistemas embebidos"},
            {"IEEE Colombian Caribbean Conference C3 2027",
             "15 al 18 de septiembre de 2027", "Colombia, sede por confirmar",
             "Investigacion en ingenieria y tecnologia"}
        };
        mostrarMensaje("AGENDA DE EVENTOS 2027 - Ingenieria Electronica y Telecomunicaciones", registro);
        for (std::size_t i = 0; i < agenda.size(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(60));
            std::lock_guard<std::mutex> bloqueo(registro.mutex);
            registro.eventos.push_back(agenda[i]);
            std::cout << "[Hilo " << std::this_thread::get_id() << "] Evento " << (i + 1) << ": "
                      << agenda[i].nombre << "\n    Fecha: " << agenda[i].fecha
                      << "\n    Lugar: " << agenda[i].lugar
                      << "\n    Tema:  " << agenda[i].tema << std::endl;
        }
    } catch (const std::exception& e) {
        registrarError(e.what(), registro);
    } catch (...) {
        registrarError("Fallo desconocido al registrar los eventos.", registro);
    }
}
