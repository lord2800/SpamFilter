#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <fstream>
#include <WinInet.h>
#include <sstream>

#include "Bayes.h"
#include "Pointers.h"

extern char *log, *path, *url;
extern wchar_t cmdprefix;
extern Bayes bayes;

extern bool logchat, logblocks, enabled, autosave;

#define _MAX_FILE _MAX_PATH+_MAX_FNAME

void Print(char* format, ...)
{
	va_list vaArgs;
	va_start(vaArgs, format);
	int len = _vscprintf(format, vaArgs)+1;
	char* str = new char[len];
	vsprintf_s(str, len, format, vaArgs);
	va_end(vaArgs);

	if(*p_D2CLIENT_PlayerUnit)
	{
		wchar_t* wstr = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, str, -1, wstr, len);
		D2CLIENT_PrintGameString(wstr, 0);
		delete[] wstr;
	}
	else // assume we're in channel
		D2MULTI_PrintChannelText(str, 0);

	delete[] str;
}

void __fastcall DoUpdate(bool display = false)
{
	HINTERNET hInt = InternetOpen("SpamFilter 1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);

	if(hInt == NULL) {
		if(display) Print(".: SpamFilter :. Update failed!");
		return;
	}

	char *upurl = new char[INTERNET_MAX_URL_LENGTH];
	memset(upurl, 0, INTERNET_MAX_URL_LENGTH);
	sprintf_s(upurl, INTERNET_MAX_URL_LENGTH, "%s?date=%d", url, bayes.GetLastUpdate());

	HINTERNET hFile = InternetOpenUrl(hInt, upurl, NULL, 0, INTERNET_FLAG_NO_UI|INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_RELOAD, NULL);
	delete[] upurl;
	if(hFile == NULL) {
		if(display) Print(".: SpamFilter :. Update failed!");
		return;
	}

	char buf[1024];
	std::stringstream result;
	BOOL done = FALSE;
	do {
		DWORD dummy;
		if((done = InternetReadFile(hFile, buf, 1024, &dummy)) == TRUE)
			result << buf;
		done = (done == TRUE && dummy == 0);
	} while(done);
	TokenList ham, spam;
	time_t updated;
	bayes.ParseUpdate(result, &updated, ham, spam);
	if(updated > bayes.GetLastUpdate()) {
		bayes.Merge(ham, Ham);
		bayes.Merge(spam, Spam);
		bayes.SaveDatabases();
		if(display) Print(".: SpamFilter :. Token database updated!");
	} else if(display) Print(".: SpamFilter :. You are already up to date!");

	InternetCloseHandle(hInt);
}

BOOL __fastcall ChatHandler(char* szAcc, char* szText)
{
	time_t currentTime;
	time(&currentTime);
	char szTime[60] = "";
	struct tm time;
	localtime_s(&time, &currentTime);
	strftime(szTime, sizeof(szTime), "%x %X", &time);

	int cat = bayes.Categorize(szText);
	bool block = cat == -1;

	if(logchat && (!block || (block && logblocks)))
	{
		FILE* chatlog = NULL;
		fopen_s(&chatlog, log, "a+");
		char* pos = strchr(szAcc, '*');
		if(pos != NULL && pos != szAcc)
			*pos = '\0';
		fprintf(chatlog, "[%s] %s%s: %s\n", szTime, (block ? "BLOCKING: " : ""), szAcc, szText);
		fclose(chatlog);
	}

	if(enabled && block)
		return FALSE;

	return TRUE;
}

bool StringToBool(char* value)
{
	switch(tolower(value[0]))
	{
		case 'T': case 't': case '1': return true;
		case 'F': case 'f': case '0': return false;
		case 'o':
			if(value[1] == 'n') return true;
			else if(value[1] == 'f') return false;
	}
	return false;
}

bool InputHandler(wchar_t* cmd, char* param) 
{
	bool parsed = false;

	if(_wcsicmp(cmd, L"spam") == 0)
	{
		parsed = true;
		int count = bayes.GetSpamTokenCount();
		bayes.AddSpam(param);
		count = bayes.GetSpamTokenCount() - count;
		Print(".: SpamFilter :. Added %d new spam tokens (previous token counts may have been updated)!", count);
		if(autosave)
			bayes.SaveDatabases();
	}
	else if(_wcsicmp(cmd, L"ham") == 0)
	{
		parsed = true;
		int count = bayes.GetHamTokenCount();
		bayes.AddHam(param);
		count = bayes.GetHamTokenCount() - count;
		Print(".: SpamFilter :. Added %d new ham tokens (previous token counts may have been updated)!", count);
		if(autosave)
			bayes.SaveDatabases();
	}
	else if(_wcsicmp(cmd, L"update") == 0)
	{
		parsed = true;
		DoUpdate(true);
	}
	else if(_wcsicmp(cmd, L"save") == 0)
	{
		parsed = true;
		bayes.SaveDatabases();
		Print(".: SpamFilter :. Saved databases!");
	}
	else if(_wcsicmp(cmd, L"loadfile") == 0)
	{
		parsed = true;
		char *buf = param, *ctx = NULL, *seps = " ";
		char* token = strtok_s(buf, seps, &ctx);
		char* file = buf+strlen(token)+1;
		bool spam = (_stricmp(token, "spam") == 0) ? true : false;
		bool ham = (_stricmp(token, "ham") == 0) ? true : false;
		int initialcount = (spam ? bayes.GetSpamTokenCount() : ham ? bayes.GetHamTokenCount() : -1);
		if((!spam && !ham) || (spam && ham))
			Print(".: SpamFilter :. Invalid option! You must specify whether the file is spam or ham.");
		else {
			int len = strlen(file);
			if(len > 1)
			{
				char fname[_MAX_FILE];
				sprintf_s(fname, _MAX_FILE, "%s\\%s", path, file);
				std::ifstream infile(fname);
				std::string s;
				while(!infile.eof())
				{
					std::getline(infile, s);
					if(s.size() < 1) continue;
					else if(spam) bayes.AddSpam(s);
					else if(ham) bayes.AddHam(s);
				}
				int finalcount = (spam ? bayes.GetSpamTokenCount() : ham ? bayes.GetHamTokenCount() : 0);
				Print(".: SpamFilter :. Added %d new tokens (previous token counts may have been updated)!", (finalcount - initialcount));
				if(autosave) bayes.SaveDatabases();
			}
			else
				Print(".: SpamFilter :. File '%s' not found!", file);
		}
	}
	else if(_wcsicmp(cmd, L"reload") == 0)
	{
		parsed = true;
		bayes.LoadDatabases();
		Print(".: SpamFilter :. Loaded databases! Total spam tokens: %d, ham tokens: %d", bayes.GetSpamTokenCount(), bayes.GetHamTokenCount());
	}
	else if(_wcsicmp(cmd, L"set") == 0)
	{
		parsed = true;
		if(strlen(param) == 0)
		{
			Print(".: SpamFilter :. Options are currently: enabled: %s, logchat: %s, logblocks: %s, autosave: %s",
					(enabled ? "on" : "off"), (logchat ? "on" : "off"), (logblocks ? "on" : "off"), (autosave ? "on" : "off"));
			return parsed;
		}
		char *buf = param, *ctx = NULL, *seps = " ";
		char* token = strtok_s(buf, seps, &ctx);
		char* value = buf+strlen(token)+1;
		int len = strlen(value);
		if(len > 1)
		{
			if(_stricmp(token, "enabled") == 0)
			{
				enabled = StringToBool(value);
				Print(".: SpamFilter :. SpamFilter is now %s", (enabled ? "enabled" : "disabled"));
			}
			else if(_stricmp(token, "logchat") == 0)
			{
				logchat = StringToBool(value);
				Print(".: SpamFilter :. SpamFilter is now %slogging chat", (logchat ? "" : "not "));
			}
			else if(_stricmp(token, "logblocks") == 0)
			{
				logblocks = StringToBool(value);
				Print(".: SpamFilter :. SpamFilter is now %slogging blocked messages (though chat logging may still be off!)", (logblocks ? "" : "not "));
			}
			else if(_stricmp(token, "autosave") == 0)
			{
				autosave = StringToBool(value);
				Print(".: SpamFilter :. SpamFilter is now %sautosaving the databases", (autosave ? "" : "not "));
			}
			else Print(".: SpamFilter :. Unknown option!");
		}
	}
	else if(_wcsicmp(cmd, L"help") == 0)
	{
		parsed = true;
		Print(".: SpamFilter :. Options");
		Print(".: SpamFilter :. .reload - Reload the spam and ham databases");
		Print(".: SpamFilter :. .save - Save the spam and ham databases");
		Print(".: SpamFilter :. .update - Check to see if there is a spam/ham database update");
		Print(".: SpamFilter :. .spam <string> - Add <string> to the spam database");
		Print(".: SpamFilter :. .ham <string> - Add <string> to the ham database");
		Print(".: SpamFilter :. .set <param> <value> - Set various parameters (true, TRUE, 1, and on are all enabled, false, FALSE, 0, and off are all disabled)");
		Print(".: SpamFilter :. Parameters for .set:");
		Print(".: SpamFilter :. enabled - Enable or disable spam blocking (default: true)");
		Print(".: SpamFilter :. logchat - Enable or disable chat logging (default: true)");
		Print(".: SpamFilter :. logblocks - Enable or disable logging blocked chat messages (default: false)");
		Print(".: SpamFilter :. autosave - Enable or disable auto-saving the spam and ham databases (default: true)");
	}

	return parsed;
}

DWORD __fastcall GamePacketReceived(BYTE* pPacket, DWORD dwSize)
{
	switch(pPacket[0])
	{
	case 0x26:
		char* pName = (char*)pPacket+10;
		char* pMessage = (char*)pPacket + strlen(pName) + 11;
		return ChatHandler(pName, pMessage);
	}

	return TRUE;
}

DWORD __fastcall GameInput(wchar_t* wMsg)
{
	bool hasCmd = wcslen(wMsg) > 1 && wMsg[0] == cmdprefix;
	if(hasCmd)
	{
		wchar_t *buf = wMsg+1, *ctx = NULL, *seps = L" ";
		wchar_t* token = wcstok_s(buf, seps, &ctx);
		wchar_t* wparam = buf+wcslen(token)+1;
		int len = wcslen(wparam)+1;
		if(len > 0)
		{
			char* param = new char[len];
			memset(param, 0, len);
			WideCharToMultiByte(CP_ACP, 0, wparam, -1, param, len, "?", NULL);
			if(!InputHandler(token, param))
				hasCmd = false;
			delete[] param;
		}
	}

	return hasCmd ? -1 : 0;
}

DWORD __fastcall ChannelInput(wchar_t* wMsg)
{
	bool hasCmd = wcslen(wMsg) > 1 && wMsg[0] == cmdprefix;
	if(hasCmd)
	{
		wchar_t *buf = wMsg+1, *ctx = NULL, *seps = L" ";
		wchar_t* token = wcstok_s(buf, seps, &ctx);
		wchar_t* wparam = buf+wcslen(token)+1;
		int len = wcslen(wparam)+1;
		if(len > 0)
		{
			char* param = new char[len];
			memset(param, 0, len);
			WideCharToMultiByte(CP_ACP, 0, wparam, -1, param, len, "?", NULL);
			if(!InputHandler(token, param))
				hasCmd = false;
			D2WIN_SetControlText(*p_D2WIN_ChatInputBox, L"");
			delete[] param;
		}
	}

	return hasCmd ? FALSE : TRUE;
}
