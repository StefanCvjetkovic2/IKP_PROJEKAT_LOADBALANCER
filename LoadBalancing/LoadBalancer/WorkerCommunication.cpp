#include "WorkerCommunication.h"

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

// Kreiranje serverskog soketa
SOCKET createServerSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create server socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

// Podesavanje soketa za slusanje konekcija
void bindAndListen(SOCKET serverSocket, int backlog) {
    if (listen(serverSocket, backlog) < 0) {
        fprintf(stderr, "Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Load Balancer is listening for Worker connections...\n");
}

// Prihvatanje konekcije od Workera
SOCKET acceptWorkerConnection(SOCKET serverSocket) {
    struct sockaddr_in workerAddress;
    int workerAddressSize = sizeof(workerAddress);
    SOCKET workerSocket = accept(serverSocket, (struct sockaddr*)&workerAddress, &workerAddressSize);

    if (workerSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept Worker connection\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Worker connected\n");
    return workerSocket;
}

// Rukovanje komunikacijom sa Workerom
void handleWorkerCommunication(SOCKET workerSocket) {
    const char* message = "Welcome, Worker!";
    if (send(workerSocket, message, strlen(message), 0) < 0) {
        fprintf(stderr, "Failed to send message to Worker\n");
    }
    else {
        printf("Message sent to Worker: %s\n", message);
    }

    // Zatvaranje konekcije sa Workerom
    closesocket(workerSocket);
}

// Glavna funkcija za Load Balancer
DWORD WINAPI startLoadBalancer(LPVOID param) {
    initializeWinsock();

    SOCKET serverSocket = createServerSocket(5060); // Port Load Balancera
    bindAndListen(serverSocket, 5);

    while (true) {
        SOCKET workerSocket = acceptWorkerConnection(serverSocket);
        handleWorkerCommunication(workerSocket);
    }

    // Zatvaranje serverskog soketa
    closesocket(serverSocket);
    WSACleanup();
}
