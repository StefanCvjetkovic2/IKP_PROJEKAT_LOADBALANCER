#ifndef SYNCHRONIZATION_H
#define SYNCHRONIZATION_H

#include <windows.h>

// Semafori za sinhronizaciju
extern HANDLE hSendSemaphore;
extern HANDLE hReceiveSemaphore;

void initializeSemaphores();  // Funkcija za inicijalizaciju semafora
void cleanupSemaphores();     // Funkcija za ?iš?enje semafora

#endif // SYNCHRONIZATION_H
