#include "Registro.h"
#include "Util.h"
#include <iostream>

Registro::Registro() : totalExitosas(0), totalErrores(0) {}

int Registro::indiceHilo() {
    const std::thread::id yo = std::this_thread::get_id();
    std::map<std::thread::id, int>::iterator it = hilosVistos.find(yo);
    if (it != hilosVistos.end()) return it->second;
    const int nuevo = static_cast<int>(hilosVistos.size()) + 1;
    hilosVistos[yo] = nuevo;
    return nuevo;
}

void Registro::escribir(const std::string& linea) {
    std::cout << "[H" << indiceHilo() << "] " << linea << std::endl;
}

void Registro::imprimir(const std::string& mensaje) {
    std::lock_guard<std::mutex> bloqueo(mtx);  // seccion critica: consola + mapa de hilos
    escribir(mensaje);
}

void Registro::registrarExito(const std::string& mensaje) {
    std::lock_guard<std::mutex> bloqueo(mtx);
    ++totalExitosas;
    escribir("OK    " + mensaje);
}

void Registro::registrarError(const std::string& mensaje) {
    std::lock_guard<std::mutex> bloqueo(mtx);
    ++totalErrores;
    mensajesError.push_back(mensaje);
    escribir("ERROR " + mensaje);
}

void Registro::agregarEvento(const EventoIngenieria& evento) {
    std::lock_guard<std::mutex> bloqueo(mtx);
    eventos.push_back(evento);
    escribir("Evento " + aTexto(eventos.size()) + ": " + evento.nombre + " | " + evento.fecha + " | " + evento.lugar +
             " | " + evento.tema);
}

void Registro::agregarTrama(const std::string& descripcion) {
    std::lock_guard<std::mutex> bloqueo(mtx);
    tramas.push_back(descripcion);
    escribir("Trama recibida " + descripcion);
}

int Registro::exitosas() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return totalExitosas;
}
int Registro::errores() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return totalErrores;
}
std::size_t Registro::cantidadEventos() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return eventos.size();
}
std::size_t Registro::cantidadTramas() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return tramas.size();
}
std::size_t Registro::cantidadHilos() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return hilosVistos.size();
}
std::vector<std::string> Registro::copiaErrores() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return mensajesError;
}
std::vector<std::string> Registro::copiaTramas() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return tramas;
}

void ContadorSeguro::incrementar() {
    std::lock_guard<std::mutex> bloqueo(mtx);
    ++valorActual;  // leer-sumar-escribir sin que otro hilo se interponga
}

long ContadorSeguro::valor() const {
    std::lock_guard<std::mutex> bloqueo(mtx);
    return valorActual;
}

void incrementarVeces(ContadorSeguro& contador, int veces) {
    for (int i = 0; i < veces; ++i) contador.incrementar();
}
