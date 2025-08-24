#pragma once

#include "windef.h"
#include "process.h"

bool Wait(int sec)
{
	clock_t beg = clock(), now = clock();
	while (now - beg <= sec * CLOCKS_PER_SEC)
	{
		now = clock();
		Sleep(50);
	}
	return true;
};

bool CheckSelfExists(HANDLE& hMutex, const char* MutexName) {

	hMutex = CreateMutexA(NULL, true, MutexName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return true;
	}
	return false;
}

void MBX(const char* str)
{
	MessageBoxA(NULL, str, "Notice", MB_OK | MB_SYSTEMMODAL);
};

class SimpleThread
{
public:
	HANDLE hThread;
	unsigned int  threadId;

	SimpleThread()
	{
		this->hThread = NULL;
		this->threadId = NULL;
	};

	HANDLE StartThreadEx(unsigned WINAPI Func(LPVOID p), LPVOID p)
	{
		this->hThread = (HANDLE)_beginthreadex(NULL, 0, Func, p, 0, &(this->threadId));
		return this->hThread;
	};

	HANDLE StartThread(void(*Func)(void*), LPVOID p)
	{
		this->hThread = (HANDLE)_beginthread(Func, 0, p);
		return this->hThread;
	};

};
