/*!	@file
	メモリバッファクラス

	@author Norio Nakatani
	@date 1998/03/06 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, mik, misaka, Stonee, hor
	Copyright (C) 2002, Moca, sui, aroka, genta
	Copyright (C) 2003, genta, Moca, かろと
	Copyright (C) 2004, Moca
	Copyright (C) 2005, Moca, D.S.Koba
	Copyright (C) 2006, rastiv

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
#include <stdio.h>
#include <windows.h>
#include <mbstring.h>
#include <ctype.h>
#include <locale.h>
#include <limits.h>
#include "CMemory.h"
#include "etc_uty.h"
#include "CEol.h"// 2002/2/3 aroka
#include "charcode.h"
#include "CESI.h"  // 2006.12.16  rastiv
#include "my_icmp.h" // Nov. 29, 2002 genta/moca

//#ifdef _DEBUG
#include "global.h"
#include "Debug.h"
#include "CRunningTimer.h"
//#endif



#define ESC_JIS		"\x01b$B"
#define ESC_ASCII	"\x01b(B"
#define ESC_8BIT	"\x01b(I"

#define MIME_BASE64	1
#define MIME_QUOTED	2

// #define UUDECODE_CHAR(c) ((((unsigned char)c) - ((unsigned char)32)) & (unsigned char)0x3f)

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
	SetString( pData, nDataLen );
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
	SetString( pszChar, 1 );
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
		SetNativeData( (CMemory*)&(cMemory) );
	}
	return *this;
}

const CMemory& CMemory::operator += ( const CMemory& cMemory )
{
	int			nDataLen;
	const char*	pData;
	if( this == &cMemory ){
		CMemory cm = cMemory;
		pData = cm.GetStringPtr( &nDataLen );
		AllocStringBuffer( m_nDataLen + nDataLen );
		AddData( pData, nDataLen );
	}else{
		pData = cMemory.GetStringPtr( &nDataLen );
		AllocStringBuffer( m_nDataLen + nDataLen );
		AddData( (const char*)pData, nDataLen );
	}
	return *this;
}


const CMemory& CMemory::operator += ( char ch )
{
	char szChar[2];
	szChar[0] = ch;
	szChar[1] = '\0';
	AllocStringBuffer( m_nDataLen + sizeof( ch ) );
	AddData( szChar, sizeof( ch ) );
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

/*!
	@brief 拡張版 SJIS→JIS変換

	SJISコードをJISに変換する．その際，JISに対応領域のないIBM拡張文字を
	NEC選定IBM拡張文字に変換する．

	Shift_JIS fa40～fc4b の範囲の文字は 8754～879a または ed40～eefc に
	散在する文字に変換された後に，JISに変換されます．
	
	@param pszSrc [in] 変換する文字列へのポインタ (Shift JIS)
	
	@author すい
	@date 2002.10.03 1文字のみ扱い，変換まで行うように変更 genta
*/
unsigned short CMemory::_mbcjmstojis_ex( unsigned char* pszSrc )
{
	unsigned int	tmpw;	/* ← int が 16 bit 以上である事を期待しています。 */
	
	if(	_IS_SJIS_1(* pszSrc    ) &&	/* Shift_JIS 全角文字の 1バイト目 */
		_IS_SJIS_2(*(pszSrc+1) )	/* Shift_JIS 全角文字の 2バイト目 */
	){	/* Shift_JIS全角文字である */
		tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( *pszSrc == 0x0fa ) ||
			( *pszSrc == 0x0fb ) ||
			( ( *pszSrc == 0x0fc ) && ( *(pszSrc+1) <= 0x04b ) )
		) {		/* fa40～fc4b の文字である。 */
			/* 文字コード変換処理 */
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40～fa49 → eeef～eef8 (ⅰ～ⅹ) */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a～fa53 → 8754～875d (Ⅰ～Ⅹ) */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54～fa57 → eef9～eefc (￢～＂) */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* ㈱ */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* № */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* ℡ */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* ∵ */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c～fa7e → ed40～ed62 (纊～兊) */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80～fa9b → ed63～ed7e (兤～﨏) */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c～fafc → ed80～ede0 (塚～浯) */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40～fb5b → ede1～edfc (涖～犱) */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c～fb7e → ee40～ee62 (犾～神) */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80～fb9b → ee63～ee7e (祥～蕙) */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c～fbfc → ee80～eee0 (蕫～髙) */
			else{							tmpw -= 0x0d5f;	}	/* fc40～fc4b → eee1～eeec (髜～黑) */
		}
		return (unsigned short) _mbcjmstojis( tmpw );
	}
	return 0;
}


/* コード変換 SJIS→JIS */
void CMemory::SJIStoJIS( void )
{
	char*	pBufJIS;
	int		nBufJISLen;
	CMemory	cMem;

	/* SJIS→JIS */
	StrSJIStoJIS( &cMem, (unsigned char *)m_pData, m_nDataLen );
	pBufJIS = cMem.GetStringPtr( &nBufJISLen );
	SetString( pBufJIS, nBufJISLen );
	return;
}


