#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <windows.h>


// Struktura za element reda
typedef struct {
    char* clientName;  // Ime klijenta
    int result;        // Rezultat
} QUEUEELEMENTRESULT;

// Struktura za red
typedef struct {
    CRITICAL_SECTION cs;  // Za sinhronizaciju pristupa u vi�e niti
    int front, rear, currentSize;  // Indeksi fronta, rear-a i trenutna veli?ina reda
    int capacity;  // Kapacitet reda
    QUEUEELEMENTRESULT* elements;  // Niz elemenata reda
} QUEUER;


extern QUEUER* ResultsQueue;

// Funkcije za rad sa redom
QUEUER* init_queue2(int capacity);  // Kreira red sa datim kapacitetom
int is_queue_full2(QUEUER* q);      // Proverava da li je red pun
int is_queue_empty2(QUEUER* q);     // Proverava da li je red prazan
void enqueue2(QUEUER* q, QUEUEELEMENTRESULT* element);  // Dodaje element u red
QUEUEELEMENTRESULT* dequeue2(QUEUER* q);  // Uklanja element sa po?etka reda
void print_queue2(QUEUER* q);        // Ispisuje sadr�aj reda
int get_current_size_queue2(QUEUER* q);  // Vra?a trenutnu veli?inu reda
int get_capacity_queue2(QUEUER* q);    // Vra?a kapacitet reda
void delete_queue2(QUEUER* q);       // Bri�e red i osloba?a memoriju
QUEUEELEMENTRESULT* create_queue_element2(const char* clientName, int result);  // Kreira novi element reda