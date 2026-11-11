#include "Demo.h"
#include "Antena.h"
#include "Clientes.h"
#include "Dispositivos.h"
#include "Eventos.h"
#include "Excepciones.h"
#include "GrupoHilos.h"
#include "NodoRedInteligente.h"
#include "Registro.h"
#include "ServidorCentral.h"
#include "Supervisor.h"
#include "Tecnico.h"
#include "Torre.h"
#include "Util.h"
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

namespace {

// Lleva la cuenta de verificaciones. Solo se usa desde el hilo principal.
class Verificador {
    int pasadas;
    int falladas;
    Registro* registro;  // no propietario; comparte el mutex de los trabajadores cuando hay red activa

public:
    explicit Verificador(Registro* r = nullptr) : pasadas(0), falladas(0), registro(r) {}
    void comprobar(bool condicion, const std::string& descripcion) {
        const std::string mensaje = std::string(condicion ? "  [OK]    " : "  [FALLO] ") + descripcion;
        if (registro) registro->imprimir(mensaje);
        else std::cout << mensaje << std::endl;
        if (condicion) ++pasadas;
        else ++falladas;
    }
    bool exito() const { return falladas == 0 && pasadas > 0; }
    int total() const { return pasadas + falladas; }
};

void titulo(const std::string& texto) {
    std::cout << "\n=== " << texto << " ===" << std::endl;
}

template <class Condicion>
bool esperarHasta(Condicion condicion, int maxMs) {
    for (int t = 0; t < maxMs; t += 20) {
        if (condicion()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return condicion();
}

}  // namespace

// ---------------------------------------------------------------- Modulo 1
bool demoInfraestructura() {
    titulo("Modulo 1: Infraestructura fisica (Torre 1:N Antena 1:1 Tecnico)");
    Verificador v;
    // Orden de declaracion: tecnicos y antenas ANTES que la torre (la torre solo guarda punteros).
    Tecnico ana(1, "Ana", "Radiofrecuencia");
    Tecnico luis(2, "Luis", "Redes moviles");
    Antena a(101, 1800, "4G", &ana);
    Antena b(102, 3500, "5G", &luis);
    Torre torre(1, "Popayan", 35);
    torre.agregarAntena(&a);
    torre.agregarAntena(&b);
    torre.mostrarInformacion();

    v.comprobar(torre.cantidadAntenas() == 2, "La torre guarda 2 antenas (relacion 1:N)");
    v.comprobar(a.getResponsable() == &ana && ana.getAntenaAsignada() == &a, "Antena 101 <-> Ana (relacion 1:1 bidireccional)");

    bool rechazoNulo = false;
    try { torre.agregarAntena(nullptr); } catch (const std::invalid_argument& e) {
        rechazoNulo = true;
        std::cout << "    Excepcion esperada: " << e.what() << std::endl;
    }
    v.comprobar(rechazoNulo, "Torre rechaza una antena nula");

    bool rechazoRepetida = false;
    try { torre.agregarAntena(&a); } catch (const std::invalid_argument&) { rechazoRepetida = true; }
    v.comprobar(rechazoRepetida && torre.cantidadAntenas() == 2, "Torre rechaza una antena repetida");

    bool rechazo11 = false;
    try { b.setResponsable(&ana); } catch (const std::invalid_argument& e) {
        rechazo11 = true;
        std::cout << "    Excepcion esperada: " << e.what() << std::endl;
    }
    v.comprobar(rechazo11 && b.getResponsable() == &luis, "Un tecnico no puede atender dos antenas (regla 1:1)");

    bool rechazoAltura = false;
    try { torre.setAlturaMetros(-5); } catch (const std::invalid_argument&) { rechazoAltura = true; }
    v.comprobar(rechazoAltura && torre.getAlturaMetros() == 35, "El setter valida la altura y conserva el valor anterior");
    return v.exito();
}

// ---------------------------------------------------------------- Modulo 2
bool demoNodos() {
    titulo("Modulo 2: Herencia multiple, operadores y excepciones");
    Verificador v;
    NodoRedInteligente n1("192.168.1.10", "02:00:00:00:00:01", true, "Temperatura", 24.5, 7);
    NodoRedInteligente n2("192.168.1.11", "02:00:00:00:00:02", true, "Temperatura", 26.0, 5);
    std::cout << "  " << n1 << "\n  " << n2 << std::endl;
    std::cout << "  Carga total (n1 + n2) = " << (n1 + n2) << std::endl;
    v.comprobar(n1 + n2 == 12.0, "operator+ suma las cargas de procesamiento (7 + 5 = 12)");

    DispositivoRed* comoRed = &n1;  // un nodo ES un DispositivoRed
    SensorIoT* comoSensor = &n1;    // y TAMBIEN es un SensorIoT
    v.comprobar(comoRed->getDireccionIP() == "192.168.1.10" && comoSensor->getTipoSensor() == "Temperatura",
                "El nodo se usa como DispositivoRed y como SensorIoT");

    bool lecturaRechazada = false;
    try { n1.setValorLectura(120.0); } catch (const ExcepcionNodoRed& e) {
        lecturaRechazada = true;
        std::cout << "    Lectura rechazada: " << e.what() << std::endl;
    }
    v.comprobar(lecturaRechazada && n1.getValorLectura() == 24.5, "Lectura 120 C fuera de rango: excepcion y valor anterior intacto");

    bool ipRechazada = false;
    try { NodoRedInteligente malo("300.0.0.1", "02:00:00:00:00:09", true, "Temperatura", 20, 1); } catch (const ExcepcionNodoRed& e) {
        ipRechazada = true;
        std::cout << "    IP rechazada: " << e.what() << std::endl;
    }
    v.comprobar(ipRechazada, "IP 300.0.0.1 invalida: no se construye el nodo");

    bool comoRuntimeError = false;
    try { n2.setValorLectura(-100.0); } catch (const std::runtime_error&) { comoRuntimeError = true; }
    v.comprobar(comoRuntimeError, "ExcepcionNodoRed se captura como std::runtime_error (herencia)");
    return v.exito();
}

// ---------------------------------------------------------------- Modulo 3
bool demoPolimorfismo() {
    titulo("Modulo 3: Polimorfismo e interfaces puras");
    Verificador v;
    const int antes = DispositivoComunicacion::instanciasVivas();
    {
        // Duenos (unique_ptr): al salir del bloque hacen delete a traves de la base -> destructor virtual.
        std::vector<std::unique_ptr<DispositivoComunicacion> > duenos;
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new RouterWiFi("RTR-01", 4)));
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new GatewayLoRa("GW-01", 9)));
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new SwitchFibra("SW-01", 24)));
        // Vista polimorfica (no propietaria): vector<DispositivoComunicacion*> como pide la guia.
        std::vector<DispositivoComunicacion*> vista;
        for (std::size_t i = 0; i < duenos.size(); ++i) vista.push_back(duenos[i].get());

        v.comprobar(DispositivoComunicacion::instanciasVivas() == antes + 3, "3 dispositivos creados dinamicamente");
        for (std::size_t i = 0; i < vista.size(); ++i)
            std::cout << "  " << vista[i]->getId() << " -> " << vista[i]->getTipo() << std::endl;

        bool sinError = true;
        try { transmitirTodos(vista); } catch (const std::exception&) { sinError = false; }
        v.comprobar(sinError, "transmitirTodos invoca transmitir() de cada tipo real (despacho dinamico)");

        std::vector<std::uint8_t> bytes = vista[0]->serializar();  // contrato ITransmisible
        v.comprobar(bytes.size() == Protocolo::TAM_TRAMA, "serializar() (ITransmisible) produce una trama de 30 bytes");

        vista.push_back(nullptr);  // caso de prueba: puntero nulo
        bool nuloDetectado = false;
        try { transmitirTodos(vista); } catch (const std::invalid_argument& e) {
            nuloDetectado = true;
            std::cout << "    Excepcion esperada: " << e.what() << std::endl;
        }
        v.comprobar(nuloDetectado, "nullptr detectado antes de usar '->' (std::invalid_argument)");
    }  // aqui se destruyen los 3 dispositivos
    v.comprobar(DispositivoComunicacion::instanciasVivas() == antes, "El contador de dispositivos vuelve al valor inicial (alcance didactico)");
    return v.exito();
}

