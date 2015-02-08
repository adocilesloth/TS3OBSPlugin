/*****************************
2015 <adocilesloth@gmail.com>
*****************************/

#ifndef THREADHSAFE_H
#define THREADSAFE_H

#include "OBSApi.h"

class safebool
{
	DWORD WaitResult;
	HANDLE Mutex;
	bool val;

public:
	safebool()
	{
		val = false;
		Mutex = CreateMutex(NULL, false, NULL);
	}

	safebool(bool initaliser)
	{
		val = initaliser;
		Mutex = CreateMutex(NULL, false, NULL);
	}

	bool value()
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		bool b = val;
		ReleaseMutex(Mutex);
		return b;
	}

	void setvalue(bool nval)
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		val = nval;
		ReleaseMutex(Mutex);
	}

	void operator=(bool nval)
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		val = nval;
		ReleaseMutex(Mutex);
	}
};

class safeint
{
	DWORD WaitResult;
	HANDLE Mutex;
	int val;

public:
	safeint()
	{
		val = false;
		Mutex = CreateMutex(NULL, false, NULL);
	}

	safeint(int initaliser)
	{
		val = initaliser;
		Mutex = CreateMutex(NULL, false, NULL);
	}

	int value()
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		int i = val;
		ReleaseMutex(Mutex);
		return i;
	}

	void setvalue(int nval)
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		val = nval;
		ReleaseMutex(Mutex);
	}

	void operator=(int nval)
	{
		WaitResult = WaitForSingleObject(Mutex, INFINITE);
		val = nval;
		ReleaseMutex(Mutex);
	}
};

#endif //THREADSAFE_H