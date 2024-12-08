#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ClientQueue.h"
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startClient(LPVOID param);
void initializeWinsock();
SOCKET createClientSocket();
void connectToServer(SOCKET workerSocket, const char* lbAddress, int lbPort);
void handleCommunication(SOCKET workerSocket);
void deserializeQueueElementResult(char* buffer, QUEUEELEMENTRESULT* q);

