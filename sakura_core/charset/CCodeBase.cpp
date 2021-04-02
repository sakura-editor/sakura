﻿/*! @file */
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
#include "CCodeBase.h"
#include "charcode.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "CEol.h"
#include "env/CommonSetting.h"

void CCodeBase::GetBom(CMemory* pcmemBom){ pcmemBom->Clear(); }					//!< BOMデータ取得

// 表示用16表示	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	if (IsUTF16High(cSrc[0]) && iSLen >= 2 && IsUTF16Low(cSrc[1])) {
		// サロゲートペア
		if (psStatusbar->m_bDispSPCodepoint) {
			auto_sprintf( pDst, L"U+%05X", 0x10000 + ((cSrc[0] & 0x3FF)<<10) + (cSrc[1] & 0x3FF));
		}
		else {
			auto_sprintf( pDst, L"%04X%04X", cSrc[0], cSrc[1]);
		}
	}
	else {
		auto_sprintf( pDst, L"U+%04X", cSrc[0] );
	}

	return RESULT_COMPLETE;
}

/*!
	MIME デコーダー

	@param[out] pcMem デコード済みの文字列を格納
*/
bool CCodeBase::MIMEHeaderDecode( const char* pSrc, const int nSrcLen, CMemory* pcMem, const ECodeType eCodetype )
{
	ECodeType ecodetype;
	int nskip_bytes;

	// ソースを取得
	pcMem->AllocBuffer( nSrcLen );
	char* pdst = reinterpret_cast<char*>( pcMem->GetRawPtr() );
	if( pdst == NULL ){
		pcMem->SetRawData( "", 0 );
		return false;
	}

	CMemory cmembuf;
	int i = 0;
	int j = 0;
	while( i < nSrcLen ){
		if( pSrc[i] != '=' ){
			pdst[j] = pSrc[i];
			++i;
			++j;
			continue;
		}
		nskip_bytes = _DecodeMimeHeader( &pSrc[i], nSrcLen-i, &cmembuf, &ecodetype );
		if( nskip_bytes < 1 ){
			pdst[j] = pSrc[i];
			++i;
			++j;
		}else{
			if( ecodetype == eCodetype ){
				// eChartype が ecodetype と一致している場合にだけ、
				// 変換結果をコピー
				memcpy( &pdst[j], cmembuf.GetRawPtr(), cmembuf.GetRawLength() );
				i += nskip_bytes;
				j += cmembuf.GetRawLength();
			}else{
				memcpy( &pdst[j], &pSrc[i], nskip_bytes );
				i += nskip_bytes;
				j += nskip_bytes;
			}
		}
	}

	pcMem->_SetRawLength( j );
	return true;
}

/*!
	改行データ取得
*/
// CShiftJisより移動 2010/6/13 Uchi
void CCodeBase::S_GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	static const struct{
		const char* szData;
		int nLen;
	}
	aEolTable[EOL_TYPE_NUM] = {
		{ "",			0 },	// EEolType::none
		{ "\x0d\x0a",	2 },	// EEolType::cr_and_lf
		{ "\x0a",		1 },	// EEolType::line_feed
		{ "\x0d",		1 },	// EEolType::carriage_return
		{ "",			0 },	// EEolType::next_line
		{ "",			0 },	// EEolType::line_separator
		{ "",			0 },	// EEolType::paragraph_separator
	};
	auto& data = aEolTable[static_cast<size_t>(eEolType)];
	pcmemEol->SetRawData(data.szData, data.nLen);
}
