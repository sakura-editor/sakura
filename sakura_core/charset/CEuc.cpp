#include "StdAfx.h"
#include "CEuc.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"

/*!
	EUCJP → Unicode 変換関数
*/
int CEuc::EucjpToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError )
{
	const unsigned char *pr, *pr_end;
	unsigned short *pw;
	int nclen;
	ECharSet echarset;
	bool berror_tmp, berror=false;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);

	for( ; (nclen = CheckEucjpChar(reinterpret_cast<const char*>(pr), pr_end-pr, &echarset)) != 0; pr += nclen ){
		switch( echarset ){
		case CHARSET_ASCII7:
			// 保護コード
			if( nclen != 1 ){
				nclen = 1;
			}
			// 7-bit ASCII の変換
			*pw = *pr;
			++pw;
			break;
		case CHARSET_JIS_HANKATA:
		case CHARSET_JIS_ZENKAKU:
			// 保護コード
			if( echarset == CHARSET_JIS_HANKATA && nclen != 2 ){
				nclen = 2;
			}
			if( echarset == CHARSET_JIS_ZENKAKU && nclen != 2 ){
				nclen = 2;
			}
			// 全角文字・半角カタカナ文字の変換
			pw += _EucjpToUni_char( pr, pw, echarset, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			break;
		default:// case CHARSET_BINARY:
			// 保護コード
			if( nclen != 1 ){
				nclen = 1;
			}
			// 読み込みエラーになった文字を PUA に対応づける
			pw += BinToText( pr, nclen, pw );
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}


/* EUC→Unicodeコード変換 */
//2007.08.13 kobake 追加
EConvertResult CEuc::EUCToUnicode(const CMemory& cSrc, CNativeW* pDstMem)
{
	// エラー状態
	bool bError = false;

	// ソース取得
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );

	// 変換先バッファサイズとその確保
	wchar_t* pDst = new (std::nothrow) wchar_t[nSrcLen];
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = EucjpToUni( pSrc, nSrcLen, pDst, &bError );

	// pDstMem を更新
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen*sizeof(wchar_t) );

	// 後始末
	delete [] pDst;

	//$$ SJISを介しているので無駄にデータを失うかも？
	// エラーを返すようにする。	2008/5/12 Uchi
	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}





int CEuc::UniToEucjp( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError )
{
	int nclen;
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	bool berror=false, berror_tmp;
	ECharSet echarset;

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
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
			pw += _UniToEucjp_char( pr, pw, echarset, &berror_tmp );
			// 保護コード
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}else{
			if( nclen == 1 && IsBinaryOnSurrogate(static_cast<wchar_t>(*pr)) ){
				*pw = static_cast<unsigned char>( TextToBin(*pr) & 0x00ff );
				++pw;
			}else{
				// 保護コード
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


EConvertResult CEuc::UnicodeToEUC(const CNativeW& cSrc, CMemory* pDstMem)
{
	// エラー状態
	bool bError = false;

	const wchar_t* pSrc = cSrc.GetStringPtr();
	int nSrcLen = cSrc.GetStringLength();

	// 必要なバッファサイズを調べてメモリを確保
	char* pDst = new (std::nothrow) char[nSrcLen * 2];
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = UniToEucjp( pSrc, nSrcLen, pDst, &bError );

	// pDstMem を更新
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	// 後始末
	delete [] pDst;

	if( bError == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}




// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CEuc::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	CNativeW		cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 
	bool			bbinary=false;

	// 2008/6/21 Uchi
	if (psStatusbar->m_bDispUniInEuc) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	// 1文字データバッファ
	cCharBuffer.SetString(cSrc, 1);

	if( IsBinaryOnSurrogate(cSrc[0]) ){
		bbinary = true;
	}

	// EUC-JP 変換
	res = UnicodeToEUC(cCharBuffer, cCharBuffer._GetMemory());
	if (res != RESULT_COMPLETE) {
		return res;
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
