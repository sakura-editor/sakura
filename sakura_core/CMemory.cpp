//	$Id$
/************************************************************************

	CMemory.cpp
	CREATE: 1998/3/6  新規作成

	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
//#include <windows.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <windows.h>
#include <mbstring.h>
#include <ctype.h>
#include <locale.h>
#include "CMemory.h"
#include "etc_uty.h"

//#ifdef _DEBUG
#include "global.h"
#include "debug.h"
#include "CRunningTimer.h"
//#endif

#ifndef BOOL
#define BOOL	int
#endif
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif


#ifndef CRLF
#define CRLF	"\015\012"
#endif
#ifndef CR
#define CR		(char)'\015'
#endif
#ifndef LF
#define LF		(char)'\012'
#endif
#ifndef TAB
#define TAB		(char)'\011'
#endif
#ifndef SPACE
#define SPACE	(char)' '
#endif
#ifndef ESC
#define ESC		(char)'\x01b'
#endif


#define ESC_JIS		"\x01b$B"
#define ESC_ASCII	"\x01b(B"
#define ESC_8BIT	"\x01b(I"

#define TAB_SPACE	"  "

#define MEME_B_HEAD	"=?ISO-2022-JP?B?"
#define MEME_X_FOOT	"?="

#define MIME_BASE64	1
#define MIME_QUOTED	2

#define UUDECODE_CHAR(c) ((((unsigned char)c) - ((unsigned char)32)) & (unsigned char)0x3f)

/* 文字種別 */
#define CHAR_ASCII		0	/* ASCII文字 */
#define CHAR_8BITCODE	1	/* 8ビットコード(半角カタカナなど) */
#define CHAR_ZENKAKU	2	/* 全角文字 */
#define CHAR_NULL		3	/* なにもない */



/*///////////////////////////////////////////////////////////////////////////
//
//	CMemory::CMemory
//	CMemory()
//
//	説明
//		CMemoryクラス コンストラクタ
//
///////////////////////////////////////////////////////////////////////////*/
CMemory::CMemory()
{
	m_nDataBufSize = 0;
	m_pData = NULL;
	m_nDataLen = 0;
	return;
}



/*///////////////////////////////////////////////////////////////////////////
//
//	CMemory::CMemory
//	CMemory( const char* pData, int nDataLen )
//
//	引数
//	  pData		格納データアドレス
//	  nDataLen	格納データの有効長
//
//	説明
//		CMemoryクラス  コンストラクタ
//
//
//	戻値
//		なし
//
//	備考
//		格納データにはNULLを含むことができる
//
///////////////////////////////////////////////////////////////////////////*/
CMemory::CMemory( const char* pData, int nDataLen )
{
	m_nDataBufSize = 0;
	m_pData = NULL;
	m_nDataLen = 0;
	SetData( pData, nDataLen );
	return;
}


CMemory::~CMemory()
{
	Empty();
	return;
}




const CMemory& CMemory::operator = ( char cChar )
{
	char pszChar[2];
	pszChar[0] = cChar;
	pszChar[1] = '\0';
	SetData( pszChar, 1 );
	return *this;
}


//const CMemory& CMemory::operator = ( const char* pszStr )
//{
//	SetData( pszStr, strlen( pszStr ) );
//	return *this;
//}

/******
const CMemory& CMemory::operator=( const char* pData, int nDataLen )
{
	SetData( pData, nDataLen );
	return *this;
}
*******/

const CMemory& CMemory::operator = ( const CMemory& cMemory )
{
	if( this != &cMemory ){
		SetData( (CMemory*)&(cMemory) );
	}
	return *this;
}





//const CMemory& CMemory::operator += ( const char* pszStr )
//{
//	AllocBuffer( m_nDataLen + strlen( pszStr ) );
//	AddData( pszStr, strlen( pszStr ) );
//	return *this;
//}


///* データの最後に追加 publicメンバ */
//void Append( const char* pData, int nDataLen )
//{
//	AllocBuffer( m_nDataLen + nDataLen );
//	AddData( pData, nDataLen );
//	return;
//}


const CMemory& CMemory::operator += ( const CMemory& cMemory )
{
	int			nDataLen;
	const char*	pData;
	if( this == &cMemory ){
		CMemory cm = cMemory;
		pData = cm.GetPtr( &nDataLen );
		AllocBuffer( m_nDataLen + nDataLen );
		AddData( pData, nDataLen );
	}else{
		pData = cMemory.GetPtr( &nDataLen );
		AllocBuffer( m_nDataLen + nDataLen );
		AddData( (const char*)pData, nDataLen );
	}
	return *this;
}


const CMemory& CMemory::operator += ( char ch )
{
	char*	pszStr = new char[sizeof(ch) + 1];
	memcpy( pszStr, &ch, sizeof(ch) );
	pszStr[sizeof(ch)] = '\0';
	AllocBuffer( m_nDataLen + sizeof( ch ) );
	AddData( pszStr, strlen( pszStr ) );
	delete [] pszStr;
	return *this;
}



//CMemory::operator char*() const
//{
//	return (char*)m_pData;
//}
//CMemory::operator const char*() const
//{
//	return (const char*)m_pData;
//}
//CMemory::operator unsigned char*() const
//{
//	return (unsigned char*)m_pData;
//}
//CMemory::operator const unsigned char*() const
//{
//	return (const unsigned char*)m_pData;
//}
//
//CMemory::operator void*() const
//{
//	return (void*)m_pData;
//}
//CMemory::operator const void*() const
//{
//	return (const void*)m_pData;
//}


/*
||  GetAt()と同機能
*/
const char CMemory::operator[](int nIndex) const
{
	if( nIndex < m_nDataLen ){
		return m_pData[nIndex];
	}else{
		return 0;
	}
}








/*
|| バッファの最後にデータを追加する（protectメンバ
*/
void CMemory::AddData( const char* pData, int nDataLen )
{
	if( NULL == m_pData ){
		return;
	}
	memcpy( &m_pData[m_nDataLen], pData, nDataLen );
	m_nDataLen += nDataLen;
	m_pData[m_nDataLen] = '\0';
	return;
}



/* コード変換 SJIS→JIS */
void CMemory::SJIStoJIS( void )
{
	char*	pBufJIS;
	int		nBufJISLen;
	CMemory	cMem;

	/* SJIS→JIS */
	StrSJIStoJIS( &cMem, (unsigned char *)m_pData, m_nDataLen );
	pBufJIS = cMem.GetPtr( &nBufJISLen );
	SetData( pBufJIS, nBufJISLen );
	return;
}


/* SJIS→JIS */
int CMemory::StrSJIStoJIS( CMemory* pcmemDes, unsigned char* pszSrc, int nSrcLen )
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

	pcmemDes->SetData( "", strlen("") );
//	bSJISKAN  = FALSE;
	nWorkBgn = 0;
	for( i = 0;; i++ ){
		/* 処理が終了した */
		if( i >= nSrcLen ){
			nCharKind = CHAR_NULL;	/* なにもない */
		}else
		// 漢字か？
		if( ( i < nSrcLen - 1) &&
			/* SJIS全角コードの1バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			(
			  ( (unsigned char)0x81 <= (unsigned char)pszSrc[i + 0] && (unsigned char)pszSrc[i + 0] <= (unsigned char)0x9F ) ||
			  ( (unsigned char)0xE0 <= (unsigned char)pszSrc[i + 0] && (unsigned char)pszSrc[i + 0] <= (unsigned char)0xFC )
			)
			&&
			/* SJIS全角コードの2バイト目か */
			(
			  ( (unsigned char)0x40 <= (unsigned char)pszSrc[i + 1] && (unsigned char)pszSrc[i + 1] <= (unsigned char)0x7E ) ||
			  ( (unsigned char)0x80 <= (unsigned char)pszSrc[i + 1] && (unsigned char)pszSrc[i + 1] <= (unsigned char)0xFC )
			)
		){
			nCharKind = CHAR_ZENKAKU;	/* 全角文字 */
//			++i;
		}else
		if( (unsigned char)pszSrc[i] & (unsigned char)0x80 ){
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
					pcmemDes->Append( (char *)&(pszSrc[nWorkBgn]), nWorkLen );
				}
				break;
			case CHAR_8BITCODE:	/* 8ビットコード(半角カタカナなど) */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					for( j = 0; j < nWorkLen; ++j ){
						(unsigned char)pszWork[j] -= (unsigned char)0x80;
					}
					pcmemDes->Append( (char *)pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			case CHAR_ZENKAKU:	/* 全角文字 */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					// SJIS→JIS変換
					nWorkLen = MemSJIStoJIS( (unsigned char*)pszWork, nWorkLen );
					pcmemDes->Append( (char *)pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			}
			/* 新しい文字種類 */
			switch( nCharKind ){
			case CHAR_ASCII:	/* ASCII文字 */
				pcmemDes->AppendSz( ESC_ASCII );
				break;
			case CHAR_NULL:		/* なにもない */
				if( bChange &&					/* 一回は文字種が変化した */
					nCharKindOld != CHAR_ASCII	/* 直前がASCII文字ではない */
				){
					pcmemDes->AppendSz( ESC_ASCII );
				}
				break;
			case CHAR_8BITCODE:	/* 8ビットコード(半角カタカナなど) */
				pcmemDes->AppendSz( ESC_8BIT );
				break;
			case CHAR_ZENKAKU:	/* 全角文字 */
				pcmemDes->AppendSz( ESC_JIS );
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
	return pcmemDes->m_nDataLen;
}



/* SJIS→JIS変換 */
long CMemory::MemSJIStoJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char *			pszDes;
	unsigned short	sCode;

	pszDes = new char[nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		sCode = (unsigned short)_mbcjmstojis(
			(unsigned int)
			(((unsigned short)pszSrc[i	  ] << 8) |
			 ((unsigned short)pszSrc[i + 1]))
		);
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
	memcpy( (char *)pszSrc, (const char *)pszDes, j );
	delete [] pszDes;
	return j;
}


/* コード変換 JIS→SJIS */
void CMemory::JIStoSJIS( bool bMIMEdecode )
{
	int				i;
	int				j;
	BOOL			bJISKAN = FALSE;
	BOOL			b8BITCODE = FALSE;
	unsigned char*	pszDes;
	unsigned char*	pszSrc = (unsigned char*)m_pData;
	long			nSrcLen = m_nDataLen;
	BOOL			bMIME = FALSE;
	int				nMEME_Selected;
	long			nWorkBgn;
	long			nWorkLen;
	unsigned char*	pszWork;
	pszDes = new unsigned char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( bMIMEdecode && i <= nSrcLen - 16 ){
			if( 0 == _memicmp( "=?ISO-2022-JP?B?", &pszSrc[i], 16 ) ){
				nMEME_Selected = MIME_BASE64;
				bMIME = TRUE;
				i += 15;
				nWorkBgn = i + 1;
				continue;
			}
			if( 0 == _memicmp( "=?ISO-2022-JP?Q?", &pszSrc[i], 16 ) ){
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
				memset( pszWork, 0, nWorkLen + 1 );
				memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
				switch( nMEME_Selected ){
				  case MIME_BASE64:
					// Base64デコード
					nWorkLen = MemBASE64_Decode(pszWork, nWorkLen);
					break;
				  case MIME_QUOTED:
					// Quoted-Printableデコード
					nWorkLen = QuotedPrintable_Decode( (char*)pszWork, nWorkLen );
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
		nWorkLen = i - nWorkBgn;
		memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
		j += nWorkLen;
	}

/*****
	// 非ASCIIテキスト対応メッセージヘッダのMIMEコード
*****/
	pszDes[j] = 0;
	strcpy( (char *)pszSrc, (const char *)pszDes );

	nSrcLen = j;
	j = 0;
	for( i = 0; i < nSrcLen; i++ ){
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '$'	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == '@') ){
			b8BITCODE = FALSE;
			bJISKAN = TRUE;
			i += 2;
			nWorkBgn = i + 1;
			continue;
		}
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
			pszSrc[i + 2] == 'I' ){
			if( bJISKAN == TRUE && 0 < i - nWorkBgn ){
				nWorkLen = i - nWorkBgn;
				pszWork = new unsigned char [nWorkLen + 1];
				memset( pszWork, 0, nWorkLen + 1 );
				memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
				// JIS→SJIS変換
				nWorkLen = MemJIStoSJIS( (unsigned char*)pszWork, nWorkLen );
				memcpy( &pszDes[j], pszWork, nWorkLen );
				bJISKAN = FALSE;
				j += nWorkLen;
//				i += 2;
				delete [] pszWork;
//				continue;
			}
			b8BITCODE = TRUE;
			i += 2;
			continue;
		}
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == 'J') ){
			b8BITCODE = FALSE;
			if( bJISKAN == TRUE ){
				nWorkLen = i - nWorkBgn;
				pszWork = new unsigned char [nWorkLen + 1];
				memset( pszWork, 0, nWorkLen + 1 );
				memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
				// JIS→SJIS変換
				nWorkLen = MemJIStoSJIS( (unsigned char*)pszWork, nWorkLen );
				memcpy( &pszDes[j], pszWork, nWorkLen );
				bJISKAN = FALSE;
				j += nWorkLen;
				i += 2;
				delete [] pszWork;
				continue;
			}else
			if( b8BITCODE ){
				b8BITCODE = FALSE;
				i += 2;
				continue;
			}else{
				i += 2;
				continue;
			}
		}else{
			if( b8BITCODE ){
				pszDes[j] = (unsigned char)0x80 | pszSrc[i];
				j++;
			}else{
				if( bJISKAN == TRUE ){
					continue;
				}else{
					pszDes[j] = pszSrc[i];
					j++;
				}
			}
		}
	}
	pszDes[j] = 0;
	strcpy( (char *)pszSrc, (const char *)pszDes );
	m_nDataLen = j;
	delete [] pszDes;
	return;
}




