#include <iostream>

#include <windows.h>

using namespace std;

#define MAX_CLIENTS 4

// clients queue
HANDLE clients_queue;

// output state
HANDLE output;

int qs = 0;

[[noreturn]] void barber()
{
    while (true)
	{
        WaitForSingleObject(clients_queue, INFINITE);
		WaitForSingleObject(output, INFINITE);
		if (qs == 0) cout << "queue is EMPTY\n";
		else {
            qs--;
		    cout << "Barber served the client \nqueue length:" << qs << endl;
		}
        ReleaseSemaphore(output, 1, nullptr);
        Sleep(3000);
        ReleaseSemaphore(clients_queue, -1, nullptr);
	}
}

DWORD WINAPI client()
{
    //we check if queue is available and add 1 client
	if (ReleaseSemaphore(clients_queue, 1, nullptr))
	{
		WaitForSingleObject(output, INFINITE);
		qs++;
		cout << "One more client is waiting to be served \nqueue length:" << qs << endl;
		ReleaseSemaphore(output, 1, nullptr);
	}else
	{
		WaitForSingleObject(output, INFINITE);
		cout << "queue is FULL, so barber does not serve the client" << endl;
		ReleaseSemaphore(output, 1, nullptr);
	}
	return 0;
}


int main()
{
	clients_queue = CreateSemaphore(nullptr, 0, MAX_CLIENTS, reinterpret_cast<LPCSTR>((LPCWSTR) "queue"));
	output = CreateSemaphore(nullptr, 1, 1, reinterpret_cast<LPCSTR>((LPCWSTR) "output"));

	// barber thread
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)barber, 0, 0, nullptr);

	// we create the clients
	while (true)
	{
	    // client thread
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)client, 0, 0,nullptr);
        Sleep(1000);
	}

    return 0;
}

