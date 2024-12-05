#include <thread>
#include "ClientCommunication.h"
#include "WorkerCommunication.h"


int main() {
    startLoadBalancer(); // Komunikacija sa klijentima
    startServer(); // Komunikacija sa radnicima
  


    return 0;
}
