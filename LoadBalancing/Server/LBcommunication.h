#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startWorker(LPVOID param);
void initializeWinsock();
SOCKET createWorkerSocket();
void connectToLoadBalancer(SOCKET workerSocket, const char* lbAddress, int lbPort);
void handleCommunication(SOCKET workerSocket);