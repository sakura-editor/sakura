//	この行は文字化け対策用です．消さないでください

#include "StdAfx.h"
#include "CUnicodeBe.h"
#include "CEol.h"

#include "codechecker.h"


void CUnicodeBe::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF16BE_BOM[]={0xFE,0xFF};
	pcmemBom->SetRawData(UTF16BE_BOM, sizeof(UTF16BE_BOM));
}

void CUnicodeBe::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",					0 * sizeof(wchar_t) },	// EOL_NONE
		{ "\x00\x0d\x00\x0a",	2 * sizeof(wchar_t) },	// EOL_CRLF
		{ "\x00\x0a",			1 * sizeof(wchar_t) },	// EOL_LF
		{ "\x00\x0d",			1 * sizeof(wchar_t) },	// EOL_CR
		{ "\x00\x85",			1 * sizeof(wchar_t) },	// EOL_NEL
		{ "\x20\x28",			1 * sizeof(wchar_t) },	// EOL_LS
		{ "\x20\x29",			1 * sizeof(wchar_t) },	// EOL_PS
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}
