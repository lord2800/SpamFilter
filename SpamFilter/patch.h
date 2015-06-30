#include <windows.h>
#include "intercepts.h"

enum PatchType {
	Call,
	Jump,
	Bytes
};

struct Patch {
	char* dll;
	DWORD offset;
	short length;
	void* func;
	PatchType type;
	BYTE* oldCode;
};

Patch patches[] = {
	{"D2Client.dll", 0x83301, 5, GamePacketReceived_Intercept, Call}, //1.13d
	{"D2Client.dll", 0xB24FF, 5, GameInput_Intercept, Call}, //1.13d
	{"D2Multi.dll", 0x11D63, 5, ChannelInput_Intercept, Call}, //1.13d
	{"D2Multi.dll", 0x14A9A, 5, ChannelWhisper_Intercept, Call}, //1.13d
	{"D2Multi.dll", 0x14BE0, 6, ChannelChat_Intercept, Jump}, //1.13d
	{"D2Multi.dll", 0x14850, 6, ChannelEmote_Intercept, Jump}, //1.13d
	{0}
};
