// bugEnumChildWindows.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cassert>

#include "../../../lsMisc/CHandle.h"
#include "../../../lsMisc/OpenCommon.h"
#include "../../../lsMisc/FindTopWindowFromPID.h"
#include "../../../lsMisc/GetChildWindowBy.h"

using namespace Ambiesoft;
using namespace std;

void RETRUN_WITH_ERROR(LPCWSTR p) {
	wcerr << p << endl;
	exit(1);
}

void PinUser32Dll()
{
	HMODULE h = LoadLibrary(L"User32.dll");
	assert(h);
}

static BOOL CALLBACK testEnumProc(HWND hwnd, LPARAM lParam)
{
	HWND* pHwnd = (HWND*)lParam;
	TCHAR szT[1024]; szT[0] = 0;
	GetClassName(hwnd, szT, _countof(szT));
	if (lstrcmp(szT, L"Edit") == 0)
	{
		*pHwnd = hwnd;
		return TRUE;
	}
	return FALSE;
}
int main()
{
	PinUser32Dll();

	CHandle process;
	if (!OpenCommon(nullptr, L"notepad", nullptr, nullptr, &process))
	{
		RETRUN_WITH_ERROR(L"Failed to open notepad");
	}

	if (WAIT_OBJECT_0 != WaitForInputIdle(process, INFINITE))
	{
		RETRUN_WITH_ERROR(L"Failed to wait notepad");
	}

	auto allTops = FindTopWindowFromPID(GetProcessId(process));
	HWND hEditNotepad = NULL;
	for (auto hNotepad : allTops)
	{
		HWND h = NULL;
		EnumChildWindows(hNotepad, testEnumProc, (LPARAM)&h);
		if (h)
		{
			hEditNotepad = h;
			break;
		}
	}
	if (!IsWindow(hEditNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");
	}

	wcout << L"Succeeded!" << endl;
	return 0;
}
