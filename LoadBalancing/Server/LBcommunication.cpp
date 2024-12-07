#include "LBcommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Queue.h"
#include <iostream>
using namespace std;

#define THREAD_POOL_SIZE 100
#define SAFE_DELETE_HANDLE(a) if(a){CloseHandle(a);} 

HANDLE hAddToQueueSemaphore;
HANDLE hLoadBalancerThread1Semaphore;
HANDLE hThreadPoolSemaphore[THREAD_POOL_SIZE];
HANDLE hThreadPoolSemaphoreFinish[THREAD_POOL_SIZE];
HANDLE hThreadPoolThread[THREAD_POOL_SIZE];


QUEUE* queue = NULL;
CRITICAL_SECTION QueueCS;

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

     

        print_queue(queue);
       
       
        int v = get_current_size_queue(queue);
        printf("\nCurrent_size_queue %d\n",v);
       

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

   
    
    while (true) {
        // Main worker loop (add relevant code or logic here)
        handleCommunication(workerSocket);
    }

    // Close the socket
    closesocket(workerSocket);
    WSACleanup();
}

QUEUE* workerQueues[THREAD_POOL_SIZE];

DWORD WINAPI loadBalancerThread2(LPVOID lpParam) {
    const int INITIAL_WORKER_THREADS = 10;  // Početni broj niti
    const int MAX_QUEUE_CAPACITY = 2;      // Maksimalna veličina reda

    InitializeCriticalSection(&QueueCS); // Inicijalizacija kritične sekcije

    int numOfWorkerRoles = 0;

    // Inicijalizacija početnih niti i njihovih redova
    for (int i = 0; i < INITIAL_WORKER_THREADS; i++) {
        hThreadPoolSemaphore[i] = CreateSemaphore(0, 0, 1, NULL);
        hThreadPoolSemaphoreFinish[i] = CreateSemaphore(0, 0, 1, NULL);

        workerQueues[i] = init_queue(MAX_QUEUE_CAPACITY);
        if (workerQueues[i] == NULL) {
            printf("Failed to initialize queue for worker %d.\n", i);
            exit(1);
        }

        HANDLE hThreadPoolThread = CreateThread(
            NULL, 0, &workerRole, (LPVOID)i, 0, NULL);

        busyThreads[i] = false;
        numOfWorkerRoles++;
    }

    // Glavna petlja balansiranja
    while (true) {
        EnterCriticalSection(&QueueCS); // Zaštita pristupa globalnom redu
        int globalQueueSize = get_current_size_queue(queue);
        LeaveCriticalSection(&QueueCS);

        if (globalQueueSize > 0) {
            QUEUEELEMENT* task = dequeue(queue);

            bool taskAssigned = false;
            for (int i = 0; i < numOfWorkerRoles; i++) {
                if (get_current_size_queue(workerQueues[i]) < MAX_QUEUE_CAPACITY) {
                    EnterCriticalSection(&QueueCS);
                    enqueue(workerQueues[i], task);
                    LeaveCriticalSection(&QueueCS);

                    ReleaseSemaphore(hThreadPoolSemaphore[i], 1, NULL);
                    taskAssigned = true;
                    break;
                }
            }

            // Kreiraj novu nit ako nijedna nije dostupna
            if (!taskAssigned && numOfWorkerRoles < THREAD_POOL_SIZE) {
                hThreadPoolSemaphore[numOfWorkerRoles] = CreateSemaphore(0, 0, 1, NULL);
                hThreadPoolSemaphoreFinish[numOfWorkerRoles] = CreateSemaphore(0, 0, 1, NULL);

                workerQueues[numOfWorkerRoles] = init_queue(MAX_QUEUE_CAPACITY);
                if (workerQueues[numOfWorkerRoles] == NULL) {
                    printf("Failed to initialize queue for new worker %d.\n", numOfWorkerRoles);
                    exit(1);
                }

                HANDLE hThreadPoolThread = CreateThread(
                    NULL, 0, &workerRole, (LPVOID)numOfWorkerRoles, 0, NULL);

                EnterCriticalSection(&QueueCS);
                enqueue(workerQueues[numOfWorkerRoles], task);
                LeaveCriticalSection(&QueueCS);

                ReleaseSemaphore(hThreadPoolSemaphore[numOfWorkerRoles], 1, NULL);
                numOfWorkerRoles++;
            }
        }

        Sleep(1000);
    }

    DeleteCriticalSection(&QueueCS); // Oslobađanje kritične sekcije
    return 0;
}









DWORD WINAPI workerRole(LPVOID lpParam) {
    int n = (int)lpParam;

    if (n < 0 || n >= THREAD_POOL_SIZE) {
        printf("Invalid thread index: %d\n", n);
        return -1;
    }

    if (workerQueues[n] == NULL) {
        printf("Error: workerQueues[%d] is not initialized.\n", n);
        return -1;
    }

    HANDLE semaphores[] = { hThreadPoolSemaphoreFinish[n], hThreadPoolSemaphore[n] };

    while (true) {
        DWORD waitResult = WaitForMultipleObjects(2, semaphores, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0) {
            break; // Završni signal
        }

        while (true) {
            EnterCriticalSection(&QueueCS);
            QUEUEELEMENT* task = dequeue(workerQueues[n]);
            LeaveCriticalSection(&QueueCS);

            if (!task) {
                break; // Red je prazan
            }

            int sum = 0;
            for (int i = 0; i < task->dataSize; i++) {
                sum += task->data[i];
            }

            printf("Worker %d processed data for client '%s'. Sum: %d\n", n, task->clientName, sum);
            Sleep(20000);

            free(task->data);
            free(task->clientName);
            free(task);
        }

        EnterCriticalSection(&QueueCS);
        if (is_queue_empty(workerQueues[n])) {
            busyThreads[n] = false;
        }
        LeaveCriticalSection(&QueueCS);
    }

    return 0;
}












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



