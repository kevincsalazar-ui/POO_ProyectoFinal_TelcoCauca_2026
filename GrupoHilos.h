#ifndef GRUPOHILOS_H
#define GRUPOHILOS_H
#include <cstddef>
#include <thread>
#include <utility>
#include <vector>

// Agrupa std::thread y garantiza join(). Un std::thread que sigue "joinable" al destruirse
// llama a std::terminate; el destructor de esta clase lo evita aunque otra cosa lance una excepcion.
class GrupoHilos {
    std::vector<std::thread> hilos;

public:
    GrupoHilos() {}
    GrupoHilos(const GrupoHilos&) = delete;
    GrupoHilos& operator=(const GrupoHilos&) = delete;
    ~GrupoHilos() { esperar(); }

    // Acepta funciones libres, lambdas y miembros: agregar(&Clase::metodo, &objeto, std::ref(dato))
    template <class Funcion, class... Args>
    void agregar(Funcion&& funcion, Args&&... argumentos) {
        hilos.emplace_back(std::forward<Funcion>(funcion), std::forward<Args>(argumentos)...);
    }

    void esperar() {
        for (std::size_t i = 0; i < hilos.size(); ++i)
            if (hilos[i].joinable()) hilos[i].join();
        hilos.clear();
    }

    std::size_t cantidad() const { return hilos.size(); }
};

#endif
