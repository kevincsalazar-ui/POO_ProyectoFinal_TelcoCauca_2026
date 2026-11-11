#ifndef CLIENTES_H
#define CLIENTES_H
#include "Protocolo.h"
#include "Registro.h"
#include <cstdint>
#include <string>
#include <vector>

struct ResultadoCliente {
    int enviadas;
    int ack;
    int nack;
    ResultadoCliente() : enviadas(0), ack(0), nack(0) {}
};

// Cliente TCP: connect() -> (send() + recv() por cada trama) -> cierre. Lanza ExcepcionRedTCP si
// no conecta, si el servidor no responde a tiempo o si cierra la conexion.
ResultadoCliente clienteTcp(const std::string& ip, std::uint16_t puerto, const std::vector<Protocolo::Trama>& tramas,
                            Registro& registro, int pausaMs = 0, int timeoutMs = 2000);

// Cliente UDP: sendto() + recvfrom() con tiempo de espera. Lanza ExcepcionRedUDP si no hay respuesta.
ResultadoCliente clienteUdp(const std::string& ip, std::uint16_t puerto, const std::vector<Protocolo::Trama>& tramas,
                            Registro& registro, int pausaMs = 0, int timeoutMs = 2000);

// Roles de proceso independiente: "router", "gateway" o "sensor". Devuelve 0 si todo fue ACK.
int ejecutarRol(const std::string& rol, const std::string& ip, std::uint16_t puerto);

#endif
