#include <winsock2.h>
#include <ws2tcpip.h>
#include "QueueResult.h"
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startWorkerToClient(LPVOID param);
void initializeWinsock2();
SOCKET createServerSocket2(int port);
void bindAndListen2(SOCKET serverSocket, int backlog);
SOCKET acceptClientConnection(SOCKET serverSocket);
void handleClientCommunication(SOCKET clientSocket);
void serializeQueueElementResult(QUEUEELEMENTRESULT* q, char** buffer, int* bufferSize);