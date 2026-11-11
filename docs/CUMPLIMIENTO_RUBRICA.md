# Cumplimiento de la rúbrica final

Esta matriz se preparó con la guía de entrega final y la presentación de arquitectura publicada el 10 de noviembre de 2026.

## 1. Arquitectura POO y polimorfismo - 1,0

| Requisito | Ubicación |
|---|---|
| Clase abstracta con método virtual puro | `DispositivoComunicacion.h` - `virtual void transmitir() = 0` |
| Interfaz pura | `ITransmisible.h` - `serializar() const = 0` y destructor virtual puro definido |
| Clases concretas con `override` | `Dispositivos.h/.cpp` - `RouterWiFi`, `GatewayLoRa`, `SwitchFibra` |
| Destructor virtual | `DispositivoComunicacion.h` - `virtual ~DispositivoComunicacion() = default` |
| Colección polimórfica | `Demo.cpp` - `vector<unique_ptr<...>>` y vista `vector<DispositivoComunicacion*>` |
| Guarda de puntero nulo | `Dispositivos.cpp::transmitirTodos` y `Supervisor.cpp::supervisar` |

La vista de punteros crudos no posee los objetos. El vector de `unique_ptr` realiza la liberación automática a través del destructor virtual.

## 2. Multihilo y exclusión mutua - 1,0

| Requisito | Ubicación |
|---|---|
| Hilos independientes | `Demo.cpp::demoConcurrencia` y `ServidorCentral.cpp` |
| Paso por referencia | `Demo.cpp` - `std::ref(registro)` |
| Control de ciclo de vida | `GrupoHilos.h` y `ServidorCentral.cpp::detener` - `join()` |
| Mutex y RAII | `Registro.cpp` - `std::lock_guard<std::mutex>` |
| Eventos 2027 | `Eventos.cpp::tareaEventos2027` - cuatro eventos en hilo independiente |
| Excepciones dentro de hilos | `Supervisor.cpp`, `Eventos.cpp` y `ServidorCentral.cpp` |

`std::ref` evita copiar `Registro`, que contiene un mutex y no es copiable. `lock_guard` libera el mutex automáticamente al salir del ámbito, incluso si ocurre una excepción.

## 3. Sockets TCP/UDP y red distribuida - 1,0

| Requisito | Ubicación |
|---|---|
| Secuencia TCP | `ServidorCentral.cpp::iniciar` y `bucleAceptar`: `socket`, `bind`, `listen`, `accept` |
| Un hilo por cliente | `ServidorCentral.cpp::bucleAceptar` y `atenderCliente` |
| Clientes TCP | `Clientes.cpp::clienteTcp`: `connect`, envío y recepción exacta |
| UDP | `Clientes.cpp::clienteUdp` y `ServidorCentral.cpp::bucleUdp`: `sendto`, `recvfrom` |
| Timeouts | `Socket.cpp::fijarTimeout`: `SO_RCVTIMEO` y `SO_SNDTIMEO` |
| Serialización binaria | `Protocolo.h/.cpp`: `#pragma pack(push, 1)`, `htonl`, `htons`, CRC-16 |
| Cierre defensivo | `Socket::~Socket` y objetos `Socket` movibles con un único propietario |

La trama ocupa exactamente 30 bytes y la respuesta 7 bytes, verificado con `static_assert`. TCP trata los datos como flujo y repite `recv` hasta completar el tamaño esperado.

## 4. Resiliencia y excepciones - 1,0

| Requisito | Ubicación |
|---|---|
| Jerarquía desde `runtime_error` | `Excepciones.h` |
| Captura por referencia constante | `Demo.cpp`, `Supervisor.cpp`, `ServidorCentral.cpp` y `main.cpp` |
| `nullptr` controlado | Módulos 1, 3 y 4 de `Demo.cpp` |
| Desconexión y timeout | `Socket.cpp::recibirExacto`, `ServidorCentral.cpp::atenderCliente` |
| Servidor continúa tras fallos | `Demo.cpp::demoRed` prueba trama corrupta, valor inválido, cliente inactivo, puerto ocupado y una conexión posterior válida |

La opción 6 del menú provoca fallos de forma deliberada y presenta cada comprobación como `[OK]` o `[FALLO]`.

## 5. GitHub y sustentación - 1,0

- Repositorio público.
- Código C++11 completo: 16 `.cpp`, 21 `.h`, `TelcoCauca.zpr` y `.gitignore`.
- README con los nombres de Juan Esteban Caldon Rengifo y Kevin Camilo Salazar Cordoba.
- Invitación a `PABTORO` como colaboradora.
- Commits reales y significativos de ambos integrantes.
- Dominio individual de vtable, destructor virtual, RAII, `std::ref`, `join`, sockets, serialización y gestión de memoria.

Los dos últimos puntos se verifican en GitHub y durante la sustentación. No deben simularse autores de commits.

## Lista previa a la entrega

- [ ] Abrir `TelcoCauca.zpr` y compilar en ZinjaI.
- [ ] Ejecutar la opción 6 y confirmar cero `[FALLO]`.
- [x] Confirmar que el repositorio es público.
- [x] Confirmar que no hay `.exe`, `.o`, `.o.d` ni carpetas de compilación versionadas.
- [x] Confirmar que el `.zpr` referencia los 37 archivos fuente.
- [ ] Confirmar que `PABTORO` aparece como invitación pendiente o colaboradora.
- [ ] Confirmar commits reales de ambos integrantes en el historial.
- [ ] Ensayar la demostración con cronómetro y preparar ambos integrantes para cualquier módulo.
