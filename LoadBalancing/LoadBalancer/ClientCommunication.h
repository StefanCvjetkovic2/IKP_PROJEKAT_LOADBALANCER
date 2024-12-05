#include <winsock2.h>
#include <ws2tcpip.h>
#include "QueueClinet.h"


#pragma comment(lib, "Ws2_32.lib")

typedef struct {
    char clientName[50];
    int* numbers;
} QUEUEELEMENT2;

// Deklaracija funkcija
void generateRandomNumbers(int* numbers, int size, int min, int max);
DWORD WINAPI startServer(LPVOID param);