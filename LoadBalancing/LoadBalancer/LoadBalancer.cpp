#include "clientcommunication.h"
#include <iostream>



auto& cout = std::cout;

int main() {
    // Inicijalizacija WinSock-a
    if (!initializeWinSock()) {
        return 1;
    }

    // Kreiranje i podešavanje server soketa za klijente
    SOCKET serverSocket = setupClientSocket();
    if (serverSocket == INVALID_SOCKET) {
        cleanupWinSock();
        return 1;
    }

    // Prihvatanje klijentske konekcije
    SOCKET clientSocket = acceptClient(serverSocket);
    if (clientSocket == INVALID_SOCKET) {
        closesocket(serverSocket);
        cleanupWinSock();
        return 1;
    }

    // Primanje podataka od klijenta
    int receivedNumber;
    while (true) {
        int bytesReceived = receiveFromClient(clientSocket, receivedNumber);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            cout << "Client disconnected\n";
            break;
        }

        cout << "Received from client: " << receivedNumber << "\n";
    }

    // Zatvaranje klijentskog soketa
    closeClientSocket(clientSocket);

    // Zatvaranje server soketa i čišćenje resursa
    closesocket(serverSocket);
    cleanupWinSock();

    return 0;
}


























//#include <iostream>
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <vector>
//#include <thread>
//#include <mutex>
//
//#pragma comment(lib, "ws2_32.lib")
//
//auto& cout = std::cout;
//
//#define SERVER_PORT 8080
//#define WORKER_PORT 8081
//#define BUFFER_SIZE 4
//
//std::vector<SOCKET> workers; // Lista Worker soketa
//std::mutex workerMutex;      // Mutex za bezbedan pristup listi Workera
//
//// Funkcija za rukovanje konekcijama sa Workerima
//void acceptWorkers(SOCKET workerSocket) {
//    sockaddr_in workerClientAddr{};
//    int workerClientAddrSize = sizeof(workerClientAddr);
//
//    while (true) {
//        SOCKET workerClientSocket = accept(workerSocket, (sockaddr*)&workerClientAddr, &workerClientAddrSize);
//        if (workerClientSocket == INVALID_SOCKET) {
//            cout << "Failed to accept worker connection\n";
//            continue;
//        }
//
//        // Dodavanje novog Workera u listu
//        {
//            std::lock_guard<std::mutex> lock(workerMutex);
//            workers.push_back(workerClientSocket);
//        }
//        cout << "Worker connected. Total workers: " << workers.size() << "\n";
//    }
//}
//
//int main() {
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        cout << "WSAStartup failed\n";
//        return 1;
//    }
//
//    // Kreiranje TCP soketa za Load Balancer
//    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if (serverSocket == INVALID_SOCKET) {
//        cout << "Socket creation failed\n";
//        WSACleanup();
//        return 1;
//    }
//
//    // Konfiguracija adrese Load Balancera za klijente
//    sockaddr_in serverAddr{};
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_port = htons(SERVER_PORT);
//    serverAddr.sin_addr.s_addr = INADDR_ANY;
//
//    // Bindovanje i slušanje za klijente
//    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
//        cout << "Bind failed\n";
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//    if (listen(serverSocket, SOMAXCONN) < 0) {
//        cout << "Listen failed\n";
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//    cout << "Load Balancer is listening for clients on port " << SERVER_PORT << "\n";
//
//    // Kreiranje TCP soketa za Workere
//    SOCKET workerSocket = socket(AF_INET, SOCK_STREAM, 0);
//    if (workerSocket == INVALID_SOCKET) {
//        cout << "Worker socket creation failed\n";
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    // Konfiguracija adrese za Workere
//    sockaddr_in workerAddr{};
//    workerAddr.sin_family = AF_INET;
//    workerAddr.sin_port = htons(WORKER_PORT);
//    workerAddr.sin_addr.s_addr = INADDR_ANY;
//
//    // Bindovanje i slušanje za Workere
//    if (bind(workerSocket, (sockaddr*)&workerAddr, sizeof(workerAddr)) < 0) {
//        cout << "Worker bind failed\n";
//        closesocket(workerSocket);
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//    if (listen(workerSocket, SOMAXCONN) < 0) {
//        cout << "Worker listen failed\n";
//        closesocket(workerSocket);
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//    cout << "Load Balancer is listening for workers on port " << WORKER_PORT << "\n";
//
//    // Startovanje niti za prihvatanje Workera
//    std::thread workerThread(acceptWorkers, workerSocket);
//    workerThread.detach();
//
//    // Prihvatanje klijentske konekcije
//    sockaddr_in clientAddr{};
//    int clientAddrSize = sizeof(clientAddr);
//    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
//    if (clientSocket == INVALID_SOCKET) {
//        cout << "Failed to accept client connection\n";
//        closesocket(workerSocket);
//        closesocket(serverSocket);
//        WSACleanup();
//        return 1;
//    }
//    cout << "Client connected\n";
//
//    // Primanje podataka od klijenta i prosleđivanje Workerima
//    int receivedNumber;
//    while (true) {
//        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&receivedNumber), BUFFER_SIZE, 0);
//        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
//            cout << "Client disconnected\n";
//            break;
//        }
//
//        cout << "Received from client: " << receivedNumber << "\n";
//
//        // Prosleđivanje primljenog broja Workerima
//        std::lock_guard<std::mutex> lock(workerMutex);
//        for (auto it = workers.begin(); it != workers.end(); ) {
//            int sendResult = send(*it, reinterpret_cast<char*>(&receivedNumber), sizeof(receivedNumber), 0);
//            if (sendResult == SOCKET_ERROR) {
//                cout << "Worker disconnected\n";
//                closesocket(*it);
//                it = workers.erase(it); // Ukloni Workera ako je prekinuo vezu
//            }
//            else {
//                cout << "Forwarded to worker\n";
//                ++it;
//            }
//        }
//    }
//
//    // Zatvaranje svih soketa i čišćenje resursa
//    {
//        std::lock_guard<std::mutex> lock(workerMutex);
//        for (auto& worker : workers) {
//            closesocket(worker);
//        }
//    }
//    closesocket(clientSocket);
//    closesocket(serverSocket);
//    closesocket(workerSocket);
//    WSACleanup();
//
//    return 0;
//}