/*!	SJIS→JIS

	@date 2003.09.07 genta 不要なキャスト除去
*/
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

	pcmemDes->SetString( "" );
	pcmemDes->AllocStringBuffer( nSrcLen );
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
			_IS_SJIS_1( pszSrc[i + 0] ) &&
			/* SJIS全角コードの2バイト目か */
			_IS_SJIS_2( pszSrc[i + 1] )
		){
			nCharKind = CHAR_ZENKAKU;	/* 全角文字 */
//			++i;
		}else
		if( pszSrc[i] & (unsigned char)0x80 ){
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
					pcmemDes->AppendString( (char *)&(pszSrc[nWorkBgn]), nWorkLen );
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
					pcmemDes->AppendString( (char *)pszWork, nWorkLen );
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
					pcmemDes->AppendString( (char *)pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			}
			/* 新しい文字種類 */
			switch( nCharKind ){
			case CHAR_ASCII:	/* ASCII文字 */
				pcmemDes->AppendString( ESC_ASCII );
				break;
			case CHAR_NULL:		/* なにもない */
				if( bChange &&					/* 一回は文字種が変化した */
					nCharKindOld != CHAR_ASCII	/* 直前がASCII文字ではない */
				){
					pcmemDes->AppendString( ESC_ASCII );
				}
				break;
			case CHAR_8BITCODE:	/* 8ビットコード(半角カタカナなど) */
				pcmemDes->AppendString( ESC_8BIT );
				break;
			case CHAR_ZENKAKU:	/* 全角文字 */
				pcmemDes->AppendString( ESC_JIS );
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
	memcpy( (char *)pszSrc, (const char *)pszDes, j );
	delete [] pszDes;
	return j;
}


/* コード変換 JIS→SJIS */
void CMemory::JIStoSJIS( bool bMIMEdecode )
{
	int				i;
	int				j;
	unsigned char*	pszDes;
	unsigned char*	pszSrc = (unsigned char*)m_pData;
	long			nSrcLen = m_nDataLen;
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
	if( false != bMIMEdecode ){
		for( i = 0; i < nSrcLen; i++ ){
			if( i <= nSrcLen - 16 && '=' == pszSrc[i] ){
				if( 0 == my_memicmp( "=?ISO-2022-JP?B?", &pszSrc[i], 16 ) ){
					nMEME_Selected = MIME_BASE64;
					bMIME = TRUE;
					i += 15;
					nWorkBgn = i + 1;
					continue;
				}
				if( 0 == my_memicmp( "=?ISO-2022-JP?Q?", &pszSrc[i], 16 ) ){
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
//					memset( pszWork, 0, nWorkLen + 1 );
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
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
			nWorkBgn -= 16; // MIMEヘッダをそのままコピー
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}

		// 非ASCIIテキスト対応メッセージヘッダのMIMEコード
		memcpy( (char *)pszSrc, (const char *)pszDes, j );

		nSrcLen = j;
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
		}else
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
			pszSrc[i + 2] == 'I' ){

			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_8BITCODE;
			nESCSeqLen = 2;
		}else
		if( i <= nSrcLen - 3		&&
			pszSrc[i + 0] == 0x1b	&&
			pszSrc[i + 1] == '('	&&
		   (pszSrc[i + 2] == 'B' || pszSrc[i + 2] == 'J') ){
			
			bFindESCSeq = TRUE;
			nOldISOCode = nISOCode;
			nISOCode = CHAR_ASCII;
			nESCSeqLen = 2;
		}
		// else{}

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
		}else
		if( CHAR_ASCII == nISOCode ){
			pszDes[j] = pszSrc[i];
			j++;
			continue;
		}else
		if( CHAR_8BITCODE == nISOCode ){
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

	memcpy( pszSrc, pszDes, j );
 	m_nDataLen = j;
	delete [] pszDes;
	return;
}





/* 文字がBase64のデータか */
int CMemory::IsBASE64Char( char cData )
{
	int nret = -1;
	if( (unsigned char)cData < 0x80 ){
		nret = Charcode::BASE64VAL[cData];
		if( nret == 0xff ){
			nret = -1;
		}
	}
	return nret;
}

// BASE64 => デコード後
// 4文字  => 3文字

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
			cmemBuf.AppendString( pData, nDesLen );
			delete [] pData;
		}
		nBgn = nPos;
	}
	SetString( cmemBuf.GetStringPtr(), cmemBuf.m_nDataLen );
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
					cmemBuf.AppendString( (char*)uchDecode, iByteIndex );
				}
			}
			delete [] pBuf;
		}
		nBgn = nPos;
		while( nBgn < nSrcLen && ( pszSrc[nBgn] == CR || pszSrc[nBgn] == LF ) ){
			nBgn++;
		}
	}
	SetString( cmemBuf.GetStringPtr(), cmemBuf.m_nDataLen );
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
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e && nPtr < nBufLen - 1 ){
			/* 半角カタカナ */
			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nPtrDes++;
			nPtr += 2;
		}else
		/* EUC漢字コードか? */
		if( nPtr < nBufLen - 1 && Charcode::IsEucKan1(pBuf[nPtr]) && Charcode::IsEucKan2(pBuf[nPtr + 1L]) ){
			/* 通常のJISコードに変換 */
			pBuf[nPtr	  ] &= 0x7f;
			pBuf[nPtr + 1L] &= 0x7f;

			/* SJISコードに変換 */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			sCode = (unsigned short)_mbcjistojms(
				(unsigned int)
				(((unsigned short)pBuf[nPtr] << 8) |
				 ((unsigned short)pBuf[nPtr + 1]))
			);
			if( sCode != 0 ){
				pszDes[nPtrDes	  ] = (unsigned char)(sCode >> 8);
				pszDes[nPtrDes + 1] = (unsigned char)(sCode);
				nPtrDes += 2;
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
	SetString( pszDes, nPtrDes );
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
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
			//	Oct. 3, 2002 genta IBM拡張文字対応
			sCode =	(unsigned short)_mbcjmstojis_ex( pBuf + i );
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
	SetString( (const char *)pDes, nDesIdx );
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
	SetString( (const char *)pDes, k );
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
		}else
		if( 0x0080 <= pUniBuf[i] && 0x07ff >= pUniBuf[i] ){
			k += 2;
		}else
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
	SetString( (const char *)pDes, k );
	delete [] pDes;
	return;
}

