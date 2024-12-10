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

// Handle communication with Worker
void handleWorkerCommunication(SOCKET workerSocket) {
    // Send message to Worker if there's data in the queue
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
    }

    char buffer2[4096] = { 0 };
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(workerSocket, &readfds);
    struct timeval timeout = { 0, 100000 };  // Timeout na 100 ms
    int activity = select(0, &readfds, NULL, NULL, &timeout);
    if (activity > 0 && FD_ISSET(workerSocket, &readfds)) {
        int receivedBytes = recv(workerSocket, buffer2, sizeof(buffer2) - 1, 0);
        if (receivedBytes > 0) {
            buffer2[receivedBytes] = '\0'; // Null-terminate the received string
            printf("Message received from Worker: %s\n", buffer2);

            // Check if the message indicates stopping operations
            if (strcmp(buffer2, "Operations have been stopped by Admin") == 0) {
                printf("Received stop message from Worker. Closing connection.\n");

                // Close the worker socket and clean up
                closesocket(workerSocket);
                //WSACleanup(); //Ako nije komentarisano onda nema komunikacije sa clientom, ako je komentarisano i dalje postoji komunikacija sa clientom
                //exit(EXIT_SUCCESS); // Exit the program or handle accordingly
            }
        }
        else {
            // Error occurred in recv
            fprintf(stderr, "Failed to receive message from Worker\n");
        }
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
        //handleWorkerCommunicationReceive(workerSocket);
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

