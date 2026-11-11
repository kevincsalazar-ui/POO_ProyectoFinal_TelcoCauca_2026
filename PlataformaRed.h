#ifndef PLATAFORMARED_H
#define PLATAFORMARED_H
// Cabeceras y tipos de plataforma; Socket.cpp y main.cpp tambien contienen ramas especificas.
// En Windows (ZinjaI/MinGW) usa Winsock2 y enlaza ws2_32.
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET SocketHandle;
typedef int TamanoDireccion;
#else
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
typedef int SocketHandle;
typedef socklen_t TamanoDireccion;
#endif
#endif
