# Sustentación de TelcoCauca

## Demostración de cuatro minutos

- 0:00 a 0:40. Abre el proyecto compilado en ZinjaI. Usa la opción 1 para mostrar la torre y sus dos antenas. Cada antena apunta a su técnico y la torre guarda punteros a las antenas.
- 0:40 a 1:20. Usa la opción 2. Explica qué aporta cada clase base del nodo. Muestra la suma de cargas y las excepciones de lectura e IP.
- 1:20 a 2:15. Usa la opción 3. Identifica las tres implementaciones de transmitir, la excepción del puntero nulo y el hilo que imprime la agenda.
- 2:15 a 4:00. Usa la opción 4 para probar TCP y UDP con sockets reales. Señala las dos confirmaciones TCP y la UDP, la desconexión y el rechazo de conexión controlados. Si ya tienes las consolas preparadas, usa los procesos separados descritos en el README.

## Preguntas individuales

**¿Cómo se elige transmitir?** La llamada se hace a través de un puntero a DispositivoComunicacion. La función virtual se resuelve según el tipo concreto, normalmente mediante una tabla de funciones virtuales. override permite al compilador verificar la firma.

**¿Para qué sirve el destructor virtual?** Al destruir una instancia derivada desde un puntero base se ejecutan los destructores correspondientes. Los unique_ptr son propietarios y el vector polimórfico toma prestados los punteros.

**¿Qué protege el mutex?** La consola, los contadores, la agenda y el vector de tramas. lock_guard adquiere el mutex al crearse y lo suelta al salir del bloque, incluso al lanzar una excepción. Las esperas de red ocurren fuera del bloqueo.

**¿Por qué std::ref y join?** std::ref permite compartir el mismo Registro. join espera el fin de cada hilo antes de leer resultados o destruir objetos. GrupoHilos también espera los hilos ya creados si falla la creación de otro.

**¿Cómo se establece una conexión TCP?** El servidor llama socket, bind, listen y accept. El cliente llama socket y connect. accept devuelve un socket distinto para cada cliente, que se transfiere a un hilo.

**¿Qué diferencia hay entre TCP y UDP?** TCP entrega un flujo ordenado. recv puede devolver solo parte de una trama. UDP entrega datagramas individuales y puede perderlos. La demo confirma la recepción y aplica tiempos límite.

**¿Qué se serializa?** La trama TC1 conserva versión, tipo, identificador, IP y valor. El servidor valida el formato y reconstruye una Trama. Una entrada inválida se registra como excepción y el hilo libera su socket.

## Entrega

Classroom fija el 11 de noviembre de 2026 a la 1:00 p. m. para la entrega y el 17 de noviembre para la sustentación en el laboratorio 333. Cada pareja tiene 8 a 9 minutos. El PDF indica 45 % para el proyecto final. Classroom menciona 25 % para la nota de sustentación. La relación entre esos porcentajes queda por confirmar.

Para publicar, crea o reutiliza un repositorio público, sube fuentes y proyecto y verifica que no se incluyan binarios. En Settings → Collaborators → Add people se invita al usuario exacto PABTORO. Comprueba el estado de la invitación. Adjunta la URL del repositorio a la tarea de Classroom antes del envío.
