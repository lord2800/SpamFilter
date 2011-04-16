#include <windows.h>
#include "patch.h"

#define _DEFINE_VARS

#include "Pointers.h"

#define INST_INT3	0xCC
#define INST_CALL	0xE8
#define INST_NOP	0x90
#define INST_JMP	0xE9
#define INST_RET	0xC3

void PatchCall(DWORD dwAddr, DWORD dwFunc, DWORD dwLen);
void PatchJmp(DWORD dwAddr, DWORD dwFunc, DWORD dwLen);
void PatchBytes(DWORD dwAddr, DWORD dwValue, DWORD dwLen);
BOOL WriteBytes(void *pAddr, void *pData, DWORD dwLen);


DWORD GetDllOffset(char *DllName, int Offset)
{
	HMODULE hMod = GetModuleHandle(DllName);

	if(!hMod)
		hMod = LoadLibrary(DllName);

	if(!hMod)
		return 0;

	return (Offset < 0 ? (DWORD)GetProcAddress(hMod, (LPCSTR)(-Offset)) : ((DWORD)hMod)+Offset);
}

DWORD GetDllOffset(int num)
{
	static char *dlls[] = {"D2Client.DLL", "D2Common.DLL", "D2Gfx.DLL", "D2Lang.DLL", 
		"D2Win.DLL", "D2Net.DLL", "D2Game.DLL", "D2Launch.DLL", "Fog.DLL", "BNClient.DLL",
		"Storm.DLL", "D2Cmp.DLL", "D2Multi.DLL"};
	if((num&0xff) > 12)
		return 0;
	return GetDllOffset(dlls[num&0xff], num>>8);
}

void InstallPatches(void)
{
	for(int x = 0; patches[x].dll != NULL; x++)
	{
		patches[x].oldCode = new BYTE[patches[x].length];
		memset(patches[x].oldCode, 0, patches[x].length);
		void* addr = (void*)GetDllOffset(patches[x].dll, patches[x].offset);
		::ReadProcessMemory(GetCurrentProcess(), addr, patches[x].oldCode, patches[x].length, NULL);
		switch(patches[x].type)
		{
			case Call: PatchCall((DWORD)addr, (DWORD)patches[x].func, patches[x].length); break;
			case Jump: PatchJmp((DWORD)addr, (DWORD)patches[x].func, patches[x].length); break;
			case Bytes: PatchBytes((DWORD)addr, (DWORD)patches[x].func, patches[x].length); break;
		}
	}
}

void RemovePatches(void)
{
	for(int x = 0; patches[x].oldCode != NULL; x++)
	{
		void* addr = (void*)GetDllOffset(patches[x].dll, patches[x].offset);
		WriteBytes(addr, patches[x].oldCode, patches[x].length);
		delete[] patches[x].oldCode;
		patches[x].oldCode = NULL;
	}
}

void InitOffsets(void)
{
	DWORD *p = (DWORD *)&_D2PTRS_START;
	do {
		*p = GetDllOffset(*p);
	} while(++p <= (DWORD *)&_D2PTRS_END);
}

BOOL WriteBytes(void *pAddr, void *pData, DWORD dwLen)
{
	DWORD dwOld;

	if(!VirtualProtect(pAddr, dwLen, PAGE_READWRITE, &dwOld))
		return FALSE;

	::memcpy(pAddr, pData, dwLen);
	return VirtualProtect(pAddr, dwLen, dwOld, &dwOld);
}

void InterceptLocalCode(BYTE bInst, DWORD pAddr, DWORD pFunc, DWORD dwLen)
{
	BYTE *bCode = new BYTE[dwLen];
	::memset(bCode, 0x90, dwLen);
	DWORD dwFunc = pFunc - (pAddr + 5);

	bCode[0] = bInst;
	*(DWORD *)&bCode[1] = dwFunc;
	WriteBytes((void*)pAddr, bCode, dwLen);

	delete[] bCode;
}

void PatchCall(DWORD dwAddr, DWORD dwFunc, DWORD dwLen)
{
	InterceptLocalCode(INST_CALL, dwAddr, dwFunc, dwLen);
}

void PatchJmp(DWORD dwAddr, DWORD dwFunc, DWORD dwLen)
{
	InterceptLocalCode(INST_JMP, dwAddr, dwFunc, dwLen);
}

void PatchBytes(DWORD dwAddr, DWORD dwValue, DWORD dwLen)
{
	BYTE *bCode = new BYTE[dwLen];
	::memset(bCode, (BYTE)dwValue, dwLen);

	WriteBytes((LPVOID)dwAddr, bCode, dwLen);

	delete[] bCode;
}
