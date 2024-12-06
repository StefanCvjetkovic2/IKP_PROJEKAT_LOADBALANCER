#include "LBcommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Queue.h"
#include <iostream>
using namespace std;

#define THREAD_POOL_SIZE 100

HANDLE hAddToQueueSemaphore;
HANDLE hLoadBalancerThread1Semaphore;
HANDLE hThreadPoolSemaphore[THREAD_POOL_SIZE];
HANDLE hThreadPoolSemaphoreFinish[THREAD_POOL_SIZE];
HANDLE hThreadPoolThread[THREAD_POOL_SIZE];


QUEUE* queue = NULL;

// niz zauzetih worker role-a
bool busyThreads[THREAD_POOL_SIZE];
int counter = 0;

// trenutan broj worker role-a
int numOfWorkerRoles = -1;


// Inicijalizacija Winsock-a
void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSA Initialization Failed\n");
        exit(EXIT_FAILURE);
    }
}

// Kreiranje soketa
SOCKET createWorkerSocket() {
    SOCKET workerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (workerSocket == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create Worker socket\n");
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    return workerSocket;
}

// Povezivanje sa Load Balancer-om
void connectToLoadBalancer(SOCKET workerSocket, const char* lbAddress, int lbPort) {
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(lbPort);
    inet_pton(AF_INET, lbAddress, &serverAddress.sin_addr);

    if (connect(workerSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Failed to connect to Load Balancer\n");
        closesocket(workerSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    printf("Connected to Load Balancer\n");
}

// Rukovanje komunikacijom sa Load Balancer-om
void handleCommunication(SOCKET workerSocket) {
    char buffer[1024] = { 0 };

    // Receive the message from the Load Balancer
    int receivedBytes = recv(workerSocket, buffer, sizeof(buffer), 0);
    if (receivedBytes > 0) {
        printf("Message received from Load Balancer\n");

        QUEUEELEMENT q;
        deserializeQueueElement(buffer, &q);

        
        enqueue(queue, create_queue_element(q.clientName, q.data, q.dataSize));

        // Process the received QUEUEELEMENT (e.g., print its contents)
        printf("Client Name: %s\n", q.clientName);
        printf("Data Size: %d\n", q.dataSize);
        for (int i = 0; i < q.dataSize; i++) {
            printf("Data[%d]: %d\n", i, q.data[i]);
        }

        print_queue(queue);

        // Free the allocated memory for clientName and data
        free(q.clientName);
        free(q.data);

    }
    else {
        fprintf(stderr, "Failed to receive message from Load Balancer\n");
    }

}

// Glavna funkcija koja poziva manje funkcije
DWORD WINAPI startWorker(LPVOID param) {
    initializeWinsock();

    SOCKET workerSocket = createWorkerSocket();
    connectToLoadBalancer(workerSocket, "127.0.0.1", 5060); // Local IP address and Load Balancer port

    queue = init_queue(10);
    
    while (true) {
        // Main worker loop (add relevant code or logic here)
        handleCommunication(workerSocket);
    }

    // Close the socket
    closesocket(workerSocket);
    WSACleanup();
}


/*DWORD WINAPI addToQueue(LPVOID lpParam) {
    QUEUEELEMENT* element = (QUEUEELEMENT*)lpParam;

    while (true) {
        WaitForSingleObject(hAddToQueueSemaphore, INFINITE);

        // Ensure that queue is initialized
        if (queue == NULL) {
            fprintf(stderr, "Queue is not initialized.\n");
            continue;
        }

        // Add the element to the queue
        enqueue(queue, create_queue_element(element->clientName, element->data, element->dataSize));
        print_queue(queue);

        // Free the allocated memory for clientName and data (if not needed anymore)
        free(element->clientName);
        free(element->data);
        free(element);

        ReleaseSemaphore(hLoadBalancerThread1Semaphore, 1, NULL);
    }

    return 0;
}*/

DWORD WINAPI loadBalancerThread1(LPVOID lpParam) {
    while (true) {
        WaitForSingleObject(hLoadBalancerThread1Semaphore, INFINITE);

        bool found = false;
        while (!found) {
            if (!busyThreads[counter]) {
                ReleaseSemaphore(hThreadPoolSemaphore[counter], 1, NULL);
                busyThreads[counter] = true;
                printf("Sent to workerRole %d.\n", counter);
                found = true;
            }

            counter++;
            counter = counter % numOfWorkerRoles;
        }
    }
    return 0;
}


/*DWORD WINAPI loadBalancerThread2(LPVOID lpParam) {
    DWORD threadPoolThreadID[THREAD_POOL_SIZE];
    numOfWorkerRoles = 3;

    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        busyThreads[i] = false;
    }

    for (int i = 0; i < numOfWorkerRoles; i++) {
        hThreadPoolSemaphore[i] = CreateSemaphore(0, 0, 1, NULL);
        hThreadPoolSemaphoreFinish[i] = CreateSemaphore(0, 0, 1, NULL);
        if (hThreadPoolSemaphore[i] && hThreadPoolSemaphoreFinish[i]) {
            hThreadPoolThread[i] = CreateThread(NULL, 0, &workerRole, (LPVOID)i, 0, &threadPoolThreadID[i]);
        }
    }

    while (true)
    {
        double result = ((double)queue->elements / (double)queue->capacity) * 100;
        printf("Current size of Q is: %.2f %%\n", result);

        if (result > 70) {
            hThreadPoolSemaphore[numOfWorkerRoles] = CreateSemaphore(0, 0, 1, NULL);
            hThreadPoolSemaphoreFinish[numOfWorkerRoles] = CreateSemaphore(0, 0, 1, NULL);

            if (hThreadPoolSemaphore[numOfWorkerRoles] && hThreadPoolSemaphoreFinish[numOfWorkerRoles]) {
                hThreadPoolThread[numOfWorkerRoles] = CreateThread(NULL, 0, &workerRole, (LPVOID)numOfWorkerRoles, 0, &threadPoolThreadID[numOfWorkerRoles]);
                numOfWorkerRoles++;
            }
        }
        else if (result < 30 && numOfWorkerRoles > 1) {
            ReleaseSemaphore(hThreadPoolSemaphoreFinish[numOfWorkerRoles - 1], 1, NULL);
            numOfWorkerRoles--;
        }

        Sleep(2000);
    }

    return 0;
}*/
void deserializeQueueElement(char* buffer, QUEUEELEMENT* q) {
    char* ptr = buffer;

    // Deserialize the clientName length and clientName
    int nameLen;
    memcpy(&nameLen, ptr, sizeof(int));
    ptr += sizeof(int);
    q->clientName = (char*)malloc(nameLen);
    memcpy(q->clientName, ptr, nameLen);
    ptr += nameLen;

    // Deserialize the data size and data array
    int dataLen;
    memcpy(&dataLen, ptr, sizeof(int));
    ptr += sizeof(int);
    q->dataSize = dataLen;
    q->data = (int*)malloc(dataLen * sizeof(int));
    memcpy(q->data, ptr, dataLen * sizeof(int));
    ptr += dataLen * sizeof(int);
}



