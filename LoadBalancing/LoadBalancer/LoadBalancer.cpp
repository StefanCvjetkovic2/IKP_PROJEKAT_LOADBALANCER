#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <time.h>
#include "QueueClinet.h"
//#include "queue.h"
#pragma comment(lib, "Ws2_32.lib")

QUEUE* ClientQueue = NULL;

typedef struct {
    char clientName[50];
    int* numbers;
    
} QUEUEELEMENT2;

void generateRandomNumbers(int* numbers, int size, int min, int max) {
    for (int i = 0; i < size; i++) {
        numbers[i] = rand() % (max - min + 1) + min;
    }
}

int main() {
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    ClientQueue = init_queue(10);

    // Inicijalizacija Winsock-a
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        return EXIT_FAILURE;
    }

    // Kreiranje soketa
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Konfigurisanje server adrese
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5059);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Bindovanje soketa
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Bind failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Slušanje konekcija
    if (listen(serverSocket, 5) < 0) {
        fprintf(stderr, "Listen failed\n");
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Load Balancer is listening on port 5059...\n");

    // Prihvatanje klijentske konekcije
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to accept client connection\n");
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Client connected\n");
    int brojac = 0;
    while (true) {
        // Prima broj od klijenta
        int number;
        if (recv(clientSocket, (char*)&number, sizeof(number), 0) > 0) {
            printf("Received number from client: %d\n", number);
        }
        else {
            fprintf(stderr, "Failed to receive number\n");
            closesocket(clientSocket);
            WSACleanup();
            return EXIT_FAILURE;
        }

        // Prima opseg brojeva (min i max)
        int min, max;
        if (recv(clientSocket, (char*)&min, sizeof(min), 0) > 0 &&
            recv(clientSocket, (char*)&max, sizeof(max), 0) > 0) {
            printf("Received range from client: %d - %d\n", min, max);
        }
        else {
            fprintf(stderr, "Failed to receive range\n");
            closesocket(clientSocket);
            WSACleanup();
            return EXIT_FAILURE;
        }
       
        // Kreira i popunjava dinamički bafer
        //QUEUEELEMENT2 *clientData;
        
        QUEUEELEMENT2* clientData = (QUEUEELEMENT2*)malloc(sizeof(QUEUEELEMENT2));

        if (clientData == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            return EXIT_FAILURE;
        }

        snprintf(clientData->clientName, sizeof(clientData->clientName), "Client %d", brojac++); // Ime klijenta
        
        clientData->numbers = (int*)malloc(number * sizeof(int)); // Dinamički alociran bafer


        if (clientData->numbers == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            closesocket(clientSocket);
            WSACleanup();
            return EXIT_FAILURE;
        }
       

        // Popunjava bafer sa slučajnim brojevima
        generateRandomNumbers(clientData->numbers, number, min, max);
        
        
       
        // Ispisivanje brojeva u baferu
        printf("Client %d generated numbers: ", number);
        for (int i = 0; i < number; i++) {
            printf("%d ", clientData->numbers[i]);
        }
        printf("\n");
        printf("%d", number);

        enqueue(ClientQueue, create_queue_element(clientData->clientName,clientData->numbers,number ));

        print_queue(ClientQueue);
       

        // Oslobađanje memorije
        free(clientData->numbers);

        

    }

    // Zatvaranje soketa
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return EXIT_SUCCESS;
}
