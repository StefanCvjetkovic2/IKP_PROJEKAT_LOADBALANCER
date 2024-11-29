#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstdlib> // Za rand() i srand()
#include <ctime>   // Za time()

#pragma comment(lib, "ws2_32.lib") // Linkovanje WinSock biblioteke

// Definišemo alias za cout
auto& cout = std::cout;

// Parametri servera (Load Balancera)
#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"

int brojac = 0;

int main() {
    // Inicijalizacija WinSock-a
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed\n";
        return 1;
    }

    // Kreiranje soketa
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    // Definisanje adrese servera (Load Balancera)
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    // Konekcija na Load Balancer
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cout << "Connection to Load Balancer failed\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    cout << "Connected to Load Balancer\n";
    int sendResult;
    int randomNum;
    // Generisanje i slanje slučajnih brojeva
    srand(static_cast<unsigned>(time(0))); // Inicijalizacija generatora slučajnih brojeva
    while (true) {
        if (brojac < 20) {
            randomNum = rand() % 100; // Broj između 0 i 99
            sendResult = send(clientSocket, reinterpret_cast<char*>(&randomNum), sizeof(randomNum), 0);
            brojac ++;
        }
        else {
            brojac = 0;
            cout << "Prvi zahtjev je poslat\n";
            Sleep(60000);
            

        }
        if (sendResult == SOCKET_ERROR) {
            cout << "Failed to send data\n";
            break;
        }

        cout << "Sent: " << randomNum << "\n";
        Sleep(1000); // Pauza od 1 sekunde
    }

    // Zatvaranje konekcije i čišćenje resursa
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
