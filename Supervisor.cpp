#include "Supervisor.h"
#include "Excepciones.h"
#include <stdexcept>

void Supervisor::supervisar(DispositivoComunicacion* dispositivo, Registro& registro) const {
    try {
        if (dispositivo == nullptr) throw std::invalid_argument("Puntero nulo detectado: no se transmite");
        registro.imprimir("Inicia " + dispositivo->getId() + " (" + dispositivo->getTipo() + ")");
        dispositivo->transmitir();  // trabajo lento FUERA de la seccion critica
        registro.registrarExito("Transmision de " + dispositivo->getId() + " (" + dispositivo->getTipo() + ")");
    } catch (const ExcepcionNodoRed& e) {
        registro.registrarError(std::string("ExcepcionNodoRed: ") + e.what());
    } catch (const std::exception& e) {
        registro.registrarError(std::string("Excepcion: ") + e.what());
    } catch (...) {
        registro.registrarError("Fallo desconocido en el hilo supervisor");
    }
}
