# TelcoCauca

Proyecto integrador de Programación Orientada a Objetos. Modela infraestructura de telecomunicaciones, supervisa dispositivos en paralelo y transmite estado TCP y telemetría UDP.

## Compilación en Windows

1. Usa ZinjaI con la actualización mingw64-gcc7 indicada en Classroom.
2. Abre TelcoCauca.zpr y pulsa Compilar y ejecutar. El proyecto usa C++11, -pthread y la biblioteca Winsock.
3. Elige 5 para ejecutar la demostración completa.

La consigna escribe .zpj. Esta instalación de ZinjaI utiliza .zpr como extensión de proyecto.

Compilación equivalente desde PowerShell con MinGW en PATH:

```powershell
g++ -std=c++11 -Wall -Wextra -pedantic -pthread -static main.cpp Torre.cpp NodoRedInteligente.cpp Trama.cpp Registro.cpp Eventos.cpp Red.cpp -o TelcoCauca.exe -lws2_32
.\TelcoCauca.exe --pruebas
```

## Prueba con procesos independientes

Primera consola:

```powershell
.\TelcoCauca.exe --servidor 127.0.0.1 5050
```

Cuando diga «Servidor listo», ejecuta en otra consola:

```powershell
.\TelcoCauca.exe --router 127.0.0.1 5050
.\TelcoCauca.exe --gateway 127.0.0.1 5050
.\TelcoCauca.exe --sensor 127.0.0.1 5050
```

El servidor de demostración atiende dos conexiones TCP y un datagrama UDP. Para usar dos equipos, inicia el servidor con su IPv4 local y usa esa dirección en los clientes. El puerto debe estar disponible para TCP y UDP.

El formato TC1 conserva versión, tipo, identificador, IP y valor. TCP delimita las tramas con un salto de línea. UDP utiliza una trama por datagrama. El servidor valida y reconstruye los datos.

## Rúbrica y pruebas

| Criterio (1 punto cada uno) | Archivos | Prueba |
| --- | --- | --- |
| Arquitectura POO y polimorfismo | Tecnico.h, Antena.h, Torre.h/.cpp, Dispositivos.h e ITransmisible.h | Opción 1 muestra torre, antenas y responsables. Opción 3 ejecuta las tres subclases a través de punteros base. |
| Multihilo y exclusión mutua | GrupoHilos.h, Supervisor en main.cpp, Registro.cpp y Eventos.cpp | Opción 3 ejecuta funciones miembro con std::ref, protege datos compartidos con mutex y lock_guard y espera con join. La agenda muestra cinco eventos en un hilo dedicado. |
| Sockets TCP y UDP | ServidorCentral en Red.cpp y Trama.cpp | Opción 4 transmite dos estados TCP y una lectura UDP mediante sockets reales. Cada cliente recibe confirmación. |
| Resiliencia y excepciones | NodoRedInteligente.cpp, Registro.cpp y Red.cpp | Opción 2 prueba herencia múltiple, suma de cargas, impresión, lectura e IP inválidas. Opción 3 captura el puntero nulo. Opción 4 controla trama inválida, desconexión y conexión rechazada. |
| GitHub y sustentación | Fuentes, proyecto ZinjaI, .gitignore y guion | Comprobar repositorio público, invitación a PABTORO y preparación individual. |

Los cinco eventos se anunciaron para 2027. El día exacto de Cambiemos el Chip Fest y la sede de C3 figuran por confirmar.

- Andina Link: https://cartagena.andinalink.com/registro-andina-2027/
- IEEE PES GT&D: https://gtdla27.ieee.org.co/
- IEEE ACSOS: https://conf.researchr.org/home/acsos-2027
- Cambiemos el Chip Fest: https://r9.ieee.org/event/cambiemos-el-chipfest-medellin/
- IEEE C3: https://colcaribe.ieee.org/eventos/mes/2027-09/
