#include "WorkerCommunication.h"
#include "ClientCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QueueClinet.h"



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
    if (ClientQueue->currentSize > 0) {
        QUEUEELEMENT* q = dequeue(ClientQueue);

        char* buffer;
        int bufferSize;
        serializeQueueElement(q, &buffer, &bufferSize);

        if (send(workerSocket, buffer, bufferSize, 0) < 0) {
            fprintf(stderr, "Failed to send message to Worker\n");
        }
        else {
            printf("Message sent to Worker\n");
        }

        free(buffer);
        // Close the connection with the Worker
        //closesocket(workerSocket);
    }
}


// Glavna funkcija za Load Balancer
DWORD WINAPI startLoadBalancer(LPVOID param) {
    initializeWinsock();

    SOCKET serverSocket = createServerSocket(5060); // Port Load Balancera
    bindAndListen(serverSocket, 5);
    SOCKET workerSocket = acceptWorkerConnection(serverSocket);
    while (true) {
        
        handleWorkerCommunication(workerSocket);
        
    }

    // Zatvaranje serverskog soketa
    closesocket(serverSocket);
    WSACleanup();
}

void serializeQueueElement(QUEUEELEMENT* q, char** buffer, int* bufferSize) {
    // Calculate the size needed for the buffer
    int clientNameLen = strlen(q->clientName) + 1; // +1 for the null terminator
    int dataSize = q->dataSize * sizeof(int);
    *bufferSize = sizeof(int) + clientNameLen + sizeof(int) + dataSize + sizeof(int);

    // Allocate the buffer
    *buffer = (char*)malloc(*bufferSize);
    char* ptr = *buffer;

    // Serialize the clientName length and clientName
    int nameLen = clientNameLen;
    memcpy(ptr, &nameLen, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, q->clientName, clientNameLen);
    ptr += clientNameLen;

    // Serialize the data size and data array
    int dataLen = q->dataSize;
    memcpy(ptr, &dataLen, sizeof(int));
    ptr += sizeof(int);
    memcpy(ptr, q->data, dataSize);
    ptr += dataSize;
}

