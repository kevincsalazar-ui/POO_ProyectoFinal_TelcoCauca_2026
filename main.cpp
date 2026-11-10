#include "Antena.h"
#include "Torre.h"
#include "NodoRedInteligente.h"
#include "Red.h"
#include "GrupoHilos.h"
#include <functional>
#include <iostream>
#include <memory>

namespace {
void infraestructura() {
    Tecnico ana(1, "Ana", "Radiofrecuencia");
    Tecnico luis(2, "Luis", "Redes moviles");
    Antena a(101, 1800, "4G", &ana);
    Antena b(102, 3500, "5G", &luis);
    Torre torre(1, "Popayan", 35);
    torre.agregarAntena(&a);
    torre.agregarAntena(&b);
    torre.mostrarInformacion();
}

void nodos() {
    NodoRedInteligente a("127.0.0.1", "02:00:00:00:00:01", true, "Temperatura", 24.5, 7);
    const NodoRedInteligente b("127.0.0.2", "02:00:00:00:00:02", true, "Temperatura", 26, 5);
    std::cout << "Nodo: " << a << "\nCarga total: " << a + b << '\n';
    try {
        a.setValorLectura(120);
        throw std::logic_error("Falto validar la lectura");
    } catch (const ExcepcionNodoRed& e) {
        std::cout << "Lectura rechazada: " << e.what() << '\n';
    }
    try {
        NodoRedInteligente invalido("300.0.0.1", "02:00:00:00:00:03", false, "Temperatura", 20, 2);
        throw std::logic_error("Falto validar la IP");
    } catch (const ExcepcionNodoRed& e) {
        std::cout << "IP rechazada: " << e.what() << '\n';
    }
}

class Supervisor {
public:
    void supervisar(DispositivoComunicacion* dispositivo, Registro& registro) const {
        tareaTransmisionHilo(dispositivo, registro);
    }
};

void concurrencia() {
    Registro registro;
    std::vector<std::unique_ptr<DispositivoComunicacion>> propietarios;
    propietarios.reserve(3);
    propietarios.emplace_back(new RouterWiFi("RTR-01", 12));
    propietarios.emplace_back(new GatewayLoRa("GW-01", 868.1f));
    propietarios.emplace_back(new SwitchFibra("SW-01", 8));
    std::vector<DispositivoComunicacion*> red;
    for (const auto& dispositivo : propietarios) red.push_back(dispositivo.get());
    red.push_back(nullptr);

    const Supervisor supervisor;
    GrupoHilos hilos(red.size() + 1);
    for (auto* dispositivo : red)
        hilos.agregar(&Supervisor::supervisar, &supervisor, dispositivo, std::ref(registro));
    hilos.agregar(tareaEventos2027, std::ref(registro));
    hilos.esperar();
    if (registro.exitosas != 3 || registro.errores != 1 || registro.eventos.size() < 3)
        throw std::runtime_error("Resultado inesperado de concurrencia");
    std::cout << "Resumen: " << registro.exitosas << " transmisiones, "
              << registro.errores << " excepcion controlada y "
              << registro.eventos.size() << " eventos de 2027.\n";
}

unsigned short puertoDe(const std::string& texto) {
    std::size_t fin = 0;
    const int puerto = std::stoi(texto, &fin);
    if (fin != texto.size() || puerto <= 0 || puerto > 65535)
        throw std::invalid_argument("Puerto invalido");
    return static_cast<unsigned short>(puerto);
}

void redLocal() {
    Registro registro;
    demostrarRed(registro);
}

void demostracion() {
    infraestructura();
    nodos();
    concurrencia();
    redLocal();
}

void menu() {
    std::string opcion;
    while (true) {
        std::cout << "\nTELCOCAUCA\n1 Infraestructura\n2 Nodos y excepciones\n"
                     "3 Dispositivos, hilos y agenda\n4 Red TCP y UDP\n5 Demostracion completa\n0 Salir\n> ";
        if (!std::getline(std::cin, opcion) || opcion == "0") return;
        try {
            if (opcion == "1") infraestructura();
            else if (opcion == "2") nodos();
            else if (opcion == "3") concurrencia();
            else if (opcion == "4") redLocal();
            else if (opcion == "5") demostracion();
            else std::cout << "Opcion invalida\n";
        } catch (const std::exception& e) {
            std::cerr << "Fallo controlado: " << e.what() << '\n';
        }
    }
}
}

int main(int argc, char* argv[]) {
    try {
        if (argc == 1) {
            menu();
            return 0;
        }
        const std::string modo(argv[1]);
        if (argc == 2 && modo == "--pruebas") {
            demostracion();
            return 0;
        }
        if (argc != 4) throw std::invalid_argument("Uso: --servidor|--router|--gateway|--sensor IPv4 puerto");
        const std::string ip(argv[2]);
        const auto puerto = puertoDe(argv[3]);
        Registro registro;
        if (modo == "--servidor") {
            ServidorCentral servidor;
            servidor.atender(ip, puerto, 2, registro);
        } else if (modo == "--router") {
            RouterWiFi router("RTR-01", 12);
            enviarEstado(ip, puerto, router, registro);
        } else if (modo == "--gateway") {
            GatewayLoRa gateway("GW-01", 868.1f);
            enviarEstado(ip, puerto, gateway, registro);
        } else if (modo == "--sensor") {
            enviarTelemetria(ip, puerto, registro);
        } else {
            throw std::invalid_argument("Modo invalido");
        }
        return registro.errores ? 1 : 0;
    } catch (const ExcepcionNodoRed& e) {
        std::cerr << "Error de nodo o red: " << e.what() << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 1;
}
