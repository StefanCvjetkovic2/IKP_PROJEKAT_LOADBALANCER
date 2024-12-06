#include "WorkerCommunication.h"
#include "ClientCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "QueueClinet.h"

// Globalna promenljiva
static QUEUE* ClientQueue = NULL;

void generateRandomNumbers(int* numbers, int size, int min, int max) {
    for (int i = 0; i < size; i++) {
        numbers[i] = rand() % (max - min + 1) + min;
    }
}

DWORD WINAPI startServer(LPVOID param) {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    int brojac = 0;

    ClientQueue = init_queue(10);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5059);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) < 0) {
        fprintf(stderr, "Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Load Balancer is listening on port 5059...\n");

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept client connection\n");
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Client connected\n");

    while (true) {
        int number, min, max;

        if (recv(clientSocket, (char*)&number, sizeof(number), 0) <= 0 ||
            recv(clientSocket, (char*)&min, sizeof(min), 0) <= 0 ||
            recv(clientSocket, (char*)&max, sizeof(max), 0) <= 0) {
            fprintf(stderr, "Failed to receive data from client\n");
            closesocket(clientSocket);
            WSACleanup();
            exit(EXIT_FAILURE);
        }

        QUEUEELEMENT2* clientData = (QUEUEELEMENT2*)malloc(sizeof(QUEUEELEMENT2));
        if (!clientData) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }

        snprintf(clientData->clientName, sizeof(clientData->clientName), "Client %d", brojac++);
        clientData->numbers = (int*)malloc(number * sizeof(int));
        if (!clientData->numbers) {
            fprintf(stderr, "Memory allocation failed\n");
            free(clientData);
            exit(EXIT_FAILURE);
        }

        generateRandomNumbers(clientData->numbers, number, min, max);

        printf("Client %d generated numbers: ", number);
        for (int i = 0; i < number; i++) {
            printf("%d ", clientData->numbers[i]);
        }
        printf("\n");

        enqueue(ClientQueue, create_queue_element(clientData->clientName, clientData->numbers, number));
        print_queue(ClientQueue);

        free(clientData->numbers);
        free(clientData);
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}

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

