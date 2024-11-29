#include "clientcommunication.h"

// Inicijalizacija WinSock-a
bool initializeWinSock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }
    return true;
}

// Kreiranje i podešavanje server soketa za klijente
SOCKET setupClientSocket() {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        return INVALID_SOCKET;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    if (listen(serverSocket, SOMAXCONN) < 0) {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        return INVALID_SOCKET;
    }

    std::cout << "Server is listening for clients on port " << SERVER_PORT << "\n";
    return serverSocket;
}

// Prihvatanje klijentske konekcije
SOCKET acceptClient(SOCKET& serverSocket) {
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to accept client connection\n";
        return INVALID_SOCKET;
    }

    std::cout << "Client connected\n";
    return clientSocket;
}

// Primanje podataka od klijenta
int receiveFromClient(SOCKET& clientSocket, int& receivedNumber) {
    return recv(clientSocket, reinterpret_cast<char*>(&receivedNumber), BUFFER_SIZE, 0);
}

// Zatvaranje klijentskog soketa
void closeClientSocket(SOCKET& clientSocket) {
    closesocket(clientSocket);
    std::cout << "Client socket closed\n";
}

// Čišćenje resursa WinSock-a
void cleanupWinSock() {
    WSACleanup();
    std::cout << "WinSock resources cleaned up\n";
}
