#include "Registro.h"
#include <iostream>
#include <stdexcept>

void mostrarMensaje(const std::string& mensaje, Registro& registro) {
    std::lock_guard<std::mutex> bloqueo(registro.mutex);
    std::cout << "[Hilo " << std::this_thread::get_id() << "] " << mensaje << std::endl;
}

void registrarError(const std::string& mensaje, Registro& registro) {
    std::lock_guard<std::mutex> bloqueo(registro.mutex);
    ++registro.errores;
    std::cout << "[Hilo " << std::this_thread::get_id() << "] Excepcion capturada: "
              << mensaje << std::endl;
}

void tareaTransmisionHilo(DispositivoComunicacion* dispositivo, Registro& registro) {
    try {
        if (dispositivo == nullptr)
            throw std::invalid_argument("Puntero nulo recibido. No se realiza la transmision.");
        mostrarMensaje("Inicia " + dispositivo->getId() + " - " + dispositivo->getTipo(), registro);
        dispositivo->transmitir();
        std::lock_guard<std::mutex> bloqueo(registro.mutex);
        ++registro.exitosas;
        std::cout << "[Hilo " << std::this_thread::get_id() << "] Transmision exitosa: "
                  << dispositivo->getId() << " (" << dispositivo->getTipo() << ")" << std::endl;
    } catch (const std::exception& e) {
        registrarError(e.what(), registro);
    } catch (...) {
        registrarError("Fallo desconocido en una transmision.", registro);
    }
}
