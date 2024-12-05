#include "LBcommunication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

    HANDLE hWorkerToLoadBalancer;
    DWORD hWorkerToLoadBalancerID;

    hWorkerToLoadBalancer = CreateThread(NULL, 0, &startWorker, (LPVOID)0, 0, &hWorkerToLoadBalancerID);

    if (hWorkerToLoadBalancer)
        WaitForSingleObject(hWorkerToLoadBalancer, INFINITE);

    printf("Uspjesno povezan");
    return 0;
}
