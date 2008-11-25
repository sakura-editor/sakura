#include "stdafx.h"
#include "CJis.h"
#include "CShiftJis.h"
#include <mbstring.h>
#include "charset/charcode.h"
#include "codeutil.h"
#include "convert/convert_util2.h"

#define ESC_JIS		"\x01b$B"
#define ESC_ASCII	"\x01b(B"
#define ESC_8BIT	"\x01b(I"

#define MIME_BASE64	1
#define MIME_QUOTED	2

/* 文字種別 */
#define CHAR_ASCII		0	/* ASCII文字 */
#define CHAR_8BITCODE	1	/* 8ビットコード(半角カタカナなど) */
#define CHAR_ZENKAKU	2	/* 全角文字 */
#define CHAR_NULL		3	/* なにもない */


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       各種判定定数                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

const char CJis::JISESCDATA_ASCII[]				= "\x1b" "(B";
const char CJis::JISESCDATA_JISX0201Latin[]		= "\x1b" "(J";
const char CJis::JISESCDATA_JISX0201Latin_OLD[]	= "\x1b" "(H";
const char CJis::JISESCDATA_JISX0201Katakana[]	= "\x1b" "(I";
const char CJis::JISESCDATA_JISX0208_1978[]		= "\x1b" "$@";
const char CJis::JISESCDATA_JISX0208_1983[]		= "\x1b" "$B";
const char CJis::JISESCDATA_JISX0208_1990[]		= "\x1b" "&@""\x1b""$B";

// 順序は enumJISEscSeqType に依存 (charcode.h にて定義されている)
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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     インターフェース                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* E-Mail(JIS→Unicode)コード変換 */
//2007.08.13 kobake 追加
EConvertResult CJis::JISToUnicode(CMemory* pMem, bool base64decode)
{
	//$$ SJISを介しているため、無駄にデータが失われるかもしれません
	JIStoSJIS(pMem,base64decode);
	return CShiftJis::SJISToUnicode(pMem);		//	エラーを返すようにする。	2008/5/12 Uchi
}

EConvertResult CJis::UnicodeToJIS(CMemory* pMem)
{
	EConvertResult	res;

	//$$ SJISを介しているため、無駄にデータが失われるかもしれません
	res = CShiftJis::UnicodeToSJIS(pMem);
	if (res != RESULT_COMPLETE) {
		return res;				//	エラーがあったならばエラーを返すようにする。	2008/5/12 Uchi
	}
	SJIStoJIS(pMem);

	return RESULT_COMPLETE;
}


/* JIS→SJIS変換 */
long CJis::MemJIStoSJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char*			pszDes;
	unsigned short	sCode;

	pszDes = new char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		sCode = (unsigned short)_mbcjistojms(
			(unsigned int)
			(((unsigned short)pszSrc[i	  ] << 8) |
			 ((unsigned short)pszSrc[i + 1]))
		);
		if( sCode != 0 ){
			pszDes[j	] = (unsigned char)(sCode >> 8);
			pszDes[j + 1] = (unsigned char)(sCode);
			++i;
			j+=2;
		}else{
			pszDes[j] = pszSrc[i];
			j++;
		}
	}
	pszDes[j] = 0;
	memcpy( pszSrc, pszDes, j );
	delete [] pszDes;
	return j;
}



