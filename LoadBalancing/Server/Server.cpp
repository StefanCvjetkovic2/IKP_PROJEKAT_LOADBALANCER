#include "LBcommunication.h"
#include "WorkerClientCommunication.h"
#include "AdminCommunication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    HANDLE hWorkerToLoadBalancer;
    DWORD hWorkerToLoadBalancerID;
    DWORD loadBalancerThread2ID;

    HANDLE hLoadBalancerThread2;

    HANDLE hWorkerToClient;
    DWORD WorkerToClientID;

    HANDLE hWorkerToAdmin;
    DWORD workerToAdminID;

    queue = init_queue(10);
    //startWorkerToClient

    hWorkerToLoadBalancer = CreateThread(NULL, 0, &startWorker, (LPVOID)0, 0, &hWorkerToLoadBalancerID);
    hLoadBalancerThread2 = CreateThread(NULL, 0, &loadBalancerThread2, NULL, 0, &loadBalancerThread2ID);
    hWorkerToClient = CreateThread(NULL, 0, &startWorkerToClient, NULL, 0, &loadBalancerThread2ID);
    hWorkerToAdmin = CreateThread(NULL, 0, &startWorkerToAdmin, NULL, 0, &workerToAdminID);



    if (hWorkerToLoadBalancer)
        WaitForSingleObject(hWorkerToLoadBalancer, INFINITE);

    if (hWorkerToClient)
        WaitForSingleObject(hWorkerToClient, INFINITE);



    printf("Uspjesno povezan");
    return 0;
}
