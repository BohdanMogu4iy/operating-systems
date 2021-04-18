#include <windows.h>
#include <stdio.h>
#include "psapi.h"
#include <algorithm>
#include <vector>
#include <iostream>
#include <clocale>

using namespace std;

struct ProcessInfo
{
    DWORD processId;
    WCHAR processName[MAX_PATH];
    WCHAR processUser[MAX_PATH];
    WCHAR processUserDomain[MAX_PATH];
    DWORD processState;
    DWORDLONG MemoryPercent;
    DOUBLE CpuTimePercent;
};

struct TimeInfo
{
    FILETIME processKernelTime = { 0,0 };
    FILETIME processUserTime = { 0,0 };
    FILETIME systemKernelTime = { 0,0 };
    FILETIME systemUserTime = { 0,0 };
};

void GetTime(DWORD processID, TimeInfo* timeInfo)
{
    FILETIME idleTimeSys;
    FILETIME systemKernelTime;
    FILETIME systemUserTime;

    FILETIME processCreationTime;
    FILETIME processExitTime;
    FILETIME processKernelTime;
    FILETIME processUserTime;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
    if (hProcess != NULL)
    {
        if (!GetProcessTimes(hProcess, &processCreationTime, &processExitTime, &processKernelTime, &processUserTime)) return;
        if (!GetSystemTimes(&idleTimeSys, &systemKernelTime, &systemUserTime)) return;

        timeInfo->systemKernelTime = systemKernelTime;
        timeInfo->systemUserTime = systemUserTime;
        timeInfo->processKernelTime = processKernelTime;
        timeInfo->processUserTime = processUserTime;
    }
}

ProcessInfo setProcessInfo(DWORD Id) {
    ProcessInfo current;

    WCHAR name[MAX_PATH] = L"";
    WCHAR userName[MAX_PATH] = L"";
    WCHAR userDomain[MAX_PATH] = L"";
    DWORD state = 0;
    DWORDLONG workingSetSize = 0;

    PROCESS_MEMORY_COUNTERS memory;

    current.processId = Id;
    current.processState = state;
    current.MemoryPercent = workingSetSize;
    current.CpuTimePercent = 0;
    wcscpy_s(current.processName, name);
    wcscpy_s(current.processUser, userName);
    wcscpy_s(current.processUserDomain, userDomain);


    // Get handle to process
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
        FALSE, Id);

    if (hProcess != NULL)
    {
        GetExitCodeProcess(hProcess, &state);

        DWORD size = MAX_PATH;

        GetProcessImageFileName(hProcess, name, MAX_PATH);

        HANDLE tokenHandle;
        if (OpenProcessToken(hProcess, TOKEN_READ, &tokenHandle))
        {
            PTOKEN_USER user = NULL;
            DWORD length = 0;
            GetTokenInformation(tokenHandle, TokenUser, user, length, &length);
            user = (PTOKEN_USER)GlobalAlloc(GPTR, length);
            if (user)
            {
                if (GetTokenInformation(tokenHandle, TokenUser, user, length, &length))
                {
                    SID_NAME_USE sidType;
                    LookupAccountSid(NULL, user->User.Sid,
                        userName, &size,
                        userDomain, &size, &sidType);
                }
            }
            if (user)
                GlobalFree(user);
            CloseHandle(tokenHandle);
        }
        if (GetProcessMemoryInfo(hProcess, &memory, sizeof(memory)))
        {
            workingSetSize = memory.WorkingSetSize;
        }

        current.processId = Id;
        current.processState = state;
        current.MemoryPercent = workingSetSize;

        memset(current.processName, 0, sizeof(current.processName) / sizeof(*current.processName));
        int pos = 0;
        for (int j = wcslen(name) - 1; j >= 0; j--) {
            if (name[j] == '\\') {
                pos = j + 1;
                break;
            }
        }
        if (pos != 0) {
            for (int j = pos; j < wcslen(name); j++) {
                current.processName[j - pos] = name[j];
            }
            wcout << endl;
        }
        if (wcslen(current.processName) > 35) {
            current.processName[35] = '\0';
            current.processName[34] = current.processName[33] = current.processName[32] = '.';
        }

        wcscpy_s(current.processUser, userName);
        wcscpy_s(current.processUserDomain, userDomain);

        CloseHandle(hProcess);
    }
    else {
        current.processId = 0;
    }

    return current;
}

