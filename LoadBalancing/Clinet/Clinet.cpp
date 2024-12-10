#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ClientWorkerCommunication.h"

#pragma comment(lib, "Ws2_32.lib")



int main() {

    HANDLE hClientServer;
    DWORD hClientServerID;
    hClientServer = CreateThread(NULL, 0, &startClient, (LPVOID)0, 0, &hClientServerID);

    WSADATA wsaData;
    SOCKET clientSocket;
    struct sockaddr_in serverAddress;

    // Inicijalizacija Winsock-a
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        return EXIT_FAILURE;
    }

    // Kreiranje soketa
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\n");
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Konfigurisanje server adrese
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5059);
    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    // Povezivanje sa serverom
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }

    printf("Connected to Load Balancer\n");

    while (true) {
        // Slanje broja serveru

        char input[10];
        printf(" >> Enter a number to send to Load Balancer or 'r' for results: <<  ");
        scanf_s("%s", input, (unsigned)_countof(input)); // Koristimo string za unos

        if (strcmp(input, "r") == 0 || strcmp(input, "R") == 0) {

            print_queue2(queue);
        }
        else {
            // Pretpostavlja se da je unet broj
            int number = atoi(input);


            if (send(clientSocket, (char*)&number, sizeof(number), 0) == SOCKET_ERROR) {
                fprintf(stderr, "Failed to send number\n");
                closesocket(clientSocket);
                WSACleanup();
                return EXIT_FAILURE;
            }
            printf("Number %d sent to Load Balancer\n", number);

            // Slanje opsega za generisanje brojeva
            int min, max;
            while (true) {
                printf("Enter the range (min max) for the random numbers: ");
                scanf_s("%d %d", &min, &max);

                if (min >= max) {
                    printf("Error: Min should be less than Max. Please try again.\n");
                }
                else {
                    break;  // Ako je unos validan, izlazimo iz petlje
                }
            }

            if (send(clientSocket, (char*)&min, sizeof(min), 0) == SOCKET_ERROR ||
                send(clientSocket, (char*)&max, sizeof(max), 0) == SOCKET_ERROR) {
                fprintf(stderr, "Failed to send range\n");
                closesocket(clientSocket);
                WSACleanup();
                return EXIT_FAILURE;
            }
            printf("Range (%d, %d) sent to Load Balancer\n", min, max);
        }
        int s;
        getchar();  // Potrošnja novog reda pre unosa
        printf("Stop input -1 :  ");
        scanf_s("%d", &s);
        if (s == -1) {
            break;
        }
    }

    // Zatvaranje soketa
    closesocket(clientSocket);
    WSACleanup();




    return EXIT_SUCCESS;
}