/* コード変換 JIS→SJIS */
void CJis::JIStoSJIS( CMemory* pMem, bool bMIMEdecode )
{
	//データ取得
	CMemory cSrcMem(pMem->GetRawPtr(),pMem->GetRawLength());
	int						nSrcLen;
	const unsigned char*	pszSrc = (const unsigned char*)cSrcMem.GetRawPtr(&nSrcLen);

	CMemory cTmpMem;

	int				i;
	int				j;
	unsigned char*	pszDes;
	BOOL			bMIME = FALSE;
	int				nMEME_Selected;
	long			nWorkBgn;
	long			nWorkLen;
	unsigned char*	pszWork;

	int				nISOCode = CHAR_ASCII;
	int				nOldISOCode = nISOCode;
	BOOL			bFindESCSeq = FALSE;
	int				nESCSeqLen  = - 1; // エスケープシーケンス長 - 1

	pszDes = new unsigned char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	if( bMIMEdecode ){
		for( i = 0; i < nSrcLen; i++ ){
			if( i <= nSrcLen - 16 && '=' == pszSrc[i] ){
				if( 0 == auto_memicmp( "=?ISO-2022-JP?B?", (char*)&pszSrc[i], 16 ) ){
					nMEME_Selected = MIME_BASE64;
					bMIME = TRUE;
					i += 15;
					nWorkBgn = i + 1;
					continue;
				}
				if( 0 == auto_memicmp( "=?ISO-2022-JP?Q?", (char*)&pszSrc[i], 16 ) ){
					nMEME_Selected = MIME_QUOTED;
					bMIME = TRUE;
					i += 15;
					nWorkBgn = i + 1;
					continue;
				}
			}
			if( bMIME == TRUE ){
				if( i <= nSrcLen - 2  &&
					0 == memcmp( "?=", &pszSrc[i], 2 ) ){
					nWorkLen = i - nWorkBgn;
					pszWork = new unsigned char [nWorkLen + 1];
					//memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					//pszWork[nWorkLen] = '\0';
					switch( nMEME_Selected ){
					case MIME_BASE64:
						// Base64デコード
						nWorkLen = _DecodeBase64( reinterpret_cast<const char*>(&pszSrc[nWorkBgn]), nWorkLen,
								reinterpret_cast<char*>(pszWork) );
						break;
					case MIME_QUOTED:
						// Quoted-Printableデコード
						nWorkLen = _DecodeQP(  reinterpret_cast<const char*>(&pszSrc[nWorkBgn]), nWorkLen,
								reinterpret_cast<char*>(pszWork) );
						break;
					}
					memcpy( &pszDes[j], pszWork, nWorkLen );
					bMIME = FALSE;
					j += nWorkLen;
					++i;
					delete [] pszWork;
					continue;
				}else{
					continue;
				}
			}
			pszDes[j] = pszSrc[i];
			j++;
		}
		if( bMIME ){
			nWorkBgn -= 16; // MIMEヘッダをそのままコピー
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}

		// 非ASCIIテキスト対応メッセージヘッダのMIMEコード
		cTmpMem.SetRawData(pszDes,j);
		//memcpy( pszSrc, pszDes, j );

		nSrcLen = j;
		pszSrc=(const unsigned char*)cTmpMem.GetRawPtr();
	}

	nWorkBgn = 0;
	j = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '$'	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == '@') ){

			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_ZENKAKU;
			nESCSeqLen = 2;
		}
		else if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
			pszSrc[i + 2] == 'I' ){

			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_8BITCODE;
			nESCSeqLen = 2;
		}
		else if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == 'J') ){
			
			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_ASCII;
			nESCSeqLen = 2;
		}

		if( bFindESCSeq ){
			if( 0 < i - nWorkBgn ){
				if( CHAR_ZENKAKU == nOldISOCode ){
					nWorkLen = i - nWorkBgn;
					pszWork = new unsigned char [nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
					// JIS→SJIS変換
					nWorkLen = MemJIStoSJIS( (unsigned char*)pszWork, nWorkLen );
					memcpy( &pszDes[j], pszWork, nWorkLen );
					j += nWorkLen;
					delete [] pszWork;
				}
			}
			i += nESCSeqLen;
			nWorkBgn = i + 1;
			bFindESCSeq = FALSE;
			continue;
		}
		else if( CHAR_ASCII == nISOCode ){
			pszDes[j] = pszSrc[i];
			j++;
			continue;
		}
		else if( CHAR_8BITCODE == nISOCode ){
			pszDes[j] = (unsigned char)0x80 | pszSrc[i];
			j++;
			continue;
		}
	}

	// ESCSeqがASCIIに戻らなかったときに，データを失わないように
	if( CHAR_ZENKAKU == nISOCode ){
		if( 0 < i - nWorkBgn ){
			nWorkBgn -= nESCSeqLen + 1; // ESCSeqも残しておきたい
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}
	}

	pMem->SetRawData(pszDes,j);
