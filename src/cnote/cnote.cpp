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
#include "../../../lsMisc/IsWindowsVersion.h"
#include "../../../lsMisc/CommandLineParser.h"

#include "../../../profile/cpp/Profile/include/ambiesoft.profile.h"

#include "detect.h"

#define SECTION_OPTION "Option"
#define KEY_DEFAULT_VIEWER "DefaultViewer"

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

#define CNOTE_APPNAME L"cnote"
#define CNOTE_VERSION L"1.0.10"

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
//void ShowHelpAndExit()
//{
//	wcout << 
//R"(cnote can be used as if 'more' or 'less'.
//
//ex:
//    dir | cnote
//
//options:
//	-W : Convert to Windows-styled newline
//	-V -v --version: Shows version
//	-h --help : Shows help
//)" << endl;
//	exit(0);
//}

wstring GetIniFile()
{
	return stdCombinePath(
		stdGetParentDirectory(stdGetModuleFileName()),
		stdGetFileNameWitoutExtension(stdGetModuleFileName()) + L".ini");
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
	bool bVersion = false;
	bool bHelp = false;
	bool bGetDefaultViewer = false;
	wstring defaultViewerToSet;
	wstring viewer;

	const bool bIsWin11 = IsWindows11OrAbove();

	CCommandLineParser parser(I18N(L"Views standard outputs in a gui viewer"), CNOTE_APPNAME);

	parser.AddOption(L"-W",
		ArgCount::ArgCount_Zero,
		&bCRLF,
		ArgEncodingFlags_Default,
		I18N(L"Convert to Windows-styled newline"));
	parser.AddOption(L"--verbose",
		ArgCount::ArgCount_Zero,
		&bVerbose,
		ArgEncodingFlags_Default,
		I18N(L"Shows verbose outputs"));
	parser.AddOptionRange({ L"-v",L"-V",L"--version" },
		ArgCount::ArgCount_Zero,
		&bVersion,
		ArgEncodingFlags_Default,
		I18N(L"Shows version"));
	parser.AddOptionRange({ L"-h",L"--help",L"/?",L"/h" },
		ArgCount::ArgCount_Zero,
		&bHelp,
		ArgEncodingFlags_Default,
		I18N(L"Shows Help"));
	parser.AddOption(L"--get-default-viewer",
		ArgCount::ArgCount_Zero,
		&bGetDefaultViewer,
		ArgEncodingFlags_Default,
		I18N(L"Shows default viewer"));
	parser.AddOption(L"--set-default-viewer",
		ArgCount::ArgCount_One,
		&defaultViewerToSet,
		ArgEncodingFlags_Default,
		I18N(L"Sets default viewer, one of 'notepad' or 'txvr'"));
	parser.AddOption(L"--viewer",
		ArgCount::ArgCount_One,
		&viewer,
		ArgEncodingFlags_Default,
		I18N(L"Viewer to use"));

	parser.Parse();

	if (parser.hadUnknownOption())
	{
		ShowErrorAndExit(stdFormat(L"Unknown option '%s'", parser.getUnknowOptionStrings().c_str()));
	}
	if (bVersion)
	{
		ShowVersionAndExit();
	}
	if (bHelp)
	{
		wcout << parser.getHelpMessage() << endl;
		return 0;
	}
	if (bGetDefaultViewer)
	{
		string dv;
		Profile::GetString(SECTION_OPTION, KEY_DEFAULT_VIEWER, "", dv, GetIniFile());
		if (dv.empty())
			dv = "notepad";
		cout << dv << endl;
		return 0;
	}
	if (!defaultViewerToSet.empty())
	{
		if (defaultViewerToSet != L"notepad" && defaultViewerToSet != L"txvr")
		{
			wcerr << I18N(L"Default viewer must be one of 'notepad' or 'txvr'.") << endl;
			return 1;
		}
		bool success = true;
		success &= Profile::WriteString(SECTION_OPTION, KEY_DEFAULT_VIEWER, 
			toStdUtf8String(defaultViewerToSet), GetIniFile());
		if (!success)
		{
			wcerr << I18N(L"Failed to save ini file.") << endl;
			return 1;
		}
		return 0;
	}
	if (viewer.empty())
	{
		string v;
		Profile::GetString(SECTION_OPTION, KEY_DEFAULT_VIEWER, "", v, GetIniFile());
		if (v.empty())
			v = "notepad";
		viewer = toStdWstringFromUtf8(v);
	}
	wstring viewerArg;
	if (viewer == L"txvr")
	{
		viewer = stdCombinePath(
			stdGetParentDirectory(stdGetModuleFileName()),
			viewer);
		viewerArg = L"--blank";
		bCRLF = true;
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
    CKernelHandle process;
	if (!OpenCommon(nullptr, viewer.c_str(), viewerArg.c_str(), nullptr, &process))
	{
		RETRUN_WITH_ERROR(stdFormat(I18N(L"Failed to open '%s'"), viewer.c_str()).c_str());
	}

	if (WAIT_OBJECT_0 != WaitForInputIdle(process, INFINITE))
	{
		RETRUN_WITH_ERROR(stdFormat(I18N(L"Failed to wait '%s'"),viewer.c_str()).c_str());
	}

	auto allTops = FindTopWindowFromPID(GetProcessId(process));
	if (allTops.empty())
	{
		RETRUN_WITH_ERROR(stdFormat(I18N(L"Failed to find '%s'"), viewer.c_str()).c_str());
	}

	HWND hEditNotepad = NULL;
	DWORD dwStartTick = GetTickCount();
	do
	{
		for (auto hNotepad : allTops)
		{
			HWND h = GetChildWindowByClassName(hNotepad, L"Edit");
			if (!h)
			{
				// In case of Windows 11's notepad
				h = GetChildWindowByClassName(hNotepad, L"RichEditD2DPT");
			}
			if (h)
			{
				hEditNotepad = h;
				break;
			}
		}
		if (!IsWindow(hEditNotepad))
		{
			if (!bIsWin11)
				RETRUN_WITH_ERROR(L"Failed to obtain edit control of notepad");

			// In windows11, it fails to find RichEdit control,
			// retry during 3000 milisec
			Sleep(500);
			if ((GetTickCount() - dwStartTick) > 3000)
				RETRUN_WITH_ERROR(L"Failed to obtain richedit control of notepad in 3000ms");
			continue;
		}
		break;
	} while (true);
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
	if (bIsWin11)
	{
		// TODO: Set modify to false, following code does not work
		
		//UINT ttt = SendMessage(hEditNotepad, EM_GETMODIFY, 0, 0);
		//SendMessage(hEditNotepad, EM_SETMODIFY, TRUE, FALSE);
		//ttt = SendMessage(hEditNotepad, EM_GETMODIFY, 0, 0);
		//SendMessage(hEditNotepad, EM_SETMODIFY, FALSE, FALSE);
		//ttt = SendMessage(hEditNotepad, EM_GETMODIFY, 0, 0);
	}
	return 0;
}
