#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../Common/Queue.h"
#pragma comment(lib, "Ws2_32.lib")

extern SOCKET workerSocket;
extern int newQueueSize;
extern bool queueSizeChanged;

// Deklaracija funkcija
DWORD WINAPI startWorker(LPVOID param);
void initializeWinsock();
SOCKET createWorkerSocket();
void connectToLoadBalancer(SOCKET workerSocket, const char* lbAddress, int lbPort);
void handleCommunication(SOCKET workerSocket);
DWORD WINAPI addToQueue(LPVOID lpParam);
DWORD WINAPI loadBalancerThread1(LPVOID lpParam);
DWORD WINAPI loadBalancerThread2(LPVOID lpParam);
DWORD WINAPI workerRole(LPVOID lpParam);
void deserializeQueueElement(char* buffer, QUEUEELEMENT* q);

