#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startAdmin(LPVOID param);
void initializeWinsock();
SOCKET createServerSocket(int port);
void bindAndListen(SOCKET serverSocket, int backlog);
SOCKET acceptWorkerConnection(SOCKET serverSocket);
void handleWorkerCommunication(SOCKET workerSocket);