//	memcpy( pszSrc, pszDes, j );
 //	m_nDataLen = j;
	delete [] pszDes;
	return;
}



/* Quoted-Printableデコード */
long CJis::QuotedPrintable_Decode( char* pszSrc, long nSrcLen )
{
	int			i;
	char*		pszDes;
	long		lDesSize;
	char		szHex[3];
	int			nHex;

	memset( szHex, 0, 3 );
	pszDes = new char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	lDesSize = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( pszSrc[i] == '=' ){
			szHex[0] = pszSrc[i + 1];
			szHex[1] = pszSrc[i + 2];
			sscanf( szHex, "%x", &nHex );
			pszDes[lDesSize] = (char)nHex;
			lDesSize++;
			i += 2;
		}else{
			pszDes[lDesSize] = pszSrc[i];
			lDesSize++;
		}
	}
	pszDes[lDesSize] = 0;
	memcpy( pszSrc, pszDes, lDesSize );
	delete [] pszDes;
	return lDesSize;
}






/* コード変換 SJIS→JIS */
void CJis::SJIStoJIS( CMemory* pMem )
{
	void*	pBufJIS;
	int		nBufJISLen;
	CMemory	cMem;

	/* SJIS→JIS */
	StrSJIStoJIS( &cMem, (unsigned char *)pMem->GetRawPtr(), pMem->GetRawLength() );
	pBufJIS = cMem.GetRawPtr( &nBufJISLen );
	pMem->SetRawData( pBufJIS, nBufJISLen );
	return;
}



