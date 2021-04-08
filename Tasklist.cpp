#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <psapi.h>
#include <stdio.h>
#include <Npapi.h>

//  Forward declarations:
BOOL GetProcessList();
BOOL ListProcessModules(DWORD dwPID);
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);
int services_flag = 1;
int verbose = 0;


//Objects for collected info and Verbose switch
PROCESS_MEMORY_COUNTERS ppsmemcounters;
FILETIME lpCreationTime, lpExitTime, lpKernelTime, lpUserTime;
SYSTEMTIME lpSystemTime;

//Objects for the Services Information
SC_HANDLE hSCManager;
DWORD lpServicesReturned;
LPBYTE lpServices;
DWORD cBuffSize = 262144;
LPDWORD pcbBytesNeeded;
void* buf = NULL;


LPNETRESOURCE lpNetResource;
char machine_name;
LPWSTR user_name;
LPWSTR password;

//Not currently implemented
int RemoteConnect(LPNETRESOURCE lpNetResource, LPWSTR password, LPWSTR user_name) {

    //NPAddConnection(lpNetResource, password, user_name);
    return 0;
}


LPCSTR lpServiceName;
LPSTR lpDisplayName;
DWORD lpcchBuffer;


//Gets the Service Info
//Currently does not goet the full Service name?
int GetServiceInfo() {


    hSCManager = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT | SC_MANAGER_ENUMERATE_SERVICE);

    //An alternate method to get the service display info, not functional
    //GetServiceDisplayNameA(hSCManager, lpServiceName, lpDisplayName, &lpcchBuffer);


    void* buf = NULL;
    DWORD bufSize = 0;
    DWORD moreBytesNeeded, serviceCount;
    for (;;) {
        printf("Calling EnumServiceStatusEx with bufferSize %d\n", bufSize);
        if (EnumServicesStatusEx(
            hSCManager,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            (LPBYTE)buf,
            bufSize,
            &moreBytesNeeded,
            &serviceCount,
            NULL,
            NULL)) {
            ENUM_SERVICE_STATUS_PROCESSA* services = (ENUM_SERVICE_STATUS_PROCESSA*)buf;


            for (DWORD i = 0; i < serviceCount; ++i) {
                ENUM_SERVICE_STATUS_PROCESSA individual = services[i];

                int serv_pid = services[i].ServiceStatusProcess.dwProcessId;
                LPSTR test = individual.lpServiceName;
                if (serv_pid != 0) {
                    //Something is happeneing with the dispaly name and service name
                    printf("%s\t", services[i].lpDisplayName);
                    printf("%d\n", serv_pid);
                }

            }
            free(buf);
            return 0;
        }
        int err = GetLastError();
        if (ERROR_MORE_DATA != err) {
            free(buf);
            return err;
        }
        bufSize += moreBytesNeeded;
        free(buf);
        buf = malloc(bufSize);
    }

    return 0;
}

//The main function parses out the commandline arguments and calls the functions
//The individual functions perform the actual functionality and optional flags
int main(int argc, char* argv[])
{


    for (int i = 1; i < argc; i++) {
        //Checks for the Remote Computer Flag
        //Not currently Implemented
        if (!strcmp(argv[i], "/S")) {
            i++;
            //NETRESOURCEA lpNetResource(RESOURCE_CONNECTED, RESOURCETYPE_ANY, RESOURCEDISPLAYTYPE_DOMAIN, RESOURCEUSAGE_CONNECTABLE, NULL, argv[i], NULL, NULL);
            return 0;
        }
        //Checks for the User Flag
        if (!strcmp(argv[i], "/U")) {
            return 0;
        }

        //Checks for the Services Flag
        if (!strcmp(argv[i], "/SVC")) {
            GetServiceInfo();
            return 0;
        }
        //Checks for the verbose Flag
        if (!strcmp(argv[i], "/V")) {
            verbose = 1;
        }

    }

    GetProcessList();
    return 0;
}

BOOL GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;
    int services_PID = 999;
    char services[] = "services.exe";


    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        //printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        //printError(TEXT("Process32First")); // show cause of failure
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do
    {
        if (!wcscmp(pe32.szExeFile, L"services.exe")) {
            services_PID = (int)pe32.th32ProcessID;
        }


        if ((services_flag == 1 && (int)pe32.th32ParentProcessID == services_PID) || services_flag == 0) {

            _tprintf(TEXT("\n\n====================================================="));
            _tprintf(TEXT("\nImage Name:  %s\n"), pe32.szExeFile);

            const char* name = (char*)pe32.szExeFile;

            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            HANDLE parent_proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ParentProcessID);
            TCHAR parent_nam[256] = {};
            GetProcessImageFileName(parent_proc, parent_nam, 256);

            _tprintf(TEXT("Process ID = %d"), pe32.th32ProcessID);
            GetProcessTimes(hProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime);
            FileTimeToSystemTime(&lpCreationTime, &lpSystemTime);
            _tprintf(TEXT("\t  Process Creation TIME = %d:%d:%d"), lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);

            if ((int)pe32.th32ParentProcessID == services_PID) {
                _tprintf(TEXT("\t  Session: Service"));
            }
            else {
                _tprintf(TEXT("\t  Session: Console"));
            }

            if (verbose == 1) {

                _tprintf(TEXT("\n  Parent process ID = %d"), pe32.th32ParentProcessID);
                _tprintf(TEXT("\t  Thread count = %d"), pe32.cntThreads);
                GetProcessMemoryInfo(hProcess, &ppsmemcounters, sizeof(ppsmemcounters));
                _tprintf(TEXT("\t Working Set size  = %d"), ppsmemcounters.WorkingSetSize);
                FileTimeToSystemTime(&lpKernelTime, &lpSystemTime);
                _tprintf(TEXT("\n  Process Kernel TIME = %d:%d:%d"), lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);
                FileTimeToSystemTime(&lpUserTime, &lpSystemTime);
                _tprintf(TEXT("\t  Process User TIME = %d:%d:%d"), lpSystemTime.wHour, lpSystemTime.wMinute, lpSystemTime.wSecond);

            }

        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}




