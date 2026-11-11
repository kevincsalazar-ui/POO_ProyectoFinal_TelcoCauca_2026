#ifndef DISPOSITIVORED_H
#define DISPOSITIVORED_H
#include <string>

// Primera clase base de NodoRedInteligente: identidad de red (IP, MAC, estado).
class DispositivoRed {
    std::string direccionIP;
    std::string direccionMAC;
    bool estadoOperativo;

public:
    DispositivoRed(const std::string& ip, const std::string& mac, bool activo);
    virtual ~DispositivoRed() = default;  // base de herencia multiple: destructor virtual

    static bool ipValida(const std::string& ip);    // IPv4 "a.b.c.d", cada parte 0..255
    static bool macValida(const std::string& mac);  // "AA:BB:CC:DD:EE:FF"

    const std::string& getDireccionIP() const { return direccionIP; }
    const std::string& getDireccionMAC() const { return direccionMAC; }
    bool getEstadoOperativo() const { return estadoOperativo; }

    void setDireccionIP(const std::string& ip);  // lanza ExcepcionNodoRed si es invalida
    void setDireccionMAC(const std::string& mac);
    void setEstadoOperativo(bool activo) { estadoOperativo = activo; }
};

#endif
