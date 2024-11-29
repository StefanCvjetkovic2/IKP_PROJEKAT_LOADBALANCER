#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib") // Linkovanje WinSock biblioteke

// Definišemo alias za cout
auto& cout = std::cout;

// Parametri Workera
#define LOAD_BALANCER_IP "127.0.0.1" // IP adresa Load Balancera
#define LOAD_BALANCER_PORT 8081      // Port na kojem Load Balancer prihvata Workere
#define BUFFER_SIZE 4                // Veličina buffer-a za int (4 bajta)

int main() {
    // Inicijalizacija WinSock-a
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

    // Kreiranje TCP soketa za Workera
    SOCKET workerSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (workerSocket == INVALID_SOCKET) {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // Konfiguracija adrese Load Balancera
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(LOAD_BALANCER_PORT);
    inet_pton(AF_INET, LOAD_BALANCER_IP, &serverAddr.sin_addr);

    // Povezivanje na Load Balancer
    if (connect(workerSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Connection to Load Balancer failed\n";
        closesocket(workerSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to Load Balancer\n";

    // Primanje podataka od Load Balancera
    int receivedNumber;
    while (true) {
        int bytesReceived = recv(workerSocket, reinterpret_cast<char*>(&receivedNumber), BUFFER_SIZE, 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            cout << "Connection closed by Load Balancer\n";
            break;
        }

        // Ispis primljenog broja
        cout << "Received from Load Balancer: " << receivedNumber << "\n";
    }

    // Zatvaranje konekcije i čišćenje resursa
    closesocket(workerSocket);
    WSACleanup();

    return 0;
}
