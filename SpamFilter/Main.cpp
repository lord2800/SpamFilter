#define DEBUG

#include <windows.h>
#include <shlwapi.h>
#include "Offsets.h"
#include "Bayes.h"
#include "handlers.h"
#include <WinInet.h>

#ifdef DEBUG
#include "D2Loader.h"
#endif

#pragma comment(lib, "shlwapi")

#define _MAX_FILE _MAX_PATH+_MAX_FNAME

Bayes bayes;
char *log, *db, *ini, *path, *url;
wchar_t cmdprefix;
bool logchat = true, logblocks = false, enabled = true, autosave = true;

struct Module
{	
	union {
		HMODULE hModule;
		DWORD dwBaseAddress;
	};
	DWORD _1;
	char szPath[MAX_PATH];
};


BOOL WINAPI DllMain(HMODULE hDll, DWORD Reason, LPVOID Reserved)
{
	switch(Reason)
	{
		case DLL_PROCESS_ATTACH:
			path = new char[_MAX_FILE];
			log = new char[_MAX_FILE];
			db = new char[_MAX_FILE];
			ini = new char[_MAX_FILE];
			url = new char[INTERNET_MAX_URL_LENGTH];

			if(Reserved != NULL)
			{
				Module* pModule = (Module*)Reserved;

				if(!pModule)
					return FALSE;

				strcpy_s(path, MAX_PATH, pModule->szPath);
			}
			else
			{
				DisableThreadLibraryCalls(hDll);

				GetModuleFileName(hDll, path, MAX_PATH);
				PathRemoveFileSpec(path);
			}

			sprintf_s(ini, _MAX_FILE, "%s\\spamfilter.ini", path);

			char logfile[255], tokendb[255],  blogchat[3], blogblocks[3], benabled[3], bautosave[3], cmdpref[2];
			GetPrivateProfileString("SpamFilter", "UpdateUrl", "http://lord2800.dyndns-free.com/sbupdate.php", url, INTERNET_MAX_URL_LENGTH, ini);
			GetPrivateProfileString("SpamFilter", "ChatLog", "chatlog.txt", logfile, 255, ini);
			GetPrivateProfileString("SpamFilter", "TokenDatabase", "tokens.db", tokendb, 255, ini);
			GetPrivateProfileString("SpamFilter", "LogChat", "on", blogchat, 3, ini);
			GetPrivateProfileString("SpamFilter", "LogBlocks", "of", blogblocks, 3, ini);
			GetPrivateProfileString("SpamFilter", "Enabled", "on", benabled, 3, ini);
			GetPrivateProfileString("SpamFilter", "Autosave", "on", bautosave, 3, ini);
			GetPrivateProfileString("SpamFilter", "CommandPrefix", ".", cmdpref, 2, ini);
			MultiByteToWideChar(CP_ACP, 0, cmdpref, -1, &cmdprefix, 1);

			logchat = StringToBool(blogchat);
			logblocks = StringToBool(blogblocks);
			enabled = StringToBool(benabled);
			autosave = StringToBool(bautosave);

			sprintf_s(db, _MAX_FILE, "%s\\%s", path, tokendb);
			sprintf_s(log, _MAX_FILE, "%s\\%s", path, logfile);

			// with cGuard, we will never get a call to Start() so we have to do it manually
			if(Reserved != NULL) Start();
			break;
		case DLL_PROCESS_DETACH:
			delete[] log;
			delete[] db;
			delete[] path;
			Stop();
			break;
	}
	return TRUE;
}

extern "C"
{

__declspec(dllexport) BOOL WINAPI Start(void)
{
	InitOffsets();
	InstallPatches();
	bayes = Bayes(db);
	return TRUE;
}

__declspec(dllexport) BOOL WINAPI Stop(void)
{
	RemovePatches();
	return TRUE;
}

};