/* 文字がBase64のデータか */
int CMemory::IsBASE64Char( char cData )
{
#if 0
	static const char	szBASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int					nBASE64CODE_Num = sizeof( szBASE64CODE ) / sizeof( szBASE64CODE[0] );
	int					k;
	for( k = 0; k < nBASE64CODE_Num; ++k ){
		if( cData == szBASE64CODE[k] ){
			break;
		}
	}
	if( k < nBASE64CODE_Num ){
		return k;
	}else{
		return -1;
	}
#else
	static const int NA = -1;
	static const int nBASE64Table[] = {
					62, NA, NA, NA, 63,	//	+,-./
		52, 53, 54, 55, 56, 57, 58, 59,	//	01234567
		60, 61, NA, NA, NA, NA, NA, NA,	//	89:;<=>?
		NA,  0,  1,  2,  3,  4,  5,  6,	//	@ABCDEFG
		 7,  8,  9, 10, 11, 12, 13, 14,	//	HIJKLMNO
		15, 16, 17, 18, 19, 20, 21, 22,	//	PQRSTUVW
		23, 24, 25, NA, NA, NA, NA, NA,	//	XYZ[\]^_
		NA, 26, 27, 28, 29, 30, 31, 32,	//	`abcdefg
		33, 34, 35, 36, 37, 38, 39, 40,	//	hijklmno
		41, 42, 43, 44, 45, 46, 47, 48,	//	pqrstuvw
		49, 50, 51						//	xyz
	};
	//	Oct. 10, 2000 genta
	if( cData < '+' || 'z' < cData ){
		return  -1;
	}
	return nBASE64Table[cData - '+'];

#endif
}



// Base64デコード
long CMemory::MemBASE64_Decode( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j, k;
	long			nSrcSize;
	long			lData;
	unsigned char*	pszDes;
	long			nDesLen;
	long			lDesSize;
	int				sMax;

	//Srcの有効長の算出
	for( i = 0; i < nSrcLen; i++ ){
		if( pszSrc[i] == '=' ){
			break;
		}
	}
	nSrcSize = i;
	nDesLen = (nSrcSize / 4) * 3;
	nDesLen += ((nSrcSize % 4) * 6) / 8;
	pszDes = new unsigned char[nDesLen + 1];
	memset( pszDes, 0, nDesLen + 1 );

	lDesSize = 0;
	for( i = 0; i < nSrcSize; i++ ){
		if( i < nSrcSize - (nSrcSize % 4) ){
			sMax = 4;
		}else{
			sMax = (nSrcSize % 4);
		}
		lData = 0;
		for( j = 0; j < sMax; j++ ){
			k = IsBASE64Char( pszSrc[i + j] );
			if( k >= 0 ){
				lData |= (((long)k) << ((4 - j - 1) * 6));
			}else{
			}
		}
		for( j = 0; j < (sMax * 6)/ 8 ; j++ ){
			pszDes[lDesSize] = (unsigned char)((lData >> (8 * (2 - j))) & 0x0000ff);
			lDesSize++;
		}
		i+= 3;
	}
	pszDes[lDesSize] = 0;
	memcpy( (char *)pszSrc, (const char *)pszDes, lDesSize );
	delete [] pszDes;
	return lDesSize;
}


/*
* Base64エンコード
*
*	符号化したデータは、新たに確保したメモリに格納されます
*	終了時に、そのメモリハンドルを指定されたアドレスに格納します
*	符号化されたデータ列は、一応NULL終端文字列になっています
*
*/
int CMemory::MemBASE64_Encode(
	const char*	pszSrc		,	// エンコード対象データ
	int			nSrcLen		,	// エンコード対象データ長
	char**		ppszDes		,	// 結果データ格納メモリポインタのアドレス
//	HGLOBAL*	phgDes		,	// 結果データのメモリハンドル格納場所
//	long*		pnDesLen	,	// 結果データのデータ長格納場所
	int			nWrap		,	// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
	int			bPadding		// パディングするか
)
{
	int			i, j, k, m, n;
	long		nDesIdx;
//	char*		pszDes;
	long		nDataDes;
	long		nDataSrc;
	long		nLineLen;
	char		cw;
	const char	szBASE64CODE[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int			nBASE64CODE_Num = sizeof( szBASE64CODE ) / sizeof( szBASE64CODE[0] );
	int			nDesLen;

	// 符号化後の長さを算出（ヘッダー・フッターを除く）
	//=?ISO-2022-JP?B? GyRC QXc/ Lkgi JE4/ NiRq Siwk MRso Qg== ?=
//	if( bPadding ){
		nDesLen = ((nSrcLen / 3) + ((nSrcLen % 3)? 1:0)) * 4;
		if( -1 != nWrap ){
			nDesLen += 2 * ( nDesLen / nWrap + ((nDesLen % nWrap)? 1:0 ));
		}
//	}else{
//		nDesLen = ?????????????????????????????????????????;
//	}

//	*phgDes = GlobalAlloc( GHND, nDesLen + 1 );
//	pszDes = (char *)GlobalLock( *phgDes );
	(*ppszDes) = new char[nDesLen + 1];
	memset( (*ppszDes), 0, nDesLen + 1 );

	nDesIdx = 0;
	nLineLen = 0;
	for( i = 0; i < nSrcLen; i += 3 ){
		memcpy( (void *)&nDataDes, "====", 4 );
		nDataSrc = 0;
		if( nSrcLen - i < 3 ){
			k = (nSrcLen % 3) * 8 / 6 + 1;
			for( m = 0; m < nSrcLen % 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m];
			}
		}else{
			k = 4;
			for( m = 0; m < 3; m++ ){
				((char*)&nDataSrc)[3 - m] = pszSrc[i + m]; 
			}
		}
		for( j = 0; j < k; j++ ){
			cw = (char)((nDataSrc >> (6 * (3 - j) + 8)) & 0x0000003f);
			((char*)&nDataDes)[j] = szBASE64CODE[(int)cw];
		}
		if( bPadding ){		// パディングするか
			k = 4;
		}else{
			nDesLen -= (4 - k);
		}
		for( n = 0; n < k; n++ ){
			// 自動折り返しの処理
			if( nWrap != -1 && nLineLen == nWrap ){
				(*ppszDes)[nDesIdx + 0] = CR;
				(*ppszDes)[nDesIdx + 1] = LF;
				nDesIdx += 2;
				nLineLen = 0;
			}
			(*ppszDes)[nDesIdx] = ((char*)&nDataDes)[n];
			nDesIdx++;
			nLineLen++;
		}
	}
//	GlobalUnlock( *phgDes );
	return nDesLen;
}



/* Base64デコード */
void CMemory::BASE64Decode( void )
{
	int		nBgn;
	int		nPos;
	char*	pszSrc = m_pData;
	long	nSrcLen = m_nDataLen;
	CMemory	cmemBuf;

	nBgn = 0;
	nPos = 0;
	while( nBgn < nSrcLen ){
		while( nBgn < nSrcLen && 0 > IsBASE64Char( pszSrc[nBgn] ) ){
			nBgn++;
		}
		nPos = nBgn + 1;
		while( nPos < nSrcLen && 0 <= IsBASE64Char( pszSrc[nPos] ) ){
			nPos++;
		}
		if( nBgn < nSrcLen && nPos - nBgn > 0 ){
			char*	pData;
			int		nDesLen;
			pData = new char[nPos - nBgn];
			memcpy( pData, &pszSrc[nBgn], nPos - nBgn );

			// Base64デコード
			nDesLen = MemBASE64_Decode( (unsigned char *)pData, nPos - nBgn );
			cmemBuf.Append( pData, nDesLen );
			delete [] pData;
		}
		nBgn = nPos;
	}
	SetData( cmemBuf.GetPtr( NULL ), cmemBuf.m_nDataLen );
	return;
}