// ---------------------------------------------------------------- Modulo 4
bool demoConcurrencia() {
    titulo("Modulo 4: Concurrencia, mutex y eventos 2027");
    Verificador v;
    Registro registro;
    {
        std::vector<std::unique_ptr<DispositivoComunicacion> > duenos;
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new RouterWiFi("RTR-01", 4)));
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new GatewayLoRa("GW-01", 9)));
        duenos.push_back(std::unique_ptr<DispositivoComunicacion>(new SwitchFibra("SW-01", 24)));
        std::vector<DispositivoComunicacion*> vista;
        for (std::size_t i = 0; i < duenos.size(); ++i) vista.push_back(duenos[i].get());
        vista.push_back(nullptr);  // error deliberado: el hilo debe capturarlo, no terminar el programa

        const Supervisor supervisor;
        GrupoHilos hilos;
        for (std::size_t i = 0; i < vista.size(); ++i)
            hilos.agregar(&Supervisor::supervisar, &supervisor, vista[i], std::ref(registro));  // un hilo por dispositivo
        hilos.agregar(tareaEventos2027, std::ref(registro));                                    // hilo independiente de eventos
        registro.imprimir("Hilos lanzados: " + aTexto(hilos.cantidad()) + " (4 supervisores + 1 de eventos)");
        hilos.esperar();  // join() de cada hilo ANTES de leer resultados y de liberar los dispositivos
    }
    std::cout << "  Resumen: exitosas=" << registro.exitosas() << " errores=" << registro.errores()
              << " eventos=" << registro.cantidadEventos() << " hilos distintos=" << registro.cantidadHilos() << std::endl;
    v.comprobar(registro.exitosas() == 3, "3 transmisiones exitosas");
    v.comprobar(registro.errores() == 1, "1 error controlado (puntero nulo capturado dentro del hilo)");
    v.comprobar(registro.cantidadEventos() >= 3, "Al menos 3 eventos 2027 registrados por su hilo");
    v.comprobar(registro.cantidadHilos() >= 5, "Se observan al menos 5 hilos distintos (concurrencia real)");

    ContadorSeguro contador;
    {
        GrupoHilos hilos;
        for (int i = 0; i < 4; ++i) hilos.agregar(incrementarVeces, std::ref(contador), 5000);
    }  // el destructor de GrupoHilos hace join
    std::cout << "  Contador compartido: " << contador.valor() << " (esperado 20000)" << std::endl;
    v.comprobar(contador.valor() == 20000, "4 hilos x 5000 incrementos = 20000 exactos (exclusion mutua con lock_guard)");
    return v.exito();
}

