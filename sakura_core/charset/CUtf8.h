/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CUTF8_EDEFA3B8_A37B_4416_9EC5_0A16659C92059_H_
#define SAKURA_CUTF8_EDEFA3B8_A37B_4416_9EC5_0A16659C92059_H_

#include "CCodeBase.h"
#include "charset/codeutil.h"

struct CommonSetting_Statusbar;

class CUtf8 : public CCodeBase{
public:

	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< 特定コード → UNICODE    変換
		return UTF8ToUnicode(cSrc, pDst);
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    → 特定コード 変換
		return UnicodeToUTF8(cSrc, pDst);
	}
	void GetBom(CMemory* pcmemBom);																			//!< BOMデータ取得
	void GetEol(CMemory* pcmemEol, EEolType eEolType);
	EConvertResult _UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar, const bool CESU8Mode);			//!< UNICODE → Hex 変換
	EConvertResult UnicodeToHex(const wchar_t* ps, const int nsl, TCHAR* pd, const CommonSetting_Statusbar* psStatusbar){ return _UnicodeToHex(ps, nsl, pd, psStatusbar, false); }

public:
	// UTF-8 / CESU-8 <-> Unicodeコード変換
	//2007.08.13 kobake 追加
	//2009.01.08        CESU-8 に対応
	static EConvertResult _UTF8ToUnicode( const CMemory& cSrc, CNativeW* pDstMem, bool bCESU8Mode );
	static EConvertResult _UnicodeToUTF8( const CNativeW& cSrc, CMemory* pDstMem, bool bCESU8Mode );
	inline static EConvertResult UTF8ToUnicode( const CMemory& cSrc, CNativeW* pDst ){ return _UTF8ToUnicode(cSrc, pDst, false); }	// UTF-8 -> Unicodeコード変換
	inline static EConvertResult CESU8ToUnicode( const CMemory& cSrc, CNativeW* pDst ){ return _UTF8ToUnicode(cSrc, pDst, true); }	// CESU-8 -> Unicodeコード変換
	inline static EConvertResult UnicodeToUTF8( const CNativeW& cSrc, CMemory* pDst ){ return  _UnicodeToUTF8(cSrc, pDst, false); }	// Unicode → UTF-8コード変換
	inline static EConvertResult UnicodeToCESU8( const CNativeW& cSrc, CMemory* pDst ){ return _UnicodeToUTF8(cSrc, pDst, true); }	// Unicode → CESU-8コード変換

protected:

	//変換の実装
	// 2008.11.10 変換ロジックを書き直す
	inline static int _Utf8ToUni_char( const unsigned char*, const int, unsigned short*, bool bCESU8Mode );
	static int Utf8ToUni( const char*, const int, wchar_t*, bool bCESU8Mode );
	inline static int _UniToUtf8_char( const unsigned short*, const int, unsigned char*, const bool bCSU8Mode );
	static int UniToUtf8( const wchar_t*, const int, char*, bool* pbError, bool bCSU8Mode );
};



/*!
	UTF-8 の一文字変換

	UTF-8 と CESU-8 とを場合分けして、それぞれ変換する

	高速化のため、インライン化

*/
inline int CUtf8::_Utf8ToUni_char( const unsigned char* pSrc, const int nSrcLen, unsigned short* pDst, bool bCESUMode )
{
	int nret;

	if( nSrcLen < 1 ){
		return 0;
	}

	if( bCESUMode != true ){
		// UTF-8 の処理
		if( nSrcLen < 4 ){
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if( nSrcLen == 4 ){
			// UTF-8 サロゲート領域の処理
			wchar32_t wc32 = DecodeUtf8( pSrc, 4 );
			EncodeUtf16Surrog( wc32, pDst );
			nret = 2;
		}else{
			// 保護コード
			pDst[0] = L'?';
			nret = 1;
		}
	}else{
		// CESU-8 の処理
		if( nSrcLen < 4 ){
			pDst[0] = static_cast<unsigned short>(DecodeUtf8(pSrc, nSrcLen) & 0x0000ffff);
			nret = 1;
		}else if( nSrcLen == 6 ){
			// CESU-8 サロゲート領域の処理
			pDst[0] = static_cast<unsigned short>( DecodeUtf8(&pSrc[0], 3) & 0x0000ffff );
			pDst[1] = static_cast<unsigned short>( DecodeUtf8(&pSrc[3], 3) & 0x0000ffff );
			nret = 2;
		}else{
			// 保護コード
			pDst[0] = L'?';
			nret = 1;
		}
	}

	return nret;
}



/*!
	Unicode -> UTF-8 の一文字変換

	nSrcLen は 1 または 2

	高速化のため、インライン化
*/
inline int CUtf8::_UniToUtf8_char( const unsigned short* pSrc, const int nSrcLen, unsigned char* pDst, bool bCESU8Mode )
{
	int nret;

	if( nSrcLen < 1 ){
		return 0;
	}

	if( bCESU8Mode != true ){
		// UTF-8 の処理
		wchar32_t wc32;
		if( nSrcLen == 2 ){
			wc32 = DecodeUtf16Surrog( pSrc[0], pSrc[1] );
		}else if( nSrcLen == 1 ){	// nSrcLen == 1
			wc32 = pSrc[0];
		}else{
			wc32 = L'?';
		}
		nret = EncodeUtf8( wc32, &pDst[0] );
	}else{
		// CESU-8 の処理
		int nclen = 0;
		nclen += EncodeUtf8( pSrc[0], &pDst[0] );
		if( nSrcLen == 2 ){
			nclen += EncodeUtf8( pSrc[1], &pDst[nclen] );
		}else{
			;
		}
		nret = nclen;
	}

	return nret;
}

#endif /* SAKURA_CUTF8_EDEFA3B8_A37B_4416_9EC5_0A16659C92059_H_ */
/*[EOF]*/