/* Uudecode (デコード）*/
void CMemory::UUDECODE( char* pszFileName )
{
	unsigned char*	pszSrc = (unsigned char *)m_pData;
	long			nSrcLen = m_nDataLen;
	CMemory			cmemBuf;
	int				nBgn;
	int				nPos;
	BOOL			bBEGIN;
	unsigned char	uchDecode[64];
	bBEGIN = FALSE;
	nBgn = 0;
	while( nBgn < nSrcLen ){
		nPos = nBgn;
		while( nPos < nSrcLen && pszSrc[nPos] != CR && pszSrc[nPos] != LF ){
			nPos++;
		}
		if( nBgn < nSrcLen && nPos - nBgn > 0 ){
			int				nBufLen = nPos - nBgn;
			unsigned char*	pBuf = new unsigned char[ nBufLen + 1];
			memset( pBuf, 0, nBufLen + 1 );
			memcpy( pBuf, &pszSrc[nBgn], nBufLen ); 
			if( !bBEGIN	){
				if( 0 == memcmp( pBuf, "begin ", 6 ) ){
					bBEGIN = TRUE;
					char*	pTok;
					/* 最初のトークンを取得します。*/
					pTok = strtok( (char*)pBuf, " " );
					/* 次のトークンを取得します。*/
					pTok = strtok( NULL, " " );
					/* 更に次のトークンを取得します。*/
					pTok = strtok( NULL, "\0" );
					if( NULL != pszFileName ){
						strcpy( pszFileName, pTok );
					}
				}
			}else{
				if( 0 == memcmp( pBuf, "end", 3 ) ){
					break;
				}else{
					int		nBytes;
					nBytes = UUDECODE_CHAR( pBuf[0] );
					if( 0 == nBytes ){
						break;
					}
					int		iByteIndex = 0;
					int		iCharIndex = 1;
					while ( iByteIndex < nBytes ) {
						// Note that nBytes may not be a multiple of 3, but the
						// number of characters on the line should be a multiple of 4.
						// If the number of encoded bytes is not a multiple of 3 then
						// extra pad characters should have been added to the text
						// by the encoder to make the character count a multiple of 4.
//						if( pBuf[iCharIndex] == '\0' ||
//							pBuf[iCharIndex + 1] == '\0' ||
//							pBuf[iCharIndex + 2] == '\0' ||
//							pBuf[iCharIndex + 3] == '\0') {
//							//break;
//						}
						// Decode the line in 3-byte (=4-character) jumps.
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex]) << 2) |
								(UUDECODE_CHAR(pBuf[iCharIndex + 1]) >> 4));
							iByteIndex++;
						}
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex + 1]) << 4) |
								(UUDECODE_CHAR(pBuf[iCharIndex + 2]) >> 2));
							iByteIndex++;
						}
						if( iByteIndex < nBytes ){
							uchDecode[iByteIndex] =
								(unsigned char)((UUDECODE_CHAR(pBuf[iCharIndex + 2]) << 6) |
								UUDECODE_CHAR(pBuf[iCharIndex + 3]));
							iByteIndex++;
						}
						iCharIndex += 4;
					}
					cmemBuf.Append( (char*)uchDecode, iByteIndex );
				}
			}
			delete [] pBuf;
		}
		nBgn = nPos;
		while( nBgn < nSrcLen && ( pszSrc[nBgn] == CR || pszSrc[nBgn] == LF ) ){
			nBgn++;
		}
	}
	SetData( cmemBuf.GetPtr( NULL ), cmemBuf.m_nDataLen );
	return;
}



/* Quoted-Printableデコード */
long CMemory::QuotedPrintable_Decode( char* pszSrc, long nSrcLen )
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
	memcpy( (char *)pszSrc, (const char *)pszDes, lDesSize );
	delete [] pszDes;
	return lDesSize;
}





/* JIS→SJIS変換 */
long CMemory::MemJIStoSJIS( unsigned char* pszSrc, long nSrcLen )
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
	memcpy( (char *)pszSrc, (const char *)pszDes, j );
	delete [] pszDes;
	return j;
}



/************************************************************************
*
*【関数名】
*	EUCToSJIS
*
*【機能】
*	指定範囲のバッファ内にEUC漢字コード
*	があればSJIS全角コードに変換する。	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
*	半角文字は変換せずにそのまま残す。
*
*	制御文字CRLF以外のバイナリコードが混入している場合には結果が不定と
*	なることがあるので注意。
*	バッファの最後に漢字コードの1バイト目だけがあると困る
*
*【入力】	なし
*
*【戻り値】	なし
*
************************************************************************/
/* EUC→SJISコード変換 */
void CMemory::EUCToSJIS( void )
{
	char*			pBuf = m_pData;
	int				nBufLen = m_nDataLen;
	int				nPtr = 0L;
	int				nPtrDes = 0L;
	char*			pszDes = new char[nBufLen];
	unsigned int	sCode;

	while( nPtr < nBufLen ){
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e ){
			/* 半角カタカナ */
			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nPtrDes++;
			nPtr += 2;
		}else
		/* EUC漢字コードか? */
		if( nPtr < nBufLen - 1 && IsEUCKan1(pBuf[nPtr]) && IsEUCKan2(pBuf[nPtr + 1L])){
			/* 通常のJISコードに変換 */
			pBuf[nPtr	  ] &= 0x7f;
			pBuf[nPtr + 1L] &= 0x7f;

			/* SJISコードに変換 */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			sCode = (unsigned short)_mbcjistojms(
				(unsigned int)
				(((unsigned short)pBuf[nPtr	   ] << 8) |
				 ((unsigned short)pBuf[nPtr + 1]))
			);
			if( sCode != 0 ){
				pszDes[nPtrDes	  ] = (unsigned char)(sCode >> 8);
				pszDes[nPtrDes + 1] = (unsigned char)(sCode);
				nPtrDes += 2;;
				nPtr += 2;
			}else{
				pszDes[nPtrDes] = pBuf[nPtr];
				nPtrDes++;
				nPtr++;
			}
		}else{
			pszDes[nPtrDes] = pBuf[nPtr];
			nPtrDes++;
			nPtr++;
		}
	}
	SetData( pszDes, nPtrDes );
	delete [] pszDes;
	return;
}




/* SJIS→EUCコード変換 */
void CMemory::SJISToEUC( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				nCharChars;
	unsigned char*	pDes;
	int				nDesIdx;
	unsigned short	sCode;

	pDes = new unsigned char[nBufLen * 2];
	nDesIdx = 0;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1 ){
			if( pBuf[i] >= (unsigned char)0x80 ){
				/* 半角カタカナ */
				pDes[nDesIdx	] = (unsigned char)0x8e;
				pDes[nDesIdx + 1] = pBuf[i];
				nDesIdx += 2;
			}else{
				pDes[nDesIdx] = pBuf[i];
				nDesIdx++;
			}
		}else
		if( nCharChars == 2 ){
			/* 全角文字 */
			sCode =	(unsigned short)_mbcjmstojis(
				(unsigned int)
				(((unsigned short)pBuf[i	] << 8) |
				 ((unsigned short)pBuf[i + 1]))
			);
			if(sCode != 0){
				pDes[nDesIdx	] = (unsigned char)0x80 | (unsigned char)(sCode >> 8);
				pDes[nDesIdx + 1] = (unsigned char)0x80 | (unsigned char)(sCode);
				nDesIdx += 2;
				++i;
			}else{
				pDes[nDesIdx	] = pBuf[i];
				pDes[nDesIdx + 1] = pBuf[i + 1];
				nDesIdx += 2;
				++i;
			}
		}else
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	SetData( (const char *)pDes, nDesIdx );
	delete [] pDes;
	return;
}

/****
UTF-8のコード
	ビット列		内容
	0xxx xxxx	1バイトコード
	10xx xxxx	2バイトコード、3バイトコードの2, 3文字目
	110x xxxx	2バイトコードの先頭バイト
	1110 xxxx	3バイトコードの先頭バイト

UTF-8のエンコーディング
	フォーマット	Unicode			Unicodeビット列						UTFビット列				備考
	1バイトコード	\u0～\u7F		0000 0000 0aaa bbbb					0aaa bbbb
	2バイトコード	\u80～\u7FF		0000 0aaa bbbb cccc					110a aabb 10bb cccc
	3バイトコード	\u800～\uFFFF	aaaa bbbb cccc dddd					1110 aaaa 10bb bbcc 10cc dddd
	4バイトコード	Surrogate		110110wwwwzzzzyy + 110111yyyyxxxxxx 1111 0uuu 10uu zzzz 10yy yyyy 10xx xxxx Javaでは未使用
***/

/* UTF-8の文字か */
int CMemory::IsUTF8( const unsigned char* pData, int nDataLen )
{
	if( 3 <= nDataLen
	 && (unsigned char)0xe0 == (unsigned char)(pData[0] & (unsigned char)0xf0)
	 && (unsigned char)0x80 == (unsigned char)(pData[1] & (unsigned char)0xc0)
	 && (unsigned char)0x80 == (unsigned char)(pData[2] & (unsigned char)0xc0)
	){
		return 3;
	}
	if( 2 <= nDataLen
	 && (unsigned char)0xc0 == (unsigned char)(pData[0] & (unsigned char)0xe0)
	 && (unsigned char)0x80 == (unsigned char)(pData[1] & (unsigned char)0xc0)
	){
		return 2;
	}
//	if( 1 <= nDataLen
//	 && (unsigned char)0 == (unsigned char)(pData[0] & (unsigned char)0x80)
//	){
//		return 1;
//	}
	return 0;

}

int CMemory::DecodeUTF8toUnicode( const unsigned char* pData, int nUTF8Bytes, unsigned char* pDes )
{
	switch( nUTF8Bytes ){
	case 1:
		pDes[1] = 0x00;
		pDes[0] = pData[0];
		return 2;
	case 2:
		pDes[1] = ( pData[0] & 0x1c ) >> 2;
		pDes[0] = ( pData[0] << 6 ) | ( pData[1] & 0x3f );
		return 2;
	case 3:
		pDes[1] =  ( ( pData[0] & 0x0f ) << 4 )
				 | ( ( pData[1] & 0x3c ) >> 2 );
		pDes[0] =  (   pData[2] & 0x3f )
				 | ( ( pData[1] & 0x03 ) << 6 );
		return 2;
	}
	return 0;
}

/* UTF-8→SJISコード変換 */
void CMemory::UTF8ToSJIS( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				j;
//	int				nCharChars;
	unsigned char*	pDes;
	int				k;
//	unsigned short	sCode;
	int				nUTF8Bytes;
	int				nUNICODEBytes;
	unsigned char	pWork[100];

    setlocale( LC_ALL, "Japanese" );
	pDes = new unsigned char[nBufLen * 2];
	k = 0;
	for( i = 0; i < nBufLen; ){
		/* UTF-8の文字か */
		nUTF8Bytes = IsUTF8( &pBuf[i], nBufLen - i );
		switch( nUTF8Bytes ){
		case 1:
		case 2:
		case 3:
			nUNICODEBytes = DecodeUTF8toUnicode( &pBuf[i], nUTF8Bytes, pWork );
//			memcpy( &pDes[k], pWork, nUNICODEBytes );
//			k += nUNICODEBytes;

			if( 2 == nUNICODEBytes ){
				j = wctomb( (char*)&(pDes[k]), ((wchar_t*)pWork)[0] );
				if( -1 == j ){
					memcpy( &pDes[k], &pBuf[i], nUTF8Bytes );
					k += nUTF8Bytes;
				}else{
					k += j;
				}
			}else{
				memcpy( &pDes[k], &pBuf[i], nUTF8Bytes );
				k += nUTF8Bytes;
			}

			i += nUTF8Bytes;
			break;
		default:
			pDes[k] = pBuf[i];
			++i;
			k++;
			break;
		}
	}
	SetData( (const char *)pDes, k );
	delete [] pDes;
	return;

}





