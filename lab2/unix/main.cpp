#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <clocale>

#define MAX_PATH 260

using namespace std;

struct ProcessInfo
{
    uint processId = 0;
    char processName[MAX_PATH] = "-";
    uint processUser = 0;
    char processState = '-';
    ulong Memory = 0;
    double CpuTimePercent = 0;
};



struct TimeInfo

{

    ulong processTime = 0;

    ulong systemTime = 0;

};

void Skip(FILE* file, int count)

{

  if (file!=NULL)

  {

    for (int i = 0; i < count; i++)

    {

      fscanf(file, "%*s");

    }

  }

}

void GetTime(int processID, TimeInfo* time)

{

    FILE* file;

    char path[MAX_PATH] = "";

    sprintf(path,"%s%d%s", "/proc/", processID, "/stat");

    file = fopen(path, "r");

    if (file)

    {

        ulong userTime, kerTime;

        Skip(file, 13);

        fscanf(file, "%lu %lu", &userTime, &kerTime);

        time->processTime = userTime + kerTime;

        fclose(file);

    }

    file = fopen("/proc/stat", "r");

    if (file)

    {

        ulong userTime, niceTime, sysTime, idleTime;

        Skip(file, 1);

        fscanf(file,"%*s %lu %lu %lu %lu", &userTime, &niceTime, &sysTime, &idleTime);

        time->systemTime = userTime + niceTime + sysTime + idleTime;

        fclose(file);

    }

}



ProcessInfo setProcessInfo(int Id) {

    ProcessInfo current;



    current.processId = Id;

    FILE* file;

    char path[MAX_PATH] = "";

    sprintf(path,"%s%d%s", "/proc/", Id, "/stat");

    file = fopen(path, "r");

    if (file)

    {

        fscanf(file, "%d %s %c", &current.processId, &current.processName, &current.processState);

        fclose(file);

    }



    sprintf(path,"%s%d%s", "/proc/", Id, "/statm");

    file = fopen(path, "r");

    if (file)

    {

        fscanf(file, "%d", &current.Memory);

        fclose(file);

    }



    sprintf(path,"%s%d%s", "/proc/", Id, "/loginuid");

    file = fopen(path, "r");

    if (file)

    {

        fscanf(file, "%lu", &current.processUser);

        fclose(file);

    }



    return current;

}



string StateToString(char c)

{

  switch (c)

  {

  case 'R': return "Running";

  case 'S': return "Sleeping";

  case 'D': return "Waiting";

  case 'Z': return "Zombie";

  case 'T': return "Stopped";

  case 't': return "Stopped";

  case 'X': return "Dead";

  case 'x': return "Dead";

  case 'K': return "Wakekill";

  case 'W': return "Waking";

  case 'P': return "Parked";

  default: return "ERROR";

  }

}



void PrintProcesses(vector<ProcessInfo>& processes, int freq)

{

    printf("--------------------------------------------------------------------------------------------------------------------\n");

    printf("| %-13s | %-40s | %-13s | %-15s | %-9s | %-9s |\n",

            "Id", "Name", "User", "State", "Memory%", "Time%");

    printf("-----------------------------------------------------------------------------------------------------------------------\n");

    char name[MAX_PATH];

    string state;

    struct sysinfo mem;

    sysinfo (&mem);

    ulong tph = mem.totalram;

    for (int i = 0; i < processes.size(); i++)

    {

        if (strcmp(name, "-"))

        {

            state = StateToString(processes[i].processState);

            printf("| %13u | %-40s | %-13u | %-15s | %8.5lf%% | %8.5lf%% |\n",

                    processes[i].processId, processes[i].processName, processes[i].processUser, state, processes[i].Memory * 100.0 /tph,

                    processes[i].CpuTimePercent);

        }

    }

    printf("-----------------------------------------------------------------------------------------------------------------------\n");

    cout << "\nTable wil be refreshed in " << freq << " seconds." << endl;

    sleep(freq);

}



void PrintMemoryInfo()

