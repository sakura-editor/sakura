// 2008.11.10  変換ロジックを書き直す

#include "StdAfx.h"
#include <mbstring.h>
#include "charset/CJis.h"
#include "charset/charcode.h"
#include "charset/codeutil.h"
#include "charset/codechecker.h"

// 非依存推奨
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       各種判定定数                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//	@author D. S. Koba
//	星マークを添えてあるものは、書き込みで使われる。
const char CJis::JISESCDATA_ASCII7[]			= "\x1b" "(B";  // ☆
const char CJis::JISESCDATA_JISX0201Latin[]		= "\x1b" "(J";
const char CJis::JISESCDATA_JISX0201Latin_OLD[]	= "\x1b" "(H";
const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";  // ☆
const char CJis::JISESCDATA_JISX0208_1978[]		= "\x1b" "$@";
const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";  // ☆
const char CJis::JISESCDATA_JISX0208_1990[]		= "\x1b" "&@""\x1b""$B";

#if 0 // 未使用
const int CJis::TABLE_JISESCLEN[] = {
	0,		// JISESC_UNKNOWN
	3,		// JISESC_ASCII
	3,		// JISESC_JISX0201Latin
	3,		// JISESC_JISX0201Latin_OLD
	3,		// JISESC_JISX0201Katakana
	3,		// JISESC_JISX0208_1978
	3,		// JISESC_JISX0208_1983
	6,		// JISESC_JISX0208_1990
};
const char* CJis::TABLE_JISESCDATA[] = {
	NULL,
	JISESCDATA_ASCII,
	JISESCDATA_JISX0201Latin,
	JISESCDATA_JISX0201Latin_OLD,
	JISESCDATA_JISX0201Katakana,
	JISESCDATA_JISX0208_1978,
	JISESCDATA_JISX0208_1983,
	JISESCDATA_JISX0208_1990,
};
#endif

/*!
	JIS の一ブロック（エスケープシーケンスとエスケープシーケンスの間の区間）を変換 

	eMyJisesc は、MYJISESC_HANKATA か MYJISESC_ZENKAKU。
*/
int CJis::_JisToUni_block( const unsigned char* pSrc, const int nSrcLen, unsigned short* pDst, const EMyJisEscseq eMyJisesc, bool* pbError )
{
	const unsigned char* pr;
	unsigned short* pw;
	unsigned char chankata;
	unsigned char czenkaku[2];
	unsigned int ctemp;
	bool berror=false;
	int nret;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = pSrc;
	pw = pDst;

	switch( eMyJisesc ){
	case MYJISESC_ASCII7:
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsAscii7(static_cast<const char>(*pr)) ){
				pw[0] = *pr;
			}else{
				berror = true;
				pw[0] = L'?';
			}
			++pw;
		}
		break;
	case MYJISESC_HANKATA:
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsJisHankata(static_cast<const char>(*pr)) ){
				// JIS → SJIS
				chankata = (*pr | 0x80);
				// SJIS → Unicode
				nret = MyMultiByteToWideChar_JP( &chankata, 1, pw, false );
				if( nret < 1 ){
					nret = 1;
				}
				pw += nret;
			}else{
				berror = true;
				pw[0] = L'?';
				++pw;
			}
		}
		break;
	case MYJISESC_ZENKAKU:
		for( ; pr < pSrc+nSrcLen-1; pr += 2 ){
			if( IsJisZen(reinterpret_cast<const char*>(pr)) ){
				// JIS -> SJIS
				ctemp = _mbcjistojms( (static_cast<unsigned int>(pr[0]) << 8) | pr[1] );
				if( ctemp != 0 ){
				// 変換に成功。
					// SJIS → Unicode
					czenkaku[0] = static_cast<unsigned char>( (ctemp & 0x0000ff00) >> 8 );
					czenkaku[1] = static_cast<unsigned char>( ctemp & 0x000000ff );
					nret = MyMultiByteToWideChar_JP( &czenkaku[0], 2, pw, false );
					if( nret < 1 ){
						// SJIS → Unicode 変換に失敗
	  					berror = true;
						pw[0] = L'?';
						nret = 1;
					}
					pw += nret;
				}else{
				// 変換に失敗。
					berror = true;
					pw[0] = L'?';
					++pw;
				}
			}else{
				berror = true;
				pw[0] = L'?';
				++pw;
			}
		}
		break;
	case MYJISESC_UNKNOWN:
		berror = true;
		for( ; pr < pSrc+nSrcLen; ++pr ){
			if( IsJis(static_cast<const char>(*pr)) ){
				pw[0] = *pr;
			}else{
				pw[0] = L'?';
			}
			++pw;
		}
		break;
	default:
		// 致命的エラー回避コード
		berror = true;
		for( ; pr < pSrc+nSrcLen; ++pr ){
			pw[0] = L'?';
			++pw;
		}
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - pDst;
}