/* コード変換 SJIS→UTF-8 */
void CMemory::SJISToUTF8( void )
{
	/* コード変換 SJIS→Unicode */
	SJISToUnicode();
	
	/* コード変換 Unicode→UTF-8 */
	UnicodeToUTF8();
	return;
}

/* コード変換 SJIS→UTF-7 */
void CMemory::SJISToUTF7( void )
{
	/* コード変換 SJIS→Unicode */
	SJISToUnicode();
	
	/* コード変換 Unicode→UTF-7 */
	UnicodeToUTF7();
	return;
}




/* コード変換 Unicode→UTF-8 */
void CMemory::UnicodeToUTF8( void )
{
	wchar_t*		pUniBuf = (wchar_t*)m_pData;
//	char*			pBuf = (char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
//	int				j;
//	int				nCharChars;
	unsigned char*	pWork;
	unsigned char*	pDes;
	int				k;
//	unsigned short	sCode;
//	int				nUTF8Bytes;
//	int				nUNICODEBytes;
//	unsigned char	pWork[100];
//	unsigned char*	pBufUTF8;
//	wchar_t*		pUniBuf;


    setlocale( LC_ALL, "Japanese" );
	k = 0;
	for( i = 0; i < (int)(nBufLen / sizeof( wchar_t )); ++i ){
		if( 0x0000 <= pUniBuf[i] && 0x007f >= pUniBuf[i] ){
			k += 1;
		}
		if( 0x0080 <= pUniBuf[i] && 0x07ff >= pUniBuf[i] ){
			k += 2;
		}
		if( 0x0800 <= pUniBuf[i] && 0xffff >= pUniBuf[i] ){
			k += 3;
		}
	}
	pDes = new unsigned char[k + 1];
	memset( pDes, 0, k + 1 );
	k = 0;
	for( i = 0; i < (int)(nBufLen / sizeof( wchar_t )); ++i ){
		pWork = (unsigned char*)&((pUniBuf[i]));
//		pWork = (unsigned char*)pUniBuf;
//		pWork += sizeof( wchar_t ) * i;

		if( 0x0000 <= pUniBuf[i] && 0x007f >= pUniBuf[i] ){
			pDes[k] = pWork[0];
			k += 1;
		}else
		if( 0x0080 <= pUniBuf[i] && 0x07ff >= pUniBuf[i] ){
			pDes[k] = ( ( pWork[0] & 0xc0 ) >> 6 )
					| ( ( pWork[1] & 0x07 ) << 2 )
					| 0xc0;
			pDes[k + 1] = ( pWork[0] & 0x3f ) | 0x80;
			k += 2;
		}else
		if( 0x0800 <= pUniBuf[i] && 0xffff >= pUniBuf[i] ){
			pDes[k] = ( ( pWork[1] & 0xf0 ) >> 4 )
						| 0xe0;
			pDes[k + 1] = 0x80 
						| ( ( pWork[1] & 0x0f ) << 2 )
						| ( ( pWork[0] & 0xc0 ) >> 6 );
			pDes[k + 2] = 0x80 
						| ( pWork[0] & 0x3f );

			k += 3;
		}else{
		}
	}
	SetData( (const char *)pDes, k );
	delete [] pDes;
	return;
}

/*****:
UTF-7

	メールにおいて Unicode を用いたいときに使います。メールは、7 bit 文字が前提の環境で用いられるので、
	Unicode を UTF-7 によって 7bit 文字にエンコードするのです。

	エンコード方法：
		ASCII 文字と、space、タブ文字、CR、LF は直接エンコードします。
		その他の Unicode 文字は、Base64 エンコードします。
		ASCII文字と、Base64 エンコードされた Unicode を区別するために、+ と - を使います。 + が現れたら、
		以後は Base64 部分。- が現れたら、あるいはBase64で使われない文字が現れたら以後は ASCII 部分。
		Base64 部分の後ろの'-'はデコード時に取り除かれますが、それ以外の文字は取り除かれません。
		(特殊ケース : + を文字として使いたいときは、+- と書きます。)

	Unicode:
		My name is サーブル !
	UTF-7:
		My name is +MLUw/DDWMOs- !

	UTF-7 は1994/7月に RFC 1642 (現在は Obsolete)として規格化され、1997/5月に RFC 2152 として再規格化されました。
***/

