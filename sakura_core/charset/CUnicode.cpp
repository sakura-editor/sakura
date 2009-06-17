//	この行は文字化け対策用です．消さないでください

#include "stdafx.h"
#include "CUnicode.h"
#include "codechecker.h"
#include "mem/CMemory.h"


EConvertResult CUnicode::_UnicodeToUnicode_in( CMemory* pMem, const bool bBigEndian )
{
	// ソース取得
	int nSrcLen;
	unsigned char* pSrc = reinterpret_cast<unsigned char*>( pMem->GetRawPtr(&nSrcLen) );

	if( bBigEndian ){
		pMem->SwapHLByte();  // UnicodeBe -> Unicode
	}

	if( nSrcLen % 2 == 1 ){
		// 最後の1バイトを U+0000 から U+00FF までにマップする。
		pMem->AllocBuffer( nSrcLen + 1 );
		if( pMem->GetRawPtr() != NULL ){
			pSrc[nSrcLen] = 0;
			pMem->_SetRawLength( nSrcLen + 1 );
		}

		return RESULT_LOSESOME;
	}
	return RESULT_COMPLETE;
}


EConvertResult CUnicode::_UnicodeToUnicode_out( CMemory* pMem, const bool bBigEndian )
{
	if( bBigEndian == true ){
		pMem->SwapHLByte();   // Unicode -> UnicodeBe
	}

	return RESULT_COMPLETE;   // 何もしない
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
