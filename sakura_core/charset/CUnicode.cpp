/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CUnicode.h"
#include "codechecker.h"
#include "mem/CMemory.h"

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
			pDstMem2->SetRawDataHoldBuffer( cSrc );
		}
		pDstMem2->SwapHLByte();  // UnicodeBe -> Unicode
	}else if( !bCopy ){
		pDstMem2->SetRawDataHoldBuffer(pSrc, nSrcLen);
	}
	return res;
}

EConvertResult CUnicode::_UnicodeToUnicode_out( const CNativeW& cSrc, CMemory* pDstMem, const bool bBigEndian )
{
	if( bBigEndian == true ){
		if( cSrc._GetMemory() != pDstMem ){
			pDstMem->SetRawDataHoldBuffer( *(cSrc._GetMemory()) );
		}
		pDstMem->SwapHLByte();   // Unicode -> UnicodeBe
	}else{
		if( cSrc._GetMemory() != pDstMem ){
			pDstMem->SetRawDataHoldBuffer(*(cSrc._GetMemory()));
		}else{
			// 何もしない
		}
	}

	return RESULT_COMPLETE;   // 何もしない
}