/* コード変換 UTF-7→SJIS */
void CMemory::UTF7ToSJIS( void )
{
	char*		pBuf = (char*)m_pData;
	int			nBufLen = m_nDataLen;
	int			i;
	int			j;
	char*		pszWork;
	int			nWorkLen;
	char*		pDes;
	int			k;
	BOOL		bBASE64;
	int			nBgn;
//	CMemory		cmemWork;
	CMemory*	pcmemWork;
	char		cWork;

	pcmemWork = new CMemory;

    pDes = new char[nBufLen + 1];
	setlocale( LC_ALL, "Japanese" );
	k = 0;
	bBASE64 = FALSE;
	for( i = 0; i < nBufLen; ++i ){
		if( !bBASE64 ){
			if( i < nBufLen - 1
			  &&  '+' == pBuf[i] 
			  &&  '-' == pBuf[i + 1]
			){
				pDes[k] = '+';
				k++;
				++i;
			}else
			if( i < nBufLen - 2
			  &&  '+' == pBuf[i] 
			  &&  -1 != IsBASE64Char( pBuf[i + 1] ) /* 文字がBase64のデータか */
			){
				nBgn = i + 1;
				bBASE64 = TRUE;
			}else{
				pDes[k] = pBuf[i];
				k++;
			}
		}else{
			if( -1 == IsBASE64Char( pBuf[i] ) ){	//	Oct. 10, 2000 genta
				nWorkLen = i - nBgn;
				BOOL bSuccess;
				bSuccess = TRUE;
				
				if( 3 <= nWorkLen ){
					pszWork = new char [nWorkLen + 1];
//					memset( pszWork, 0, nWorkLen + 1 );
					memcpy( pszWork, &pBuf[nBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
					// Base64デコード
					nWorkLen = MemBASE64_Decode( (unsigned char *)pszWork, nWorkLen );
					pszWork[nWorkLen] = '\0';
					if( 0 == nWorkLen % 2 ){
						/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
						for( j = 0; j < nWorkLen; j += 2 ){
							cWork = pszWork[j + 1];
							pszWork[j + 1] = pszWork[j];
							pszWork[j] = cWork;
						}
						pcmemWork->SetData( pszWork, nWorkLen );
						/* コード変換 Unicode→SJIS */
						pcmemWork->UnicodeToSJIS();
						memcpy( &pDes[k], pcmemWork->m_pData, pcmemWork->m_nDataLen );
						k += pcmemWork->m_nDataLen;	//	Oct. 10, 2000 genta

						//	Oct. 10, 2000 genta
						//	'-'はBase64部の終わりを示す記号
						// それ以外はBase64の終わりを示すと同時に有意な文字列
						if( '-' != pBuf[i] )
							--i;
					}else{
						bSuccess = FALSE;
					}
					delete [] pszWork;
					pszWork = NULL;
				}else{
					bSuccess = FALSE;

				}
				if( FALSE == bSuccess ){
					memcpy( &pDes[k],  &pBuf[nBgn - 1], i - nBgn + 2 );
					k += (i - nBgn + 2);
				}
				bBASE64 = FALSE;
			}
		}
	}
	pDes[k] = '\0';
	SetData( (const char *)pDes, k );
	delete [] pDes;
	delete pcmemWork;
	return;
}




/* コード変換 Unicode→UTF-7 */
void CMemory::UnicodeToUTF7( void )
{
	wchar_t*		pUniBuf = (wchar_t*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				j;
	unsigned char*	pDes;
	int				k;
	BOOL			bBASE64;
	char			mbchar[4];
	int				nBgn;
	char*			pszBase64Buf;
	int				nBase64BufLen;
	char*			pszWork;
	char			cWork;

	setlocale( LC_ALL, "Japanese" );
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
	pUniBuf = new wchar_t[nBufLen + 1];
	memset( (char*)pUniBuf, 0, nBufLen + 1 );
	memcpy( (char*)pUniBuf, m_pData, nBufLen );
	for( i = 0; i < (int)(nBufLen / sizeof( wchar_t )); ++i ){
		j = wctomb( (char*)mbchar, pUniBuf[i] );
		if( !bBASE64 ){
			if( 1 == j && '+' == mbchar[0] ){
				k += 2;
			}else
			if( 1 == j ){
				k++;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				/* Base64エンコード */
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// エンコード対象データ
					(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
					&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
					-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
					FALSE	// パディングするか
				);
//				MYTRACE( "pszBase64Buf=[%s]\n", pszBase64Buf );
				//////////////
				k++;
				k += nBase64BufLen;
				k++;
				//////////////
				delete [] pszBase64Buf;
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64エンコード */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// エンコード対象データ
			(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
			&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
			-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
			FALSE	// パディングするか
		);
//		MYTRACE( "pszBase64Buf=[%s]\n", pszBase64Buf );
		//////////////
		k++;
		k += nBase64BufLen;
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	delete [] pUniBuf;

	pDes = new unsigned char[k + 1];
	memset( pDes, 0, k + 1 );
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
	pUniBuf = (wchar_t*)m_pData;
	for( i = 0; i < (int)(nBufLen / sizeof( wchar_t )); ++i ){
		j = wctomb( (char*)mbchar, pUniBuf[i] );
		if( !bBASE64 ){
			if( 1 == j && '+' == mbchar[0] ){
				memcpy( &pDes[k], "+-", 2 );
				k += 2;
			}else
			if( 1 == j ){
				pDes[k] = mbchar[0];
				k++;
			}else{
				bBASE64 = TRUE;
				nBgn = i;
			}
		}else{
			if( 1 == j ){
				/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				char*	pszBase64Buf;
				int		nBase64BufLen;
				/* Base64エンコード */
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// エンコード対象データ
					(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
					&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
					-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
					FALSE	// パディングするか
				);
//				MYTRACE( "pszBase64Buf=[%s]\n", pszBase64Buf );
				//////////////
				pDes[k] = '+';
				k++;
				memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
				k += nBase64BufLen;
				pDes[k] = '-';
				k++;
				//////////////
				delete [] pszBase64Buf;
				bBASE64 = FALSE;
				i--;
			}else{
			}
		}
	}
	if( bBASE64 && 0 < (i - nBgn) ){
		/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64エンコード */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// エンコード対象データ
			(i - nBgn) * sizeof( wchar_t ), // エンコード対象データ長
			&pszBase64Buf,					// 結果データ格納メモリポインタのアドレス
			-1,		// エンコード後のデータを自動的にCRLFで折り返す場合の１行最大文字数 (-1が指定された場合は折り返さない)
			FALSE	// パディングするか
		);
//		MYTRACE( "pszBase64Buf=[%s]\n", pszBase64Buf );
		//////////////
		pDes[k] = '+';
		k++;
		memcpy( &pDes[k], pszBase64Buf, nBase64BufLen );
		k += nBase64BufLen;
		pDes[k] = '-';
		k++;
		//////////////
		delete [] pszBase64Buf;
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		bBASE64 = FALSE;
	}
	SetData( (const char *)pDes, k );
	delete [] pDes;
	return;
}



/************************************************************************
*
* EUC全角コードの1バイト目ならTRUEをかえす
*
************************************************************************/
int CMemory::IsEUCKan1( unsigned char c )
{
	/* EUC全角コードの1バイト目ならTRUEをかえす */
	if( c >= 0xa1 && c <= 0xfe ){
		return TRUE;
	}
	return FALSE;
}
/************************************************************************
*
*  EUC全角コードの2バイト目ならTRUEをかえす
*
************************************************************************/
int CMemory::IsEUCKan2( unsigned char c )
{
	/* EUC全角コードの2バイト目ならTRUEをかえす */
	if( c >= 0xa1 && c <= 0xfe ){
		return TRUE;
	}
	return FALSE;
}


/* 英大文字→英小文字 */
void CMemory::ToLower( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1 ){
			uc = (unsigned char)tolower( pBuf[i] );
			pBuf[i] = uc;
		}else
		if( nCharChars == 2 ){
			/* 全角英大文字→全角英小文字 */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x60 && pBuf[i + 1] <= 0x79 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x21;
//@@@ 2001.02.03 Start by MIK: ギリシャ文字変換
			//大文字:0x839f～0x83b6
			//小文字:0x83bf～0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0x9f && pBuf[i + 1] <= 0xb6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ロシア文字変換
			//大文字:0x8440～0x8460
			//小文字:0x8470～0x8491 0x847fがない！
			}else if( pBuf[i] == 0x84 && pBuf[i + 1] >= 0x40 && pBuf[i + 1] <= 0x60 ){
				pBuf[i] = pBuf[i];
				if( pBuf[i + 1] >= 0x4f ){
					pBuf[i + 1] = pBuf[i + 1] + 0x31;
				}else{
					pBuf[i + 1] = pBuf[i + 1] + 0x30;
				}
//@@@ 2001.02.03 End
			}
		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	return;
}





/* 英小文字→英大文字 */
void CMemory::ToUpper( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1 ){
			uc = (unsigned char)toupper( pBuf[i] );
			pBuf[i] = uc;
		}else
		if( nCharChars == 2 ){
			/* 全角英小文字→全角英大文字 */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x81 && pBuf[i + 1] <= 0x9a ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x21;
//@@@ 2001.02.03 Start by MIK: ギリシャ文字変換
			//大文字:0x839f～0x83b6
			//小文字:0x83bf～0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0xbf && pBuf[i + 1] <= 0xd6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ロシア文字変換
			//大文字:0x8440～0x8460
			//小文字:0x8470～0x8491 0x847fがない！
			}else if( pBuf[i] == 0x84 && pBuf[i + 1] >= 0x70 && pBuf[i + 1] <= 0x91 && pBuf[i + 1] != 0x7f ){
				pBuf[i] = pBuf[i];
				if( pBuf[i + 1] >= 0x7f ){
					pBuf[i + 1] = pBuf[i + 1] - 0x31;
				}else{
					pBuf[i + 1] = pBuf[i + 1] - 0x30;
				}
//@@@ 2001.02.03 End
			}
		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	return;
}



/* ポインタで示した文字の次にある文字の位置を返します */
/* 次にある文字がバッファの最後の位置を越える場合は&pData[nDataLen]を返します */
const char* CMemory::MemCharNext( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CMemory::MemCharNext" );
//#endif

	const char*	pNext;
	if( pDataCurrent[0] == '\0' ){
		pNext = pDataCurrent + 1;
	}else
	{
//		pNext = ::CharNext( pDataCurrent );
		if(
			/* SJIS全角コードの1バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			(
			  ( (unsigned char)0x81 <= (unsigned char)pDataCurrent[0] && (unsigned char)pDataCurrent[0] <= (unsigned char)0x9F ) ||
			  ( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[0] && (unsigned char)pDataCurrent[0] <= (unsigned char)0xFC )
			)
			&&
			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			( 
			  ( (unsigned char)0x40 <= (unsigned char)pDataCurrent[1] && (unsigned char)pDataCurrent[1] <= (unsigned char)0x7E ) ||
			  ( (unsigned char)0x80 <= (unsigned char)pDataCurrent[1] && (unsigned char)pDataCurrent[1] <= (unsigned char)0xFC )
			)
		){
			pNext = pDataCurrent + 2;
		}else{
			pNext = pDataCurrent + 1;
		}
	}

	if( pNext >= &pData[nDataLen] ){
		pNext = &pData[nDataLen];
	}
	return pNext;
}



/* ポインタで示した文字の直前にある文字の位置を返します */
/* 直前にある文字がバッファの先頭の位置を越える場合はpDataを返します */
const char* CMemory::MemCharPrev( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CMemory::MemCharPrev" );
//#endif


	const char*	pPrev;
	pPrev = ::CharPrev( pData, pDataCurrent );

//===1999.08.12  このやり方だと、ダメだった。===============-
//
//	if( (pDataCurrent - 1)[0] == '\0' ){
//		pPrev = pDataCurrent - 1;
//	}else{
//		if( pDataCurrent - pData >= 2 &&
//			/* SJIS全角コードの1バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			 ( (unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F ) ||
//			 ( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC )
//			) &&
//			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			 ( (unsigned char)0x40 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0x7E ) ||
//		     ( (unsigned char)0x80 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0xFC )
//			)
//		){
//			pPrev = pDataCurrent - 2;
//		}else{
//			pPrev = pDataCurrent - 1;
//		}
//	}
//	if( pPrev < pData ){
//		pPrev = pData; 
//	}
	return pPrev;
}



/* コード変換 SJIS→Unicode */
void CMemory::SJISToUnicode( void )
{
	char*	pBufUnicode;
	int		nBufUnicodeLen;
	nBufUnicodeLen = CMemory::MemSJISToUnicode( &pBufUnicode, m_pData, m_nDataLen );
	SetData( pBufUnicode, nBufUnicodeLen );
	delete [] pBufUnicode;
	return;
}



/* コード変換 Unicode→SJIS */
void CMemory::UnicodeToSJIS( void )
{
	char*			pBufUnicode;
	int				nBufUnicodeLen;
	unsigned char*	pBuf;
	pBuf = (unsigned char*)m_pData;
	if( 2 <= m_nDataLen &&
		pBuf[0] == 0xff &&
		pBuf[0 + 1] == 0xfe
	){
		if( 2 == m_nDataLen ){
			SetDataSz( "" );
		}else{
			nBufUnicodeLen = CMemory::MemUnicodeToSJIS( &pBufUnicode, (const char *)(pBuf + 2), m_nDataLen - 2 );
			SetData( pBufUnicode, nBufUnicodeLen );
			delete [] pBufUnicode;
		}
	}else{
		nBufUnicodeLen = CMemory::MemUnicodeToSJIS( &pBufUnicode, m_pData, m_nDataLen );
		SetData( pBufUnicode, nBufUnicodeLen );
		delete [] pBufUnicode;
	}
	return;
}


/* ASCII&SJIS文字列をUnicodeに変換 */
int CMemory::MemSJISToUnicode( char** ppBufUnicode, const char*pBuf, int nBufLen )
{
	int			i, j, k;;
	wchar_t		wchar;
	char*		pBufUnicode;
	int			nCharChars;

    setlocale( LC_ALL, "Japanese" );
	i = 0;
	k = 0;
	nCharChars = CMemory::MemCharNext( pBuf, nBufLen, &pBuf[i] ) - &pBuf[i];
	while( nCharChars > 0 && i < nBufLen ){
        i += nCharChars;
		k += 2;
		nCharChars = CMemory::MemCharNext( pBuf, nBufLen, &pBuf[i] ) - &pBuf[i];
	}
	pBufUnicode = new char[k + 1];
	i = 0;
	k = 0;
	nCharChars = CMemory::MemCharNext( pBuf, nBufLen, &pBuf[i] ) - &pBuf[i];
	while( nCharChars > 0 && i < nBufLen ){
		j = mbtowc( &wchar, &pBuf[i], nCharChars );
		if( j == -1 || j == 0 ){
			pBufUnicode[k] = 0;
			pBufUnicode[k + 1] = pBuf[i];
			++i;
		}else{
			*((wchar_t*)&(pBufUnicode[k])) = wchar;
			i += j;	
		}
        k += 2;
		nCharChars = CMemory::MemCharNext( pBuf, nBufLen, &pBuf[i] ) - &pBuf[i];
	}
	*ppBufUnicode = pBufUnicode;
	return k;
}





/* Unicode文字列をASCII&SJISに変換 */
int CMemory::MemUnicodeToSJIS( char** ppBufSJIS, const char*pBuf, int nBufLen )
{
	int			i, j, k;;
	char*		pBufSJIS;
	wchar_t*	pUniBuf;
	char		mbchar[4];

    setlocale( LC_ALL, "Japanese" );
	pUniBuf = (wchar_t*)pBuf;
	i = 0;
	k = 0;
	for( i = 0; i < nBufLen / (int)sizeof( wchar_t ); ++i ){
		j = wctomb( (char*)mbchar, pUniBuf[i] );
		if( -1 == j ){
			k += sizeof( wchar_t );
		}else{
			k += j;
		}
	}
	pBufSJIS = new char[k];

	k = 0;
	for( i = 0; i < nBufLen / (int)sizeof( wchar_t ); ++i ){
		j = wctomb( (char*)&(pBufSJIS[k]), pUniBuf[i] );
		if( -1 == j ){
			memcpy( &pBufSJIS[k], &pUniBuf[i], sizeof( wchar_t ) );
			k += sizeof( wchar_t );
		}else{
			k += j;
		}
	}
	*ppBufSJIS = pBufSJIS;
	return k;
}





/* 文字列置換 */
void CMemory::Replace( char* pszFrom, char* pszTo )
{
	CMemory		cmemWork;
	int			nFromLen = strlen( pszFrom );
	int			nToLen = strlen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= m_nDataLen - nFromLen ){
		if( 0 == memcmp( &m_pData[nBgn], pszFrom, nFromLen ) ){
			if( 0  < nBgn - nBgnOld ){
				cmemWork.Append( &m_pData[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.Append( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( 0  < m_nDataLen - nBgnOld ){
		cmemWork.Append( &m_pData[nBgnOld], m_nDataLen - nBgnOld );
	}
	SetData( &cmemWork );
	return;
}



/* 等しい内容か */
int CMemory::IsEqual( CMemory& cmem1, CMemory& cmem2 )
{
	char*	psz1;
	char*	psz2;
	int		nLen1;
	int		nLen2;

	psz1 = cmem1.GetPtr( &nLen1 );
	psz2 = cmem2.GetPtr( &nLen2 );
	if( nLen1 == nLen2 ){
		if( 0 == memcmp( psz1, psz2, nLen1 ) ){
			return TRUE;
		}
	}
	return FALSE;
}





/* 半角→全角 */
void CMemory::ToZenkaku(
		int bHiragana,		/* 1== ひらがな 0==カタカナ */
		int bHanKataOnly	/* 1== 半角カタカナにのみ作用する */
)
{
	unsigned char*			pBuf = (unsigned char*)m_pData;
	int						nBufLen = m_nDataLen;
	int						i;
	int						nCharChars;
//	unsigned char			uc;
	unsigned short			usSrc;
	unsigned short			usDes;
	unsigned char*			pBufDes;
	int						nBufDesLen;
	static unsigned char*	pszHanKataSet = (unsigned char*)"｡｢｣､･ｦｧｨｩｪｫｬｭｮｯｰｱｲｳｴｵｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾅﾆﾇﾈﾉﾊﾋﾌﾍﾎﾏﾐﾑﾒﾓﾔﾕﾖﾗﾘﾙﾚﾛﾜﾝﾞﾟ";
	static unsigned char*	pszDakuSet = (unsigned char*)"ｶｷｸｹｺｻｼｽｾｿﾀﾁﾂﾃﾄﾊﾋﾌﾍﾎ";
	static unsigned char*	pszYouSet = (unsigned char*)"ﾊﾋﾌﾍﾎ";
	BOOL					bHenkanOK;

	pBufDes = new unsigned char[nBufLen * 2 + 1];
	if( NULL ==	pBufDes ){
		return;
	}
	nBufDesLen = 0;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1 ){
			bHenkanOK = FALSE;
			if( bHanKataOnly ){	/* 1== 半角カタカナにのみ作用する */
				if( NULL != strchr( (const char *)pszHanKataSet, pBuf[i] ) ){
					bHenkanOK = TRUE;
				}
			}else{
				if( ( (unsigned char)0x20 <= pBuf[i] && pBuf[i] <= (unsigned char)0x7E ) ||
					( (unsigned char)0xA1 <= pBuf[i] && pBuf[i] <= (unsigned char)0xDF )
				){
					bHenkanOK = TRUE;
				}
			}
			if( bHenkanOK ){
				usSrc = pBuf[i];
				if( FALSE == bHiragana &&
					pBuf[i]		== (unsigned char)'ｳ' &&
					pBuf[i + 1] == (unsigned char)'ﾞ'
				){
					usDes = (unsigned short)0x8394; /* ヴ */
					nCharChars = 2;
				}else{
					usDes = _mbbtombc( usSrc );
					/* 濁音 */
					if( pBuf[i + 1] == (unsigned char)'ﾞ' && NULL != strchr( (const char *)pszDakuSet, pBuf[i] ) ){
						usDes++;
						nCharChars = 2;
					}
					/* 拗音 */
					if( pBuf[i + 1] == (unsigned char)'ﾟ' && NULL != strchr( (const char *)pszYouSet, pBuf[i] ) ){
						usDes += 2;
						nCharChars = 2;
					}
				}
				
				if( TRUE == bHiragana ){
					/* ひらがなに変換可能なカタカナならば、ひらがなに変換する */
					if( (unsigned short)0x8340 <= usDes && usDes <= (unsigned short)0x837e ){	/* ァ～ミ */
						usDes-= (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usDes && usDes <= (unsigned short)0x8393 ){	/* ム～ン */
						usDes-= (unsigned short)0x00a2;
					}
				} 
				pBufDes[nBufDesLen]		= ( usDes & 0xff00 ) >>  8;
				pBufDes[nBufDesLen + 1] = ( usDes & 0x00ff );
				nBufDesLen += 2;
			}else{
				memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
				nBufDesLen += nCharChars;

			}
		}else
		if( nCharChars == 2 ){
			usDes = usSrc = pBuf[i + 1] | ( pBuf[i] << 8 );
			if( !bHanKataOnly ){
				if( TRUE == bHiragana ){
					/* 全角ひらがなに変換可能な全角カタカナならば、ひらがなに変換する */
					if( (unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ){	/* ァ～ミ */
						usDes = usSrc - (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ){	/* ム～ン */
						usDes = usSrc - (unsigned short)0x00a2;
					}
				}else{
					/* 全角カタカナに変換可能な全角ひらがなならば、カタカナに変換する */
					if( (unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ){	/* ぁ～み */
						usDes = usSrc + (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ){	/* む～ん */
						usDes = usSrc + (unsigned short)0x00a2;
					}
				}
			}
			pBufDes[nBufDesLen]		= ( usDes & 0xff00 ) >> 8;
			pBufDes[nBufDesLen + 1] = ( usDes & 0x00ff );
			nBufDesLen += 2;
		}else{
			memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
			nBufDesLen += nCharChars;

		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	pBufDes[nBufDesLen] = '\0';
	SetData( (const char *)pBufDes, nBufDesLen );
	delete [] pBufDes;


	return;
}



/* 全角→半角 */
void CMemory::ToHankaku( void )
{
	unsigned char*			pBuf = (unsigned char*)m_pData;
	int						nBufLen = m_nDataLen;
	int						i;
	int						nCharChars;
//	unsigned char			uc;
//	unsigned short			usSrc;
//	unsigned short			usDes;
	unsigned char*			pBufDes;
	int						nBufDesLen;
	unsigned int			uiSrc;
	unsigned int			uiDes;
	static unsigned char*	pszZenDAKU = (unsigned char*)"がぎぐげござじずぜぞだぢづでどばびぶべぼガギグゲゴザジズゼゾダヂヅデドバビブベボヴ";
	static unsigned char*	pszZenYOU  = (unsigned char*)"ぱぴぷぺぽパピプペポ";
	pBufDes = new unsigned char[nBufLen + 1];
	unsigned char			pszZen[3];
	if( NULL ==	pBufDes ){
		return;
	}
	nBufDesLen = 0;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 2 ){
			uiSrc = pBuf[i + 1] | ( pBuf[i] << 8 );
			uiDes = _mbctombb( uiSrc );
			if( uiDes == uiSrc ){
				memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
				nBufDesLen += nCharChars;
			}else{
				pBufDes[nBufDesLen] = (unsigned char)uiDes;
				nBufDesLen++;

				memcpy( pszZen, &pBuf[i], 2 );
				pszZen[2] = '\0';
				if( NULL != strstr( (const char *)pszZenDAKU, (const char *)pszZen ) ){
					pBufDes[nBufDesLen] = (unsigned char)'ﾞ';
					nBufDesLen++;
				}else
				if( NULL != strstr( (const char *)pszZenYOU,  (const char *)pszZen ) ){
					pBufDes[nBufDesLen] = (unsigned char)'ﾟ';
					nBufDesLen++;
				}
			}
		}else{
			memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
			nBufDesLen += nCharChars;
		}
		if( nCharChars > 0 ){
			i += nCharChars - 1;
		}
	}
	pBufDes[nBufDesLen] = '\0';
	SetData( (const char *)pBufDes, nBufDesLen );
	delete [] pBufDes;

}


/* 
|| ファイルの日本語コードセット判別
||
|| 【戻り値】
||	SJIS	0
||	JIS		1
||	EUC		2
||	Unicode	3
||	UTF-8	4
||	UTF-7	5
||	エラー	-1
*/
int CMemory::CheckKanjiCodeOfFile( const char* pszFile )
{
	HFILE					hFile;
	HGLOBAL					hgData;
	const unsigned char*	pBuf;
	int						nBufLen;
	int						nCodeType;
	
	/* メモリ確保 & ファイル読み込み */
	hgData = NULL;
	hFile = _lopen( pszFile, OF_READ );
	if( HFILE_ERROR == hFile ){
		return FALSE;
	}
	nBufLen = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		_lclose( hFile );
		return FALSE;
	}
	pBuf = (const unsigned char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );

	/* 日本語コードセット判別 */
	nCodeType = CheckKanjiCode( pBuf, nBufLen );

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
	return nCodeType;

}


/* 
|| 日本語コードセット判別
||
|| 【戻り値】
||	SJIS	0
||	JIS		1
||	EUC		2
||	Unicode	3
||	UTF-8	4
||	UTF-7	5
||	エラー	-1
*/
int CMemory::CheckKanjiCode( const unsigned char* pBuf, int nBufLen )
{
	int			nEUCMojiNum, nEUCCodeNum;
	int			nSJISMojiNum, nSJISCodeNum;
	int			nUNICODEMojiNum, nUNICODECodeNum;
	int			nJISMojiNum, nJISCodeNum;
	int			nUTF8MojiNum, nUTF8CodeNum;
	int			nUTF7MojiNum, nUTF7CodeNum;

	/*
	||日本語コードセット判別: Unicodeか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UNICODE( pBuf, nBufLen, &nUNICODEMojiNum, &nUNICODECodeNum ) ){
		if( 0 < nUNICODEMojiNum && nUNICODEMojiNum == nUNICODECodeNum ){
			return 3; /* Unicode */
		}
	}else{
		return -1;
	}
	/* 
	||日本語コードセット判別: EUCか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_EUC( pBuf, nBufLen, &nEUCMojiNum, &nEUCCodeNum ) ){
	}else{
		return -1;
	}
	/* 
	||日本語コードセット判別: SJISか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_SJIS( pBuf, nBufLen, &nSJISMojiNum, &nSJISCodeNum ) ){
	}else{
		return -1;
	}
	/*
	||日本語コードセット判別: JISか？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_JIS( pBuf, nBufLen, &nJISMojiNum, &nJISCodeNum ) ){
	}else{
		return -1;
	}

	/*
	||日本語コードセット判別: UTF-8か？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UTF8( pBuf, nBufLen, &nUTF8MojiNum, &nUTF8CodeNum ) ){
	}else{
		return -1;
	}
	/* 
	||日本語コードセット判別: UTF-7か？
	|| エラーの場合、FALSEを返す
	*/
	if( CMemory::CheckKanjiCode_UTF7( pBuf, nBufLen, &nUTF7MojiNum, &nUTF7CodeNum ) ){
	}else{
		return -1;
	}

	if( nEUCCodeNum > 0
	 && nEUCCodeNum >= nSJISCodeNum
	 && nEUCCodeNum >= nJISCodeNum
	 && nEUCCodeNum >= nUTF8CodeNum
	 && nEUCCodeNum >= nUTF7CodeNum
	){
		return 2; /* EUC */
	}
	if( nUTF7CodeNum > 0
	 && nUTF7CodeNum >= nSJISCodeNum
	 && nUTF7CodeNum >= nJISCodeNum
	 && nUTF7CodeNum >= nEUCCodeNum
	 && nUTF7CodeNum >= nUTF8CodeNum
	){
		return 5; /* UTF-7 */
	}
	if( nUTF8CodeNum > 0
	 && nUTF8CodeNum >= nSJISCodeNum
	 && nUTF8CodeNum >= nJISCodeNum
	 && nUTF8CodeNum >= nEUCCodeNum
	 && nUTF8CodeNum >= nUTF7CodeNum
	){
		return 4; /* UTF-8 */
	}
	if( nJISCodeNum > 0
	 && nJISCodeNum >= nEUCCodeNum
	 && nJISCodeNum >= nSJISCodeNum
	 && nJISCodeNum >= nUTF8CodeNum
	 && nJISCodeNum >= nUTF7CodeNum
	){
		return 1; /* JIS */
	}
//	if( nSJISCodeNum > 0
//	 && nSJISCodeNum >= nEUCCodeNum
//	 && nSJISCodeNum >= nJISCodeNum
//	 && nSJISCodeNum >= nUTF8CodeNum
//	 && nSJISCodeNum >= nUTF7CodeNum
//	){
		return 0; /* SJIS */
//	}
}



/* 
||日本語コードセット判別: Unicodeか？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_UNICODE( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUNICODECodeNum )
{
//	LONG			lFileSize;
//	HGLOBAL			hgData;
//	unsigned char*	pBuf;
//	int				nPtr;
	int				nMojiNum;
	int				nUNICODECodeNum;
//	HFILE			hFile;

	*pnMojiNum = 0;
	*pnUNICODECodeNum = 0;
	nMojiNum = 0;
	nUNICODECodeNum = 0;

	if( (unsigned char)(pBuf[0	  ]) == (unsigned char)0xff &&
	    (unsigned char)(pBuf[0 + 1]) == (unsigned char)0xfe ){
		nMojiNum = 1;
		nUNICODECodeNum = 1;
	}
	*pnMojiNum = nMojiNum;
	*pnUNICODECodeNum = nUNICODECodeNum;
	return TRUE;
}

/* 
||日本語コードセット判別: EUCか？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_EUC( const unsigned char* pBuf, int nBufLen, int*	pnMojiNum, int* pnEUCCodeNum )
{
//	HFILE			hFile;
//	LONG			lFileSize;
//	HGLOBAL			hgData;
//	char*			pBuf;
	int				nPtr;
	int				nMojiNum;
	int				nEUCCodeNum;
	char			pTest[3];
	unsigned int	sCode;

	*pnMojiNum = 0;
	*pnEUCCodeNum = 0;
	nMojiNum = 0;
	nEUCCodeNum = 0;

	pTest[2] = '\0';
	nPtr = 0;
	while( nPtr < nBufLen ){
		/* EUC漢字コードか? */
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e ){
			/* 半角カタカナ */
//			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nMojiNum++;
			nEUCCodeNum++;

//			nPtrDes++;
			nPtr += 2;
		}else
		if( nPtr < nBufLen - 1 && IsEUCKan1( pBuf[nPtr] ) && IsEUCKan2( pBuf[nPtr + 1L] ) ){
			/* 通常のJISコードに変換 */
			pTest[0] = pBuf[nPtr	 ] & 0x7f;
			pTest[1] = pBuf[nPtr + 1L] & 0x7f;

			/* SJISコードに変換 */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			sCode = (unsigned short)_mbcjistojms(
				(unsigned int)
				(((unsigned short)pTest[0] << 8) |
				 ((unsigned short)pTest[1]))
			);
			if( sCode != 0 ){
				pTest[0] = (unsigned char)(sCode >> 8);
				pTest[1] = (unsigned char)(sCode);
//				nPtrDes += 2;;
				nPtr += 2;

				if( IsZenHiraOrKata( sCode ) ){
					nEUCCodeNum++;
				}

			}else{
//				pszDes[nPtrDes] = pBuf[nPtr];
//				nPtrDes++;
				nPtr++;
			}







			nMojiNum++;

//			nPtrDes += 2;;
//			nPtr += 2;
		}else{
//			pszDes[nPtrDes] = pBuf[nPtr];
//			nPtrDes++;
			nMojiNum++;
			nPtr++;
		}
	}
	*pnMojiNum = nMojiNum;
	*pnEUCCodeNum = nEUCCodeNum;
	return TRUE;
}



/*
||日本語コードセット判別: SJISか？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_SJIS( const unsigned char* pBuf, int nBufLen, int*	pnMojiNum, int* pnSJISCodeNum )
{
//	HFILE			hFile;
//	LONG			lFileSize;
//	HGLOBAL			hgData;
//	unsigned char*	pBuf;
	int				nPtr;
	int				nMojiNum;
	int				nSJISCodeNum;
	int				i;
	int				nCharChars;
	unsigned short	sCode;
	wchar_t			wchar;
	int				j;

	*pnMojiNum = 0;
	*pnSJISCodeNum = 0;
	nMojiNum = 0;
	nSJISCodeNum = 0;


	nPtr = 0;

//	nDesIdx = 0;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1 ){
//			if( pBuf[i] >= (unsigned char)0x80 ){
//				/* 半角カタカナ */
//				nMojiNum++;
//				nSJISCodeNum++;
//			}else{
				nMojiNum++;
//			}
		}else
		if( nCharChars == 2 ){
			/* 全角文字 */
			j = mbtowc( &wchar, (const char*)&pBuf[i], nCharChars );

			sCode =	(unsigned short)_mbcjmstojis(
				(unsigned int)
				(((unsigned short)pBuf[i	] << 8) |
				 ((unsigned short)pBuf[i + 1]))
			);
			if( !( j == -1 || j == 0 )
			 && sCode != 0
			){
				nMojiNum++;

				sCode = 
					(((unsigned short)pBuf[i	] << 8) |
					 ((unsigned short)pBuf[i + 1]));
				if( 0xe380 > sCode ){
//				if( IsZenHiraOrKata( sCode ) ){
					nSJISCodeNum++;
				}



//				pDes[nDesIdx	] = (unsigned char)0x80 | (unsigned char)(sCode >> 8);
//				pDes[nDesIdx + 1] = (unsigned char)0x80 | (unsigned char)(sCode);
//				nDesIdx += 2;
				++i;
			}else{
//				pDes[nDesIdx	] = pBuf[i];
//				pDes[nDesIdx + 1] = pBuf[i + 1];
//				nDesIdx += 2;
				nMojiNum++;
				++i;
			}
		}else
		if( nCharChars > 0 ){
			nMojiNum++;
			i += nCharChars - 1;
		}
	}
	*pnMojiNum = nMojiNum;
	*pnSJISCodeNum = nSJISCodeNum;
	return TRUE;
}


/*
||日本語コードセット判別: JISか？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_JIS( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnJISCodeNum )

{
//	HFILE			hFile;
//	LONG			lFileSize;
//	HGLOBAL			hgData;
//	char*			pBuf;
//	int				nPtr;
	int				nMojiNum;
	int				nJISCodeNum;
	int				i;
//	int				nCharChars;
//	unsigned int	sCode;
	BOOL			bJISKAN, b8BITCODE, nWorkBgn, nWorkLen;

	*pnMojiNum = 0;
	*pnJISCodeNum = 0;
	nMojiNum = 0;
	nJISCodeNum = 0;

	bJISKAN = FALSE;
	b8BITCODE = FALSE;
	for( i = 0; i < nBufLen; i++ ){
		if( i <= nBufLen - 3	 &&
			 pBuf[i + 0] == 0x1b &&
			 pBuf[i + 1] == '$'  &&
			(pBuf[i + 2] == 'B' || pBuf[i + 2] == '@') ){
			b8BITCODE = FALSE;
			bJISKAN = TRUE;
			i += 2;
			nWorkBgn = i + 1;

			nMojiNum += 3;
//			nJISCodeNum++;

			continue;
		}
		if( i <= nBufLen - 3	&&
			pBuf[i + 0] == 0x1b	&&
			pBuf[i + 1] == '('	&&
			pBuf[i + 2] == 'I' ){
			if( bJISKAN == TRUE && 0 < i - nWorkBgn ){
				nWorkLen = i - nWorkBgn;

				nMojiNum += ( nWorkLen / 2 );
				nJISCodeNum += ( nWorkLen / 2 );
				bJISKAN = FALSE;
			}
			b8BITCODE = TRUE;
			i += 2;
			nMojiNum += 3;
			continue;
		}
		if( i <= nBufLen - 3	 &&
			 pBuf[i + 0] == 0x1b &&
			 pBuf[i + 1] == '('  &&
			(pBuf[i + 2] == 'B' || pBuf[i + 2] == 'J') ){
			b8BITCODE = FALSE;
			nMojiNum += 3;
			if( bJISKAN == TRUE ){
				nWorkLen = i - nWorkBgn;
				nMojiNum += ( nWorkLen / 2 );
				nJISCodeNum += ( nWorkLen / 2 );
				bJISKAN = FALSE;
				i += 2;
				continue;
			}else
			if( b8BITCODE ){
				b8BITCODE = FALSE;
				i += 2;
				continue;
			}else{
				i += 2;
				continue;
			}
		}else{
			if( b8BITCODE ){
				nMojiNum++;
				nJISCodeNum++;
			}else{
				if( bJISKAN == TRUE ){
					continue;
				}else{
					nMojiNum++;
				}
			}
		}
	}
	*pnMojiNum = nMojiNum;
	*pnJISCodeNum = nJISCodeNum;
	return TRUE;
}





/* 
||日本語コードセット判別: UTF-8か？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_UTF8( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUTF8CodeNum )
{
//	HFILE			hFile;
//	LONG			lFileSize;
//	HGLOBAL			hgData;
//	char*			pBuf;
	int				nMojiNum;
	int				nUTF8CodeNum;
	int				i;
	int				j;
	unsigned char	pDes[64];
	int				nUTF8Bytes;
	int				nUNICODEBytes;
	unsigned char	pWork[100];
	unsigned short*	pusTest;
	unsigned short	usTest;

	*pnMojiNum = 0;
	*pnUTF8CodeNum = 0;
	nMojiNum = 0;
	nUTF8CodeNum = 0;

    setlocale( LC_ALL, "Japanese" );
	for( i = 0; i < nBufLen; ){
		/* UTF-8の文字か */
		nUTF8Bytes = IsUTF8( (const unsigned char *)&pBuf[i], nBufLen - i );
		switch( nUTF8Bytes ){
		case 1:
		case 2:
		case 3:
			nUNICODEBytes = DecodeUTF8toUnicode( (const unsigned char *)&pBuf[i], nUTF8Bytes, pWork );
			if( 2 == nUNICODEBytes ){
				j = wctomb( (char*)pDes, ((wchar_t*)pWork)[0] );
				if( -1 == j ){
				}else
				if( 2 == j ){
//					nUTF8CodeNum += nUTF8Bytes;
					pusTest = (unsigned short*)pDes;

					usTest = (((unsigned short)pDes[0] << 8) |
							 ( (unsigned short)pDes[1]) );

//					MYTRACE( "*pusTest=%xh\n", *pusTest );
//					MYTRACE( "usTest=%xh\n", usTest );
					if( IsZenHiraOrKata( usTest ) ){
//						nUTF8CodeNum++;
						nUTF8CodeNum += nUTF8Bytes;
					}
				}
			}else{
			}
			i += nUTF8Bytes;
			break;
		default:
			++i;
			break;
		}
		nMojiNum++;
	}
	*pnMojiNum = nMojiNum;
	*pnUTF8CodeNum = nUTF8CodeNum;
	return TRUE;
}






/* 
||日本語コードセット判別: UTF-7か？
|| エラーの場合、FALSEを返す
*/
int CMemory::CheckKanjiCode_UTF7( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUTF7CodeNum )
{
//	HFILE		hFile;
//	HGLOBAL		hgData;
//	char*		pBuf;
	int			nMojiNum;
	int			nUTF7CodeNum;
	int			i;
	int			j;
//	int			nBufLen;
	char*		pszWork;
	int			nWorkLen;
//	int			k;
	BOOL		bBASE64;
	int			nBgn;
	CMemory		cmemWork;
	char		cWork;
	int			nUniBytes;

	*pnMojiNum = 0;
	*pnUTF7CodeNum = 0;
	nMojiNum = 0;
	nUTF7CodeNum = 0;
	
	setlocale( LC_ALL, "Japanese" );
//	k = 0;
	bBASE64 = FALSE;
	for( i = 0; i < nBufLen; ++i ){
		if( !bBASE64 ){
			if( i < nBufLen - 1
				&& '+' == pBuf[i]
				&& '-' == pBuf[i + 1]
				){
				//	Apr. 1, 2001 genta
				//	+のエンコードはUTF-7のポイント対象外とする
				//	nUTF7CodeNum++;
				++i;
			}else
			if( i < nBufLen - 2
				&& '+' == pBuf[i]
				&& -1 != IsBASE64Char( pBuf[i + 1] )	/* 文字がBase64のデータか */
				){
				nBgn = i + 1;
				bBASE64 = TRUE;
			}else{
//				k++;
				nMojiNum++;
			}
		}else{
			if( '-' == pBuf[i] ){
				nWorkLen = i - nBgn;
				if( 3 <= nWorkLen ){
					pszWork = new char [nWorkLen + 1];
					memset( pszWork, 0, nWorkLen + 1 );
					memcpy( pszWork, &pBuf[nBgn], nWorkLen );
					// Base64デコード
					nWorkLen = MemBASE64_Decode( (unsigned char *)pszWork, nWorkLen );
					if( 0 == nWorkLen % 2 ){
						nMojiNum += (nWorkLen / 2);
						/* 2バイトのUnicodeがあるという前提でLO/HIバイトを交換 */
						for( j = 0; j < nWorkLen; j += 2 ){
							cWork = pszWork[j + 1];
							pszWork[j + 1] = pszWork[j];
							pszWork[j] = cWork;
							/* 変換可能なUnicodeか */
							nUniBytes = wctomb( (char*)NULL, *(wchar_t*)(&pszWork[j]) );
							if( -1 != nUniBytes ){
								nUTF7CodeNum++;
							}
						}
					}else{
#ifdef _DEBUG
						MYTRACE( "★★★★2の倍数でない長さのUnicodeデータ列 nWorkLen=%d\n", nWorkLen );
#endif
					}
					delete [] pszWork;
//					k += nWorkLen;
				}else{
//					k++;
					nMojiNum++;
				}
				bBASE64 = FALSE;
			}else
			if( CR == pBuf[i] || LF == pBuf[i] ){
				bBASE64 = FALSE;
			}else{
				if( -1 == IsBASE64Char( pBuf[i] )  ){	/* 文字がBase64のデータか */
					bBASE64 = FALSE;
					//	Oct. 10, 2000 genta
					if( pBuf[i] & 0x80 ){
						//	8bitコードが入っていたらUTF-7ではあり得ないのでカウンタを0に戻す
						nUTF7CodeNum = 0;
					}
				}
			}
		}
	}
	*pnMojiNum = nMojiNum;
	*pnUTF7CodeNum = nUTF7CodeNum;
	return TRUE;
}



/* 自動判別→SJISコード変換 */
void CMemory::AUTOToSJIS( void )
{
	int	nCodeType;
	/* 
	|| 日本語コードセット判別
	||
	|| 【戻り値】
	||	SJIS	0
	||	JIS		1
	||	EUC		2
	||	Unicode	3
	||	UTF-8	4
	||	UTF-7	5
	||	エラー	-1
	*/
	nCodeType = CheckKanjiCode( (const unsigned char*)m_pData, m_nDataLen );
	switch( nCodeType ){
	case 1:	JIStoSJIS();break;		/* E-Mail(JIS→SJIS)コード変換 */
	case 2:	EUCToSJIS();break;		/* EUC→SJISコード変換 */
	case 3:	UnicodeToSJIS();break;	/* Unicode→SJISコード変換 */
	case 4:	UTF8ToSJIS();break;		/* UTF-8→SJISコード変換 */
	case 5:	UTF7ToSJIS();break;		/* UTF-7→SJISコード変換 */
	};
	return;
}



int CMemory::IsZenHiraOrKata( unsigned short usSrc )
{
	/* 全角カタカナ */
	if( ((unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ) /* ァ～ミ */
	 || ((unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ) /* ム～ン */
	/* 全角ひらがな */
	 || ((unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ) /* ぁ～み */
	 || ((unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ) /* む～ん */
	){
		return TRUE;
	}else{
		return FALSE;
	}
}



/* TAB→空白 */
void CMemory::TABToSPACE( int nTabSpace	/* TABの文字数 */ )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
//	BOOL		bEOL;
	int			nPosX;
	int			nWork;
	CEOL		cEol;
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = nTabSpace - ( nPosX % nTabSpace );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					nPosDes++;
					nPosX++;
				}
			}
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( TAB == pLine[i]	){
					nWork = nTabSpace - ( nPosX % nTabSpace );
					memset( (void*)&pDes[nPosDes], ' ', nWork );
					nPosDes += nWork;
					nPosX += nWork;
				}else{
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					nPosX++;
				}
			}
		}
		memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = '\0';

	SetData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}


//!空白→TAB変換
/*!
	@param nTabSpace TABの文字数
	単独のスペースは変換しない
	
	@Author Stonee
	@Date 2001/5/27
*/
void CMemory::SPACEToTAB( int nTabSpace )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
	int			nPosX;
	CEOL		cEol;

	BOOL		bSpace = FALSE;     //スペースの処理中かどうか
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	/* 変換後に必要なバイト数を調べる */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosDes += nLineLen;
		}
		nPosDes += cEol.GetLen();
	}
	if( 0 >= nPosDes ){
		return;
	}
	pDes = new char[nPosDes + 1];
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			bSpace = FALSE;
			nStartPos = 0;
			for( i = 0; i < nLineLen; ++i ){
				if( SPACE == pLine[i] || TAB == pLine[i] ){
					if (bSpace == FALSE) {
						nStartPos = nPosX;
					}
					bSpace = TRUE;
					if( SPACE == pLine[i] ){
						nPosX++;
					}else if( TAB == pLine[i] ){
						nPosX += nTabSpace - (nPosX % nTabSpace);
					}
				}else{
					if (bSpace) {
						if ((1 == nPosX - nStartPos) && (SPACE == pLine[i - 1])) {
							pDes[nPosDes] = SPACE;
							nPosDes++;
						} else {
							for (j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++) {
								pDes[nPosDes] = TAB;
								nPosDes++;
							}
							for (j = 0; j < (nPosX % nTabSpace); j++) {
								pDes[nPosDes] = SPACE;
								nPosDes++;
							}
						}
					}
					nPosX++;
					pDes[nPosDes] = pLine[i];
					nPosDes++;
					bSpace = FALSE;
				}
			}
			//for( ; i < nLineLen; ++i ){
			//	pDes[nPosDes] = pLine[i];
			//	nPosDes++;
			//}
			if (bSpace) {
				if ((1 == nPosX - nStartPos) && (SPACE == pLine[i - 1])) {
					pDes[nPosDes] = SPACE;
					nPosDes++;
				} else {
					//for (j = nStartPos - 1; (j + nTabSpace) <= nPosX + 1; j+=nTabSpace) {
					for (j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++) {
						pDes[nPosDes] = TAB;
						nPosDes++;
					}
					for (j = 0; j < nPosX % nTabSpace; j++) {
						pDes[nPosDes] = SPACE;
						nPosDes++;
					}
				}
			}
		}

		/* 行末の処理 */
		memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = '\0';

	SetData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}




