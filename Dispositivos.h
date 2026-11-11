#ifndef DISPOSITIVOS_H
#define DISPOSITIVOS_H
#include "DispositivoComunicacion.h"
#include <vector>

class RouterWiFi : public DispositivoComunicacion {
    int canalesActivos;

public:
    RouterWiFi(const std::string& codigo, int canales, const std::string& direccion = "127.0.0.1");
    void transmitir() override;  // lanza ExcepcionNodoRed si no hay canales activos
};

class GatewayLoRa : public DispositivoComunicacion {
    int factorExpansion;  // spreading factor LoRa: 7..12

public:
    GatewayLoRa(const std::string& codigo, int sf, const std::string& direccion = "127.0.0.1");
    void transmitir() override;
};

class SwitchFibra : public DispositivoComunicacion {
    int puertosOcupados;

public:
    SwitchFibra(const std::string& codigo, int puertos, const std::string& direccion = "127.0.0.1");
    void transmitir() override;
};

// Recorre el vector de punteros a la base. Valida nullptr ANTES de usar '->' y lanza
// std::invalid_argument si encuentra uno. La llamada es polimorfica (vtable).
void transmitirTodos(const std::vector<DispositivoComunicacion*>& dispositivos);

#endif
