#include <winsock2.h>
#include <ws2tcpip.h>
#include "QueueClinet.h"
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startLoadBalancer(LPVOID param);
void initializeWinsock();
SOCKET createServerSocket(int port);
void bindAndListen(SOCKET serverSocket, int backlog);
SOCKET acceptWorkerConnection(SOCKET serverSocket);
void handleWorkerCommunication(SOCKET workerSocket);
void handleWorkerCommunicationReceive(SOCKET workerSocket);
void serializeQueueElement(QUEUEELEMENT* q, char** buffer, int* bufferSize);