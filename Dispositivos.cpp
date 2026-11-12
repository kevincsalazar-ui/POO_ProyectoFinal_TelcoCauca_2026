#include "Dispositivos.h"
#include "DispositivoRed.h"
#include "Excepciones.h"
#include <chrono>
#include <stdexcept>
#include <thread>

std::atomic<int> ContadorVida::vivos(0);

DispositivoComunicacion::DispositivoComunicacion(const std::string& codigo, const std::string& nombreTipo,
                                                 const std::string& direccion)
    : id(codigo), tipo(nombreTipo), ip(direccion) {
    if (codigo.empty()) throw ExcepcionNodoRed("El identificador del dispositivo no puede estar vacio");
    if (!DispositivoRed::ipValida(direccion)) throw ExcepcionNodoRed("Direccion IPv4 invalida: " + direccion);
}

Protocolo::Trama DispositivoComunicacion::aTrama(std::uint32_t secuencia) const {
    return Protocolo::crearEstado(id, ip, secuencia, true);
}

std::vector<std::uint8_t> DispositivoComunicacion::serializar() const {
    return Protocolo::serializar(aTrama(0));
}

RouterWiFi::RouterWiFi(const std::string& codigo, int canales, const std::string& direccion)
    : DispositivoComunicacion(codigo, "Router WiFi", direccion), canalesActivos(canales) {
    if (canales < 0) throw ExcepcionNodoRed("Numero de canales invalido");
}

void RouterWiFi::transmitir() {
    if (canalesActivos == 0) throw ExcepcionNodoRed("El router " + getId() + " no tiene canales activos");
    std::this_thread::sleep_for(std::chrono::milliseconds(60));  // simula el tiempo en el aire
}

GatewayLoRa::GatewayLoRa(const std::string& codigo, int sf, const std::string& direccion)
    : DispositivoComunicacion(codigo, "Gateway LoRa", direccion), factorExpansion(sf) {
    if (sf < 7 || sf > 12) throw ExcepcionNodoRed("Factor de expansion LoRa fuera de 7..12");
}

void GatewayLoRa::transmitir() {
    std::this_thread::sleep_for(std::chrono::milliseconds(30 + 10 * factorExpansion));  // SF alto = mas lento
}

SwitchFibra::SwitchFibra(const std::string& codigo, int puertos, const std::string& direccion)
    : DispositivoComunicacion(codigo, "Switch Fibra", direccion), puertosOcupados(puertos) {
    if (puertos < 1) throw ExcepcionNodoRed("El switch necesita al menos un puerto ocupado");
}

void SwitchFibra::transmitir() {
    std::this_thread::sleep_for(std::chrono::milliseconds(40));
}

void transmitirTodos(const std::vector<DispositivoComunicacion*>& dispositivos) {
    for (std::size_t i = 0; i < dispositivos.size(); ++i) {
        DispositivoComunicacion* dev = dispositivos[i];
        if (dev == nullptr) throw std::invalid_argument("Puntero nulo detectado");
        dev->transmitir();  // despacho dinamico: ejecuta la version del tipo real
    }
}

