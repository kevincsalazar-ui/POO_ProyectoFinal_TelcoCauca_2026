#include "Protocolo.h"
#include "DispositivoRed.h"
#include "Excepciones.h"
#include "PlataformaRed.h"  
#include "SensorIoT.h"
#include "Util.h"
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>

namespace Protocolo {

namespace {

const std::uint8_t VERSION = 1;

std::uint32_t ipAEntero(const std::string& ip) {
    std::uint32_t resultado = 0;
    std::uint32_t parte = 0;
    for (std::size_t i = 0; i <= ip.size(); ++i) {
        if (i == ip.size() || ip[i] == '.') {
            resultado = (resultado << 8) | parte;
            parte = 0;
        } else {
            parte = parte * 10 + static_cast<std::uint32_t>(ip[i] - '0');
        }
    }
    return resultado;  // orden de host: 127.0.0.1 -> 0x7F000001
}

std::string enteroAIp(std::uint32_t v) {
    return aTexto((v >> 24) & 0xFF) + "." + aTexto((v >> 16) & 0xFF) + "." + aTexto((v >> 8) & 0xFF) + "." +
           aTexto(v & 0xFF);
}

void validar(const Trama& t) {
    if (t.tipo != ESTADO && t.tipo != TEMPERATURA) throw ExcepcionNodoRed("Tipo de trama desconocido");
    if (t.origen.empty() || t.origen.size() > 11) throw ExcepcionNodoRed("El origen debe tener 1 a 11 caracteres");
    for (std::size_t i = 0; i < t.origen.size(); ++i)
        if (t.origen[i] < 32 || t.origen[i] > 126) throw ExcepcionNodoRed("El origen contiene caracteres no imprimibles");
    if (!DispositivoRed::ipValida(t.ip)) throw ExcepcionNodoRed("Direccion IPv4 invalida en la trama: " + t.ip);
    if (!std::isfinite(t.valor)) throw ExcepcionNodoRed("Valor no finito en la trama");
    if (t.tipo == ESTADO && t.valor != 0.0 && t.valor != 1.0) throw ExcepcionNodoRed("El estado debe ser 0 o 1");
    if (t.tipo == TEMPERATURA) SensorIoT::validarLectura(t.valor);
}

}

std::uint16_t crc16(const std::uint8_t* datos, std::size_t n) {
    std::uint16_t crc = 0xFFFF;
    for (std::size_t i = 0; i < n; ++i) {
        crc = static_cast<std::uint16_t>(crc ^ (static_cast<std::uint16_t>(datos[i]) << 8));
        for (int bit = 0; bit < 8; ++bit)
            crc = static_cast<std::uint16_t>((crc & 0x8000) ? ((crc << 1) ^ 0x1021) : (crc << 1));
    }
    return crc;
}

Trama crearEstado(const std::string& origen, const std::string& ip, std::uint32_t secuencia, bool operativo) {
    Trama t = {static_cast<std::uint8_t>(ESTADO), origen, ip, secuencia, operativo ? 1.0 : 0.0};
    return t;
}

Trama crearTemperatura(const std::string& origen, const std::string& ip, std::uint32_t secuencia, double grados) {
    Trama t = {static_cast<std::uint8_t>(TEMPERATURA), origen, ip, secuencia, grados};
    return t;
}

std::vector<std::uint8_t> serializar(const Trama& trama, bool validarDatos) {
    if (validarDatos) validar(trama);
    TramaWire w;
    std::memset(&w, 0, sizeof(w));
    w.magic[0] = 'T';
    w.magic[1] = 'C';
    w.version = VERSION;
    w.tipo = trama.tipo;
    std::memcpy(w.origen, trama.origen.data(), trama.origen.size() < 11 ? trama.origen.size() : 11);
    w.ip = htonl(ipAEntero(trama.ip));
    w.secuencia = htonl(trama.secuencia);
    const std::int32_t milli = static_cast<std::int32_t>(std::lround(trama.valor * 1000.0));
    w.valorMilli = static_cast<std::int32_t>(htonl(static_cast<std::uint32_t>(milli)));
    std::vector<std::uint8_t> bytes(sizeof(w));
    std::memcpy(&bytes[0], &w, sizeof(w));
    const std::uint16_t crc = crc16(&bytes[0], sizeof(w) - sizeof(w.crc));
    const std::uint16_t crcRed = htons(crc);
    std::memcpy(&bytes[sizeof(w) - sizeof(w.crc)], &crcRed, sizeof(crcRed));
    return bytes;
}

Trama deserializar(const std::uint8_t* datos, std::size_t n) {
    if (datos == nullptr || n != sizeof(TramaWire))
        throw ExcepcionNodoRed("Tamano de trama invalido: " + aTexto(n) + " bytes (se esperaban " +
                               aTexto(sizeof(TramaWire)) + ")");
    TramaWire w;
    std::memcpy(&w, datos, sizeof(w));  // copia: evita leer campos desalineados por puntero
    if (w.magic[0] != 'T' || w.magic[1] != 'C') throw ExcepcionNodoRed("Firma de trama incorrecta");
    if (w.version != VERSION) throw ExcepcionNodoRed("Version de protocolo no soportada");
    std::uint16_t crcRecibido;
    std::memcpy(&crcRecibido, &w.crc, sizeof(crcRecibido));
    if (ntohs(crcRecibido) != crc16(datos, sizeof(w) - sizeof(w.crc))) throw ExcepcionNodoRed("CRC incorrecto: trama corrupta");
    if (w.origen[11] != '\0') throw ExcepcionNodoRed("Origen sin terminador");
    Trama t;
    t.tipo = w.tipo;
    t.origen = std::string(w.origen);
    t.ip = enteroAIp(ntohl(w.ip));
    t.secuencia = ntohl(w.secuencia);
    t.valor = static_cast<std::int32_t>(ntohl(static_cast<std::uint32_t>(w.valorMilli))) / 1000.0;
    validar(t);
    return t;
}

std::vector<std::uint8_t> serializarRespuesta(CodigoRespuesta codigo, std::uint32_t secuencia) {
    RespuestaWire r;
    std::memset(&r, 0, sizeof(r));
    r.magic[0] = 'T';
    r.magic[1] = 'R';
    r.codigo = static_cast<std::uint8_t>(codigo);
    r.secuencia = htonl(secuencia);
    std::vector<std::uint8_t> bytes(sizeof(r));
    std::memcpy(&bytes[0], &r, sizeof(r));
    return bytes;
}

bool interpretarRespuesta(const std::uint8_t* datos, std::size_t n, CodigoRespuesta& codigo, std::uint32_t& secuencia) {
    if (datos == nullptr || n != sizeof(RespuestaWire)) return false;
    RespuestaWire r;
    std::memcpy(&r, datos, sizeof(r));
    if (r.magic[0] != 'T' || r.magic[1] != 'R' || r.codigo > NACK) return false;
    codigo = static_cast<CodigoRespuesta>(r.codigo);
    secuencia = ntohl(r.secuencia);
    return true;
}

std::uint32_t secuenciaDe(const std::uint8_t* datos, std::size_t n) {
    if (datos == nullptr || n != sizeof(TramaWire)) return 0;
    TramaWire w;
    std::memcpy(&w, datos, sizeof(w));
    return ntohl(w.secuencia);
}

std::string describir(const Trama& t) {
    std::ostringstream s;
    s << (t.tipo == ESTADO ? "ESTADO" : "TEMPERATURA") << " origen=" << t.origen << " ip=" << t.ip
      << " seq=" << t.secuencia << " valor=" << std::fixed << std::setprecision(t.tipo == ESTADO ? 0 : 1) << t.valor;
    return s.str();
}

}
