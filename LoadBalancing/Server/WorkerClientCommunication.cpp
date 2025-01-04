#include "WorkerClientCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../Common/QueueResults.h"



// Inicijalizacija Winsock-a
void initializeWinsock2() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }
}

// Kreiranje serverskog soketa
SOCKET createServerSocket2(int port) {
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
void bindAndListen2(SOCKET serverSocket, int backlog) {
    if (listen(serverSocket, backlog) < 0) {
        fprintf(stderr, "Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    printf("Server is listening for Client connections...\n");
}

// Prihvatanje konekcije od Klijenta
SOCKET acceptClientConnection(SOCKET serverSocket) {
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);

    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept Client connection\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");
    return clientSocket;
}

// Rukovanje komunikacijom sa clientom
void handleClientCommunication(SOCKET clientSocket) {
    if (ResultsQueue->currentSize > 0) {
        QUEUEELEMENTRESULT* q = dequeue2(ResultsQueue);

        char* buffer;
        int bufferSize;
        serializeQueueElementResult(q, &buffer, &bufferSize);

        if (send(clientSocket, buffer, bufferSize, 0) < 0) {
            fprintf(stderr, "Failed to send message to Client\n");
        }
        else {
            printf("Message sent to Client\n");
        }

        free(buffer);
        // Close the connection with the Worker
        //closesocket(workerSocket);
    }
}


// Glavna funkcija za Load Balancer
DWORD WINAPI startWorkerToClient(LPVOID param) {
    initializeWinsock2();

    SOCKET serverSocket = createServerSocket2(5061); // Port za komunikaciju sa clientom
    bindAndListen2(serverSocket, 5);
    SOCKET clientSocket = acceptClientConnection(serverSocket);
    while (true) {

        handleClientCommunication(clientSocket);

    }

    // Zatvaranje serverskog soketa
    closesocket(serverSocket);
    WSACleanup();
}

void serializeQueueElementResult(QUEUEELEMENTRESULT* q, char** buffer, int* bufferSize) {
    int nameLen = strlen(q->clientName) + 1;  // Include the null terminator

    // Calculate the total size needed for the buffer
    *bufferSize = sizeof(int) + nameLen + sizeof(int);  // Size of name length, client name, and result

    // Allocate memory for the buffer
    *buffer = (char*)malloc(*bufferSize);
    if (*buffer == NULL) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        return;
    }

    char* ptr = *buffer;

    // Serialize the client name length
    memcpy(ptr, &nameLen, sizeof(int));
    ptr += sizeof(int);

    // Serialize the client name
    memcpy(ptr, q->clientName, nameLen);
    ptr += nameLen;

    // Serialize the result
    memcpy(ptr, &(q->result), sizeof(int));
}