/*!	SJIS→JIS

	@date 2003.09.07 genta 不要なキャスト除去
*/
int CJis::StrSJIStoJIS( CMemory* pcmemDes, unsigned char* pszSrc, int nSrcLen )
{
//	BOOL bSJISKAN	= FALSE;
//	BOOL b8BITCODE	= FALSE;

	long			nWorkBgn;
	long			nWorkLen;
	int				i;
	int				j;
	unsigned char *	pszWork;
	int				nCharKindOld;
	int				nCharKind;
	int				bChange;
	nCharKind = CHAR_ASCII;		/* ASCII文字 */
	nCharKindOld = nCharKind;
	bChange = FALSE;
//	/* 文字種別 */
//	#define CHAR_ASCII		0	/* ASCII文字 */
//	#define CHAR_8BITCODE	1	/* 8ビットコード(半角カタカナなど) */
//	#define CHAR_ZENKAKU	2	/* 全角文字 */

	pcmemDes->SetRawData("",0);
	pcmemDes->AllocBuffer( nSrcLen );
//	bSJISKAN  = FALSE;
	nWorkBgn = 0;
	for( i = 0;; i++ ){
		/* 処理が終了した */
		if( i >= nSrcLen ){
			nCharKind = CHAR_NULL;	/* なにもない */
		}else
		// 漢字か？
		if( ( i < nSrcLen - 1) && _IS_SJIS_1(pszSrc[i + 0]) && _IS_SJIS_2(pszSrc[i + 1]) ){
			nCharKind = CHAR_ZENKAKU;	/* 全角文字 */
//			++i;
		}
		else if( pszSrc[i] & (unsigned char)0x80 ){
			nCharKind = CHAR_8BITCODE;	/* 8ビットコード(半角カタカナなど) */
		}else{
			nCharKind = CHAR_ASCII;		/* ASCII文字 */
		}
		/* 文字種類が変化した */
		if( nCharKindOld != nCharKind ){
			if( CHAR_NULL != nCharKind ){
				bChange = TRUE;
			}

			nWorkLen = i - nWorkBgn;
			/* 以前の文字種類 */
			switch( nCharKindOld ){
			case CHAR_ASCII:	/* ASCII文字 */
				if( 0 < nWorkLen ){
					pcmemDes->AppendRawData( &(pszSrc[nWorkBgn]), nWorkLen );
				}
				break;
			case CHAR_8BITCODE:	/* 8ビットコード(半角カタカナなど) */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					for( j = 0; j < nWorkLen; ++j ){
						pszWork[j] -= (unsigned char)0x80;
					}
					pcmemDes->AppendRawData( pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			case CHAR_ZENKAKU:	/* 全角文字 */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					// SJIS→JIS変換
					nWorkLen = MemSJIStoJIS( pszWork, nWorkLen );
					pcmemDes->AppendRawData( pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			}
			/* 新しい文字種類 */
			switch( nCharKind ){
			case CHAR_ASCII:	/* ASCII文字 */
				pcmemDes->_AppendSz( ESC_ASCII );
				break;
			case CHAR_NULL:		/* なにもない */
				if( bChange &&					/* 一回は文字種が変化した */
					nCharKindOld != CHAR_ASCII	/* 直前がASCII文字ではない */
				){
					pcmemDes->_AppendSz( ESC_ASCII );
				}
				break;
			case CHAR_8BITCODE:	/* 8ビットコード(半角カタカナなど) */
				pcmemDes->_AppendSz( ESC_8BIT );
				break;
			case CHAR_ZENKAKU:	/* 全角文字 */
				pcmemDes->_AppendSz( ESC_JIS );
				break;
			}
			nCharKindOld = nCharKind;
			nWorkBgn = i;
			if( nCharKind == CHAR_NULL ){	/* なにもない */
				break;
			}
		}
		if( nCharKind == CHAR_ZENKAKU ){	/* 全角文字 */
			++i;
		}
	}
	return pcmemDes->GetRawLength();
}



/* SJIS→JIS変換 */
long CJis::MemSJIStoJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char *			pszDes;
	unsigned short	sCode;

	pszDes = new char[nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		//	Oct. 3, 2002 genta IBM拡張文字対応
		sCode = _mbcjmstojis_ex( pszSrc + i );
		if( sCode != 0 ){
			pszDes[j	] = (unsigned char)(sCode >> 8);
			pszDes[j + 1] = (unsigned char)(sCode);
			++i;
			j += 2;
		}else{
			pszDes[j] = pszSrc[i];
			j++;
		}
	}
	pszDes[j] = 0;
	memcpy( pszSrc, pszDes, j );
	delete [] pszDes;
	return j;
}



// 文字コード表示用	UNICODE → Hex 変換	2008/6/9 Uchi
EConvertResult CJis::UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst)
{
	static CMemory	cCharBuffer;
	EConvertResult	res;
	int				i;
	TCHAR*			pd; 
	unsigned char*	ps; 

	// 2008/6/21 Uchi
	if (CShareData::getInstance()->GetShareData()->m_Common.m_sStatusbar.m_bDispUniInJis) {
		// Unicodeで表示
		return CCodeBase::UnicodeToHex(cSrc, iSLen, pDst);
	}

	// 1文字データバッファ
	cCharBuffer.SetRawData("",0);
	cCharBuffer.AppendRawData( cSrc, sizeof(wchar_t));

	// EUC-JP 変換
	res = UnicodeToJIS(&cCharBuffer);
	if (res != RESULT_COMPLETE) {
		return res;
	}

	// Hex変換
	bool	bInEsc;
	bInEsc = false;
	pd = pDst;
	for (i = cCharBuffer.GetRawLength(), ps = (unsigned char*)cCharBuffer.GetRawPtr(); i >0; i--, ps ++) {
		if (*ps == 0x1B) {
			bInEsc = true;
		}
		else if (bInEsc) {
			if (*ps >= 'A' && *ps <='Z') {
				bInEsc = false;
			}
		}
		else {
			auto_sprintf( pd, _T("%02x"), *ps);
			pd += 2;
		}
	}

	return RESULT_COMPLETE;
}
