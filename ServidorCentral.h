#ifndef SERVIDORCENTRAL_H
#define SERVIDORCENTRAL_H
#include "Protocolo.h"
#include "Registro.h"
#include "Socket.h"
#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Servidor TCP concurrente + receptor UDP.
//   TCP: socket() -> bind() -> listen() -> accept() en bucle; UN HILO por cliente aceptado.
//   UDP: socket() -> bind() -> recvfrom() / sendto() en su propio hilo.
// Cada cliente TCP puede enviar muchas tramas por la misma conexion (monitoreo continuo).
class ServidorCentral {
    Registro& registro;
    int timeoutInactividadMs;
    Socket tcp;
    Socket udp;
    std::atomic<bool> activo;
    std::thread hiloAceptador;
    std::thread hiloUdp;
    std::vector<std::thread> hilosClientes;
    std::mutex mtxClientes;
    std::atomic<int> aceptadasTcp;
    std::atomic<int> aceptadasUdp;
    std::atomic<int> rechazadas;
    std::atomic<int> clientesAtendidos;

    void bucleAceptar();
    void atenderCliente(Socket cliente);  // hilo dedicado a UN cliente
    void bucleUdp();
    Protocolo::CodigoRespuesta procesar(const std::uint8_t* datos, std::size_t n, const char* canal,
                                        std::uint32_t& secuencia);

public:
    explicit ServidorCentral(Registro& reg, int timeoutInactividad = 3000);
    ~ServidorCentral();  // llama detener(): nunca deja hilos sin join
    ServidorCentral(const ServidorCentral&) = delete;
    ServidorCentral& operator=(const ServidorCentral&) = delete;

    // Puerto 0 = el sistema operativo elige uno libre. Lanza ExcepcionRedTCP/UDP si bind falla.
    void iniciar(const std::string& ip, std::uint16_t puertoTcp, std::uint16_t puertoUdp);
    void detener();

    std::uint16_t puertoTcp() const { return tcp.puertoLocal(); }
    std::uint16_t puertoUdp() const { return udp.puertoLocal(); }
    int tramasTcpAceptadas() const { return aceptadasTcp.load(); }
    int tramasUdpAceptadas() const { return aceptadasUdp.load(); }
    int tramasRechazadas() const { return rechazadas.load(); }
    int clientesTcpAtendidos() const { return clientesAtendidos.load(); }
};

#endif
