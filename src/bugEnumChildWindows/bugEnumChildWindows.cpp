// bugEnumChildWindows.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

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
int main()
{
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
	HWND hNotepad = allTops.empty() ? nullptr : *allTops.begin();
	if (!IsWindow(hNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to find notepad");
	}

	HWND hEditNotepad = GetChildWindowByClassName(hNotepad, L"Edit");
	if (!IsWindow(hEditNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");
	}

	wcout << L"Succeeded!" << endl;
	return 0;
}
