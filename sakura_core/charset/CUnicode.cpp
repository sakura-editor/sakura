//	‚±‚Ìs‚Í•¶Žš‰»‚¯‘Îô—p‚Å‚·DÁ‚³‚È‚¢‚Å‚­‚¾‚³‚¢

#include "stdafx.h"
#include "CUnicode.h"

EConvertResult CUnicode::UnicodeToUnicode( CMemory* pMem )
{
	//‰½‚à‚µ‚È‚¢
	return RESULT_COMPLETE;
}

void CUnicode::GetBom(CMemory* pcmemBom)
{
	static const BYTE UTF16LE_BOM[]={0xFF,0xFE};
	pcmemBom->SetRawData(UTF16LE_BOM, sizeof(UTF16LE_BOM));
}


void CUnicode::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const void* pData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		L"",			0 * sizeof(wchar_t),	// EOL_NONE
		L"\x0d\x0a",	2 * sizeof(wchar_t),	// EOL_CRLF
		L"\x0a",		1 * sizeof(wchar_t),	// EOL_LF
		L"\x0d",		1 * sizeof(wchar_t),	// EOL_CR
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}
