# TelcoCauca

Sistema distribuido concurrente de monitoreo de infraestructura de telecomunicaciones e IoT, desarrollado en **C++11** para Programación Orientada a Objetos. Incluye demostraciones de modelado del dominio, herencia múltiple, polimorfismo, hilos y comunicación TCP/UDP.

**Universidad del Cauca - FIET - 2026.2**<br>
**Docente:** Paola Andrea Bedoya Toro (`PABTORO`)<br>
**Integrantes:** Juan Esteban Caldon Rengifo y Kevin Camilo Salazar Cordoba

## Requisitos

- Compilador de C++11 con soporte para `std::thread` y `std::mutex`.
- En Windows: MinGW-w64 y la biblioteca Winsock (`ws2_32`).
- ZinjaI para abrir el proyecto `.zpr`, o una terminal con `g++` disponible.
- Para las demostraciones de red, permitir la comunicación local con `127.0.0.1`.

El proyecto de ZinjaI configura C++11, `-pthread` y el enlace con `-pthread -static -lws2_32`. Su toolchain está identificado como `mingw64-gcc7`; si no está instalado, seleccione en ZinjaI un compilador compatible disponible en su equipo.

## Compilación y ejecución

### Con ZinjaI

1. Descargar o clonar el repositorio completo y conservar juntos los archivos `.h`, `.cpp` y `.zpr`.
2. Abrir `TelcoCauca.zpr`.
3. Seleccionar la configuración **C++11** y compilar el proyecto completo.
4. Ejecutar el programa y elegir una opción del menú.

### Con g++ en Windows

Desde PowerShell, ubicado en la carpeta del repositorio y con MinGW-w64 en el `PATH`:

```powershell
g++ -std=c++11 -Wall -Wextra -pthread *.cpp -o TelcoCauca.exe -lws2_32
.\TelcoCauca.exe
```

### Con g++ en Linux

El código contiene una capa de sockets para Windows y POSIX. En Linux, el comando equivalente es:

```bash
g++ -std=c++11 -Wall -Wextra -pthread *.cpp -o TelcoCauca
./TelcoCauca
```

## Opciones del menú

| Opción | Demostración | Conceptos principales |
| --- | --- | --- |
| 1 | Infraestructura física | Asociación Torre–Antena 1:N y Antena–Tecnico 1:1; validación de datos. |
| 2 | Herencia múltiple y excepciones | `NodoRedInteligente` como `DispositivoRed` y `SensorIoT`; operadores y excepciones. |
| 3 | Polimorfismo e interfaces | Router WiFi, gateway LoRa y switch de fibra; despacho dinámico e interfaz `ITransmisible`. |
| 4 | Concurrencia y eventos 2027 | Hilos de supervisión y eventos; registro compartido y contador protegidos con mutex. |
| 5 | Comunicación TCP/UDP | Clientes concurrentes, serialización binaria, ACK/NACK e inyección de fallos. |
| 6 | Demostración completa | Ejecuta los cinco módulos y muestra un resumen. |
| 7 | Procesos independientes | En Windows, abre consolas para servidor, router, gateway y sensor. |
| 0 | Salir | Cierra el menú. |

La opción 5 inicia su propio servidor con puertos libres asignados por el sistema; no requiere iniciar previamente el modo `--servidor`.

## Servidor y clientes en procesos independientes

Todos los ejemplos siguientes se ejecutan en el **mismo equipo**. El modo servidor escucha en `127.0.0.1`, con el puerto **5000** como valor predeterminado para TCP y UDP. Ambos protocolos pueden usar el mismo número de puerto.

1. En una terminal, iniciar el servidor y esperar el mensaje `Servidor listo`:

   ```powershell
   .\TelcoCauca.exe --servidor 5000
   ```

2. En otras terminales, ejecutar los clientes:

   ```powershell
   .\TelcoCauca.exe --router 127.0.0.1 5000 --sinpausa
   .\TelcoCauca.exe --gateway 127.0.0.1 5000 --sinpausa
   .\TelcoCauca.exe --sensor 127.0.0.1 5000 --sinpausa
   ```

