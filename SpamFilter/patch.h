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
	{"D2Client.dll", 0xACE61, 5, GamePacketReceived_Intercept, Call},
	{"D2Client.dll", 0x70B75, 5, GameInput_Intercept, Call},
	{"D2Multi.dll", 0xD753, 5, ChannelInput_Intercept, Call},
	{"D2Multi.dll", 0x10781, 5, ChannelWhisper_Intercept, Call},
	{"D2Multi.dll", 0x108A0, 6, ChannelChat_Intercept, Jump},
	{"D2Multi.dll", 0x107A0, 6, ChannelEmote_Intercept, Jump},
	{0}
};
