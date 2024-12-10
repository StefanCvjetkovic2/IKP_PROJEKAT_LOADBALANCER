#pragma once

#include "ClientQueue.h"

#include <iostream>
using namespace std;


// Kreira red sa datim kapacitetom
QUEUER* init_queue2(int capacity) {
    QUEUER* q = (QUEUER*)malloc(sizeof(QUEUER));
    if (!q) {
        cout << "Memory allocation failed for queue." << endl;
        return NULL;
    }

    InitializeCriticalSection(&q->cs);

    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->currentSize = 0;

    q->elements = (QUEUEELEMENTRESULT*)malloc(sizeof(QUEUEELEMENTRESULT) * capacity);
    if (!q->elements) {
        cout << "Memory allocation failed for queue elements." << endl;
        free(q);
        return NULL;
    }

    return q;
}

// Proverava da li je red pun
int is_queue_full2(QUEUER* q) {
    return q->currentSize == q->capacity;
}

// Proverava da li je red prazan
int is_queue_empty2(QUEUER* q) {
    return q->currentSize == 0;
}

// Dodaje element u red
void enqueue2(QUEUER* q, QUEUEELEMENTRESULT* element) {
    EnterCriticalSection(&q->cs);

    if (is_queue_full2(q)) {
        cout << "Queue is full!" << endl;
        LeaveCriticalSection(&q->cs);
        return;
    }

    q->rear = (q->rear + 1) % q->capacity;
    q->elements[q->rear] = *element;  // Kopiramo podatke u red
    q->currentSize++;

    LeaveCriticalSection(&q->cs);
}

// Uklanja element sa po?etka reda i vra?a ga
QUEUEELEMENTRESULT* dequeue2(QUEUER* q) {
    EnterCriticalSection(&q->cs);

    if (is_queue_empty2(q)) {
        cout << "Queue is empty!\n" << endl;
        LeaveCriticalSection(&q->cs);
        return nullptr;
    }

    QUEUEELEMENTRESULT* removedElement = (QUEUEELEMENTRESULT*)malloc(sizeof(QUEUEELEMENTRESULT));
    if (!removedElement) {
        cout << "Memory allocation failed for removed element." << endl;
        LeaveCriticalSection(&q->cs);
        return nullptr;
    }

    *removedElement = q->elements[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->currentSize--;

    LeaveCriticalSection(&q->cs);
    return removedElement;
}

// Ispisuje sadržaj reda
void print_queue2(QUEUER* q) {
    EnterCriticalSection(&q->cs);

    if (is_queue_empty2(q)) {
        cout << "The results have not yet been processed" << endl;
    }
    else {
        cout <<endl<< "Results:" << endl;
        for (int i = 0; i < q->currentSize; i++) {
            int index = (q->front + i) % q->capacity;
            cout << q->elements[index].clientName
                << ", Result: " << q->elements[index].result << endl;
        }
    }

    LeaveCriticalSection(&q->cs);
}

// Vra?a trenutnu veli?inu reda
int get_current_size_queue2(QUEUER* q) {
    return q->currentSize;
}

// Vra?a kapacitet reda
int get_capacity_queue2(QUEUER* q) {
    return q->capacity;
}

// Briše red i osloba?a memoriju
void delete_queue2(QUEUER* q) {
    if (q == NULL) {
        return;
    }

    EnterCriticalSection(&q->cs);

    free(q->elements);
    free(q);

    LeaveCriticalSection(&q->cs);
    DeleteCriticalSection(&q->cs);
}

// Kreira i inicijalizuje QUEUEELEMENTRESULT
QUEUEELEMENTRESULT* create_queue_element2(const char* clientName, int result) {
    QUEUEELEMENTRESULT* newElement = (QUEUEELEMENTRESULT*)malloc(sizeof(QUEUEELEMENTRESULT));
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

    newElement->result = result;

    return newElement;
}
