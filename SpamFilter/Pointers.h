#pragma once
#pragma warning ( push )
#pragma warning ( disable: 4245 )

#pragma optimize ( "", off )

#include "Bayes.h"

#ifdef _DEFINE_VARS

enum {DLLNO_D2CLIENT, DLLNO_D2COMMON, DLLNO_D2GFX, DLLNO_D2LANG, DLLNO_D2WIN, DLLNO_D2NET, DLLNO_D2GAME, DLLNO_D2LAUNCH, DLLNO_FOG, DLLNO_BNCLIENT, DLLNO_STORM, DLLNO_D2CMP, DLLNO_D2MULTI};

#define DLLOFFSET(a1,b1) ((DLLNO_##a1)|((b1)<<8))
#define FUNCPTR(d1,v1,t1,t2,o1)	typedef t1 d1##_##v1##_t t2; d1##_##v1##_t *d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define VARPTR(d1,v1,t1,o1)		typedef t1 d1##_##v1##_t;    d1##_##v1##_t *p_##d1##_##v1 = (d1##_##v1##_t *)DLLOFFSET(d1,o1);
#define ASMPTR(d1,v1,o1)			DWORD d1##_##v1 = DLLOFFSET(d1,o1);

#else

#define FUNCPTR(d1,v1,t1,t2,o1)	typedef t1 d1##_##v1##_t t2; extern d1##_##v1##_t *d1##_##v1;
#define VARPTR(d1,v1,t1,o1)		typedef t1 d1##_##v1##_t;    extern d1##_##v1##_t *p_##d1##_##v1;
#define ASMPTR(d1,v1,o1)			extern DWORD d1##_##v1;

extern Bayes bayes;

#endif

#define _D2PTRS_START D2NET_ReceivePacket_I

FUNCPTR(D2NET, ReceivePacket_I, void __stdcall, (BYTE *aPacket, DWORD aLen), -10033)
FUNCPTR(D2CLIENT, PrintGameString, void __stdcall, (wchar_t *wMessage, int nColor), 0x7D850)

ASMPTR(D2MULTI, Garbage, 0x0)
FUNCPTR(D2MULTI, PrintChannelText, void __stdcall, (char *szText, DWORD dwColor),  0xFC90)
FUNCPTR(D2WIN, SetControlText, void* __fastcall, (DWORD* box, wchar_t* txt), -10042)

VARPTR(D2CLIENT, PlayerUnit, DWORD*, 0x11BBFC)
VARPTR(D2WIN, ChatInputBox, DWORD*, 0x12A0D0)

ASMPTR(D2MULTI, ChannelChat_I, 0x108A6)
ASMPTR(D2MULTI, ChannelEmote_I, 0x107A6)
ASMPTR(D2MULTI, ChannelWhisper_I, 0x10000)
ASMPTR(D2MULTI, ChannelInput_I, 0xD5B0)
ASMPTR(D2CLIENT, InputCall_I, 0x147A0)

#define _D2PTRS_END D2CLIENT_InputCall_I