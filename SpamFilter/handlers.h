#include <windows.h>

BOOL __fastcall ChatHandler(char* szAcc, char* szText);
DWORD __fastcall GamePacketReceived(BYTE* pPacket, DWORD dwSize);
DWORD __fastcall GameInput(wchar_t* wMsg);
DWORD __fastcall ChannelInput(wchar_t* wMsg);
bool StringToBool(char* value);
