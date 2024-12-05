#include "LBcommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicijalizacija Winsock-a
void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }
}

// Kreiranje soketa
SOCKET createWorkerSocket() {
    SOCKET workerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (workerSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create Worker socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return workerSocket;
}

// Povezivanje sa Load Balancer-om
void connectToLoadBalancer(SOCKET workerSocket, const char* lbAddress, int lbPort) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(lbPort);
    inet_pton(AF_INET, lbAddress, &serverAddress.sin_addr);

    if (connect(workerSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Failed to connect to Load Balancer\n");
        closesocket(workerSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected to Load Balancer\n");
}

// Rukovanje komunikacijom sa Load Balancer-om
void handleCommunication(SOCKET workerSocket) {
    char buffer[1024] = { 0 };

    // Primanje poruke od Load Balancera
    if (recv(workerSocket, buffer, sizeof(buffer) - 1, 0) > 0) {
        printf("Message received from Load Balancer: %s\n", buffer);
    }
    else {
        fprintf(stderr, "Failed to receive message from Load Balancer\n");
    }
}

// Glavna funkcija koja poziva manje funkcije
DWORD WINAPI startWorker(LPVOID param) {
    initializeWinsock();

    SOCKET workerSocket = createWorkerSocket();
    connectToLoadBalancer(workerSocket, "127.0.0.1", 5060); // Lokalna IP adresa i port Load Balancera

    handleCommunication(workerSocket);
    while (true) {

    }
    // Zatvaranje soketa
    closesocket(workerSocket);
    WSACleanup();
}