/*
	JIS → Unicode 変換
*/
int CJis::JisToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, bool* pbError )
{
	const unsigned char *pr, *pr_end;
	const unsigned char *pr_next;
	unsigned short *pw;
	bool berror=false, berror_tmp;
	int nblocklen;
	EMyJisEscseq esctype, next_esctype;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned char*>(pSrc);
	pr_end = reinterpret_cast<const unsigned char*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned short*>(pDst);
	esctype = MYJISESC_ASCII7;

//	enum EMyJisEscseq {
//		MYJISESC_NONE,
//		MYJISESC_ASCII7,
//		MYJISESC_HANKATA,
//		MYJISESC_ZENKAKU,
//		MYJISESC_UNKNOWN,
//	};

	do{
		// シーケンスのチェック
		switch( esctype ){
		case MYJISESC_ASCII7:
			// ASCII7 ブロックをチェック
			nblocklen = CheckJisAscii7Part(
				reinterpret_cast<const char*>(pr), pr_end-pr, reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		case MYJISESC_HANKATA:
			// 半角カタカナブロックをチェック
			nblocklen = CheckJisHankataPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		case MYJISESC_ZENKAKU:
			// 全角ブロックをチェック
			nblocklen = CheckJisZenkakuPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
			break;
		default: // MYJISESC_UNKNOWN:
			// 不明なエスケープシーケンスから始まるブロックをチェック
			nblocklen = CheckJisUnknownPart(
				reinterpret_cast<const char*>(pr), pr_end-pr,  reinterpret_cast<const char**>(&pr_next), &next_esctype, NULL );
		}
		// 変換実行
		pw += _JisToUni_block( pr, nblocklen, pw, esctype, &berror_tmp );
		if( berror_tmp == true ){
			berror = true;
		}
		esctype = next_esctype;
		pr = pr_next;
	}while( pr_next < pr_end );

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned short*>(pDst);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/* E-Mail(JIS→Unicode)コード変換 */
//2007.08.13 kobake 追加
EConvertResult CJis::JISToUnicode(const CMemory& cSrc, CNativeW* pDstMem, bool base64decode)
{
	// エラー状態
	bool berror;

	// ソースを取得
	int nSrcLen;
	const char* pSrc = reinterpret_cast<const char*>( cSrc.GetRawPtr(&nSrcLen) );

	// ソースバッファポインタとソースの長さ
	const char* psrc = pSrc;
	int nsrclen = nSrcLen;
	CMemory cmem;

	if( base64decode == true ){
		// ISO-2202-J 用の MIME ヘッダーをデコード
		bool bret = MIMEHeaderDecode( pSrc, nSrcLen, &cmem, CODE_JIS );
		if( bret == true ){
			psrc = reinterpret_cast<const char*>( cmem.GetRawPtr() );
			nsrclen = cmem.GetRawLength();
		}
	}


	// 変換先バッファを取得
	wchar_t* pDst;
	try{
		pDst = new wchar_t[nsrclen * 3 + 1];
		if( pDst == NULL ){
			return RESULT_FAILURE;
		}
	}catch( ... ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = JisToUni( psrc, nsrclen, pDst, &berror );

	// pDstMem にセット
	pDstMem->_GetMemory()->SetRawDataHoldBuffer( pDst, nDstLen * sizeof(wchar_t) );

	
	delete [] pDst;

	if( berror == false ){
		return RESULT_COMPLETE;
	}else{
		return RESULT_LOSESOME;
	}
}


/*!
	SJIS -> JIS 変換
*/
int CJis::_SjisToJis_char( const unsigned char* pSrc, unsigned char* pDst, ECharSet eCharset, bool* pbError )
{
	int nret;
	bool berror=false;
	unsigned int ctemp, ctemp_;

	switch( eCharset ){
	case CHARSET_ASCII7:
		*pDst = static_cast<char>( *pSrc );
		nret = 1;
		break;
	case CHARSET_JIS_HANKATA:
		*pDst = static_cast<char>(*pSrc & 0x7f);
		nret = 1;
		break;
	case CHARSET_JIS_ZENKAKU:
		// JIS -> SJIS
		ctemp_ = SjisFilter_basis( static_cast<unsigned int>(pSrc[0] << 8) | pSrc[1] );
		ctemp_ = SjisFilter_ibm2nec( ctemp_ );
		ctemp = _mbcjmstojis( ctemp_ );
		if( ctemp != 0 ){
			// 返還に成功。
			pDst[0] = static_cast<char>( (ctemp & 0x0000ff00) >> 8 );
			pDst[1] = static_cast<char>( ctemp & 0x000000ff );
			nret = 2;
		}else{
			// 変換に失敗
			berror = true;
			// '・'  0x2126(JIS) を出力
			pDst[0] = 0x21;
			pDst[1] = 0x26;
			nret = 2;
		}
		break;
	default:
		// エラー回避コード
		berror = true;
		*pDst = '?';
		nret = 1;
	}

	if( pbError ){
		*pbError = berror;
	}

	return nret;
}



int CJis::UniToJis( const wchar_t* pSrc, const int nSrcLen, char* pDst, bool* pbError )
{
	const unsigned short *pr, *pr_end;
	unsigned char* pw;
	ECharSet echarset, echarset_cur, echarset_tmp;
	unsigned char cbuf[4];
	int nlen, nclen;
	bool berror=false, berror_tmp;

	if( nSrcLen < 1 ){
		if( pbError ){
			*pbError = false;
		}
		return 0;
	}

	pr = reinterpret_cast<const unsigned short*>(pSrc);
	pr_end = reinterpret_cast<const unsigned short*>(pSrc + nSrcLen);
	pw = reinterpret_cast<unsigned char*>(pDst);
	echarset_cur = CHARSET_ASCII7;

	while( (nclen = CheckUtf16leChar(reinterpret_cast<const wchar_t*>(pr), pr_end-pr, &echarset_tmp, 0)) > 0 ){
		// Unicode -> SJIS
		nlen = MyWideCharToMultiByte_JP( pr, nclen, &cbuf[0] );
		if( nlen < 1 ){
			// Unicode -> SJIS に失敗
			berror = true;
			if( echarset_cur == CHARSET_ASCII7 ){
				*pw = '?';
				++pw;
			}else if( echarset_cur == CHARSET_JIS_HANKATA ){
				// '･' 0x25(JIS) を出力
				*pw = 0x25;
				++pw;
			}else if( echarset_cur == CHARSET_JIS_ZENKAKU ){
				// '・' 0x2126(JIS) を出力
				pw[0] = 0x21;
				pw[1] = 0x26;
				pw += 2;
			}else{
				// 保護コード
				*pw = '?';
				++pw;
			}
			pr += nclen;
		}else{
			// 文字セットを確認
			if( nlen == 1 ){
				if( IsAscii7(cbuf[0]) ){
					echarset = CHARSET_ASCII7;
				}else{
					echarset = CHARSET_JIS_HANKATA;
				}
			}else if( nlen == 2 ){
				echarset = CHARSET_JIS_ZENKAKU;
			}else{
				// エラー回避コード
				echarset = CHARSET_ASCII7;
				nlen = 1;
			}

			// const char CJis::JISESCDATA_ASCII[]				= "\x1b" "(B";  // ☆
			// const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";  // ☆
			// const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";  // ☆
			if( echarset != echarset_cur ){
				// 文字セットが変われば、
				// エスケープシーケンス文字列を出力
				switch( echarset ){
				case CHARSET_JIS_HANKATA:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_JISX0201Katakana, 3 );
					pw += 3;
					break;
				case CHARSET_JIS_ZENKAKU:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_JISX0208_1983, 3 );
					pw += 3;
					break;
				default: // case CHARSET_ASCII7:
					strncpy( reinterpret_cast<char*>(pw), JISESCDATA_ASCII7, 3 );
					pw += 3;
					break;
				}
				echarset_cur = echarset; // 現在の文字セットを設定
			}

			// SJIS -> JIS
			pw += _SjisToJis_char( &cbuf[0], pw, echarset_cur, &berror_tmp );
			if( berror_tmp == true ){
				berror = true;
			}
			pr += nclen;
		}
	}
	// CHARSET_ASCII7 でデータが終了しない場合は、変換データの最後に
	// CHARSET_ASCII7 のエスケープシーケンスを出力
	if( echarset_cur != CHARSET_ASCII7 ){
		strncpy( reinterpret_cast<char*>(pw), JISESCDATA_ASCII7, 3 );
		pw += 3;
	}

	if( pbError ){
		*pbError = berror;
	}

	return pw - reinterpret_cast<unsigned char*>(pDst);
}


EConvertResult CJis::UnicodeToJIS(const CNativeW& cSrc, CMemory* pDstMem)
{
	bool berror=false;

	// ソースを取得
	const wchar_t* pSrc = cSrc.GetStringPtr();
	int nSrcLen = cSrc.GetStringLength();

	// 必要なバッファ容量を確認してバッファを確保
	char* pDst;
	try{
		pDst = new char[nSrcLen * 8];
	}catch( ... ){
		pDst = NULL;
	}
	if( pDst == NULL ){
		return RESULT_FAILURE;
	}

	// 変換
	int nDstLen = UniToJis( pSrc, nSrcLen, pDst, &berror );

	// pDstMem をセット
	pDstMem->SetRawDataHoldBuffer( pDst, nDstLen );

	delete [] pDst;

	if( berror ){
		return RESULT_LOSESOME;
	}else{
		return RESULT_COMPLETE;
	}
}





// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst, const CommonSetting_Statusbar* psStatusbar)
{
	CNativeW		cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	// 2008/6/21 Uchi
	if (psStatusbar->m_bDispUniInJis) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst, psStatusbar);
	}

	// 1文字データバッファ
	cCharBuffer.SetString(cSrc, 1);

	// JIS 変換
	res = UnicodeToJIS(cCharBuffer, cCharBuffer._GetMemory());
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex変換
	bool	bInEsc;
	bInEsc = false;
	pd = pDst;
	for (i = cCharBuffer._GetMemory()->GetRawLength(), ps = (unsigned char*)cCharBuffer._GetMemory()->GetRawPtr(); i >0; i--, ps ++) {
		if (*ps == 0x1B) {
			bInEsc = true;
		}
		else if (bInEsc) {
			if (*ps >= 'A' && *ps <='Z') {
				bInEsc = false;
			}
		}
		else {
			auto_sprintf( pd, _T("%02X"), *ps);
			pd += 2;
		}
	}

	return RESULT_COMPLETE;
}
