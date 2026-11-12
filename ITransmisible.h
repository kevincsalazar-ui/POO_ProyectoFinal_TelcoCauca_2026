#ifndef ITRANSMISIBLE_H
#define ITRANSMISIBLE_H
#include <cstdint>
#include <vector>

// Interfaz pura: solo metodos virtuales puros y ningun atributo.
// Contrato: todo objeto transmisible sabe convertirse en bytes para la red.
class ITransmisible {
public:
    virtual ~ITransmisible() = 0;
    virtual std::vector<std::uint8_t> serializar() const = 0;
};

// Incluso un destructor virtual puro necesita definicion: se ejecuta al destruir derivados.
inline ITransmisible::~ITransmisible() = default;

#endif
