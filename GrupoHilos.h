#ifndef GRUPOHILOS_H
#define GRUPOHILOS_H
#include <thread>
#include <utility>
#include <vector>

class GrupoHilos {
    std::vector<std::thread> hilos;
public:
    explicit GrupoHilos(std::size_t capacidad) { hilos.reserve(capacidad); }
    template<class Funcion, class... Args>
    void agregar(Funcion&& funcion, Args&&... argumentos) {
        hilos.emplace_back(std::forward<Funcion>(funcion), std::forward<Args>(argumentos)...);
    }
    void esperar() {
        for (auto& hilo : hilos)
            if (hilo.joinable()) hilo.join();
        hilos.clear();
    }
    ~GrupoHilos() { esperar(); }
};
#endif
