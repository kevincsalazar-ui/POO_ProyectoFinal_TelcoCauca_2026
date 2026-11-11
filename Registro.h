#ifndef REGISTRO_H
#define REGISTRO_H
#include <cstddef>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

struct EventoIngenieria {
    std::string nombre;
    std::string fecha;
    std::string lugar;
    std::string tema;
};

// Recursos compartidos entre hilos (consola, contadores, buffers). Un unico mutex los protege:
// cada metodo publico toma un std::lock_guard, asi el bloqueo se libera solo (RAII), incluso ante excepciones.
// El Registro contiene un mutex y por eso no se puede copiar: los hilos lo reciben con std::ref.
class Registro {
    mutable std::mutex mtx;
    int totalExitosas;
    int totalErrores;
    std::vector<EventoIngenieria> eventos;
    std::vector<std::string> tramas;
    std::vector<std::string> mensajesError;
    std::map<std::thread::id, int> hilosVistos;  // id de hilo -> numero H1, H2, ...

    int indiceHilo();                         // requiere mtx tomado
    void escribir(const std::string& linea);  // requiere mtx tomado

public:
    Registro();
    Registro(const Registro&) = delete;
    Registro& operator=(const Registro&) = delete;

    void imprimir(const std::string& mensaje);        // linea de consola sin entrelazarse con otros hilos
    void registrarExito(const std::string& mensaje);
    void registrarError(const std::string& mensaje);
    void agregarEvento(const EventoIngenieria& evento);
    void agregarTrama(const std::string& descripcion);

    int exitosas() const;
    int errores() const;
    std::size_t cantidadEventos() const;
    std::size_t cantidadTramas() const;
    std::size_t cantidadHilos() const;  // hilos distintos que han escrito en el registro
    std::vector<std::string> copiaErrores() const;
    std::vector<std::string> copiaTramas() const;
};

// Contador compartido minimo para demostrar exclusion mutua: N hilos incrementan y el total es exacto.
class ContadorSeguro {
    mutable std::mutex mtx;
    long valorActual;

public:
    ContadorSeguro() : valorActual(0) {}
    ContadorSeguro(const ContadorSeguro&) = delete;
    ContadorSeguro& operator=(const ContadorSeguro&) = delete;
    void incrementar();
    long valor() const;
};

void incrementarVeces(ContadorSeguro& contador, int veces);

#endif
