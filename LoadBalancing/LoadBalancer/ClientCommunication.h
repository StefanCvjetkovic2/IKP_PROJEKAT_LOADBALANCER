#ifndef CLIENTCOMMUNICATION_H
#define CLIENTCOMMUNICATION_H

#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_PORT 8080
#define BUFFER_SIZE 4

// Inicijalizacija WinSock-a
bool initializeWinSock();

// Kreiranje i podešavanje server soketa za klijente
SOCKET setupClientSocket();

// Prihvatanje klijentske konekcije
SOCKET acceptClient(SOCKET& serverSocket);

// Primanje podataka od klijenta
int receiveFromClient(SOCKET& clientSocket, int& receivedNumber);

// Zatvaranje klijentskog soketa
void closeClientSocket(SOCKET& clientSocket);

// Čišćenje resursa WinSock-a
void cleanupWinSock();

#endif // CLIENTCOMMUNICATION_H