//	/* バッファの先頭にデータを挿入する */
//	void CMemory::InsertTop( const char* pData, int nDataLen )
//	{
//		AllocBuffer( m_nDataLen + nDataLen );
//		memmove( m_pData + nDataLen, m_pData, m_nDataLen );
//		memcpy( m_pData, pData, nDataLen );
//		m_nDataLen += nDataLen;
//		m_pData[m_nDataLen] = '\0';
//	}





/*
|| バッファサイズの調整
*/
void CMemory::AllocBuffer( int nNewDataLen )
{
	int		nWorkLen;
	char*	pWork = NULL;
	nWorkLen = nNewDataLen + 1;	/* 1バイト多くメモリ確保しておく(\0を入れる) */
	if( m_nDataBufSize == 0 ){
		/* 未確保の状態 */
		pWork = (char*)malloc( nWorkLen );
		m_nDataBufSize = nWorkLen;
	}else{
		/* 現在のバッファサイズより大きくなった場合のみ再確保する */
		if( m_nDataBufSize < nWorkLen ){
			pWork = (char*)realloc( m_pData, nWorkLen );
			m_nDataBufSize = nWorkLen;
		}else{
			return;
		}
	}


	if( NULL == pWork ){
		::MYMESSAGEBOX(	NULL, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
			"CMemory::AllocBuffer(nNewDataLen==%d)\nメモリ確保に失敗しました。\n", nNewDataLen
		);
		if( NULL != m_pData && 0 != nWorkLen ){
			/* 古いバッファを解放して初期化 */
			free( m_pData );
			m_pData = NULL;
			m_nDataBufSize = 0;
			m_nDataLen = 0;
		}
		return;
	}
	m_pData = pWork;
	return;
}



