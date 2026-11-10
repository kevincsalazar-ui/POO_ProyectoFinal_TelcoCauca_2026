#ifndef ITRANSMISIBLE_H
#define ITRANSMISIBLE_H
#include <string>

class ITransmisible {
public:
    virtual ~ITransmisible() = default;
    virtual std::string serializar() const = 0;
};
#endif