/* コード変換 UTF-7→SJIS

	UTF-7についてはRFC 2152参照．
*/
void CMemory::UTF7ToSJIS( void )
{
	char*		pBuf = (char*)m_pData;
	int			nBufLen = m_nDataLen;
	int			i;
	char*		pszWork;
	int			nWorkLen;
	char*		pDes;
	int			k;
	BOOL		bBASE64;
	int			nBgn;
//	CMemory		cmemWork;
	CMemory*	pcmemWork;

	pcmemWork = new CMemory;

	pDes = new char[nBufLen + 1];
	setlocale( LC_ALL, "Japanese" );
	k = 0;
	bBASE64 = FALSE;
	// 2002.10.05 Moca < を<= に変更 ただし，pBuf[nBufLen]は存在しないことに注意
	for( i = 0; i <= nBufLen; ++i ){
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
			}else
			if( i < nBufLen ){
				pDes[k] = pBuf[i];
				k++;
			}
		}else{
			if( i == nBufLen ||				//	2002.10.25 Moca データ終端時もデコード
				-1 == IsBASE64Char( pBuf[i] ) ){	//	Oct. 10, 2000 genta
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
						/* Unicodeは2バイト単位 */
						pcmemWork->SetString( pszWork, nWorkLen );
						/* コード変換 UnicodeBE→SJIS */
						pcmemWork->UnicodeBEToSJIS();
						memcpy( &pDes[k], pcmemWork->m_pData, pcmemWork->m_nDataLen );
						k += pcmemWork->m_nDataLen;	//	Oct. 10, 2000 genta

						//	Oct. 10, 2000 genta
						//	'-'はBase64部の終わりを示す記号
						// それ以外はBase64の終わりを示すと同時に有意な文字列
						if( i < nBufLen && '-' != pBuf[i] )
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
					if( i < nBufLen ){
						nWorkLen = i - nBgn + 2;
					}else{ // pBuf[nBufLen] は無効
						nWorkLen = i - nBgn + 1;
					}
					memcpy( &pDes[k],  &pBuf[nBgn - 1], nWorkLen );
					k += nWorkLen;
				}
				bBASE64 = FALSE;
			}
		}
	}
	pDes[k] = '\0';
	SetString( (const char *)pDes, k );
	delete [] pDes;
	delete pcmemWork;
	return;
}

/*!
	Unicodeの文字がUTF-7で直接エンコードできるか調べる
	@author Moca
	@date 2002.10.25 新規作成

	TAB SP CR LF は 直接エンコード可能
	基本セット
	         '(),-./012...789:?ABC...XYZabc...xyz
	以下はオプションでメールでは支障をきたす場合がある
	         !"#$%&*;<=>@[\]^_`{|}
	とりあえず無条件でオプションは直接変換できないと判断する
*/
int CMemory::IsUTF7Direct( wchar_t wc )
{
	int nret = 0;
	if( (wc & 0xff00) == 0 ){
		nret = Charcode::IsUtf7SetDChar( (unsigned char)wc );
	}
	return nret;
}



