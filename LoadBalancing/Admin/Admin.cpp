#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "AdminWorkerCommunication.h"


#pragma comment(lib, "Ws2_32.lib")

int main()
{
    HANDLE hAdminWorker;
    DWORD hAdminWorkerID;
    hAdminWorker = CreateThread(NULL, 0, &startAdmin, (LPVOID)0, 0, &hAdminWorkerID);

    if (hAdminWorker)
        WaitForSingleObject(hAdminWorker, INFINITE);

}
