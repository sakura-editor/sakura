/*!	@file
	@brief Latin1 (Latin1, 欧文, Windows-1252, Windows Codepage 1252 West European) 対応クラス

	@author Uchi
	@date 20010/03/20 新規作成
*/
/*
	Copyright (C) 20010, Uchi

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
#include "CLatin1.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"
#include "CEol.h"
//#include "env/CShareData.h"
#include "env/CommonSetting.h"



//! 指定した位置の文字が何バイト文字かを返す
/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)
	@retval 1  1バイト文字
	@retval 0  エラー

	@date 2010/3/20 Uchi 作成

	エラーでなければ1を返す
*/
int CLatin1::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen ){
		return 0;
	}
	return 1;
}




/*!
	Latin1 → Unicode 変換
*/
int CLatin1::Latin1ToUni( const char *pSrc, const int nSrcLen, wchar_t *pDst, bool* pbError )
{
	int nret;
	const unsigned char *pr, *pr_end;
	unsigned short *pw;

	if( pbError ){
		*pbError = false;
	}
	if( nSrcLen < 1 ){
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>( pSrc );
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; pr < pr_end; pr++ ){
		if (*pr >= 0x80 && *pr <=0x9f) {
			// Windows 拡張部
			nret = ::MultiByteToWideChar( 1252, 0, reinterpret_cast<const char*>(pr), 1, reinterpret_cast<wchar_t*>(pw), 4 );
			if( nret == 0 ){
				*pw = static_cast<unsigned short>( *pr );
			}
			pw++;
		}
		else {
			*pw++ = static_cast<unsigned short>( *pr );
		}
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



/* コード変換 Latin1→Unicode */
EConvertResult CLatin1::Latin1ToUnicode( const CMemory& cSrc, CNativeW* pDstMem )
{
	// エラー状態
	bool bError;

	//ソース取得
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );

	// 変換先バッファサイズを設定してメモリ領域確保
	wchar_t* pDst = new (std::nothrow) wchar_t[nSrcLen];
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = Latin1ToUni( pSrc, nSrcLen, pDst, &bError );

	// pDstMemを更新
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	// 後始末
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}







/*
	Unicode -> Latin1
*/
int CLatin1::UniToLatin1( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool *pbError )
{
	int nclen;
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	ECharSet echarset;
	bool berror=false, berror_tmp;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc+nSrcLen);
	pw = reinterpret_cast<unsigned char*>(pDst);

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset, 0)) > 0 ){
		// 保護コード
		switch( echarset ){
		case CHARSET_UNI_NORMAL:
			nclen = 1;
			break;
		case CHARSET_UNI_SURROG:
			nclen = 2;
			break;
		default:
			echarset = CHARSET_BINARY;
			nclen = 1;
		}
		if( echarset != CHARSET_BINARY ){
			pw += _UniToLatin1_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}else{
			if( nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>(TextToBin(*pr) & 0x000000ff);
				++pw;
			}else{
				berror = true;
				*pw = '?';
				++pw;
			}
			++pr;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}




/* コード変換 Unicode→Latin1 */
EConvertResult CLatin1::UnicodeToLatin1( const CNativeW& cSrc, CMemory* pDstMem )
{
	// 状態
	bool berror;

	// ソース取得
	const wchar_t* pSrc = cSrc.GetStringPtr();
	int nSrcLen = cSrc.GetStringLength();

	// 変換先バッファサイズを設定してバッファを確保
	char* pDst = new (std::nothrow) char[ nSrcLen * 2 ];
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = UniToLatin1( pSrc, nSrcLen, pDst, &berror );

	// pDstMemを更新
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// 後始末
	delete[] pDst;

	// 結果
	if( berror == true ){
		return RESULT_LOSESOME;
	}else{
		return RESULT_COMPLETE;
	}
}


// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CLatin1::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	CNativeW		cCharBuffer;
	EConvertResult	res;
	int				i;
	unsigned char*	ps;
	TCHAR*			pd;
	bool			bbinary=false;

	// 2008/6/21 Uchi
	if (psStatusbar->m_bDispUniInSjis) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	cCharBuffer.SetString(cSrc, 1);

	if( IsBinaryOnSurrogate(cSrc[0]) ){
		bbinary = true;
	}

	// Latin1 変換
	res = UnicodeToLatin1(cCharBuffer, cCharBuffer._GetMemory());
	if (res != RESULT_COMPLETE) {
		return RESULT_LOSESOME;
	}

	// Hex変換
	ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cCharBuffer._GetMemory()->GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, _T("%02x"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02x"), *ps );
	}

	return RESULT_COMPLETE;
}
