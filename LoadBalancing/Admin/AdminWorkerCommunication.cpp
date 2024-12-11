#include "AdminWorkerCommunication.h"
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <winsock2.h>

void handleWorkerCommunication(SOCKET workerSocket) {
    char buffer[4096];
    while (true) {
        printf("Enter an integer number to send to Worker (q to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        // Remove newline character at the end if present
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Check if the input is 'q' or a valid integer
        bool isValid = true;
        if (strcmp(buffer, "q") == 0) {
            printf("Closing connection with Worker\n");
            break;
        }
        else {
            for (size_t i = 0; i < len - 1; i++) {
                if (!isdigit(buffer[i])) {
                    isValid = false;
                    break;
                }
            }
        }

        if (isValid) {
            if (send(workerSocket, buffer, len, 0) < 0) {
                fprintf(stderr, "Failed to send message to Worker\n");
            }
            else {
                printf("Message sent to Worker\n");
            }
        }
        else {
            printf("Wrong input, please send an integer or 'q' to quit.\n");
        }
    }
}


// Glavna funkcija za Load Balancer
DWORD WINAPI startAdmin(LPVOID param) {
    initializeWinsock();

    SOCKET serverSocket = createServerSocket(5062); // Port Load Balancera
    bindAndListen(serverSocket, 5);
    SOCKET workerSocket = acceptWorkerConnection(serverSocket);

    handleWorkerCommunication(workerSocket);

    // Zatvaranje serverskog soketa
    closesocket(workerSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