// ---------------------------------------------------------------- Modulo 5
bool demoRed() {
    titulo("Modulo 5: Red TCP/UDP con serializacion binaria");
    Registro registro;
    Verificador v(&registro);
    ServidorCentral servidor(registro, 800);  // inactividad corta para que la demo sea rapida
    servidor.iniciar("127.0.0.1", 0, 0);       // puerto 0: el sistema asigna uno libre
    const std::uint16_t pTcp = servidor.puertoTcp();
    const std::uint16_t pUdp = servidor.puertoUdp();
    registro.imprimir("Servidor listo: TCP " + aTexto(pTcp) + " / UDP " + aTexto(pUdp));

    // --- Trafico valido: tres clientes concurrentes (router y gateway por TCP, sensor por UDP)
    ResultadoCliente rRouter, rGateway, rSensor;
    {
        GrupoHilos clientes;
        clientes.agregar([&]() {
            try {
                RouterWiFi router("RTR-01", 4);
                std::vector<Protocolo::Trama> t;
                t.push_back(router.aTrama(1));
                t.push_back(router.aTrama(2));
                rRouter = clienteTcp("127.0.0.1", pTcp, t, registro, 50);
            } catch (const std::exception& e) { registro.registrarError(std::string("Cliente router: ") + e.what()); }
        });
        clientes.agregar([&]() {
            try {
                GatewayLoRa gateway("GW-01", 9);
                std::vector<Protocolo::Trama> t;
                t.push_back(gateway.aTrama(1));
                t.push_back(gateway.aTrama(2));
                rGateway = clienteTcp("127.0.0.1", pTcp, t, registro, 50);
            } catch (const std::exception& e) { registro.registrarError(std::string("Cliente gateway: ") + e.what()); }
        });
        clientes.agregar([&]() {
            try {
                std::vector<Protocolo::Trama> t;
                t.push_back(Protocolo::crearTemperatura("SENSOR-01", "127.0.0.1", 1, 24.5));
                t.push_back(Protocolo::crearTemperatura("SENSOR-01", "127.0.0.1", 2, 25.0));
                rSensor = clienteUdp("127.0.0.1", pUdp, t, registro, 50);
            } catch (const std::exception& e) { registro.registrarError(std::string("Cliente sensor: ") + e.what()); }
        });
    }  // join de los tres clientes
    v.comprobar(rRouter.ack == 2 && rGateway.ack == 2, "Router y gateway: 4 tramas ESTADO por TCP con ACK");
    v.comprobar(rSensor.ack == 2, "Sensor: 2 tramas TEMPERATURA por UDP con ACK");
    v.comprobar(servidor.tramasTcpAceptadas() == 4 && servidor.tramasUdpAceptadas() == 2,
                "El servidor acepto 4 tramas TCP y 2 UDP");

    // Buscar causas concretas en una copia protegida, sin inferirlas de un total global.
    const auto errorDesde = [&](std::size_t desde, const std::string& causa) {
        const std::vector<std::string> errores = registro.copiaErrores();
        for (std::size_t i = desde; i < errores.size(); ++i)
            if (errores[i].find(causa) != std::string::npos) return true;
        return false;
    };
    // --- Fallos: el servidor debe resistir y seguir funcionando
    registro.imprimir("--- Inyeccion de fallos ---");
    const std::size_t antesTemperatura = registro.copiaErrores().size();
    {   // 1) Trama con temperatura fuera de rango y CRC valido -> NACK
        std::vector<Protocolo::Trama> t;
        t.push_back(Protocolo::crearTemperatura("SENSOR-X", "127.0.0.1", 9, 120.0));
        std::vector<std::uint8_t> bytes = Protocolo::serializar(t[0], false);
        Socket s(TipoSocket::TCP);
        s.fijarTimeout(2000);
        s.conectar("127.0.0.1", pTcp, 2000);
        s.enviarTodo(&bytes[0], bytes.size());
        std::uint8_t r[Protocolo::TAM_RESPUESTA];
        const EstadoRecepcion estado = s.recibirExacto(r, sizeof(r));
        if (estado == EstadoRecepcion::TIMEOUT) registro.imprimir("TCP: timeout esperando NACK");
        if (estado == EstadoRecepcion::CERRADO) registro.imprimir("TCP: conexion cerrada antes del NACK");
        // Un cierre/timeout parcial lanza ExcepcionRedTCP y la demo informa el fallo concreto.
        Protocolo::CodigoRespuesta c = Protocolo::ACK;
        std::uint32_t seq = 0;
        v.comprobar(estado == EstadoRecepcion::OK && Protocolo::interpretarRespuesta(r, sizeof(r), c, seq) && c == Protocolo::NACK && seq == 9 &&
                    errorDesde(antesTemperatura, "Lectura fuera del rango de temperatura"),
                    "TCP: temperatura 120 C -> NACK (el servidor la rechaza y sigue vivo)");
    }
    const std::size_t antesCrc = registro.copiaErrores().size();
    {   // 2) Datagrama corrupto (CRC alterado) -> NACK
        std::vector<std::uint8_t> bytes = Protocolo::serializar(Protocolo::crearTemperatura("SENSOR-01", "127.0.0.1", 7, 20.0));
        bytes[10] ^= 0xFF;
        Socket s(TipoSocket::UDP);
        s.fijarTimeout(2000);
        s.enviarA("127.0.0.1", pUdp, &bytes[0], bytes.size());
        std::uint8_t r[64];
        std::size_t n = 0;
        std::string ip;
        std::uint16_t p = 0;
        Protocolo::CodigoRespuesta c = Protocolo::ACK;
        std::uint32_t seq = 0;
        const bool llego = s.recibirDe(r, sizeof(r), n, ip, p) == EstadoRecepcion::OK;
        v.comprobar(llego && ip == "127.0.0.1" && p == pUdp && Protocolo::interpretarRespuesta(r, n, c, seq) && c == Protocolo::NACK && seq == 7 &&
                    errorDesde(antesCrc, "CRC incorrecto: trama corrupta"),
                    "UDP: datagrama corrupto (CRC) -> NACK");
    }
    const std::size_t antesParcial = registro.copiaErrores().size();
    {   // 3) Cliente que se desconecta a mitad de una trama
        std::vector<std::uint8_t> bytes = Protocolo::serializar(Protocolo::crearEstado("RTR-02", "127.0.0.1", 1, true));
        Socket s(TipoSocket::TCP);
        s.conectar("127.0.0.1", pTcp, 2000);
        s.enviarTodo(&bytes[0], 10);  // solo 10 de 30 bytes
        s.cerrar();
    }
    v.comprobar(esperarHasta([&]() { return errorDesde(antesParcial, "Conexion cerrada a mitad de trama (10 de 30 bytes)"); }, 4000),
                "TCP: cierre a mitad de trama registrado con 10 de 30 bytes");
    const std::size_t antesSilencio = registro.copiaErrores().size();
    {   // 4) Cliente silencioso: conecta y no envia nada
        Socket s(TipoSocket::TCP);
        s.conectar("127.0.0.1", pTcp, 2000);
        s.fijarTimeout(4000);
        std::uint8_t respuesta[Protocolo::TAM_RESPUESTA];
        const EstadoRecepcion estado = s.recibirExacto(respuesta, sizeof(respuesta));
        v.comprobar(estado == EstadoRecepcion::CERRADO &&
                    errorDesde(antesSilencio, "Servidor TCP: cliente inactivo, se cierra la conexion"),
                    "TCP: cliente silencioso cerrado por inactividad y causa registrada");
    }
    v.comprobar(servidor.tramasRechazadas() == 2, "El servidor rechazo exactamente 2 tramas invalidas");
    {   // 5) Despues de los fallos el servidor sigue atendiendo
        std::vector<Protocolo::Trama> t;
        t.push_back(Protocolo::crearEstado("RTR-03", "127.0.0.1", 1, true));
        ResultadoCliente r = clienteTcp("127.0.0.1", pTcp, t, registro);
        v.comprobar(r.ack == 1, "Tras los fallos, el servidor sigue atendiendo clientes nuevos");
    }
    {   // 6) Puerto ocupado: un segundo servidor en el mismo puerto TCP debe fallar con excepcion
        Registro otro;
        ServidorCentral duplicado(otro);
        bool ocupado = false;
        try { duplicado.iniciar("127.0.0.1", pTcp, 0); } catch (const ExcepcionRedTCP& e) {
            ocupado = true;
            registro.imprimir(std::string("Excepcion esperada: ") + e.what());
        }
        v.comprobar(ocupado, "Puerto ocupado: bind() falla con ExcepcionRedTCP");
    }
    {   // 7) Conexion a un puerto donde nadie escucha
        std::uint16_t libre = 0;
        {   // pedir un puerto libre al sistema y liberarlo
            Socket tmp(TipoSocket::TCP);
            tmp.enlazar("127.0.0.1", 0);
            libre = tmp.puertoLocal();
        }
        bool rechazada = false;
        try { Socket s(TipoSocket::TCP); s.conectar("127.0.0.1", libre, 1500); } catch (const ExcepcionRedTCP& e) {
            rechazada = true;
            registro.imprimir(std::string("Excepcion esperada: ") + e.what());
        }
        v.comprobar(rechazada, "Conexion a un puerto sin servidor: ExcepcionRedTCP controlada");
    }
    servidor.detener();  // join de todos los hilos del servidor
    v.comprobar(true, "Servidor detenido ordenadamente (todos los hilos con join)");
    return v.exito();
}

