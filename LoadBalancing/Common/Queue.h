#pragma once
#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>


// Struktura za element reda

typedef struct {
    char* clientName;  // Ime klijenta
    int* data;         // Podaci koji se ?uvaju u redu, sada je to niz celih brojeva
    int dataSize;
} QUEUEELEMENT;



// Struktura za red
typedef struct {
    CRITICAL_SECTION cs;  // Za sinhronizaciju pristupa u više niti
    int front, rear, currentSize;  // Indeksi fronta, rear-a i trenutna veli?ina reda
    int capacity;  // Kapacitet reda
    QUEUEELEMENT* elements;  // Niz elemenata reda
} QUEUE;


extern QUEUE* queue;
extern QUEUE* ClientQueue;


// Funkcije za rad sa redom
QUEUE* init_queue(int capacity);  // Kreira red sa datim kapacitetom
int is_queue_full(QUEUE* q);      // Proverava da li je red pun
int is_queue_empty(QUEUE* q);     // Proverava da li je red prazan
void enqueue(QUEUE* q, QUEUEELEMENT* element);  // Dodaje element u red
QUEUEELEMENT* dequeue(QUEUE* q);  // Uklanja element sa po?etka reda
void print_queue(QUEUE* q);        // Ispisuje sadržaj reda
int get_current_size_queue(QUEUE* q);  // Vra?a trenutnu veli?inu reda
int get_capacity_queue(QUEUE* q);    // Vra?a kapacitet reda
void delete_queue(QUEUE* q);       // Briše red i osloba?a memoriju
QUEUEELEMENT* create_queue_element(const char* clientName, int* data, int dataSize);
int resize_queue(QUEUE* queue, int new_capacity);