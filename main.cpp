// TelcoCauca - Sistema distribuido concurrente de monitoreo de infraestructura de telecomunicaciones e IoT.
// Universidad del Cauca - Programacion Orientada a Objetos - C++11.
#include "Clientes.h"
#include "Demo.h"
#include "Registro.h"
#include "ServidorCentral.h"
#include "Socket.h"
#include "Util.h"
#include <chrono>
#include <limits>
#include <stdexcept>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

void mostrarAyuda() {
    std::cout << "Uso:\n"
                 "  TelcoCauca                        menu interactivo\n"
                 "  TelcoCauca --pruebas              demostracion completa con verificacion (codigo de salida 0 = todo OK)\n"
                 "  TelcoCauca --servidor [puerto] [segundos]   servidor TCP+UDP (0 s = hasta ENTER). Por defecto 5000\n"
                 "  TelcoCauca --router|--gateway|--sensor [ip] [puerto]   cliente en proceso independiente\n"
                 "  Agregue --sinpausa para no esperar ENTER al terminar un modo de proceso.\n";
}

void esperarEnter() {
    std::cout << "Presione ENTER para cerrar..." << std::endl;
    std::string linea;
    std::getline(std::cin, linea);
}

int modoServidor(std::uint16_t puerto, int segundos, bool pausa) {
    Registro registro;
    ServidorCentral servidor(registro, 5000);
    servidor.iniciar("127.0.0.1", puerto, puerto);  // TCP y UDP usan espacios de puertos distintos
    registro.imprimir("Servidor listo en 127.0.0.1: TCP " + aTexto(puerto) + " / UDP " + aTexto(puerto));
    if (segundos > 0) {
        std::this_thread::sleep_for(std::chrono::seconds(segundos));
    } else {
        registro.imprimir("Presione ENTER para detener el servidor...");
        std::string linea;
        std::getline(std::cin, linea);
    }
    servidor.detener();
    std::cout << "Servidor detenido. TCP aceptadas=" << servidor.tramasTcpAceptadas()
              << " UDP aceptadas=" << servidor.tramasUdpAceptadas() << " rechazadas=" << servidor.tramasRechazadas()
              << " errores=" << registro.errores() << std::endl;
    if (pausa && segundos > 0) esperarEnter();
    return registro.errores() == 0 ? 0 : 1;
}

// Opcion 7: abre servidor y tres clientes en consolas separadas (solo Windows).
void lanzarProcesos() {
#ifdef _WIN32
    char ruta[MAX_PATH];
    const DWORD longitud = GetModuleFileNameA(NULL, ruta, MAX_PATH);
    if (longitud == 0 || longitud >= MAX_PATH) {
        std::cout << "No se pudo obtener una ruta completa del ejecutable (limite MAX_PATH)." << std::endl;
        return;
    }
    const std::string exe = ruta;
    const char* modos[] = {"--servidor 5000", "--router 127.0.0.1 5000", "--gateway 127.0.0.1 5000", "--sensor 127.0.0.1 5000"};
    for (int i = 0; i < 4; ++i) {
        std::string comando = "\"" + exe + "\" " + modos[i];
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        if (!CreateProcessA(exe.c_str(), &comando[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cout << "No se pudo lanzar: " << comando << " (codigo " << GetLastError()
                      << "). Revise y cierre las consolas ya abiertas." << std::endl;
            return;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        if (i == 0) std::this_thread::sleep_for(std::chrono::milliseconds(1200));  // espera orientativa: NO confirma que el servidor este listo
    }
    std::cout << "Se lanzaron 4 procesos; compruebe ACK y errores en cada consola. Cierre el servidor con ENTER." << std::endl;
#else
    std::cout << "Esta opcion abre ventanas de Windows. En Linux ejecute cada modo en una terminal distinta:\n"
                 "  ./TelcoCauca --servidor 5000   y luego   ./TelcoCauca --router 127.0.0.1 5000  (etc.)" << std::endl;
#endif
}

int menu() {
    std::string opcion;
    for (;;) {
        std::cout << "\n========== TELCOCAUCA ==========\n"
                     "1. Infraestructura fisica\n"
                     "2. Herencia multiple y excepciones\n"
                     "3. Polimorfismo e interfaces\n"
                     "4. Concurrencia y eventos 2027\n"
                     "5. Comunicacion TCP/UDP\n"
                     "6. Demostracion completa\n"
                     "7. Servidor y clientes en procesos independientes\n"
                     "0. Salir\n"
                     "Opcion: " << std::flush;
        if (!std::getline(std::cin, opcion) || opcion == "0") return 0;
        try {
            if (opcion == "1") demoInfraestructura();
            else if (opcion == "2") demoNodos();
            else if (opcion == "3") demoPolimorfismo();
            else if (opcion == "4") demoConcurrencia();
            else if (opcion == "5") demoRed();
            else if (opcion == "6") demoCompleta();
            else if (opcion == "7") lanzarProcesos();
            else std::cout << "Opcion invalida" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error controlado: " << e.what() << std::endl;
        }
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        SistemaRed red;  // WSAStartup/WSACleanup en Windows
        bool pausa = true;
        std::string args[8];
        int n = 0;
        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];
            if (a == "--sinpausa") pausa = false;
            else {
                if (n == 8) throw std::invalid_argument("Demasiados argumentos");
                args[n++] = a;
            }
        }
        if (n == 0) return menu();
        const std::string modo = args[0];
        if (modo == "--pruebas") {
            if (n != 1) throw std::invalid_argument("--pruebas no recibe argumentos posicionales");
            return demoCompleta() ? 0 : 1;
        }
        if (modo == "--servidor") {
            if (n > 3) throw std::invalid_argument("Uso: --servidor [puerto] [segundos]");
            const std::uint16_t puerto = n > 1 ? static_cast<std::uint16_t>(enteroDecimal(args[1], 1, 65535, "Puerto")) : 5000;
            const int segundos = n > 2 ? enteroDecimal(args[2], 0, std::numeric_limits<int>::max(), "Segundos") : 0;
            return modoServidor(puerto, segundos, pausa);
        }
        if (modo == "--router" || modo == "--gateway" || modo == "--sensor") {
            if (n > 3) throw std::invalid_argument("Uso: --router|--gateway|--sensor [ip] [puerto]");
            const std::string ip = n > 1 ? args[1] : "127.0.0.1";
            const std::uint16_t puerto = n > 2 ? static_cast<std::uint16_t>(enteroDecimal(args[2], 1, 65535, "Puerto")) : 5000;
            const int codigo = ejecutarRol(modo.substr(2), ip, puerto);
            if (pausa) esperarEnter();
            return codigo;
        }
        mostrarAyuda();
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "Error fatal: " << e.what() << std::endl;
        return 1;
    }
}
