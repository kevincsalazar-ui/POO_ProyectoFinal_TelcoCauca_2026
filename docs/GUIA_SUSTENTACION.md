# Guía de sustentación TelcoCauca

Duración oficial: 8 a 9 minutos por pareja. Demostración práctica: 4 minutos. Examen oral individual: 3 minutos. Verificación de GitHub: 1 a 2 minutos.

## Preparación antes del turno

1. Abrir `TelcoCauca.zpr` en ZinjaI.
2. Compilar antes de iniciar el cronómetro.
3. Dejar visible `main.cpp` y la consola lista.
4. Tener abiertas estas pestañas del repositorio: raíz, historial de commits y colaboradores.
5. Confirmar que la invitación a `PABTORO` aparece pendiente o aceptada.
6. No improvisar cambios durante la demostración.

## Guion de demostración - 4 minutos

### 0:00 a 0:30 - Presentación

> "TelcoCauca integra cinco módulos: dominio UML, herencia múltiple, polimorfismo puro, concurrencia con exclusión mutua y red distribuida TCP/UDP. Ejecutaremos la demostración completa, que además fuerza fallos y verifica la recuperación."

Ejecutar el programa y elegir la opción `6. Demostración completa`.

### 0:30 a 1:20 - POO y polimorfismo

Mientras aparecen los módulos 1 a 3, señalar:

- `DispositivoComunicacion` es abstracta porque `transmitir()` es puro.
- `RouterWiFi`, `GatewayLoRa` y `SwitchFibra` sobrescriben con `override`.
- El vector de punteros base produce ligadura dinámica mediante la vtable.
- Los objetos pertenecen a `unique_ptr`; la vista cruda no es propietaria.
- La demo inserta un `nullptr`, lanza `invalid_argument`, lo captura y continúa.

Frase clave:

> "El destructor virtual permite destruir correctamente el objeto derivado a través del puntero base."

### 1:20 a 2:10 - Hilos y exclusión mutua

En el módulo 4:

- Hay un hilo por dispositivo y otro hilo independiente para los eventos 2027.
- `std::ref(registro)` comparte el mismo objeto no copiable entre hilos.
- `Registro` protege consola, contadores y buffers con `std::lock_guard<std::mutex>`.
- `GrupoHilos` hace `join()` antes de destruir los recursos.
- La excepción del dispositivo nulo se captura dentro del hilo, evitando `std::terminate`.

Frase clave:

> "RAII garantiza que el mutex se libere al salir del ámbito, incluso si se lanza una excepción."

### 2:10 a 3:25 - TCP, UDP y resiliencia

En el módulo 5:

- El servidor ejecuta `socket -> bind -> listen -> accept` y crea un hilo por cliente TCP.
- Tres clientes concurrentes envían cuatro tramas TCP y dos UDP.
- UDP usa `sendto/recvfrom`; el timeout se configura con `SO_RCVTIMEO`.
- La trama binaria tiene 30 bytes, está empacada con `#pragma pack(1)`, usa orden de red y CRC-16.
- La demo fuerza CRC incorrecto, temperatura fuera de rango, cliente inactivo, puerto ocupado y conexión rechazada.
- Luego envía otra trama válida para demostrar que el servidor continúa vivo.

Frase clave:

> "Las excepciones se aíslan en cada hilo y el servidor conserva su ciclo de atención después de los fallos controlados."

### 3:25 a 4:00 - Cierre

Mostrar el resumen final y confirmar que todas las líneas dicen `[OK]`.

> "La demostración validó polimorfismo, mutex, hilos, TCP, UDP y recuperación ante excepciones. El repositorio incluye todas las fuentes, el proyecto ZinjaI, el `.gitignore` y la documentación de la rúbrica."

Si queda tiempo y ya fue probado en ese computador, la opción `7` abre servidor, router, gateway y sensor en cuatro consolas. No usarla por primera vez durante la evaluación.

## Respuestas orales de 20 a 30 segundos

### ¿Qué es la vtable?

Es un mecanismo habitual del compilador para implementar funciones virtuales. Cada objeto polimórfico mantiene un vínculo a una tabla de funciones del tipo real. Al llamar `dev->transmitir()` mediante un puntero base, se selecciona en tiempo de ejecución la implementación de `RouterWiFi`, `GatewayLoRa` o `SwitchFibra`.

### ¿Por qué el destructor debe ser virtual?

Porque los objetos derivados se manejan mediante punteros a `DispositivoComunicacion`. Al destruir por la base, el destructor virtual despacha primero al destructor del tipo real y después al de la base. Sin él, destruir un derivado por el puntero base produce comportamiento indefinido.

### ¿Qué aporta `std::lock_guard`?

Aplica RAII: bloquea el mutex al construirse y lo libera automáticamente al salir del bloque. Así no queda bloqueado si hay un retorno temprano o una excepción. En TelcoCauca protege los datos y la salida compartidos de `Registro`.

### ¿Por qué se usa `std::ref`?

`std::thread` copia sus argumentos por defecto. `Registro` contiene un mutex y no se puede copiar. `std::ref(registro)` pasa una referencia al mismo objeto compartido, cuya vida se mantiene hasta después de hacer `join()`.

### ¿Cuál es la secuencia del servidor TCP?

`socket()` crea el descriptor; `bind()` lo asocia con IP y puerto; `listen()` lo pone en escucha; `accept()` crea un socket nuevo para cada cliente. El socket de escucha continúa aceptando mientras cada socket aceptado es atendido por su propio hilo.

### ¿En qué se diferencian TCP y UDP aquí?

TCP es un flujo fiable y orientado a conexión, por eso se repite `recv` hasta completar 30 bytes. UDP conserva datagramas y usa `sendto/recvfrom`, pero puede perderse; por eso el cliente espera un ACK/NACK con timeout.

### ¿Qué hace `#pragma pack(1)`?

Elimina el padding entre campos para fijar el tamaño binario. No convierte el orden de bytes y no autentica el mensaje; para eso se usan `htonl/htons` y un CRC que solo detecta corrupción accidental.

### ¿Qué pasa si una excepción sale de un hilo?

Si nadie la captura dentro de la función del hilo, C++ llama a `std::terminate`. Por eso `Supervisor`, el hilo de eventos y los hilos del servidor tienen `try/catch` local y registran el error sin derribar todo el proceso.

### ¿Quién libera la memoria?

Los dispositivos polimórficos pertenecen a `vector<unique_ptr<DispositivoComunicacion>>`. La vista de punteros crudos solo permite el recorrido y no hace `delete`. Los sockets y bloqueos también usan RAII, y todos los hilos se unen antes de destruir recursos compartidos.

## Reparto recomendado

- Juan: módulos 1 a 3, vtable, destructor virtual, propiedad y `nullptr`.
- Kevin: módulos 4 y 5, RAII, `std::ref`, `join`, TCP/UDP y serialización.
- Ambos deben poder responder cualquier módulo, porque las preguntas son individuales.