{

    struct sysinfo mem;

    sysinfo (&mem);

    printf("Total physical Memory: %lu Kb \nUsed Memory: %lu Kb \nSize of swap buffer:%lu Kb \nSize of Virtual Page: %u Kb\n",

    mem.totalram,mem.totalram-mem.freeram,mem.totalswap,mem.mem_unit);

}



int choosesort = 1;



bool compareprocinfo(const ProcessInfo& a, const ProcessInfo& b) {



    switch (choosesort) {

    case 1: 

    {

        string nameA = string(a.processName);

        string nameB = string(b.processName);

        return nameA < nameB;

    }

    case 2: return a.processId > b.processId;

    case 3: {

        return a.processUser < b.processUser;

    }

    case 4: return a.Memory > b.Memory;

    case 5: return a.CpuTimePercent > b.CpuTimePercent;

    default: return a.processId > b.processId;

    }

}



bool GetPIDs(int PIDs[], int* length)

{

    DIR* directory;

    struct dirent* directoryInfo;

    directory = opendir("/proc");

    if (!directory)

    {

        return false;

    }

    int i = 0;

    while (directoryInfo = readdir(directory))

    {

        if (isdigit(directoryInfo->d_name[0]))

        {

            PIDs[i++] = atoi(directoryInfo->d_name);

        }

    }

    closedir(directory);

    *length = i;

    return true;

}



void GetProcessesInfo(vector<ProcessInfo>& processes) {



    processes.clear();

    int freq = 0;

    system("clear");

    printf("Enter the pause duration (in seconds)\n");

    cin >> freq;



    while (true)

    {

        int PIDs[1024];

        int size;

        if (GetPIDs(PIDs, &size))

        {

            TimeInfo* OldTime = new TimeInfo[size];

            TimeInfo* NewTime = new TimeInfo[size];

            for (int i = 0; i < size; i++)

            {

                ProcessInfo proc = setProcessInfo(PIDs[i]);

                processes.push_back(proc);

            }

            sort(processes.begin(), processes.end(), compareprocinfo);

            for (int i = 0; i < processes.size(); i++)

            {

                GetTime(processes[i].processId, OldTime + i);

            }

            sleep(1);

            for (int i = 0; i < processes.size(); i++)

            {

                GetTime(processes[i].processId, NewTime + i);

                processes[i].CpuTimePercent = 100.0 * (NewTime[i].processTime - OldTime[i].processTime) / (NewTime[i].systemTime - OldTime[i].systemTime);

            }



            delete[] OldTime;

            delete[] NewTime;

        }



        system("clear");

        PrintProcesses(processes, freq);

        

    }

}



int main()

{

    setlocale(LC_ALL, "en-EN.UTF-8");

    vector<ProcessInfo> processes;

    int menuItem = 0;

    while (true)

    {

        switch (menuItem)

        {

        case 1:

        {

            system("clear");

            int secondMenuItem = 10;

            choosesort = 1;

            while (secondMenuItem != 0)

            {

                switch (secondMenuItem)

                {

                case 9:

                {

                    cout << "Exit" << endl;

                    return 0;

                }

                break;

                default:

                {

                    cout << "Please, choose sort method \n\t1 - Name \n\t2 - Id  \n\t3 - User \n\t4 - Memory  \n\t5 - Time \n\t0 - Main menu \n\t9 - Exit \n";

                    cin >> secondMenuItem;

                    int types[] = {1, 2, 3, 4, 5};

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

            system("clear");

            PrintMemoryInfo();

            int secondMenuItem = 10;

            while (secondMenuItem != 0)

            {

                switch (secondMenuItem)

                {

                case 9:

                {

                    cout << "Exit" << endl;

                    return 0;

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

            return 0;

        }

        break;

        case 0:

        {

            system("clear");

            cout << "\nChoose action \n\t1 - View process table \n\t2 - View memory info  \n\t9 - Exit \n";

            cin >> menuItem;

        }

        break;

        }

    }

    return 0;

}