/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CUnicode.h"
#include "codechecker.h"
#include "mem/CMemory.h"
#include "CEol.h"

EConvertResult CUnicode::_UnicodeToUnicode_in( const CMemory& cSrc, CNativeW* pDstMem, const bool bBigEndian )
{
	// ソース取得
	int nSrcLen = cSrc.GetRawLength();
	const unsigned char* pSrc = reinterpret_cast<const unsigned char*>( cSrc.GetRawPtr() );
	CMemory* pDstMem2 = pDstMem->_GetMemory();

	EConvertResult res = RESULT_COMPLETE;
	bool bCopy = false;
	if( nSrcLen % 2 == 1 ){
		// 不足分の最終1バイトとして 0x00 を補う。
		pDstMem2->AllocBuffer( nSrcLen + 1 );
		unsigned char* pDst  = reinterpret_cast<unsigned char*>( pDstMem2->GetRawPtr() );
		if( pDstMem2->GetRawPtr() != NULL ){
			if( &cSrc != pDstMem2 ){
				pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
				bCopy = true;
			}
			pDst[nSrcLen] = 0;
			pDstMem2->_SetRawLength(nSrcLen + 1);
			res = RESULT_LOSESOME;
		}else{
			return RESULT_FAILURE;
		}
	}

	if( bBigEndian ){
		if( &cSrc != pDstMem2 && !bCopy ){
			// コピーしつつ UnicodeBe -> Unicode
			pDstMem2->SwabHLByte(cSrc);
		}else{
			pDstMem2->SwapHLByte();  // UnicodeBe -> Unicode
		}
	}else if( !bCopy ){
		pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
	}
	return res;
}

EConvertResult CUnicode::_UnicodeToUnicode_out( const CNativeW& cSrc, CMemory* pDstMem, const bool bBigEndian )
{
	if( bBigEndian == true ){
		if( cSrc._GetMemory() == pDstMem ){
			pDstMem->SwapHLByte();   // Unicode -> UnicodeBe
		}else{
			pDstMem->SwabHLByte(*(cSrc._GetMemory()));
		}
	}else{
		if( cSrc._GetMemory() != pDstMem ){
			pDstMem->SetRawDataHoldBuffer(*(cSrc._GetMemory()));
		}else{
			// 何もしない
		}
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
		{ L"",			0 * sizeof(wchar_t) },	// EOL_NONE
		{ L"\x0d\x0a",	2 * sizeof(wchar_t) },	// EOL_CRLF
		{ L"\x0a",		1 * sizeof(wchar_t) },	// EOL_LF
		{ L"\x0d",		1 * sizeof(wchar_t) },	// EOL_CR
		{ L"\x85",		1 * sizeof(wchar_t) },	// EOL_NEL
		{ L"\u2028",	1 * sizeof(wchar_t) },	// EOL_LS
		{ L"\u2029",	1 * sizeof(wchar_t) },	// EOL_PS
	};
	pcmemEol->SetRawData(aEolTable[eEolType].pData,aEolTable[eEolType].nLen);
}
