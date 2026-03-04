/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CCodeBase.h"

#include "charset/CCodeFactory.h"
#include "convert/convert_util2.h"
#include "charset/codechecker.h"
#include "basis/CEol.h"
#include "env/CommonSetting.h"

// 表示用16進表示	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CCodeBase::UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	// IVS
	if (iSLen >= 3 && IsVariationSelector(cSrc + 1)) {
		if (psStatusbar->m_bDispSPCodepoint) {
			auto_sprintf(pDst, L"%04X, U+%05X", cSrc[0], ConvertToUtf32(cSrc + 1));
		}
		else {
			auto_sprintf(pDst, L"%04X, %04X%04X", cSrc[0], cSrc[1], cSrc[2]);
		}
	}
	// サロゲートペア
	else if (iSLen >= 2 && IsSurrogatePair(cSrc)) {
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
	if( pdst == nullptr ){
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
 * BOMデータ取得
 *
 * ByteOrderMarkに対する特定コードによるバイナリ表現を取得する。
 * マルチバイトなUnicode文字セットのバイト順を識別するのに使う。
 */
void CCodeBase::GetBom(CMemory* pcmemBom)
{
	// ByteOrderMarkを特定コードに変換
	if (pcmemBom && RESULT_COMPLETE != UnicodeToCode(CNativeW{ &WCODE::BOM, 1 }, pcmemBom)) {
		// 変換できなかったらリセットする
		pcmemBom->Reset();
	}
}

/*!
 * 改行データ取得
 *
 * 指定した行終端子に対する特定コードによるバイナリ表現を取得する。
 * コードポイントとバイナリシーケンスが1対1に対応付けられる文字コードの改行を検出するのに使う。
 */
void CCodeBase::GetEol(CMemory* pcmemEol, EEolType eEolType)
{
	// 指定された行終端子を取得
	const CEol cEol{ eEolType };

	// 行終端子（UNICODE文字列）を特定コードに変換
	if (pcmemEol && 0 < cEol.GetLen() && RESULT_COMPLETE != UnicodeToCode(CNativeW{ cEol.GetValue2(), size_t(cEol.GetLen()) }, pcmemEol)) {
		// 変換できなかったらリセットする
		pcmemEol->Reset();
	}
}