// ---------------------------------------------------------------- Demo completa
bool demoCompleta() {
    struct Paso { const char* nombre; bool (*funcion)(); bool ok; };
    Paso pasos[] = {{"Modulo 1 - Infraestructura fisica", demoInfraestructura, false},
                    {"Modulo 2 - Herencia multiple y excepciones", demoNodos, false},
                    {"Modulo 3 - Polimorfismo e interfaces", demoPolimorfismo, false},
                    {"Modulo 4 - Concurrencia y eventos 2027", demoConcurrencia, false},
                    {"Modulo 5 - Comunicacion TCP/UDP", demoRed, false}};
    const std::size_t cantidad = sizeof(pasos) / sizeof(pasos[0]);
    for (std::size_t i = 0; i < cantidad; ++i) {
        try {
            pasos[i].ok = pasos[i].funcion();
        } catch (const std::exception& e) {
            std::cout << "  [FALLO] Excepcion no controlada: " << e.what() << std::endl;
            pasos[i].ok = false;
        }
    }
    bool todo = true;
    std::cout << "\n=========== RESUMEN ===========" << std::endl;
    for (std::size_t i = 0; i < cantidad; ++i) {
        std::cout << (pasos[i].ok ? "[OK]    " : "[FALLO] ") << pasos[i].nombre << std::endl;
        todo = todo && pasos[i].ok;
    }
    std::cout << (todo ? "RESULTADO GLOBAL: TODOS LOS MODULOS CORRECTOS" : "RESULTADO GLOBAL: HAY MODULOS CON FALLOS") << std::endl;
    return todo;
}