/*! コード変換 Unicode→UTF-7
	@date 2002.10.25 Moca UTF-7で直接エンコードできる文字をRFCに合わせて制限した
*/
void CMemory::UnicodeToUTF7( void )
{
	wchar_t*		pUniBuf;
//	int				nBufLen = m_nDataLen;
	int				nUniBufLen = m_nDataLen / sizeof(wchar_t);
	int				i;
	int				j;
	unsigned char*	pDes;
	int				k;
	BOOL			bBASE64;
//	char			mbchar[4];
	int				nBgn;
	char*			pszBase64Buf;
	int				nBase64BufLen;
	char*			pszWork;
	char			cWork;

//	setlocale( LC_ALL, "Japanese" ); // wctomb を使わなくなったためコメントアウト
	k = 0;
	bBASE64 = FALSE;
	nBgn = 0;
	pUniBuf = new wchar_t[nUniBufLen + 1];
//	memset( pUniBuf, 0, (nUniBufLen + 1) * sizeof( wchar_t ) );
	memcpy( pUniBuf, m_pData, nUniBufLen * sizeof( wchar_t ) );
	pUniBuf[nUniBufLen] = L'\0';
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i]);
		if( !bBASE64 ){
			if( 1 == j ){
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				k += 2;
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
//				MYTRACE_A( "pszBase64Buf=[%s]\n", pszBase64Buf );
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
//		MYTRACE_A( "pszBase64Buf=[%s]\n", pszBase64Buf );
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
	for( i = 0; i < nUniBufLen; ++i ){
		j = IsUTF7Direct( pUniBuf[i] );
		if( !bBASE64 ){
			if( 1 == j ){
				pDes[k] = (unsigned char)(pUniBuf[i] & 0x007f);
				k++;
			}else
			if( L'+' == pUniBuf[i] ){
				pDes[k    ] = '+';
				pDes[k + 1] = '-';
				k += 2;
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
//				MYTRACE_A( "pszBase64Buf=[%s]\n", pszBase64Buf );
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
//		MYTRACE_A( "pszBase64Buf=[%s]\n", pszBase64Buf );
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
	SetString( (const char *)pDes, k );
	delete [] pDes;
	return;
}





/* 小文字 */
void CMemory::ToLower( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
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





/* 大文字 */
void CMemory::ToUpper( void )
{
	unsigned char*	pBuf = (unsigned char*)m_pData;
	int				nBufLen = m_nDataLen;
	int				i;
	int				nCharChars;
	unsigned char	uc;
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
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
			_IS_SJIS_1( (unsigned char)pDataCurrent[0] )
			&&
			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
			_IS_SJIS_2( (unsigned char)pDataCurrent[1] )
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
//			( (unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F ) ||
//			( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC )
//			) &&
//			/* SJIS全角コードの2バイト目か */	//Sept. 1, 2000 jepro 'シフト'を'S'に変更
//			(
//			( (unsigned char)0x40 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0x7E ) ||
//			( (unsigned char)0x80 <= (unsigned char)pDataCurrent[-1] && (unsigned char)pDataCurrent[-1] <= (unsigned char)0xFC )
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
	SetString( pBufUnicode, nBufUnicodeLen );
	delete [] pBufUnicode;
	return;
}



/* コード変換 SJIS→UnicodeBE */
void CMemory::SJISToUnicodeBE( void )
{
	char*	pBufUnicode;
	int		nBufUnicodeLen;
	nBufUnicodeLen = CMemory::MemSJISToUnicode( &pBufUnicode, m_pData, m_nDataLen );
	SetString( pBufUnicode, nBufUnicodeLen );
	delete [] pBufUnicode;
	SwapHLByte();
	return;
}



/* コード変換 Unicode→SJIS */
void CMemory::UnicodeToSJIS( void )
{
	char*			pBufUnicode;
	int				nBufUnicodeLen;
	unsigned char*	pBuf;
	pBuf = (unsigned char*)m_pData;
//	BOMの削除はここではしないように変更
//	呼び出し側で対処してください 2002/08/30 Moca
		nBufUnicodeLen = CMemory::MemUnicodeToSJIS( &pBufUnicode, m_pData, m_nDataLen );
		SetString( pBufUnicode, nBufUnicodeLen );
		delete [] pBufUnicode;
	return;
}


/* コード変換 UnicodeBE→SJIS */
void CMemory::UnicodeBEToSJIS( void ){

	SwapHLByte();
	UnicodeToSJIS();

	return;
}


/* ASCII&SJIS文字列をUnicodeに変換 */
int CMemory::MemSJISToUnicode( char** ppBufUnicode, const char*pBuf, int nBufLen )
{
	int			i, j, k;
	wchar_t		wchar;
	char*		pBufUnicode;
	int			nCharChars;

	setlocale( LC_ALL, "Japanese" );
	i = 0;
	k = 0;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	nCharChars = CMemory::GetSizeOfChar( pBuf, nBufLen, i );
	while( nCharChars > 0 && i < nBufLen ){
		i += nCharChars;
		k += 2;
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pBuf, nBufLen, i );
	}
	pBufUnicode = new char[k + 1];
	i = 0;
	k = 0;
	// 2005-09-02 D.S.Koba GetSizeOfChar
	nCharChars = CMemory::GetSizeOfChar( pBuf, nBufLen, i );
	while( nCharChars > 0 && i < nBufLen ){
		j = mbtowc( &wchar, &pBuf[i], nCharChars );
		if( j == -1 || j == 0 ){
			pBufUnicode[k] = 0;
			pBufUnicode[k + 1] = pBuf[i];
			++i;
		}else{
#if 0
		// 2005.11.16 実用に耐えないほど速度低下するので無効化する
			if( j != 2 ){
				MYTRACE_A( "%dバイトのUnicode文字に変換された SJIS(?)=%x %x\n", j,pBuf[i],((nCharChars >= 2)?(pBuf[i + 1]):0) );
			}
#endif
			*((wchar_t*)&(pBufUnicode[k])) = wchar;
			i += j;
		}
		k += 2;
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( pBuf, nBufLen, i );
	}
	*ppBufUnicode = pBufUnicode;
	return k;
}





/* Unicode文字列をASCII&SJISに変換 */
int CMemory::MemUnicodeToSJIS( char** ppBufSJIS, const char*pBuf, int nBufLen )
{
	int			i, j, k;
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
				cmemWork.AppendString( &m_pData[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			nBgn++;
		}
	}
	if( 0  < m_nDataLen - nBgnOld ){
		cmemWork.AppendString( &m_pData[nBgnOld], m_nDataLen - nBgnOld );
	}
	SetNativeData( &cmemWork );
	return;
}



/* 文字列置換（日本語考慮版） */
void CMemory::Replace_j( char* pszFrom, char* pszTo )
{
	CMemory		cmemWork;
	int			nFromLen = strlen( pszFrom );
	int			nToLen = strlen( pszTo );
	int			nBgnOld = 0;
	int			nBgn = 0;
	while( nBgn <= m_nDataLen - nFromLen ){
		if( 0 == memcmp( &m_pData[nBgn], pszFrom, nFromLen ) ){
			if( 0  < nBgn - nBgnOld ){
				cmemWork.AppendString( &m_pData[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.AppendString( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			if( _IS_SJIS_1( (unsigned char)m_pData[nBgn] ) ) nBgn++;
			nBgn++;
		}
	}
	if( 0  < m_nDataLen - nBgnOld ){
		cmemWork.AppendString( &m_pData[nBgnOld], m_nDataLen - nBgnOld );
	}
	SetNativeData( &cmemWork );
	return;
}



/* 等しい内容か */
int CMemory::IsEqual( CMemory& cmem1, CMemory& cmem2 )
{
	char*	psz1;
	char*	psz2;
	int		nLen1;
	int		nLen2;

	psz1 = cmem1.GetStringPtr( &nLen1 );
	psz2 = cmem2.GetStringPtr( &nLen2 );
	if( nLen1 == nLen2 ){
		if( 0 == memcmp( psz1, psz2, nLen1 ) ){
			return TRUE;
		}
	}
	return FALSE;
}





/* 半角→全角 */
void CMemory::ToZenkaku(
		int bHiragana,		/* 1== ひらがな 0==カタカナ //2==英数専用 2001/07/30 Misaka 追加 */
		int bHanKataOnly	/* 1== 半角カタカナにのみ作用する*/
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
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
		if( nCharChars == 1){
			bHenkanOK = FALSE;
			if( bHanKataOnly ){	/* 1== 半角カタカナにのみ作用する */
				if( NULL != strchr( (const char *)pszHanKataSet, pBuf[i] ) ){
					bHenkanOK = TRUE;
				}
			}else{
				//! 英数変換用に新たな条件を付加 2001/07/30 Misaka
				if( ( (unsigned char)0x20 <= pBuf[i] && pBuf[i] <= (unsigned char)0x7E ) ||
					( bHiragana != 2 && (unsigned char)0xA1 <= pBuf[i] && pBuf[i] <= (unsigned char)0xDF )
				){
					bHenkanOK = TRUE;
				}
			}
			if( bHenkanOK ){
				usSrc = pBuf[i];
				if( FALSE == bHiragana &&
					pBuf[i]		== (unsigned char)'ｳ' &&
					pBuf[i + 1] == (unsigned char)'ﾞ' &&
					bHiragana != 2
				){
					usDes = (unsigned short)0x8394; /* ヴ */
					nCharChars = 2;
				}else {
					usDes = _mbbtombc( usSrc );
					/* 濁音 */
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'ﾞ' && NULL != strchr( (const char *)pszDakuSet, pBuf[i] ) ){
						usDes++;
						nCharChars = 2;
					}
					/* 拗音 */
					//! 英数変換用に新たな条件を付加 2001/07/30 Misaka
					//! bHiragana != 2 //英数変換フラグがオンではない場合
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'ﾟ' && NULL != strchr( (const char *)pszYouSet, pBuf[i] ) ){
						usDes += 2;
						nCharChars = 2;
					}
				}

				if( bHiragana == 1 ){
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
			if( bHanKataOnly == 0 ){
				if( bHiragana == 1 ){//英数変換を付加したために数値で指定した　2001/07/30 Misaka
					/* 全角ひらがなに変換可能な全角カタカナならば、ひらがなに変換する */
					if( (unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ){	/* ァ～ミ */
						usDes = usSrc - (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ){	/* ム～ン */
						usDes = usSrc - (unsigned short)0x00a2;
					}
				}else if( bHiragana == 0 ){//英数変換を付加したために数値で指定した　2001/07/30 Misaka
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
	SetString( (const char *)pBufDes, nBufDesLen );
	delete [] pBufDes;


	return;
}



/*!	全角→半角
	nMode
		0x01	カタカナに影響アリ
		0x02	ひらがなに影響アリ
		0x04	英数字に影響アリ
*/
#define TO_KATAKANA	0x01
#define TO_HIRAGANA	0x02
#define TO_EISU		0x04
void CMemory::ToHankaku(
		int nMode		/* 0==カタカナ 1== ひらがな 2==英数専用 */
)
{
	static unsigned char*	pszZenDAKU = (unsigned char*)"がぎぐげござじずぜぞだぢづでどばびぶべぼガギグゲゴザジズゼゾダヂヅデドバビブベボヴ";
	static unsigned char*	pszZenYOU  = (unsigned char*)"ぱぴぷぺぽパピプペポ";

	/* 入力 */
	unsigned char*			pBuf = (unsigned char*)m_pData;
	int						nBufLen = m_nDataLen;
	unsigned int			uiSrc;

	/* 出力 */
	unsigned char*			pBufDes = new unsigned char[nBufLen + 1];
	int						nBufDesLen = 0;
	unsigned int			uiDes;
	if( NULL ==	pBufDes ){
		return;
	}

	/* 作業用 */
	int						nCharChars;
	unsigned char			pszZen[3];	//	全角文字用バッファ
	int i;
	BOOL bHenkanOK;
	bool bInHiraKata = false;				// 前の文字がカタカナorひらがなだったなら、trueとし、長音、濁点、半濁点を半角へ変換可能とする
	for( i = 0; i < nBufLen; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pBuf, nBufLen, i );
		if( nCharChars == 2 ){
			uiSrc = pBuf[i + 1] | ( pBuf[i] << 8 );
			
			bHenkanOK = FALSE;
			if( nMode == 0x00 ){	//	どんな文字もokモード
				bHenkanOK = TRUE;
			}
			if( nMode & TO_KATAKANA ){	/* カタカナに作用する */
				if( ( 0x8340 <= uiSrc && uiSrc <= 0x8396 )
				// 2007.01.26 maru ToZenkakuの動作に合わせてカナ記号も変換対象に含める(。「」、・)
				||( 0x8141==uiSrc || 0x8142==uiSrc || 0x8145==uiSrc || 0x8175==uiSrc || 0x8176==uiSrc) ){
					bHenkanOK = TRUE;
					bInHiraKata = true;
				} else {
					// 2003-04-30 かろと 長音がが変換できなかったのを修正
					// 但し、ひらがな・カタカナの区別がない文字なので直前の文字で決定する
					if( bInHiraKata == true ) {
						// 長音"ー"(0x815b)・濁点(0x814a)・半濁点(0x814b)
						if( uiSrc == 0x815b || uiSrc == 0x814a || uiSrc == 0x814b ) {
							bHenkanOK = TRUE;
						} else {
							bInHiraKata = false;
						}
					}
				}
			}
			if( nMode & TO_HIRAGANA ){	/* ひらがなに作用する */
				if( 0x829F <= uiSrc && uiSrc <= 0x82F1 ){
					bHenkanOK = TRUE;
					bInHiraKata = true;
				} else {
					// 2003-04-30 かろと 長音がが変換できなかったのを修正
					// 但し、ひらがな・カタカナの区別がない文字なので直前の文字で決定する
					if( bInHiraKata == true ) {
						// 長音"ー"(0x815b)・濁点(0x814a)・半濁点(0x814b)
						if( uiSrc == 0x815b || uiSrc == 0x814a || uiSrc == 0x814b ) {
							bHenkanOK = TRUE;
						} else {
							bInHiraKata = false;
						}
					}
				}
			}
			if ( nMode & TO_EISU ){		/* 英数に作用する */
				/* From Here 2007.01.16 maru 7bitのASCII範囲を対象とするように変更 */
				if( 0x8140 <= uiSrc && uiSrc <= 0x8197){
					switch(uiSrc){	/* カナ記号が変換されないように */
					case 0x8141:	// 、
					case 0x8142:	// 。
					case 0x8145:	// ・
					case 0x814a:	// ゛
					case 0x814b:	// ゜
					case 0x815b:	// ー
					case 0x8175:	//「
					case 0x8176:	// 」
						break;
					default:
						bHenkanOK = TRUE;
					}
				}
				else if( 0x824f <= uiSrc && uiSrc <= 0x829a){	//	英数字
					bHenkanOK = TRUE;
				}
				/* From Here 2007.01.16 maru 7bitのASCII範囲を対象とするように変更 */
			}
			if (bHenkanOK == TRUE){
				uiDes = _mbctombb( uiSrc );
				if( uiDes == uiSrc ){	//	変換不可能
					memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
					nBufDesLen += nCharChars;
				}else{
					pBufDes[nBufDesLen] = (unsigned char)uiDes;
					nBufDesLen++;

					memcpy( pszZen, &pBuf[i], 2 );
					pszZen[2] = '\0';
					/* 濁音探し */
					if( NULL != strstr( (const char *)pszZenDAKU, (const char *)pszZen ) ){
						pBufDes[nBufDesLen] = (unsigned char)'ﾞ';
						nBufDesLen++;
					}else
					/* 拗音探し */
					if( NULL != strstr( (const char *)pszZenYOU,  (const char *)pszZen ) ){
						pBufDes[nBufDesLen] = (unsigned char)'ﾟ';
						nBufDesLen++;
					}
				}
			}
			else {
				memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
				nBufDesLen += nCharChars;
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
	SetString( (const char *)pBufDes, nBufDesLen );
	delete [] pBufDes;

}


/*
|| ファイルの日本語コードセット判別
||
|| 【戻り値】
||	SJIS		0
||	JIS			1
||	EUC			2
||	Unicode		3
||	UTF-8		4
||	UTF-7		5
||	UnicodeBE	6
||	エラー		-1
||	エラー以外は ECodeType を使う
*/
ECodeType CMemory::CheckKanjiCodeOfFile( const char* pszFile )
{
	HFILE					hFile;
	HGLOBAL					hgData;
	const unsigned char*	pBuf;
	int						nBufLen;
	ECodeType				nCodeType;

	/* メモリ確保 & ファイル読み込み */
	hgData = NULL;
	hFile = _lopen( pszFile, OF_READ );
	if( HFILE_ERROR == hFile ){
		return CODE_ERROR;
	}
	nBufLen = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );
	if( nBufLen > CheckKanjiCode_MAXREADLENGTH ){
		nBufLen = CheckKanjiCode_MAXREADLENGTH;
	}
	
	if( 0 == nBufLen ){
		_lclose( hFile );
		return CODE_SJIS;
	}
	
	hgData = ::GlobalAlloc( GHND, nBufLen + 1 );
	if( NULL == hgData ){
		_lclose( hFile );
		return CODE_ERROR;
	}
	pBuf = (const unsigned char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );

	/* 日本語コードセット判別 */
// From Here 2006.09.22  rastiv
	nCodeType = Charcode::DetectUnicodeBom( (const char *)pBuf, nBufLen );
	if( nCodeType == 0 ){
		// Unicode BOM は検出されませんでした．
		nCodeType = CheckKanjiCode( (const unsigned char *)pBuf, nBufLen );
	}
// To Here

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
||	SJIS		0
||	JIS			1
||	EUC			2
||	Unicode		3
||	UTF-8		4
||	UTF-7		5
||	UnicodeBE	6
*/

// 2006.12.16  rastiv   アルゴリズムを改定．
ECodeType CMemory::CheckKanjiCode( const unsigned char* pBuf, int nBufLen )
{
	CESI cesi;
	WCCODE_INFO wci;
	MBCODE_INFO mbci;
	int nPt;	// 
	
	if( !cesi.ScanEncoding((const char *)pBuf, nBufLen) ){
		// スキャンに失敗しました．
		return CODE_SJIS;  // ←デフォルト文字コードを返却．
	}
	
	nPt = cesi.DetectUnicode( &wci );
	if( 0 != nPt ){
		// UNICODE が検出されました.
		return wci.eCodeID;
	}//else{
		nPt = cesi.DetectMultibyte( &mbci );
		//nPt := 特有バイト数 － 不正バイト数
		if( 0 < nPt ){
			return mbci.eCodeID;
		}//else{
			return CODE_SJIS;  // ←デフォルト文字コードを返却．
		//}
	//}
}




/* 自動判別→SJISコード変換 */
void CMemory::AUTOToSJIS( void )
{
	ECodeType	nCodeType;
	/*
	|| 日本語コードセット判別
	||
	|| 【戻り値】
	||	SJIS		0
	||	JIS			1
	||	EUC			2
	||	Unicode		3
	||	UTF-8		4
	||	UTF-7		5
	||	UnicodeBE	6
	*/
	nCodeType = CheckKanjiCode( (const unsigned char*)m_pData, m_nDataLen );
	switch( nCodeType ){
	case CODE_JIS:			JIStoSJIS();break;		/* E-Mail(JIS→SJIS)コード変換 */
	case CODE_EUC:			EUCToSJIS();break;		/* EUC→SJISコード変換 */
	case CODE_UNICODE:		UnicodeToSJIS();break;	/* Unicode→SJISコード変換 */
	case CODE_UTF8:			UTF8ToSJIS();break;		/* UTF-8→SJISコード変換 */
	case CODE_UTF7:			UTF7ToSJIS();break;		/* UTF-7→SJISコード変換 */
	case CODE_UNICODEBE:	UnicodeBEToSJIS();break;/* UnicodeBE→SJISコード変換 */
	}
	return;
}





/* TAB→空白 */
void CMemory::TABToSPACE( int nTabSpace	/* TABの文字数 */, int nStartColumn )
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
	CEol		cEol;
	nBgn = 0;
	nPosDes = 0;
	/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
//			nPosX = 0;
            // 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？） 
	        nPosX = (m_pData == pLine)? nStartColumn: 0;   // 処理中のiに対応する表示桁位置 
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
//			nPosX = 0;
            // 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？） 
	        nPosX = (m_pData == pLine)? nStartColumn: 0;   // 処理中のiに対応する表示桁位置 
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

	SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}


//!空白→TAB変換
/*!
	@param nTabSpace TABの文字数
	単独のスペースは変換しない

	@author Stonee
	@date 2001/5/27
*/
void CMemory::SPACEToTAB( int nTabSpace, int nStartColumn	 )
{
	const char*	pLine;
	int			nLineLen;
	char*		pDes;
	int			nBgn;
	int			i;
	int			nPosDes;
	int			nPosX;
	CEol		cEol;

	BOOL		bSpace = FALSE;	//スペースの処理中かどうか
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
//			nPosX = 0;	// 処理中のiに対応する表示桁位置
            // 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？） 
	        nPosX = (m_pData == pLine)? nStartColumn: 0;   // 処理中のiに対応する表示桁位置 
			bSpace = FALSE;	//直前がスペースか
			nStartPos = 0;	// スペースの先頭
			for( i = 0; i < nLineLen; ++i ){
				if( SPACE == pLine[i] || TAB == pLine[i] ){
					if( bSpace == FALSE ){
						nStartPos = nPosX;
					}
					bSpace = TRUE;
					if( SPACE == pLine[i] ){
						nPosX++;
					}else if( TAB == pLine[i] ){
						nPosX += nTabSpace - (nPosX % nTabSpace);
					}
				}else{
					if( bSpace ){
						if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
							pDes[nPosDes] = SPACE;
							nPosDes++;
						} else{
							for( j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++ ){
								pDes[nPosDes] = TAB;
								nPosDes++;
								nStartPos += nTabSpace - ( nStartPos % nTabSpace );
							}
							//	2003.08.05 Moca
							//	変換後にTABが1つも入らない場合にスペースを詰めすぎて
							//	バッファをはみ出すのを修正
							for( j = nStartPos; j < nPosX; j++ ){
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
			if( bSpace ){
				if( (1 == nPosX - nStartPos) && (SPACE == pLine[i - 1]) ){
					pDes[nPosDes] = SPACE;
					nPosDes++;
				} else{
					//for( j = nStartPos - 1; (j + nTabSpace) <= nPosX + 1; j+=nTabSpace ){
					for( j = nStartPos / nTabSpace; j < (nPosX / nTabSpace); j++ ){
						pDes[nPosDes] = TAB;
						nPosDes++;
						nStartPos += nTabSpace - ( nStartPos % nTabSpace );
					}
					//	2003.08.05 Moca
					//	変換後にTABが1つも入らない場合にスペースを詰めすぎて
					//	バッファをはみ出すのを修正
					for( j = nStartPos; j < nPosX; j++ ){
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

	SetString( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}





/* !上位バイトと下位バイトを交換する

	@author Moca
	@date 2002/5/27
	
	@note	nBufLen が2の倍数でないときは、最後の1バイトは交換されない
*/
void CMemory::SwapHLByte( void ){
	unsigned char *p;
	unsigned char ctemp;
	unsigned char *p_end;
	unsigned int *pdwchar;
	unsigned int *pdw_end;
	unsigned char*	pBuf;
	int			nBufLen;

	pBuf = (unsigned char*)GetStringPtr( &nBufLen );

	if( nBufLen < 2){
		return;
	}
	// 高速化のため
	pdwchar = (unsigned int*)pBuf;
	if( (size_t)pBuf % 2 == 0){
		if( (size_t)pBuf % 4 == 2 ){
			ctemp = pBuf[0];
			pBuf[0]  = pBuf[1];
			pBuf[1]  = ctemp;
			pdwchar = (unsigned int*)(pBuf + 2);
		}
		pdw_end = (unsigned int*)(pBuf + nBufLen - sizeof(unsigned int));

		for(; pdwchar <= pdw_end ; ++pdwchar ){
			pdwchar[0] = ((pdwchar[0] & (unsigned int)0xff00ff00) >> 8) |
						 ((pdwchar[0] & (unsigned int)0x00ff00ff) << 8);
		}
	}
	p = (unsigned char*)pdwchar;
	p_end = pBuf + nBufLen - 2;
	
	for(; p <= p_end ; p+=2){
		ctemp = p[0];
		p[0]  = p[1];
		p[1]  = ctemp;
	}
	return;
}

/*
|| バッファサイズの調整
*/
void CMemory::AllocStringBuffer( int nNewDataLen )
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
			_T("CMemory::AllocStringBuffer(nNewDataLen==%d)\nメモリ確保に失敗しました。\n"), nNewDataLen
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
void CMemory::SetString( const char* pData, int nDataLen )
{
	Empty();
	AllocStringBuffer( nDataLen );
	AddData( pData, nDataLen );
	return;
}



/* バッファの内容を置き換える */
void CMemory::SetString( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );

	Empty();
	AllocStringBuffer( nDataLen );
	AddData( pszData, nDataLen );
	return;
}


/* バッファの内容を置き換える */
void CMemory::SetNativeData( const CMemory* pcmemData )
{
	char*	pData;
	int		nDataLen;
	pData = pcmemData->GetStringPtr( &nDataLen );
	Empty();
	AllocStringBuffer( nDataLen );
	AddData( pData, nDataLen );
	return;
}


/* バッファの最後にデータを追加する（publicメンバ）*/
const char* CMemory::AppendString( const char* pData, int nDataLen )
{
	AllocStringBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
	return m_pData;
}
/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendString( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );
	AllocStringBuffer( m_nDataLen + nDataLen );
	AddData( pszData, nDataLen );
}
/* バッファの最後にデータを追加する（publicメンバ）*/
void CMemory::AppendNativeData( const CMemory& pcmemData )
{
	char*	pData;
	int		nDataLen;
	pData = pcmemData.GetStringPtr( &nDataLen );
	AllocStringBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
}

void CMemory::Empty( void )
{
	free( m_pData );
	m_pData = NULL;
	m_nDataBufSize = 0;
	m_nDataLen = 0;
	return;
}

/*!
	@param[in] pData 位置を求めたい文字列の先頭
	@param[in] nDataLen 文字列長
	@param[in] nIdx 位置(0オリジン)

	@date 2005-09-02 D.S.Koba 作成

	@note nIdxは予め文字の先頭位置とわかっていなければならない．
	2バイト文字の2バイト目をnIdxに与えると正しい結果が得られない．
*/
int CMemory::GetSizeOfChar( const char* pData, const int nDataLen, const int nIdx )
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

/*[EOF]*/
