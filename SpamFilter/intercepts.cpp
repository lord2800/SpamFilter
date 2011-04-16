#include <windows.h>
#include "handlers.h"

#include "Pointers.h"

void __declspec(naked) GamePacketReceived_Intercept(void)
{
	__asm
	{
		pop ebp;
		pushad;

		call GamePacketReceived;
		test eax, eax;

		popad;
		jnz OldCode;

		mov edx, 0;

OldCode:
		call D2NET_ReceivePacket_I;

		push ebp;
		ret;
	}
}

void __declspec(naked) GameInput_Intercept(void)
{
	__asm {
		pushad
		mov ecx, ebx
		call GameInput
		cmp eax, -1
		popad
		je BlockIt
		call D2CLIENT_InputCall_I
		ret

BlockIt:
		xor eax,eax
		ret
	}
}

void __declspec(naked) ChannelInput_Intercept(void)
{
	__asm
	{
		push ecx
		mov ecx, esi

		call ChannelInput

		test eax, eax
		pop ecx

		jz SkipInput
		call D2MULTI_ChannelInput_I

SkipInput:
		ret
	}
}

void __declspec(naked) ChannelWhisper_Intercept(void)
{
	__asm
	{
		push ecx
		push edx
		mov ecx, edi
		mov edx, ebx

		call ChatHandler

		test eax, eax
		pop edx
		pop ecx

		jz SkipWhisper
		jmp D2MULTI_ChannelWhisper_I

SkipWhisper:
		ret 4
	}
}

void __declspec(naked) ChannelChat_Intercept(void)
{
	__asm
	{
		push ecx
		push edx
		mov ecx, dword ptr ss:[esp+0xC]
		mov edx, dword ptr ss:[esp+0x10]

		call ChatHandler

		test eax, eax
		pop edx
		pop ecx

		jz SkipChat
		sub esp, 0x408
		jmp D2MULTI_ChannelChat_I

SkipChat:
		ret 8
	}
}

void __declspec(naked) ChannelEmote_Intercept(void)
{
	__asm
	{
		push ecx
		push edx
		mov ecx, dword ptr ss:[esp+0xC]
		mov edx, dword ptr ss:[esp+0x10]

		call ChatHandler

		test eax, eax
		pop edx
		pop ecx

		jz SkipChat
		sub esp, 0x4F8
		jmp D2MULTI_ChannelEmote_I

SkipChat:
		ret 8
	}
}
