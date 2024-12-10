#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

// Deklaracija funkcija
DWORD WINAPI startWorkerToAdmin(LPVOID param);
void initializeWinsock3();
SOCKET createWorkerSocket3();
void connectToAdmin(SOCKET workerSocket, const char* lbAddress, int lbPort);
void handleCommunicationToAdmin(SOCKET workerSocket);


