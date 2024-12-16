#include "ClientWorkerCommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ClientQueue.h"
#include <iostream>
using namespace std;

QUEUER* queue = NULL;


// Inicijalizacija Winsock-a
void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }
}

// Kreiranje soketa
SOCKET createClientSocket() {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create Client socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return clientSocket;
}

// Povezivanje sa clientom
void connectToServer(SOCKET clientSocket, const char* workerAddress, int workerPort) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(workerPort);
    inet_pton(AF_INET, workerAddress, &serverAddress.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Failed to connect to Server\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }


}

// Rukovanje komunikacijom serverom
void handleCommunication(SOCKET clientSocket) {
    char buffer[4096] = { 0 };

    // Receive the message from the server
    int receivedBytes = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (receivedBytes > 0) {
        //printf("Message received from Server\n");

        QUEUEELEMENTRESULT q;
        deserializeQueueElementResult(buffer, &q);


        enqueue2(queue, create_queue_element2(q.clientName, q.result));


       // printf("Results recieved from workers:\n ");
        //print_queue2(queue);

        // Free the allocated memory for clientName
        free(q.clientName);

    }
    else {
        //fprintf(stderr, "Failed to receive message from Server\n");
    }

}

// Glavna funkcija koja poziva manje funkcije
//DWORD WINAPI startClient(LPVOID param) {
//    initializeWinsock();
//
//    SOCKET clientSocket = createClientSocket();
//    connectToServer(clientSocket, "127.0.0.1", 5061); // Local IP address and Load Balancer port
//
//    queue = init_queue2(100);
//
//    while (true) {
//        // Main worker loop (add relevant code or logic here)
//        handleCommunication(clientSocket);
//    }
//
//    // Close the socket
//    closesocket(clientSocket);
//    WSACleanup();
//}
DWORD WINAPI startClient(LPVOID param) {
    initializeWinsock();

    SOCKET clientSocket = createClientSocket();
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5061); // Port za server za rezultate
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);

    // Pokušaj povezivanja dok server ne postane dostupan
    while (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
      
        Sleep(10000); // Čekaj 1 sekundu pre ponovnog pokušaja
    }

    printf("Connected to results server\n");

    // Inicijalizacija reda za rezultate
    queue = init_queue2(100);

    // Glavna petlja za rukovanje komunikacijom
    while (true) {
        handleCommunication(clientSocket);
    }

    // Zatvaranje soketa
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}



void deserializeQueueElementResult(char* buffer, QUEUEELEMENTRESULT* q) {
    char* ptr = buffer;

    // Deserialize the client name length
    int nameLen;
    memcpy(&nameLen, ptr, sizeof(int));
    ptr += sizeof(int);

    // Allocate memory and deserialize the client name
    q->clientName = (char*)malloc(nameLen);
    if (q->clientName == NULL) {
        fprintf(stderr, "Failed to allocate memory for client name\n");
        return;
    }
    memcpy(q->clientName, ptr, nameLen);
    ptr += nameLen;

    // Deserialize the result
    memcpy(&(q->result), ptr, sizeof(int));
}

