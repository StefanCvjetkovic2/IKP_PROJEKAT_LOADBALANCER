#include <thread>
#include "ClientCommunication.h"
#include "WorkerCommunication.h"


int main() {

    HANDLE hLoadBalancerToClient;
    HANDLE hLoadBalancerToWorker;

    DWORD hLoadBalancerToclientID;
    DWORD hLoadBalancerToWorkerID;

    //startServer(); // Komunikacija sa radnicima
    //startLoadBalancer(); // Komunikacija sa klijentima
    hLoadBalancerToClient = CreateThread(NULL, 0, &startServer, (LPVOID)0, 0, &hLoadBalancerToclientID);
    hLoadBalancerToWorker = CreateThread(NULL, 0, &startLoadBalancer, (LPVOID)0, 0, &hLoadBalancerToWorkerID);

    if (hLoadBalancerToClient)
        WaitForSingleObject(hLoadBalancerToClient, INFINITE);
    if (hLoadBalancerToWorker)
        WaitForSingleObject(hLoadBalancerToWorker, INFINITE);
  


    return 0;
}
