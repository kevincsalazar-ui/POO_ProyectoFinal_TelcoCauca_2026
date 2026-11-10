#ifndef DISPOSITIVOS_H
#define DISPOSITIVOS_H

#include <string>
#include <chrono>
#include <stdexcept>
#include "Hilos.h"
#include "ITransmisible.h"
#include "Trama.h"


class DispositivoComunicacion : public ITransmisible {
protected:
    std::string id;
    std::string tipo;
    std::string ip;
public:
    DispositivoComunicacion(const std::string& codigo, const std::string& nombre, const std::string& direccion = "127.0.0.1")
        : id(codigo), tipo(nombre), ip(direccion) {}
    virtual ~DispositivoComunicacion() = default;
    virtual void transmitir() = 0;
    std::string serializar() const override { return Trama{"ESTADO", id, ip, 1}.serializar(); }
    std::string getId() const { return id; }
    std::string getTipo() const { return tipo; }
};


class RouterWiFi : public DispositivoComunicacion {
    int canalesActivos;
public:
    RouterWiFi(const std::string& codigo, int canales)
        : DispositivoComunicacion(codigo, "Router WiFi"), canalesActivos(canales) {}
    void transmitir() override {
        if (canalesActivos <= 0)
            throw std::invalid_argument("El router no tiene canales activos.");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
};

class GatewayLoRa : public DispositivoComunicacion {
    float frecuenciaMHz;
public:
    GatewayLoRa(const std::string& codigo, float frecuencia)
        : DispositivoComunicacion(codigo, "Gateway LoRa"), frecuenciaMHz(frecuencia) {}
    void transmitir() override {
        if (frecuenciaMHz <= 0)
            throw std::invalid_argument("La frecuencia debe ser positiva.");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
};

class SwitchFibra : public DispositivoComunicacion {
    int puertosActivos;
public:
    SwitchFibra(const std::string& codigo, int puertos)
        : DispositivoComunicacion(codigo, "Switch de fibra"), puertosActivos(puertos) {}
    void transmitir() override {
        if (puertosActivos <= 0)
            throw std::invalid_argument("El switch no tiene puertos activos.");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
};

#endif
