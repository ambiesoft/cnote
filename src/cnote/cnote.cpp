#include "stdafx.h"

#include "../../../lsMisc/OpenCommon.h"
#include "../../../lsMisc/GetLastErrorString.h"
#include "../../../lsMisc/I18N.h"
#include "../../../lsMisc/FindTopWindowFromPID.h"
#include "../../../lsMisc/CHandle.h"
#include "../../../lsMisc/GetChildWindowBy.h"
#include "../../../lsMisc/UTF16toUTF8.h"
#include "../../../lsMisc/stdosd/stdosd.h"

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

#define CNOTE_APPNAME L"cnote"
#define CNOTE_VERSION L"1.0.3"

void ShowErrorAndExit(const wstring& message)
{
	wcerr << message << endl;
	exit(1);
}
void ShowVersionAndExit()
{
	wcout << CNOTE_APPNAME << L" v" << CNOTE_VERSION << endl;
	exit(0);
}
void ShowHelpAndExit()
{
	wcout << 
R"(cnote can be used as if 'more' or 'less'.

ex:
    dir | cnote

options:
	-w : Convert to Windows-styled newline
)" << endl;
	exit(0);
}

void RETRUN_WITH_ERROR_WITHLASTERROR(LPCWSTR ERRROSTRING, DWORD dwLE)
{
	ShowErrorAndExit(I18N(ERRROSTRING) +
		wstring(L":") + GetLastErrorString(dwLE));
}
void RETRUN_WITH_ERROR(LPCWSTR ERRROSTRING)
{
	DWORD dwLE = GetLastError();
	RETRUN_WITH_ERROR_WITHLASTERROR(ERRROSTRING, dwLE);
}

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
	for (int i = 1; i < argc; ++i)
	{
		if(false){}
		else if (strcmp(argv[i], "-W") == 0)
			bCRLF = true;
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
		{
			ShowVersionAndExit();
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			ShowHelpAndExit();
		}
	}

	string all;
	char buffer[4096];
	size_t dwReaded = 0;
    setvbuf(stdin, nullptr, _IONBF, 0);

	do {
        dwReaded = fread(buffer, 1, _countof(buffer)-1, stdin);
		if(dwReaded ==0 )
			break;  // End of Stream
		buffer[dwReaded] = 0;
		all += buffer;
    } while (!feof(stdin));

	// MessageBoxA(nullptr, all.c_str(), nullptr, MB_ICONINFORMATION);

    // how to handle window handle in linux?
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

	DWORD dwLE = 0;
	HWND hEditNotepad = GetChildWindowByClassName(hNotepad, L"Edit", &dwLE);
	if (!IsWindow(hEditNotepad))
	{
		RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");
	}
	//vector<HWND> hhh = GetChildWindowsByClassName(hNotepad, L"Edit");
	//HWND hEditNotepad = hhh.empty() ? NULL : *hhh.begin();
	//if (!hEditNotepad)
	//{
	//	RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");
	//}

	if (FALSE == SendMessage(hEditNotepad, WM_SETTEXT, 0,
		(LPARAM)(toStdWstringFromUtf8(bCRLF ? toCRLF(all) : all).c_str())))
	{
		RETRUN_WITH_ERROR(L"Failed to set text on notepad");
	}
	return 0;
}
