#include <iostream>
#include <cstdlib>
#include <ctime>
#include <windows.h>

using namespace std;

#define READERS_COUNT 24
#define WRITERS_COUNT 2

CRITICAL_SECTION reader_enter_exit, data, output;

int readers = 0;
bool priority_flag = true;

DWORD WINAPI reader(LPVOID lpParam)
{
    int id = *(DWORD *)lpParam;

    while (true)
    {
        if (priority_flag) continue;
        EnterCriticalSection(&reader_enter_exit);
        if (readers == 0) EnterCriticalSection(&data);
        readers++;
        LeaveCriticalSection(&reader_enter_exit);
        EnterCriticalSection(&output);
        cout << "Reader : " << id << " got an access\n";
        cout << readers << " readers have an access to the data\n";
        LeaveCriticalSection(&output);
        Sleep(1000 * (rand() % 5));
        EnterCriticalSection(&reader_enter_exit);
        readers--;
        EnterCriticalSection(&output);
        cout << "Reader : " << id << " lost an access\n";
        cout << readers << " readers have an access to the data\n";
        LeaveCriticalSection(&output);
        if (readers == 0) LeaveCriticalSection(&data);
        LeaveCriticalSection(&reader_enter_exit);
    }

    return 0;
}

DWORD WINAPI writer(LPVOID lpParam)
{
    int id = *(DWORD *)lpParam;

    while (true)
    {
        priority_flag = true;
        EnterCriticalSection(&data);
        priority_flag = false;
        EnterCriticalSection(&output);
        cout << "Writer : " << id << " got an access\n";
        LeaveCriticalSection(&output);
        Sleep(500 * (rand() % 5));
        LeaveCriticalSection(&data);
    }

    return 0;
}

int main()
{
    srand(time(nullptr));
    InitializeCriticalSection(&reader_enter_exit);
    InitializeCriticalSection(&data);
    InitializeCriticalSection(&output);
    DWORD ReadersThreadIdArray[READERS_COUNT];
    DWORD WritersThreadIdArray[WRITERS_COUNT];

    for (int i = 0; i < WRITERS_COUNT; ++i)
    {
        WritersThreadIdArray[i] = i + 1;
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)writer, &WritersThreadIdArray[i], 0, nullptr);
    }

    for (int i = 0; i < READERS_COUNT; ++i)
    {
        ReadersThreadIdArray[i] = i + 1;
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)reader, &ReadersThreadIdArray[i], 0, nullptr);
    }

    while(true) {}
}
