#ifndef PROTOCOLO_H
#define PROTOCOLO_H
// Protocolo binario TC v1. Formato EXPLICITO en bytes, con tipos de ancho fijo y orden de red.
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace Protocolo {

enum TipoTrama { ESTADO = 1, TEMPERATURA = 2 };
enum CodigoRespuesta { ACK = 0, NACK = 1 };

// Objeto de dominio (cabe en memoria con std::string). NUNCA se envia directamente a la red.
struct Trama {
    std::uint8_t tipo;
    std::string origen;  // 1..11 caracteres imprimibles
    std::string ip;      // IPv4 declarada por el emisor
    std::uint32_t secuencia;
    double valor;  // ESTADO: 0 o 1. TEMPERATURA: grados C en [-40, 85]
};

// Estructuras de cable. Solo contienen enteros y arreglos de char de ancho fijo
// (nada de std::string ni punteros), asi su representacion en memoria ES el formato.
//
// #pragma pack(push, 1) pide al compilador alineacion de 1 byte: elimina el relleno
// (padding) que normalmente se inserta entre campos. Con eso sizeof(TramaWire) es exactamente
// la suma de sus campos. Limites: NO resuelve el orden de bytes (por eso cada campo
// multibyte se guarda en orden de red con htonl/htons) y acceder a campos desalineados
// por puntero puede ser lento o invalido en algunas CPU (por eso se copia con memcpy).
#pragma pack(push, 1)
struct TramaWire {
    std::uint8_t magic[2];   // 'T','C'
    std::uint8_t version;    // 1
    std::uint8_t tipo;       // TipoTrama
    char origen[12];         // texto, relleno con ceros, el ultimo byte siempre 0
    std::uint32_t ip;        // IPv4 en orden de red
    std::uint32_t secuencia; // orden de red
    std::int32_t valorMilli; // valor * 1000, orden de red
    std::uint16_t crc;       // CRC-16/CCITT-FALSE de los 28 bytes anteriores, orden de red
};
struct RespuestaWire {
    std::uint8_t magic[2];   // 'T','R'
    std::uint8_t codigo;     // CodigoRespuesta
    std::uint32_t secuencia; // secuencia de la trama que se responde
};
#pragma pack(pop)

static_assert(sizeof(TramaWire) == 30, "TramaWire debe medir exactamente 30 bytes");
static_assert(sizeof(RespuestaWire) == 7, "RespuestaWire debe medir exactamente 7 bytes");

constexpr std::size_t TAM_TRAMA = sizeof(TramaWire);
constexpr std::size_t TAM_RESPUESTA = sizeof(RespuestaWire);

Trama crearEstado(const std::string& origen, const std::string& ip, std::uint32_t secuencia, bool operativo);
Trama crearTemperatura(const std::string& origen, const std::string& ip, std::uint32_t secuencia, double grados);

// Valida la trama (ExcepcionNodoRed si es invalida) y la convierte en 30 bytes.
// validar=false SOLO sirve para pruebas de inyeccion de fallos.
std::vector<std::uint8_t> serializar(const Trama& trama, bool validar = true);

// Convierte bytes recibidos en Trama. Lanza ExcepcionNodoRed si el tamano, la firma, la version,
// el CRC o los valores no son correctos. NUNCA confia en los datos de la red.
Trama deserializar(const std::uint8_t* datos, std::size_t n);

std::vector<std::uint8_t> serializarRespuesta(CodigoRespuesta codigo, std::uint32_t secuencia);
bool interpretarRespuesta(const std::uint8_t* datos, std::size_t n, CodigoRespuesta& codigo, std::uint32_t& secuencia);

// Lee la secuencia de una trama sin validarla (para poder responder NACK a una trama corrupta).
std::uint32_t secuenciaDe(const std::uint8_t* datos, std::size_t n);

std::uint16_t crc16(const std::uint8_t* datos, std::size_t n);
std::string describir(const Trama& trama);

}  // namespace Protocolo

#endif
