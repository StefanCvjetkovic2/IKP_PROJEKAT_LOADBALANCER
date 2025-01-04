﻿#pragma once

#include "Queue.h"

#include <iostream>  // Za kori�?enje cout
using namespace std;



// Kreira red sa datim kapacitetom
QUEUE* init_queue(int capacity) {
    QUEUE* q = (QUEUE*)malloc(sizeof(QUEUE));  // Alocira memoriju za red
    if (!q) {
        cout << "Memory allocation failed for queue." << endl;
        return NULL;  // Ako alokacija ne uspe, vra?a NULL
    }

    // Inicijalizacija kriti?ne sekcije za sinhronizaciju u vi�e niti
    InitializeCriticalSection(&q->cs);

    q->capacity = capacity;  // Postavlja kapacitet reda
    q->front = 0;            // Inicijalizuje front na 0
    q->rear = -1;            // Inicijalizuje rear na -1 (red je prazan)
    q->currentSize = 0;      // Inicijalizuje trenutnu veli?inu reda na 0
    q->elements = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT) * capacity);  // Alocira memoriju za elemente reda
    if (!q->elements) {
        cout << "Memory allocation failed for queue elements." << endl;
        free(q);  // Osloba?a prethodno alociranu memoriju za red
        return NULL;  // Ako alokacija ne uspe, vra?a NULL
    }

    return q;  // Vra?a pokaziva? na kreirani red
}

// Proverava da li je red pun
int is_queue_full(QUEUE* q) {
    return q->currentSize == q->capacity;
}

// Proverava da li je red prazan
int is_queue_empty(QUEUE* q) {
    return q->currentSize == 0;
}

// Dodaje element u red
void enqueue(QUEUE* q, QUEUEELEMENT* element) {
    EnterCriticalSection(&q->cs);  // Ulazi u kriti?nu sekciju

    if (is_queue_full(q)) {
        cout << "Queue is full!" << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije
        return;
    }

    // Dodavanje elementa u red na poziciju rear
    q->rear = (q->rear + 1) % q->capacity;
    q->elements[q->rear] = *element;  // Kopiramo podatke u red
    q->currentSize++;  // Pove?ava trenutnu veli?inu reda

    LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije
}


// Uklanja element sa po?etka reda i vra?a ga
QUEUEELEMENT* dequeue(QUEUE* q) {
    EnterCriticalSection(&q->cs);  // Ulazi u kriti?nu sekciju

    if (is_queue_empty(q)) {
        cout << "Queue is empty!" << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije
        return nullptr;  // Vra?a NULL ako je red prazan
    }

    // Kreiramo pokaziva? na element koji treba da se vrati
    QUEUEELEMENT* removedElement = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT));
    if (!removedElement) {
        cout << "Memory allocation failed for removed element." << endl;
        LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije
        return nullptr;  // Vra?a NULL ako alokacija ne uspe
    }

    // Kopiramo element sa po?etka reda
    *removedElement = q->elements[q->front];

    // Pomeri front za jedan (cirkularni red)
    q->front = (q->front + 1) % q->capacity;
    q->currentSize--;  // Smanjuje trenutnu veli?inu reda

    LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije

    return removedElement;  // Vra?a pokaziva? na izba?eni element
}


// Ispisuje sadr�aj reda
void print_queue(QUEUE* q) {
    EnterCriticalSection(&q->cs);

    if (is_queue_empty(q)) {
        cout << "Queue is empty!" << endl;
    }
    else {
        cout << "Queue contents:" << endl;
        for (int i = 0; i < q->currentSize; i++) {
            int index = (q->front + i) % q->capacity;
            cout << "Client: " << q->elements[index].clientName << ", Data: ";

            for (int j = 0; j < q->elements[index].dataSize; j++) {
                cout << q->elements[index].data[j] << " ";
            }
            cout << endl;
        }
    }

    LeaveCriticalSection(&q->cs);
}




// Vra?a trenutnu veli?inu reda
int get_current_size_queue(QUEUE* q) {
    return q->currentSize;
}

// Vra?a kapacitet reda
int get_capacity_queue(QUEUE* q) {
    return q->capacity;
}

// Bri�e red i osloba?a memoriju
void delete_queue(QUEUE* q) {
    if (q == NULL) {
        return;  // Ako je pokaziva? NULL, nema �ta da se bri�e
    }

    EnterCriticalSection(&q->cs);  // Ulazi u kriti?nu sekciju

    // Osloba?a memoriju za elemente reda
    free(q->elements);

    // Osloba?a memoriju za samu strukturu reda
    free(q);

    LeaveCriticalSection(&q->cs);  // Odlazi iz kriti?ne sekcije

    // Bri�e kriti?nu sekciju
    DeleteCriticalSection(&q->cs);
}

// Kreira i inicijalizuje QUEUEELEMENT
QUEUEELEMENT* create_queue_element(const char* clientName, int* data, int dataSize) {
    QUEUEELEMENT* newElement = (QUEUEELEMENT*)malloc(sizeof(QUEUEELEMENT));
    if (!newElement) {
        cout << "Memory allocation failed for queue element." << endl;
        return NULL;
    }

    newElement->clientName = (char*)malloc(strlen(clientName) + 1);
    if (!newElement->clientName) {
        cout << "Memory allocation failed for clientName." << endl;
        free(newElement);
        return NULL;
    }
    strcpy_s(newElement->clientName, strlen(clientName) + 1, clientName);

    newElement->data = (int*)malloc(dataSize * sizeof(int));
    if (!newElement->data) {
        cout << "Memory allocation failed for data." << endl;
        free(newElement->clientName);
        free(newElement);
        return NULL;
    }

    memcpy(newElement->data, data, dataSize * sizeof(int));
    newElement->dataSize = dataSize;  // Postavljamo veli?inu niza

    return newElement;
}

// Funkicja za promjenu velicine reda
int resize_queue(QUEUE* queue, int new_capacity) {
    EnterCriticalSection(&queue->cs);  // Enter critical section

    // Allocate new array with new capacity
    QUEUEELEMENT* new_elements = (QUEUEELEMENT*)malloc(new_capacity * sizeof(QUEUEELEMENT));
    if (!new_elements) {
        LeaveCriticalSection(&queue->cs);  // Leave critical section
        return -1;  // Failed to allocate memory
    }

    // Copy existing elements to the new array
    for (int i = 0; i < queue->currentSize; i++) {
        new_elements[i] = queue->elements[(queue->front + i) % queue->capacity];
    }

    // Free the old array and update the queue structure
    free(queue->elements);
    queue->elements = new_elements;
    queue->capacity = new_capacity;
    queue->front = 0;
    queue->rear = queue->currentSize - 1;  // Rear should be the last element index

    LeaveCriticalSection(&queue->cs);  // Leave critical section
    return 0;  // Successfully resized the queue
}