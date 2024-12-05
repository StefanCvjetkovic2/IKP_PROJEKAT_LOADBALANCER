#include "ClientCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Globalna promenljiva
QUEUE* ClientQueue = NULL;

void generateRandomNumbers(int* numbers, int size, int min, int max) {
    for (int i = 0; i < size; i++) {
        numbers[i] = rand() % (max - min + 1) + min;
    }
}

void startServer() {
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
