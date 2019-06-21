#include "stdafx.h"

#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/I18N.h"
#include "../../lsMisc/FindTopWindowFromPID.h"
#include "../../lsMisc/CHandle.h"
#include "../../lsMisc/GetChildWindowBy.h"
#include "../../lsMisc/UTF16toUTF8.h"
#include "../../lsMisc/stdosd/stdosd.h"

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

void ShowErrorAndExit(const wstring& message)
{
	wcerr << message << endl;
	exit(1);
}

#define RETRUN_WITH_ERROR(ERRROSTRING) do {			\
	DWORD dwLE = GetLastError();					\
	ShowErrorAndExit(I18N(ERRROSTRING) +			\
	wstring(L":") + GetLastErrorString(dwLE));		\
} while(false)

string toCRLF(const string& all)
{
	string ret = stdStringReplace(all, "\r\n", "\n");
	ret = stdStringReplace(ret, "\r", "\n");
	ret = stdStringReplace(ret, "\n", "\r\n");

	return ret;
}

int main(int argc, const char* argv[])
{
	bool bCRLF = false;
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-W") == 0)
			bCRLF = true;
	}

	string all;
	char buffer[4096];
	DWORD dwReaded = 0;
	do {
		if (!ReadFile(GetStdHandle(STD_INPUT_HANDLE),
			buffer,
			_countof(buffer) - 1,
			&dwReaded,
			NULL))
		{
			// RETRUN_WITH_ERROR(L"Failed to Read from stdin");
			break;
		}
	
		if(dwReaded ==0 )
			break;  // End of Stream

		buffer[dwReaded] = 0;
		all += buffer;
	} while (true);

	// MessageBoxA(nullptr, all.c_str(), nullptr, MB_ICONINFORMATION);

	CHandle process = nullptr;
	if (!OpenCommon(nullptr, L"notepad", nullptr, nullptr, &process))
	{
		RETRUN_WITH_ERROR(L"Failed to open notepad");
	}

	if (WAIT_OBJECT_0 != WaitForInputIdle(process, INFINITE))
	{
		RETRUN_WITH_ERROR(L"Failed to wait notepad");
	}

	HWND hNotepad = FindTopWindowFromPID(GetProcessId(process));
	if (!IsWindow(hNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to wait notepad");
	}

	HWND hEditNotepad = GetChildWindowByClassName(hNotepad, L"Edit");
	if (!IsWindow(hEditNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");
	}


	if (FALSE == SendMessage(hEditNotepad, WM_SETTEXT, 0,
		(LPARAM)(toStdWstringFromUtf8(bCRLF ? toCRLF(all) : all).c_str())))
	{
		RETRUN_WITH_ERROR(L"Failed to set text on notepad");
	}
	return 0;
}
