#include "AdminCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "LBcommunication.h"
using namespace std;



// Inicijalizacija Winsock-a
void initializeWinsock3() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }
}

// Kreiranje soketa
SOCKET createWorkerSocket3() {
    SOCKET workerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (workerSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create Worker socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return workerSocket;
}

// Povezivanje sa Load Balancer-om
void connectToAdmin(SOCKET workerSocket, const char* adminAddress, int adminPort) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(adminPort);
    inet_pton(AF_INET, adminAddress, &serverAddress.sin_addr);

    if (connect(workerSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Failed to connect to Admin\n");
        closesocket(workerSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected to Admin\n");
}

// Rukovanje komunikacijom sa Load Balancer-om
void handleCommunicationToAdmin(SOCKET adminSocket, SOCKET loadBalancerSocket) {
    char buffer[4096] = { 0 };

    while (true) {
        // Receive the message from the Admin
        int receivedBytes = recv(adminSocket, buffer, sizeof(buffer), 0);
        if (receivedBytes > 0) {
            buffer[receivedBytes] = '\0'; // Null-terminate the received string
            printf("Message received from Admin: %s\n", buffer);
            if (strcmp(buffer, "q") == 0) {
                // Send "Operations have been stopped by Admin" message to Load Balancer
                const char* stopMessage = "Operations have been stopped by Admin";
                send(loadBalancerSocket, stopMessage, strlen(stopMessage), 0);
                printf("Sent stop message to Load Balancer.\n");

                printf("Closing connection as requested by Admin.\n");
                break;
            }
        }
        else {
            fprintf(stderr, "Failed to receive message from Admin\n");
            break;
        }
    }

    // Close the sockets
    closesocket(adminSocket);
    closesocket(loadBalancerSocket);
    WSACleanup();
}

// Glavna funkcija koja poziva manje funkcije
DWORD WINAPI startWorkerToAdmin(LPVOID param) {
    initializeWinsock3();

    SOCKET workerSocket2 = createWorkerSocket3();
    connectToAdmin(workerSocket2, "127.0.0.1", 5062); // Local IP address and Load Balancer port


    handleCommunicationToAdmin(workerSocket2, workerSocket);

    closesocket(workerSocket2);
    closesocket(workerSocket);
    WSACleanup();

    return 0;
}