void PrintProcesses(vector<ProcessInfo>& processes, int freq)
{
    wprintf(L"------------------------------------------------------------------------------------------------------------------------------------------\n");
    wprintf(L"| %-13s | %-35s | %-13s | %-13s | %-13s | %-9s | %-9s |\n",
        L"Id", L"Name", L"User", L"Domain", L"State", L"Memory%", L"Time%");
    wprintf(L"------------------------------------------------------------------------------------------------------------------------------------------\n");

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (!GlobalMemoryStatusEx(&statex))
        return;
    DWORDLONG tph = statex.ullTotalPhys;

    WCHAR name[MAX_PATH];
    int max_size = 35;

    for (DWORD i = 0; i < processes.size(); i++)
    {
        wprintf(L"| %13u | %-35s | %-13s | %-13s | %-13d | %8.3lf%% | %8.3lf%% |\n",
            processes[i].processId, processes[i].processName, processes[i].processUser, processes[i].processUserDomain,
            processes[i].processState, processes[i].MemoryPercent * 100.00 / tph,
            processes[i].CpuTimePercent);
        wprintf(L"------------------------------------------------------------------------------------------------------------------------------------------\n");
    }

    wcout << "\nTable wil be refreshed in " << freq << " seconds." << endl;
    Sleep(freq * 1000);
}

void PrintMemoryInfo()
{
    SYSTEM_INFO sysInf;
    GetSystemInfo(&sysInf);
    wprintf(L"Number of processors:\t\t\t%u\n", sysInf.dwNumberOfProcessors);
    wprintf(L"The page size:\t%u\n", sysInf.dwPageSize);
    MEMORYSTATUSEX statex;
    DWORDLONG DIV = 1024;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    wprintf(L"Percent of memory in use:\t\t%u %%\n", statex.dwMemoryLoad);
    wprintf(L"Total physical memory:\t\t\t%llu KB\n", statex.ullTotalPhys / DIV);
    wprintf(L"Free physical memory:\t\t\t%llu KB\n", statex.ullAvailPhys / DIV);
    wprintf(L"Total paging file:\t\t\t%llu KB\n", statex.ullTotalPageFile / DIV);
    wprintf(L"Free paging file:\t\t\t%llu KB\n", statex.ullAvailPageFile / DIV);
    wprintf(L"Total virtual memory:\t\t\t%llu KB\n", statex.ullTotalVirtual / DIV);
    wprintf(L"Free virtual memory:\t\t\t%llu KB\n", statex.ullAvailVirtual / DIV);
}

ULONGLONG operator -(const FILETIME& filetimeA, const FILETIME& filetimeB)
{
    LARGE_INTEGER a, b;

    a.LowPart = filetimeA.dwLowDateTime;
    a.HighPart = filetimeA.dwHighDateTime;

    b.LowPart = filetimeB.dwLowDateTime;
    b.HighPart = filetimeB.dwHighDateTime;
    return a.QuadPart - b.QuadPart;
}

int choosesort = 1;

bool compareprocinfo(const ProcessInfo& a, const ProcessInfo& b) {

    switch (choosesort) {
    case 1: 
    {
        wstring nameA = wstring(a.processName);
        wstring nameB = wstring(b.processName);
        return nameA < nameB;
    }
    case 2: return a.processId > b.processId;
    case 3: {
        wstring userA = wstring(a.processUser);
        wstring userB = wstring(b.processUser);
        return userA < userB;
    }
    case 4: {
        wstring userDomainA = wstring(a.processUserDomain);
        wstring userDomainB = wstring(b.processUserDomain);
        return userDomainA < userDomainB;
    }
    case 5: return a.MemoryPercent > b.MemoryPercent;
    case 6: return a.CpuTimePercent > b.CpuTimePercent;
    default: return a.processId > b.processId;
    }
}

