#include "stdafx.h"
#include <cassert>
#include "../../../lsMisc/OpenCommon.h"
#include "../../../lsMisc/GetLastErrorString.h"
#include "../../../lsMisc/I18N.h"
#include "../../../lsMisc/FindTopWindowFromPID.h"
#include "../../../lsMisc/CHandle.h"
#include "../../../lsMisc/GetChildWindowBy.h"
#include "../../../lsMisc/UTF16toUTF8.h"
#include "../../../lsMisc/stdosd/stdosd.h"


#include "detect.h"



using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

#define CNOTE_APPNAME L"cnote"
#define CNOTE_VERSION L"1.0.8"

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
	-W : Convert to Windows-styled newline
	-V -v --version: Shows version
	-h --help : Shows help
)" << endl;
	exit(0);
}

#define RETRUN_WITH_ERROR(ERRROSTRING) do {			\
	DWORD dwLE = GetLastError();					\
	ShowErrorAndExit(I18N(ERRROSTRING) +			\
	wstring(L":") + GetLastErrorString(dwLE));		\
} while(false)



int wmain(int argc, const wchar_t* argv[])
{
	bool bCRLF = false;
	bool bVerbose = false;
	for (int i = 1; i < argc; ++i)
	{
		if(false){}
		else if (wcscmp(argv[i], L"-W") == 0)
			bCRLF = true;
		else if (wcscmp(argv[i], L"-v") == 0 || wcscmp(argv[i], L"-V") == 0 || wcscmp(argv[i], L"--version") == 0)
		{
			ShowVersionAndExit();
		}
		else if (wcscmp(argv[i], L"-h") == 0 || wcscmp(argv[i], L"--help") == 0)
		{
			ShowHelpAndExit();
		}
		else if (wcscmp(argv[i], L"--verbose")==0)
		{
			bVerbose = true;
		}
		else
		{
			ShowErrorAndExit(stdFormat(L"Unknown option '%s'", argv[i]));
		}
	}

	vector<char> all;
	char buffer[4096];
	size_t dwReaded = 0;
    setvbuf(stdin, nullptr, _IONBF, 0);
	size_t totalRead = 0;
	do {
        dwReaded = fread(buffer, 1, _countof(buffer)-1, stdin);
		assert(dwReaded >= 0);
		totalRead += dwReaded;
		if(dwReaded ==0 )
			break;  // End of Stream
		buffer[dwReaded] = 0;
		all.insert(all.end(), buffer, buffer + dwReaded);
    } while (!feof(stdin));
	if (bVerbose)
	{
		wcout << L"Total read byte = " << totalRead << L"\n";
	}
	// add 4byte of 0
	all.insert(all.end(), { 0,0,0,0 });

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
	if (allTops.empty())
	{
		RETRUN_WITH_ERROR(L"Failed to find notepad");
	}

	HWND hEditNotepad = NULL;
	for (auto hNotepad : allTops)
	{
		HWND h = GetChildWindowByClassName(hNotepad, L"Edit");
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

	// convert encoding
	string encoding;
	wstring converted = ConvertEncoding(all, bVerbose ? &encoding : nullptr);
	if (bVerbose)
	{
		cout << "Detected encoding = " << encoding << "\n";
	}
	if (FALSE == SendMessage(hEditNotepad, WM_SETTEXT, 0,
		(LPARAM)(bCRLF ? stdToCRLFString(converted) : converted).c_str()))
	{
		RETRUN_WITH_ERROR(L"Failed to set text on notepad");
	}
	return 0;
}
