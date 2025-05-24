/*!	@file
	@brief Latin1 (Latin1, 欧文, Windows-1252, Windows Codepage 1252 West European) 対応クラス

	@author Uchi
	@date 20010/03/20 新規作成
*/
/*
	Copyright (C) 20010, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CLATIN1_5A6263F4_6468_4F9A_BFAE_69DC9477C1B5_H_
#define SAKURA_CLATIN1_5A6263F4_6468_4F9A_BFAE_69DC9477C1B5_H_
#pragma once

#include "CCodeBase.h"
#include "charset/codechecker.h"

class CLatin1 : public CCodeBase{

public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst) override{ return Latin1ToUnicode(cSrc, pDst); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst) override{ return UnicodeToLatin1(cSrc, pDst); }	//!< UNICODE    → 特定コード 変換
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar) override;			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult Latin1ToUnicode(const CMemory& cSrc, CNativeW* pDstMem);		// Latin1   → Unicodeコード変換
	static EConvertResult UnicodeToLatin1(const CNativeW& cSrc, CMemory* pDstMem);		// Unicode  → Latin1コード変換
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字が何バイト文字かを返す

protected:
	// 実装
	static int Latin1ToUni( const char *pSrc, const int nSrcLen, wchar_t *pDst, bool* pbError );
	inline static int _UniToLatin1_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError );
	static int UniToLatin1( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool *pbError );
};

/*!
	UNICODE -> Latin1 一文字変換

	eCharset は CHARSET_UNI_NORMAL または CHARSET_UNI_SURROG。

	高速化のため、インライン化
*/
inline int CLatin1::_UniToLatin1_char( const unsigned short* pSrc, unsigned char* pDst, const ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror = false;
	BOOL blost;

	if( eCharset == CHARSET_UNI_NORMAL ){
		if ((pSrc[0] >= 0 && pSrc[0] <= 0x7f) || (pSrc[0] >= 0xa0 && pSrc[0] <= 0xff)) {
			// ISO 58859-1の範囲
			pDst[0] = (unsigned char)pSrc[0];
			nret = 1;
		} else {
			// ISO 8859-1以外
			nret = ::WideCharToMultiByte( 1252, 0, reinterpret_cast<const wchar_t*>(pSrc), 1, reinterpret_cast<char*>(pDst), 4, NULL, &blost );
			if( blost != FALSE ){
				// Uni -> CLatin1 変換に失敗
				berror = true;
				pDst[0] = '?';
				nret = 1;
			}
		}
	}else if( eCharset == CHARSET_UNI_SURROG ){
		// サロゲートペアは CLatin1 に変換できない。
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}else{
		// 保護コード
		berror = true;
		pDst[0] = '?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}
#endif /* SAKURA_CLATIN1_5A6263F4_6468_4F9A_BFAE_69DC9477C1B5_H_ */