3. Comprobar el resumen de cada cliente: en una ejecución correcta indica `enviadas=3 ACK=3 NACK=0`. Router y gateway usan TCP; el sensor usa UDP.
4. Pulsar **ENTER en la consola del servidor** para detenerlo ordenadamente.

Para detener el servidor automáticamente después de 30 segundos:

```powershell
.\TelcoCauca.exe --servidor 5000 30 --sinpausa
```

`--sinpausa` evita la espera final de ENTER en los clientes y en el servidor con duración positiva; no sustituye la orden de detener un servidor sin duración. En Linux, usar `./TelcoCauca` en lugar de `.\TelcoCauca.exe` y abrir las terminales manualmente.

## Verificación de los cinco módulos

```powershell
.\TelcoCauca.exe --pruebas
$LASTEXITCODE
```

Una ejecución correcta termina con `RESULTADO GLOBAL: TODOS LOS MODULOS CORRECTOS` y código de salida **0**. El modo `--pruebas` devuelve **1** si alguna verificación falla.

Las pruebas provocan intencionalmente errores: punteros nulos, temperaturas fuera de rango, CRC corrupto, desconexiones a mitad de trama, inactividad y puertos ocupados. Ver mensajes `ERROR`, `NACK` o `Excepcion esperada` durante estos casos es normal: revise las líneas `[OK]`/`[FALLO]` y el resultado global. El orden de los mensajes de los hilos puede variar entre ejecuciones.

## Organización del código

| Archivos | Responsabilidad |
| --- | --- |
| `main.cpp` | Menú y argumentos de línea de comandos. |
| `Demo.h`, `Demo.cpp` | Demostraciones y verificaciones de los cinco módulos. |
| `Torre`, `Antena`, `Tecnico` (`.h`/`.cpp`) | Infraestructura física y asociaciones del dominio. |
| `DispositivoRed`, `SensorIoT`, `NodoRedInteligente` (`.h`/`.cpp`) | Herencia múltiple y validación del nodo. |
| `DispositivoComunicacion.h`, `ITransmisible.h`, `Dispositivos.h`, `Dispositivos.cpp` | Interfaces y dispositivos polimórficos. |
| `Supervisor`, `Registro`, `Eventos` (`.h`/`.cpp`), `GrupoHilos.h` | Supervisión concurrente, registro sincronizado y gestión de hilos. |
| `Socket`, `ServidorCentral`, `Clientes` (`.h`/`.cpp`), `PlataformaRed.h` | Sockets, servidor, clientes y adaptación Windows/POSIX. |
| `Protocolo.h`, `Protocolo.cpp` | Tramas binarias de 30 bytes, respuestas de 7 bytes y CRC-16. |
| `Excepciones.h`, `Util.h` | Excepciones del sistema y utilidades. |
| `TelcoCauca.zpr`, `.gitignore` | Configuración de ZinjaI y exclusión de archivos generados. |

## Solución de problemas

| Situación | Qué revisar |
| --- | --- |
| `g++` no se reconoce | Verificar la instalación del compilador y su carpeta `bin` en el `PATH`, o compilar desde ZinjaI. |
| Errores de símbolos Winsock al enlazar | En Windows, incluir `-lws2_32` al final del comando. |
| El compilador no reconoce `std::thread` | Usar un toolchain con soporte de hilos, C++11 y la opción `-pthread`. |
| `bind()` falla o el puerto está ocupado | Cerrar el servidor anterior o elegir otro puerto, por ejemplo `--servidor 5001`, y usarlo también en los clientes. |
| El cliente no conecta o agota el tiempo de espera | Iniciar primero el servidor, comprobar `Servidor listo` y verificar IP y puerto. |
| La opción 7 no abre consolas en Linux | Ejecutar manualmente los modos de servidor y clientes en terminales separadas. |

Los ejecutables, objetos y directorios de compilación están excluidos mediante `.gitignore`; cada integrante debe compilar el proyecto en su equipo.
