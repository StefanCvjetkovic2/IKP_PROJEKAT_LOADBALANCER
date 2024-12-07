#include "LBcommunication.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    HANDLE hWorkerToLoadBalancer;
    DWORD hWorkerToLoadBalancerID;
    DWORD loadBalancerThread2ID;

    HANDLE hLoadBalancerThread2;

    queue = init_queue(10);


    hWorkerToLoadBalancer = CreateThread(NULL, 0, &startWorker, (LPVOID)0, 0, &hWorkerToLoadBalancerID);
    hLoadBalancerThread2 = CreateThread(NULL, 0, &loadBalancerThread2, NULL, 0, &loadBalancerThread2ID);



    if (hWorkerToLoadBalancer)
        WaitForSingleObject(hWorkerToLoadBalancer, INFINITE);

    printf("Uspjesno povezan");
    return 0;
}