/* バッファの内容を置き換える */
void CMemory::SetData( const char* pData, int nDataLen )
{
	Empty();
	AllocBuffer( nDataLen );
	AddData( pData, nDataLen );
	return;
}



/* バッファの内容を置き換える */
void CMemory::SetDataSz( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );

	Empty();
	AllocBuffer( nDataLen );
	AddData( pszData, nDataLen );
	return;
}


/* バッファの内容を置き換える */
void CMemory::SetData( CMemory* pcmemData )
{
	char*	pData;
	int		nDataLen;
	pData = pcmemData->GetPtr( &nDataLen );
	Empty();
	AllocBuffer( nDataLen );
	AddData( pData, nDataLen );
	return;
}


/* バッファの最後にデータを追加する（publicメンバ）*/
const char* CMemory::Append( const char* pData, int nDataLen )
{
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
	return m_pData;
}
/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendSz( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pszData, nDataLen );
}
/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::Append( CMemory* pcmemData )
{
	char*	pData;
	int		nDataLen;
	pData = pcmemData->GetPtr( &nDataLen );
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
}


//void CMemory::Init( void )
//{
//	m_nDataBufSize = 0;
//	m_pData = NULL;
//	m_nDataLen = 0;
//	return;
//}

void CMemory::Empty( void )
{
	if( m_pData != NULL ){
		free( m_pData );
		m_pData = NULL;
	}
	m_nDataBufSize = 0;
	m_pData = NULL;
	m_nDataLen = 0;
	return;
}


/*[EOF]*/
