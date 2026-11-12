#ifndef DISPOSITIVOCOMUNICACION_H
#define DISPOSITIVOCOMUNICACION_H
#include "ITransmisible.h"
#include "Protocolo.h"
#include <atomic>
#include <cstdint>
#include <string>

// Cuenta las instancias instrumentadas de la base. Volver al valor inicial no prueba
// ausencia universal de fugas ni demuestra por si solo la destruccion de todos los recursos derivados.
class ContadorVida {
    static std::atomic<int> vivos;

public:
    ContadorVida() { ++vivos; }
    ContadorVida(const ContadorVida&) { ++vivos; }
    ~ContadorVida() { --vivos; }
    static int total() { return vivos.load(); }
};

// Clase base ABSTRACTA: transmitir() es virtual puro, por eso no se puede instanciar.
class DispositivoComunicacion : public ITransmisible {
    std::string id;
    std::string tipo;
    std::string ip;
    ContadorVida vida;

public:
    DispositivoComunicacion(const std::string& codigo, const std::string& nombreTipo, const std::string& direccion);
    virtual ~DispositivoComunicacion() = default;
    DispositivoComunicacion(const DispositivoComunicacion&) = delete;
    DispositivoComunicacion& operator=(const DispositivoComunicacion&) = delete;

    virtual void transmitir() = 0;

    const std::string& getId() const { return id; }
    const std::string& getTipo() const { return tipo; }
    const std::string& getIp() const { return ip; }

    // Trama de estado de este dispositivo (valor 1 = operativo) con numero de secuencia.
    Protocolo::Trama aTrama(std::uint32_t secuencia) const;
    std::vector<std::uint8_t> serializar() const override;  // contrato de ITransmisible

    static int instanciasVivas() { return ContadorVida::total(); }
};

#endif