void GetProcessesInfo(vector<ProcessInfo>& processes) {

    processes.clear();
    DWORD freq = 0;
    system("cls");
    wprintf(L"Enter the pause duration (in seconds)\n");
    wcin >> freq;

    DWORD processesID[1024], bytesReturned, returnedProcesses;
    while (true)
    {
        if (!EnumProcesses(processesID, sizeof(processesID), &bytesReturned))
            return;

        returnedProcesses = bytesReturned / sizeof(DWORD);

        processes.reserve(returnedProcesses + 1);

        TimeInfo* OldTime = new TimeInfo[returnedProcesses];
        TimeInfo* NewTime = new TimeInfo[returnedProcesses];

        for (DWORD i = 0; i <= returnedProcesses; i++)
        {
            ProcessInfo proc = setProcessInfo(processesID[i]);

            if (proc.processId != 0) {
                processes.push_back(proc);
            }
        }
        sort(processes.begin(), processes.end(), compareprocinfo);
        for (DWORD i = 0; i < processes.size(); i++)
        {
            GetTime(processes[i].processId, OldTime + i);
        }
        Sleep(1000);
        for (DWORD i = 0; i < processes.size(); i++)
        {
            GetTime(processes[i].processId, NewTime + i);
        }
        for (DWORD i = 0; i < processes.size(); i++)
        {
            processes[i].CpuTimePercent = 100 * (double)((NewTime[i].processKernelTime - OldTime[i].processKernelTime) + (NewTime[i].processUserTime - OldTime[i].processUserTime)) /
                (double)((NewTime[i].systemKernelTime - OldTime[i].systemKernelTime) + (NewTime[i].systemUserTime - OldTime[i].systemUserTime));
        }

        system("cls");
        PrintProcesses(processes, freq);
        delete[] OldTime;
        delete[] NewTime;
    }
}

void main()
{
    setlocale(LC_ALL, "Russian");
    vector<ProcessInfo> processes;
    int menuItem = 0;
    while (true)
    {
        switch (menuItem)
        {
        case 1:
        {
            system("cls");
            int secondMenuItem = 10;
            choosesort = 1;
            while (secondMenuItem != 0)
            {
                switch (secondMenuItem)
                {
                case 9:
                {
                    cout << "Exit" << endl;
                    return;
                }
                break;
                default:
                {
                    cout << "Please, choose sort method \n\t1 - Name \n\t2 - Id  \n\t3 - User \n\t4 - Domain \n\t5 - Memory  \n\t6 - Time \n\t0 - Main menu \n\t9 - Exit \n";
                    cin >> secondMenuItem;
                    int types[] = {1, 2, 3, 4, 5, 6};
                    if ((find(begin(types), end(types), secondMenuItem) != end(types))){
                        choosesort = secondMenuItem;
                        GetProcessesInfo(processes);
                    }
                }
                break;
                }
            }
            menuItem = 0;
        }
        break;
        case 2:
        {
            system("cls");
            PrintMemoryInfo();
            int secondMenuItem = 10;
            while (secondMenuItem != 0)
            {
                switch (secondMenuItem)
                {
                case 9:
                {
                    cout << "Exit" << endl;
                    return;
                }
                break;
                default:
                {
                    cout << "Please, choose action \n\t0 - Main menu \n\t9 - Exit \n";
                    cin >> secondMenuItem;
                }
                break;
                }
            }
            menuItem = 0;
        }
        break;
        case 9:
        {
            cout << "Exit" << endl;
            return;
        }
        break;
        case 0:
        {
            system("cls");
            cout << "\nChoose action \n\t1 - View process table \n\t2 - View memory info  \n\t9 - Exit \n";
            cin >> menuItem;
        }
        break;
        }
    }
    return;
}