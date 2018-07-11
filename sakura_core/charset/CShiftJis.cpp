#include "StdAfx.h"
#include "CShiftJis.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


//! 指定した位置の文字が何バイト文字かを返す
/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)
	@retval 1  1バイト文字
	@retval 2  2バイト文字
	@retval 0  エラー

	@date 2005-09-02 D.S.Koba 作成

	@note nIdxは予め文字の先頭位置とわかっていなければならない．
	2バイト文字の2バイト目をnIdxに与えると正しい結果が得られない．
*/
int CShiftJis::GetSizeOfChar( const char* pData, int nDataLen, int nIdx )
{
	if( nIdx >= nDataLen ){
		return 0;
	}else if( nIdx == (nDataLen - 1) ){
		return 1;
	}
	
	if( _IS_SJIS_1( reinterpret_cast<const unsigned char*>(pData)[nIdx] )
			&& _IS_SJIS_2( reinterpret_cast<const unsigned char*>(pData)[nIdx+1] ) ){
		return 2;
	}
	return 1;
}




/*!
	SJIS → Unicode 変換
*/
int CShiftJis::SjisToUni( const char *pSrc, const int nSrcLen, wchar_t *pDst, bool* pbError )
{
	ECharSet echarset;
	int nclen;
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	bool berror_tmp, berror=false;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>( pSrc );
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; (nclen = CheckSjisChar(reinterpret_cast<const char*>(pr), pr_end-pr, &echarset)) != 0; pr += nclen ){
		switch( echarset ){
		case CHARSET_ASCII7:
			// 保護コード
			if( nclen != 1 ){
				nclen = 1;
			}
			// 7-bit ASCII 文字を変換
			*pw = static_cast<unsigned short>( *pr );
			pw += 1;
			break;
		case CHARSET_JIS_ZENKAKU:
		case CHARSET_JIS_HANKATA:
			// 保護コード
			if( echarset == CHARSET_JIS_ZENKAKU && nclen != 2 ){
				nclen = 2;
			}
			if( echarset == CHARSET_JIS_HANKATA && nclen != 1 ){
				nclen = 1;
			}
			// 全角文字または半角カタカナ文字を変換
			pw += _SjisToUni_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			break;
		default:/* CHARSET_BINARY:*/
			if( nclen != 1 ){	// 保護コード
				nclen = 1;
			}
			// エラーが見つかった
			pw += BinToText( pr, nclen, pw );
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



/* コード変換 SJIS→Unicode */
EConvertResult CShiftJis::SJISToUnicode( const CMemory& cSrc, CNativeW* pDstMem )
{
	// エラー状態
	bool bError;

	//ソース取得
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );

	if( &cSrc == pDstMem->_GetMemory() )
	{
		// 変換先バッファサイズを設定してメモリ領域確保
		wchar_t* pDst;
		try{
			pDst = new wchar_t[nSrcLen];
		}catch( ... ){
			pDst = NULL;
		}
		if( pDst == NULL ){
			return RESULT_FAILURE;
		}

		// 変換
		int nDstLen = SjisToUni( pSrc, nSrcLen, pDst, &bError );

		// pDstを更新
		pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

		// 後始末
		delete [] pDst;
	}
	else
	{
		// 変換先バッファサイズを設定してメモリ領域確保
		pDstMem->AllocStringBuffer( nSrcLen + 1 );
		wchar_t* pDst = pDstMem->GetStringPtr();

		// 変換
		int nDstLen = SjisToUni( pSrc, nSrcLen, pDst, &bError );

		pDstMem->_SetStringLength( nDstLen );
	}

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}







/*
	Unicode -> SJIS
*/
int CShiftJis::UniToSjis( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool *pbError )
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
			pw += _UniToSjis_char( pr, pw, echarset, &berror_tmp );
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




/* コード変換 Unicode→SJIS */
EConvertResult CShiftJis::UnicodeToSJIS( const CNativeW& cSrc, CMemory* pDstMem )
{
	// 状態
	bool berror;
	const CMemory* pMem = cSrc._GetMemory();

	// ソース取得
	const wchar_t* pSrc = reinterpret_cast<const wchar_t*>( pMem->GetRawPtr() );
	int nSrcLen = pMem->GetRawLength() / sizeof(wchar_t);

	// 変換先バッファサイズを設定してバッファを確保
	char* pDst;
	try{
		pDst = new char[ nSrcLen * 2 ];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = UniToSjis( pSrc, nSrcLen, pDst, &berror );

	// pMemを更新
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
EConvertResult CShiftJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
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

	cCharBuffer.AppendString(cSrc, 1);

	if( IsBinaryOnSurrogate(cSrc[0]) ){
		bbinary = true;
	}

	// SJIS 変換
	res = UnicodeToSJIS(cCharBuffer, cCharBuffer._GetMemory());
	if (res != RESULT_COMPLETE) {
		return RESULT_LOSESOME;
	}

	// Hex変換
	ps = reinterpret_cast<unsigned char*>( cCharBuffer._GetMemory()->GetRawPtr() );
	pd = pDst;
	if( bbinary == false ){
		for (i = cCharBuffer._GetMemory()->GetRawLength(); i >0; i--, ps ++, pd += 2) {
			auto_sprintf( pd, _T("%02X"), *ps);
		}
	}else{
		auto_sprintf( pd, _T("?%02X"), *ps );
	}

	return RESULT_COMPLETE;
}
