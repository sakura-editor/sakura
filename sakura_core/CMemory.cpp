//	$Id$
/*!	@file
	ƒƒ‚ƒŠƒoƒbƒtƒ@ƒNƒ‰ƒX

	@author Norio Nakatani
	@date 1998/03/06 V‹Kì¬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro, genta
	Copyright (C) 2001, mik, misaka, Stonee, hor
	Copyright (C) 2002, Moca, sui

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

#include <stdio.h>
#include <windows.h>
#include <mbstring.h>
#include <ctype.h>
#include <locale.h>
#include "CMemory.h"
#include "etc_uty.h"
#include "CEol.h"// 2002/2/3 aroka
#include "charcode.h"


//#ifdef _DEBUG
#include "global.h"
#include "debug.h"
#include "CRunningTimer.h"
//#endif



#define ESC_JIS		"\x01b$B"
#define ESC_ASCII	"\x01b(B"
#define ESC_8BIT	"\x01b(I"

#define MIME_BASE64	1
#define MIME_QUOTED	2

#define UUDECODE_CHAR(c) ((((unsigned char)c) - ((unsigned char)32)) & (unsigned char)0x3f)

/* •¶ší•Ê */
#define CHAR_ASCII		0	/* ASCII•¶š */
#define CHAR_8BITCODE	1	/* 8ƒrƒbƒgƒR[ƒh(”¼ŠpƒJƒ^ƒJƒi‚È‚Ç) */
#define CHAR_ZENKAKU	2	/* ‘SŠp•¶š */
#define CHAR_NULL		3	/* ‚È‚É‚à‚È‚¢ */



/*///////////////////////////////////////////////////////////////////////////
//
//	CMemory::CMemory
//	CMemory()
//
//	à–¾
//		CMemoryƒNƒ‰ƒX ƒRƒ“ƒXƒgƒ‰ƒNƒ^
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
//	ˆø”
//	  pData		Ši”[ƒf[ƒ^ƒAƒhƒŒƒX
//	  nDataLen	Ši”[ƒf[ƒ^‚Ì—LŒø’·
//
//	à–¾
//		CMemoryƒNƒ‰ƒX  ƒRƒ“ƒXƒgƒ‰ƒNƒ^
//
//
//	–ß’l
//		‚È‚µ
//
//	”õl
//		Ši”[ƒf[ƒ^‚É‚ÍNULL‚ğŠÜ‚Ş‚±‚Æ‚ª‚Å‚«‚é
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


///* ƒf[ƒ^‚ÌÅŒã‚É’Ç‰Á publicƒƒ“ƒo */
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
	char szChar[2];
	szChar[0] = ch;
	szChar[1] = '\0';
	AllocBuffer( m_nDataLen + sizeof( ch ) );
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
||  GetAt()‚Æ“¯‹@”\
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
|| ƒoƒbƒtƒ@‚ÌÅŒã‚Éƒf[ƒ^‚ğ’Ç‰Á‚·‚éiprotectƒƒ“ƒo
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
	@brief Šg’£”Å SJIS¨JIS•ÏŠ·

	SJISƒR[ƒh‚ğJIS‚É•ÏŠ·‚·‚éD‚»‚ÌÛCJIS‚É‘Î‰—Ìˆæ‚Ì‚È‚¢IBMŠg’£•¶š‚ğ
	NEC‘I’èIBMŠg’£•¶š‚É•ÏŠ·‚·‚éD

	Shift_JIS fa40`fc4b ‚Ì”ÍˆÍ‚Ì•¶š‚Í 8754`879a ‚Ü‚½‚Í ed40`eefc ‚É
	Uİ‚·‚é•¶š‚É•ÏŠ·‚³‚ê‚½Œã‚ÉCJIS‚É•ÏŠ·‚³‚ê‚Ü‚·D
	
	@param pszSrc [in] •ÏŠ·‚·‚é•¶š—ñ‚Ö‚Ìƒ|ƒCƒ“ƒ^ (Shift JIS)
	
	@author ‚·‚¢
	@date 2002.10.03 1•¶š‚Ì‚İˆµ‚¢C•ÏŠ·‚Ü‚Ås‚¤‚æ‚¤‚É•ÏX genta
*/
unsigned short CMemory::_mbcjmstojis_ex( unsigned char* pszSrc )
{
	unsigned int	tmpw;	/* © int ‚ª 16 bit ˆÈã‚Å‚ ‚é–‚ğŠú‘Ò‚µ‚Ä‚¢‚Ü‚·B */
	
	if(	_IS_SJIS_1(* pszSrc    ) &&	/* Shift_JIS ‘SŠp•¶š‚Ì 1ƒoƒCƒg–Ú */
		_IS_SJIS_2(*(pszSrc+1) )	/* Shift_JIS ‘SŠp•¶š‚Ì 2ƒoƒCƒg–Ú */
	){	/* Shift_JIS‘SŠp•¶š‚Å‚ ‚é */
		tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( *pszSrc == 0x0fa ) ||
			( *pszSrc == 0x0fb ) ||
			( ( *pszSrc == 0x0fc ) && ( *(pszSrc+1) <= 0x04b ) )
		) {		/* fa40`fc4b ‚Ì•¶š‚Å‚ ‚éB */
			/* •¶šƒR[ƒh•ÏŠ·ˆ— */
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40`fa49 ¨ eeef`eef8 iîï`îøj */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a`fa53 ¨ 8754`875d i‡T`‡]j */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54`fa57 ¨ eef9`eefc iîù ` îüj */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* ‡Š */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* ‡‚ */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* ‡„ */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* ‡š */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c`fa7e ‚ÌŠ¿š */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80`fa9b ‚ÌŠ¿š */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c`fafc ‚ÌŠ¿š */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40`fb5b ‚ÌŠ¿š */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c`fb7e ‚ÌŠ¿š */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80`fb9b ‚ÌŠ¿š */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c`fbfc ‚ÌŠ¿š */
			else{							tmpw -= 0x0d5f;	}	/* fc40`fc4b ‚ÌŠ¿š */
		}
		return (unsigned short) _mbcjmstojis( tmpw );
	}
	return 0;
}


/* ƒR[ƒh•ÏŠ· SJIS¨JIS */
void CMemory::SJIStoJIS( void )
{
	char*	pBufJIS;
	int		nBufJISLen;
	CMemory	cMem;

	/* SJIS¨JIS */
	StrSJIStoJIS( &cMem, (unsigned char *)m_pData, m_nDataLen );
	pBufJIS = cMem.GetPtr( &nBufJISLen );
	SetData( pBufJIS, nBufJISLen );
	return;
}


/* SJIS¨JIS */
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
	nCharKind = CHAR_ASCII;		/* ASCII•¶š */
	nCharKindOld = nCharKind;
	bChange = FALSE;
//	/* •¶ší•Ê */
//	#define CHAR_ASCII		0	/* ASCII•¶š */
//	#define CHAR_8BITCODE	1	/* 8ƒrƒbƒgƒR[ƒh(”¼ŠpƒJƒ^ƒJƒi‚È‚Ç) */
//	#define CHAR_ZENKAKU	2	/* ‘SŠp•¶š */

	pcmemDes->SetDataSz( "" );
	pcmemDes->AllocBuffer( nSrcLen );
//	bSJISKAN  = FALSE;
	nWorkBgn = 0;
	for( i = 0;; i++ ){
		/* ˆ—‚ªI—¹‚µ‚½ */
		if( i >= nSrcLen ){
			nCharKind = CHAR_NULL;	/* ‚È‚É‚à‚È‚¢ */
		}else
		// Š¿š‚©H
		if( ( i < nSrcLen - 1) &&
			/* SJIS‘SŠpƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚© */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
			_IS_SJIS_1( (unsigned char)pszSrc[i + 0] ) &&
			/* SJIS‘SŠpƒR[ƒh‚Ì2ƒoƒCƒg–Ú‚© */
			_IS_SJIS_2( (unsigned char)pszSrc[i + 1] )
		){
			nCharKind = CHAR_ZENKAKU;	/* ‘SŠp•¶š */
//			++i;
		}else
		if( (unsigned char)pszSrc[i] & (unsigned char)0x80 ){
			nCharKind = CHAR_8BITCODE;	/* 8ƒrƒbƒgƒR[ƒh(”¼ŠpƒJƒ^ƒJƒi‚È‚Ç) */
		}else{
			nCharKind = CHAR_ASCII;		/* ASCII•¶š */
		}
		/* •¶ší—Ş‚ª•Ï‰»‚µ‚½ */
		if( nCharKindOld != nCharKind ){
			if( CHAR_NULL != nCharKind ){
				bChange = TRUE;
			}

			nWorkLen = i - nWorkBgn;
			/* ˆÈ‘O‚Ì•¶ší—Ş */
			switch( nCharKindOld ){
			case CHAR_ASCII:	/* ASCII•¶š */
				if( 0 < nWorkLen ){
					pcmemDes->Append( (char *)&(pszSrc[nWorkBgn]), nWorkLen );
				}
				break;
			case CHAR_8BITCODE:	/* 8ƒrƒbƒgƒR[ƒh(”¼ŠpƒJƒ^ƒJƒi‚È‚Ç) */
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
			case CHAR_ZENKAKU:	/* ‘SŠp•¶š */
				if( 0 < nWorkLen ){
					pszWork = new unsigned char[nWorkLen + 1];
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[ nWorkLen ] = '\0';
					// SJIS¨JIS•ÏŠ·
					nWorkLen = MemSJIStoJIS( (unsigned char*)pszWork, nWorkLen );
					pcmemDes->Append( (char *)pszWork, nWorkLen );
					delete [] pszWork;
				}
				break;
			}
			/* V‚µ‚¢•¶ší—Ş */
			switch( nCharKind ){
			case CHAR_ASCII:	/* ASCII•¶š */
				pcmemDes->AppendSz( ESC_ASCII );
				break;
			case CHAR_NULL:		/* ‚È‚É‚à‚È‚¢ */
				if( bChange &&					/* ˆê‰ñ‚Í•¶ší‚ª•Ï‰»‚µ‚½ */
					nCharKindOld != CHAR_ASCII	/* ’¼‘O‚ªASCII•¶š‚Å‚Í‚È‚¢ */
				){
					pcmemDes->AppendSz( ESC_ASCII );
				}
				break;
			case CHAR_8BITCODE:	/* 8ƒrƒbƒgƒR[ƒh(”¼ŠpƒJƒ^ƒJƒi‚È‚Ç) */
				pcmemDes->AppendSz( ESC_8BIT );
				break;
			case CHAR_ZENKAKU:	/* ‘SŠp•¶š */
				pcmemDes->AppendSz( ESC_JIS );
				break;
			}
			nCharKindOld = nCharKind;
			nWorkBgn = i;
			if( nCharKind == CHAR_NULL ){	/* ‚È‚É‚à‚È‚¢ */
				break;
			}
		}
		if( nCharKind == CHAR_ZENKAKU ){	/* ‘SŠp•¶š */
			++i;
		}
	}
	return pcmemDes->m_nDataLen;
}



/* SJIS¨JIS•ÏŠ· */
long CMemory::MemSJIStoJIS( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j;
	char *			pszDes;
	unsigned short	sCode;

	pszDes = new char[nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	for( i = 0; i < nSrcLen - 1; i++ ){
		//	Oct. 3, 2002 genta IBMŠg’£•¶š‘Î‰
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


/* ƒR[ƒh•ÏŠ· JIS¨SJIS */
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
	int				nESCSeqLen  = - 1; // ƒGƒXƒP[ƒvƒV[ƒPƒ“ƒX’· - 1

	pszDes = new unsigned char [nSrcLen + 1];
	memset( pszDes, 0, nSrcLen + 1 );
	j = 0;
	if( false != bMIMEdecode ){
		for( i = 0; i < nSrcLen; i++ ){
			if( i <= nSrcLen - 16 && '=' == pszSrc[i] ){
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
//					memset( pszWork, 0, nWorkLen + 1 );
					memcpy( pszWork, &pszSrc[nWorkBgn], nWorkLen );
					pszWork[nWorkLen] = '\0';
					switch( nMEME_Selected ){
					  case MIME_BASE64:
						// Base64ƒfƒR[ƒh
						nWorkLen = MemBASE64_Decode(pszWork, nWorkLen);
						break;
					  case MIME_QUOTED:
						// Quoted-PrintableƒfƒR[ƒh
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
			nWorkBgn -= 16; // MIMEƒwƒbƒ_‚ğ‚»‚Ì‚Ü‚ÜƒRƒs[
			nWorkLen = i - nWorkBgn;
			memcpy( &pszDes[j], &pszSrc[nWorkBgn], nWorkLen );
			j += nWorkLen;
		}

		// ”ñASCIIƒeƒLƒXƒg‘Î‰ƒƒbƒZ[ƒWƒwƒbƒ_‚ÌMIMEƒR[ƒh
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
					// JIS¨SJIS•ÏŠ·
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

	// ESCSeq‚ªASCII‚É–ß‚ç‚È‚©‚Á‚½‚Æ‚«‚ÉCƒf[ƒ^‚ğ¸‚í‚È‚¢‚æ‚¤‚É
	if( CHAR_ZENKAKU == nISOCode ){
		if( 0 < i - nWorkBgn ){
			nWorkBgn -= nESCSeqLen + 1; // ESCSeq‚àc‚µ‚Ä‚¨‚«‚½‚¢
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





/* •¶š‚ªBase64‚Ìƒf[ƒ^‚© */
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

// BASE64 => ƒGƒ“ƒR[ƒhŒã
// 4•¶š  => 3•¶š

// Base64ƒfƒR[ƒh
long CMemory::MemBASE64_Decode( unsigned char* pszSrc, long nSrcLen )
{
	int				i, j, k;
	long			nSrcSize;
	long			lData;
	unsigned char*	pszDes;
	long			nDesLen;
	long			lDesSize;
	int				sMax;

	//Src‚Ì—LŒø’·‚ÌZo
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
* Base64ƒGƒ“ƒR[ƒh
*
*	•„†‰»‚µ‚½ƒf[ƒ^‚ÍAV‚½‚ÉŠm•Û‚µ‚½ƒƒ‚ƒŠ‚ÉŠi”[‚³‚ê‚Ü‚·
*	I—¹‚ÉA‚»‚Ìƒƒ‚ƒŠƒnƒ“ƒhƒ‹‚ğw’è‚³‚ê‚½ƒAƒhƒŒƒX‚ÉŠi”[‚µ‚Ü‚·
*	•„†‰»‚³‚ê‚½ƒf[ƒ^—ñ‚ÍAˆê‰NULLI’[•¶š—ñ‚É‚È‚Á‚Ä‚¢‚Ü‚·
*
*/
int CMemory::MemBASE64_Encode(
	const char*	pszSrc		,	// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^
	int			nSrcLen		,	// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^’·
	char**		ppszDes		,	// Œ‹‰Êƒf[ƒ^Ši”[ƒƒ‚ƒŠƒ|ƒCƒ“ƒ^‚ÌƒAƒhƒŒƒX
//	HGLOBAL*	phgDes		,	// Œ‹‰Êƒf[ƒ^‚Ìƒƒ‚ƒŠƒnƒ“ƒhƒ‹Ši”[êŠ
//	long*		pnDesLen	,	// Œ‹‰Êƒf[ƒ^‚Ìƒf[ƒ^’·Ši”[êŠ
	int			nWrap		,	// ƒGƒ“ƒR[ƒhŒã‚Ìƒf[ƒ^‚ğ©“®“I‚ÉCRLF‚ÅÜ‚è•Ô‚·ê‡‚Ì‚PsÅ‘å•¶š” (-1‚ªw’è‚³‚ê‚½ê‡‚ÍÜ‚è•Ô‚³‚È‚¢)
	int			bPadding		// ƒpƒfƒBƒ“ƒO‚·‚é‚©
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

	// •„†‰»Œã‚Ì’·‚³‚ğZoiƒwƒbƒ_[Eƒtƒbƒ^[‚ğœ‚­j
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
		if( bPadding ){		// ƒpƒfƒBƒ“ƒO‚·‚é‚©
			k = 4;
		}else{
			nDesLen -= (4 - k);
		}
		for( n = 0; n < k; n++ ){
			// ©“®Ü‚è•Ô‚µ‚Ìˆ—
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



/* Base64ƒfƒR[ƒh */
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

			// Base64ƒfƒR[ƒh
			nDesLen = MemBASE64_Decode( (unsigned char *)pData, nPos - nBgn );
			cmemBuf.Append( pData, nDesLen );
			delete [] pData;
		}
		nBgn = nPos;
	}
	SetData( cmemBuf.GetPtr(), cmemBuf.m_nDataLen );
	return;
}




/* Uudecode (ƒfƒR[ƒhj*/
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
					/* Å‰‚Ìƒg[ƒNƒ“‚ğæ“¾‚µ‚Ü‚·B*/
					pTok = strtok( (char*)pBuf, " " );
					/* Ÿ‚Ìƒg[ƒNƒ“‚ğæ“¾‚µ‚Ü‚·B*/
					pTok = strtok( NULL, " " );
					/* X‚ÉŸ‚Ìƒg[ƒNƒ“‚ğæ“¾‚µ‚Ü‚·B*/
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
	SetData( cmemBuf.GetPtr(), cmemBuf.m_nDataLen );
	return;
}



/* Quoted-PrintableƒfƒR[ƒh */
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





/* JIS¨SJIS•ÏŠ· */
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
*yŠÖ”–¼z
*	EUCToSJIS
*
*y‹@”\z
*	w’è”ÍˆÍ‚Ìƒoƒbƒtƒ@“à‚ÉEUCŠ¿šƒR[ƒh
*	‚ª‚ ‚ê‚ÎSJIS‘SŠpƒR[ƒh‚É•ÏŠ·‚·‚éB	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
*	”¼Šp•¶š‚Í•ÏŠ·‚¹‚¸‚É‚»‚Ì‚Ü‚Üc‚·B
*
*	§Œä•¶šCRLFˆÈŠO‚ÌƒoƒCƒiƒŠƒR[ƒh‚ª¬“ü‚µ‚Ä‚¢‚éê‡‚É‚ÍŒ‹‰Ê‚ª•s’è‚Æ
*	‚È‚é‚±‚Æ‚ª‚ ‚é‚Ì‚Å’ˆÓB
*	ƒoƒbƒtƒ@‚ÌÅŒã‚ÉŠ¿šƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚¾‚¯‚ª‚ ‚é‚Æ¢‚é
*
*y“ü—Íz	‚È‚µ
*
*y–ß‚è’lz	‚È‚µ
*
************************************************************************/
/* EUC¨SJISƒR[ƒh•ÏŠ· */
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
			/* ”¼ŠpƒJƒ^ƒJƒi */
			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nPtrDes++;
			nPtr += 2;
		}else
		/* EUCŠ¿šƒR[ƒh‚©? */
		if( nPtr < nBufLen - 1 && IsEUCKan1(pBuf[nPtr]) && IsEUCKan2(pBuf[nPtr + 1L]) ){
			/* ’Êí‚ÌJISƒR[ƒh‚É•ÏŠ· */
			pBuf[nPtr	  ] &= 0x7f;
			pBuf[nPtr + 1L] &= 0x7f;

			/* SJISƒR[ƒh‚É•ÏŠ· */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
			sCode = (unsigned short)_mbcjistojms(
				(unsigned int)
				(((unsigned short)pBuf[nPtr] << 8) |
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




/* SJIS¨EUCƒR[ƒh•ÏŠ· */
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
				/* ”¼ŠpƒJƒ^ƒJƒi */
				pDes[nDesIdx	] = (unsigned char)0x8e;
				pDes[nDesIdx + 1] = pBuf[i];
				nDesIdx += 2;
			}else{
				pDes[nDesIdx] = pBuf[i];
				nDesIdx++;
			}
		}else
		if( nCharChars == 2 ){
			/* ‘SŠp•¶š */
			//	Oct. 3, 2002 genta IBMŠg’£•¶š‘Î‰
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
	SetData( (const char *)pDes, nDesIdx );
	delete [] pDes;
	return;
}

/****
UTF-8‚ÌƒR[ƒh
	ƒrƒbƒg—ñ		“à—e
	0xxx xxxx	1ƒoƒCƒgƒR[ƒh
	10xx xxxx	2ƒoƒCƒgƒR[ƒhA3ƒoƒCƒgƒR[ƒh‚Ì2, 3•¶š–Ú
	110x xxxx	2ƒoƒCƒgƒR[ƒh‚Ìæ“ªƒoƒCƒg
	1110 xxxx	3ƒoƒCƒgƒR[ƒh‚Ìæ“ªƒoƒCƒg

UTF-8‚ÌƒGƒ“ƒR[ƒfƒBƒ“ƒO
	ƒtƒH[ƒ}ƒbƒg	Unicode			Unicodeƒrƒbƒg—ñ						UTFƒrƒbƒg—ñ				”õl
	1ƒoƒCƒgƒR[ƒh	\u0`\u7F		0000 0000 0aaa bbbb					0aaa bbbb
	2ƒoƒCƒgƒR[ƒh	\u80`\u7FF		0000 0aaa bbbb cccc					110a aabb 10bb cccc
	3ƒoƒCƒgƒR[ƒh	\u800`\uFFFF	aaaa bbbb cccc dddd					1110 aaaa 10bb bbcc 10cc dddd
	4ƒoƒCƒgƒR[ƒh	Surrogate		110110wwwwzzzzyy + 110111yyyyxxxxxx 1111 0uuu 10uu zzzz 10yy yyyy 10xx xxxx Java‚Å‚Í–¢g—p
***/

/* UTF-8‚Ì•¶š‚© */
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

/* UTF-8¨SJISƒR[ƒh•ÏŠ· */
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
		/* UTF-8‚Ì•¶š‚© */
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





/* ƒR[ƒh•ÏŠ· SJIS¨UTF-8 */
void CMemory::SJISToUTF8( void )
{
	/* ƒR[ƒh•ÏŠ· SJIS¨Unicode */
	SJISToUnicode();

	/* ƒR[ƒh•ÏŠ· Unicode¨UTF-8 */
	UnicodeToUTF8();
	return;
}

/* ƒR[ƒh•ÏŠ· SJIS¨UTF-7 */
void CMemory::SJISToUTF7( void )
{
	/* ƒR[ƒh•ÏŠ· SJIS¨Unicode */
	SJISToUnicode();

	/* ƒR[ƒh•ÏŠ· Unicode¨UTF-7 */
	UnicodeToUTF7();
	return;
}




/* ƒR[ƒh•ÏŠ· Unicode¨UTF-8 */
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

	ƒ[ƒ‹‚É‚¨‚¢‚Ä Unicode ‚ğ—p‚¢‚½‚¢‚Æ‚«‚Ég‚¢‚Ü‚·Bƒ[ƒ‹‚ÍA7 bit •¶š‚ª‘O’ñ‚ÌŠÂ‹«‚Å—p‚¢‚ç‚ê‚é‚Ì‚ÅA
	Unicode ‚ğ UTF-7 ‚É‚æ‚Á‚Ä 7bit •¶š‚ÉƒGƒ“ƒR[ƒh‚·‚é‚Ì‚Å‚·B

	ƒGƒ“ƒR[ƒh•û–@F
		ASCII •¶š‚ÆAspaceAƒ^ƒu•¶šACRALF ‚Í’¼ÚƒGƒ“ƒR[ƒh‚µ‚Ü‚·B
		‚»‚Ì‘¼‚Ì Unicode •¶š‚ÍABase64 ƒGƒ“ƒR[ƒh‚µ‚Ü‚·B
		ASCII•¶š‚ÆABase64 ƒGƒ“ƒR[ƒh‚³‚ê‚½ Unicode ‚ğ‹æ•Ê‚·‚é‚½‚ß‚ÉA+ ‚Æ - ‚ğg‚¢‚Ü‚·B + ‚ªŒ»‚ê‚½‚çA
		ˆÈŒã‚Í Base64 •”•ªB- ‚ªŒ»‚ê‚½‚çA‚ ‚é‚¢‚ÍBase64‚Åg‚í‚ê‚È‚¢•¶š‚ªŒ»‚ê‚½‚çˆÈŒã‚Í ASCII •”•ªB
		Base64 •”•ª‚ÌŒã‚ë‚Ì'-'‚ÍƒfƒR[ƒh‚Éæ‚èœ‚©‚ê‚Ü‚·‚ªA‚»‚êˆÈŠO‚Ì•¶š‚Íæ‚èœ‚©‚ê‚Ü‚¹‚ñB
		(“ÁêƒP[ƒX : + ‚ğ•¶š‚Æ‚µ‚Äg‚¢‚½‚¢‚Æ‚«‚ÍA+- ‚Æ‘‚«‚Ü‚·B)

	Unicode:
		My name is ƒT[ƒuƒ‹ !
	UTF-7:
		My name is +MLUw/DDWMOs- !

	UTF-7 ‚Í1994/7Œ‚É RFC 1642 (Œ»İ‚Í Obsolete)‚Æ‚µ‚Ä‹KŠi‰»‚³‚êA1997/5Œ‚É RFC 2152 ‚Æ‚µ‚ÄÄ‹KŠi‰»‚³‚ê‚Ü‚µ‚½B
***/

/* ƒR[ƒh•ÏŠ· UTF-7¨SJIS */
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
			  &&  -1 != IsBASE64Char( pBuf[i + 1] ) /* •¶š‚ªBase64‚Ìƒf[ƒ^‚© */
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
					// Base64ƒfƒR[ƒh
					nWorkLen = MemBASE64_Decode( (unsigned char *)pszWork, nWorkLen );
					pszWork[nWorkLen] = '\0';
					if( 0 == nWorkLen % 2 ){
						/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
						for( j = 0; j < nWorkLen; j += 2 ){
							cWork = pszWork[j + 1];
							pszWork[j + 1] = pszWork[j];
							pszWork[j] = cWork;
						}
						pcmemWork->SetData( pszWork, nWorkLen );
						/* ƒR[ƒh•ÏŠ· Unicode¨SJIS */
						pcmemWork->UnicodeToSJIS();
						memcpy( &pDes[k], pcmemWork->m_pData, pcmemWork->m_nDataLen );
						k += pcmemWork->m_nDataLen;	//	Oct. 10, 2000 genta

						//	Oct. 10, 2000 genta
						//	'-'‚ÍBase64•”‚ÌI‚í‚è‚ğ¦‚·‹L†
						// ‚»‚êˆÈŠO‚ÍBase64‚ÌI‚í‚è‚ğ¦‚·‚Æ“¯‚É—LˆÓ‚È•¶š—ñ
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




/* ƒR[ƒh•ÏŠ· Unicode¨UTF-7 */
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
				/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				/* Base64ƒGƒ“ƒR[ƒh */
				pszBase64Buf = NULL;
				nBase64BufLen = 0;
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^
					(i - nBgn) * sizeof( wchar_t ), // ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^’·
					&pszBase64Buf,					// Œ‹‰Êƒf[ƒ^Ši”[ƒƒ‚ƒŠƒ|ƒCƒ“ƒ^‚ÌƒAƒhƒŒƒX
					-1,		// ƒGƒ“ƒR[ƒhŒã‚Ìƒf[ƒ^‚ğ©“®“I‚ÉCRLF‚ÅÜ‚è•Ô‚·ê‡‚Ì‚PsÅ‘å•¶š” (-1‚ªw’è‚³‚ê‚½ê‡‚ÍÜ‚è•Ô‚³‚È‚¢)
					FALSE	// ƒpƒfƒBƒ“ƒO‚·‚é‚©
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
		/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64ƒGƒ“ƒR[ƒh */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^
			(i - nBgn) * sizeof( wchar_t ), // ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^’·
			&pszBase64Buf,					// Œ‹‰Êƒf[ƒ^Ši”[ƒƒ‚ƒŠƒ|ƒCƒ“ƒ^‚ÌƒAƒhƒŒƒX
			-1,		// ƒGƒ“ƒR[ƒhŒã‚Ìƒf[ƒ^‚ğ©“®“I‚ÉCRLF‚ÅÜ‚è•Ô‚·ê‡‚Ì‚PsÅ‘å•¶š” (-1‚ªw’è‚³‚ê‚½ê‡‚ÍÜ‚è•Ô‚³‚È‚¢)
			FALSE	// ƒpƒfƒBƒ“ƒO‚·‚é‚©
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
				/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
				pszWork = (char*)(char*)&pUniBuf[nBgn];
				for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
					cWork = pszWork[j + 1];
					pszWork[j + 1] = pszWork[j];
					pszWork[j] = cWork;
				}
				char*	pszBase64Buf;
				int		nBase64BufLen;
				/* Base64ƒGƒ“ƒR[ƒh */
				nBase64BufLen = MemBASE64_Encode(
					(char*)&pUniBuf[nBgn],			// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^
					(i - nBgn) * sizeof( wchar_t ), // ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^’·
					&pszBase64Buf,					// Œ‹‰Êƒf[ƒ^Ši”[ƒƒ‚ƒŠƒ|ƒCƒ“ƒ^‚ÌƒAƒhƒŒƒX
					-1,		// ƒGƒ“ƒR[ƒhŒã‚Ìƒf[ƒ^‚ğ©“®“I‚ÉCRLF‚ÅÜ‚è•Ô‚·ê‡‚Ì‚PsÅ‘å•¶š” (-1‚ªw’è‚³‚ê‚½ê‡‚ÍÜ‚è•Ô‚³‚È‚¢)
					FALSE	// ƒpƒfƒBƒ“ƒO‚·‚é‚©
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
		/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
		pszWork = (char*)(char*)&pUniBuf[nBgn];
		for( j = 0; j < (int)((i - nBgn) * sizeof( wchar_t )); j += 2 ){
			cWork = pszWork[j + 1];
			pszWork[j + 1] = pszWork[j];
			pszWork[j] = cWork;
		}
		/* Base64ƒGƒ“ƒR[ƒh */
		pszBase64Buf = NULL;
		nBase64BufLen = 0;
		nBase64BufLen = MemBASE64_Encode(
			(char*)&pUniBuf[nBgn],			// ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^
			(i - nBgn) * sizeof( wchar_t ), // ƒGƒ“ƒR[ƒh‘ÎÛƒf[ƒ^’·
			&pszBase64Buf,					// Œ‹‰Êƒf[ƒ^Ši”[ƒƒ‚ƒŠƒ|ƒCƒ“ƒ^‚ÌƒAƒhƒŒƒX
			-1,		// ƒGƒ“ƒR[ƒhŒã‚Ìƒf[ƒ^‚ğ©“®“I‚ÉCRLF‚ÅÜ‚è•Ô‚·ê‡‚Ì‚PsÅ‘å•¶š” (-1‚ªw’è‚³‚ê‚½ê‡‚ÍÜ‚è•Ô‚³‚È‚¢)
			FALSE	// ƒpƒfƒBƒ“ƒO‚·‚é‚©
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
* EUC‘SŠpƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚È‚çTRUE‚ğ‚©‚¦‚·
*
************************************************************************/
int CMemory::IsEUCKan1( unsigned char c )
{
	/* EUC‘SŠpƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚È‚çTRUE‚ğ‚©‚¦‚· */
	if( c >= 0xa1 && c <= 0xfe ){
		return TRUE;
	}
	return FALSE;
}
/************************************************************************
*
*  EUC‘SŠpƒR[ƒh‚Ì2ƒoƒCƒg–Ú‚È‚çTRUE‚ğ‚©‚¦‚·
*
************************************************************************/
int CMemory::IsEUCKan2( unsigned char c )
{
	/* EUC‘SŠpƒR[ƒh‚Ì2ƒoƒCƒg–Ú‚È‚çTRUE‚ğ‚©‚¦‚· */
	if( c >= 0xa1 && c <= 0xfe ){
		return TRUE;
	}
	return FALSE;
}


/* ‰p‘å•¶š¨‰p¬•¶š */
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
			/* ‘SŠp‰p‘å•¶š¨‘SŠp‰p¬•¶š */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x60 && pBuf[i + 1] <= 0x79 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x21;
//@@@ 2001.02.03 Start by MIK: ƒMƒŠƒVƒƒ•¶š•ÏŠ·
			//‘å•¶š:0x839f`0x83b6
			//¬•¶š:0x83bf`0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0x9f && pBuf[i + 1] <= 0xb6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] + 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ƒƒVƒA•¶š•ÏŠ·
			//‘å•¶š:0x8440`0x8460
			//¬•¶š:0x8470`0x8491 0x847f‚ª‚È‚¢I
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





/* ‰p¬•¶š¨‰p‘å•¶š */
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
			/* ‘SŠp‰p¬•¶š¨‘SŠp‰p‘å•¶š */
			if( pBuf[i] == 0x82 && pBuf[i + 1] >= 0x81 && pBuf[i + 1] <= 0x9a ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x21;
//@@@ 2001.02.03 Start by MIK: ƒMƒŠƒVƒƒ•¶š•ÏŠ·
			//‘å•¶š:0x839f`0x83b6
			//¬•¶š:0x83bf`0x83d6
			}else if( pBuf[i] == 0x83 && pBuf[i + 1] >= 0xbf && pBuf[i + 1] <= 0xd6 ){
				pBuf[i] = pBuf[i];
				pBuf[i + 1] = pBuf[i + 1] - 0x20;
//@@@ 2001.02.03 End
//@@@ 2001.02.03 Start by MIK: ƒƒVƒA•¶š•ÏŠ·
			//‘å•¶š:0x8440`0x8460
			//¬•¶š:0x8470`0x8491 0x847f‚ª‚È‚¢I
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



/* ƒ|ƒCƒ“ƒ^‚Å¦‚µ‚½•¶š‚ÌŸ‚É‚ ‚é•¶š‚ÌˆÊ’u‚ğ•Ô‚µ‚Ü‚· */
/* Ÿ‚É‚ ‚é•¶š‚ªƒoƒbƒtƒ@‚ÌÅŒã‚ÌˆÊ’u‚ğ‰z‚¦‚éê‡‚Í&pData[nDataLen]‚ğ•Ô‚µ‚Ü‚· */
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
			/* SJIS‘SŠpƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚© */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
			_IS_SJIS_1( (unsigned char)pDataCurrent[0] )
			&&
			/* SJIS‘SŠpƒR[ƒh‚Ì2ƒoƒCƒg–Ú‚© */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
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



/* ƒ|ƒCƒ“ƒ^‚Å¦‚µ‚½•¶š‚Ì’¼‘O‚É‚ ‚é•¶š‚ÌˆÊ’u‚ğ•Ô‚µ‚Ü‚· */
/* ’¼‘O‚É‚ ‚é•¶š‚ªƒoƒbƒtƒ@‚Ìæ“ª‚ÌˆÊ’u‚ğ‰z‚¦‚éê‡‚ÍpData‚ğ•Ô‚µ‚Ü‚· */
const char* CMemory::MemCharPrev( const char* pData, int nDataLen, const char* pDataCurrent )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CMemory::MemCharPrev" );
//#endif


	const char*	pPrev;
	pPrev = ::CharPrev( pData, pDataCurrent );

//===1999.08.12  ‚±‚Ì‚â‚è•û‚¾‚ÆAƒ_ƒ‚¾‚Á‚½B===============-
//
//	if( (pDataCurrent - 1)[0] == '\0' ){
//		pPrev = pDataCurrent - 1;
//	}else{
//		if( pDataCurrent - pData >= 2 &&
//			/* SJIS‘SŠpƒR[ƒh‚Ì1ƒoƒCƒg–Ú‚© */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
//			(
//			( (unsigned char)0x81 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0x9F ) ||
//			( (unsigned char)0xE0 <= (unsigned char)pDataCurrent[-2] && (unsigned char)pDataCurrent[-2] <= (unsigned char)0xFC )
//			) &&
//			/* SJIS‘SŠpƒR[ƒh‚Ì2ƒoƒCƒg–Ú‚© */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
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



/* ƒR[ƒh•ÏŠ· SJIS¨Unicode */
void CMemory::SJISToUnicode( void )
{
	char*	pBufUnicode;
	int		nBufUnicodeLen;
	nBufUnicodeLen = CMemory::MemSJISToUnicode( &pBufUnicode, m_pData, m_nDataLen );
	SetData( pBufUnicode, nBufUnicodeLen );
	delete [] pBufUnicode;
	return;
}



/* ƒR[ƒh•ÏŠ· SJIS¨UnicodeBE */
void CMemory::SJISToUnicodeBE( void )
{
	char*	pBufUnicode;
	int		nBufUnicodeLen;
	nBufUnicodeLen = CMemory::MemSJISToUnicode( &pBufUnicode, m_pData, m_nDataLen );
	SetData( pBufUnicode, nBufUnicodeLen );
	delete [] pBufUnicode;
	SwapHLByte();
	return;
}



/* ƒR[ƒh•ÏŠ· Unicode¨SJIS */
void CMemory::UnicodeToSJIS( void )
{
	char*			pBufUnicode;
	int				nBufUnicodeLen;
	unsigned char*	pBuf;
	pBuf = (unsigned char*)m_pData;
//	BOM‚Ìíœ‚Í‚±‚±‚Å‚Í‚µ‚È‚¢‚æ‚¤‚É•ÏX
//	ŒÄ‚Ño‚µ‘¤‚Å‘Îˆ‚µ‚Ä‚­‚¾‚³‚¢ 2002/08/30 Moca
		nBufUnicodeLen = CMemory::MemUnicodeToSJIS( &pBufUnicode, m_pData, m_nDataLen );
		SetData( pBufUnicode, nBufUnicodeLen );
		delete [] pBufUnicode;
	return;
}


/* ƒR[ƒh•ÏŠ· UnicodeBE¨SJIS */
void CMemory::UnicodeBEToSJIS( void ){

	SwapHLByte();
	UnicodeToSJIS();

	return;
}


/* ASCII&SJIS•¶š—ñ‚ğUnicode‚É•ÏŠ· */
int CMemory::MemSJISToUnicode( char** ppBufUnicode, const char*pBuf, int nBufLen )
{
	int			i, j, k;
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
#if _DEBUG
			if( j != 2 ){
				MYTRACE( "%dƒoƒCƒg‚ÌUnicode•¶š‚É•ÏŠ·‚³‚ê‚½ SJIS(?)=%x %x\n", j,pBuf[i],((nCharChars >= 2)?(pBuf[i + 1]):0) );
			}
#endif
			*((wchar_t*)&(pBufUnicode[k])) = wchar;
			i += j;
		}
		k += 2;
		nCharChars = CMemory::MemCharNext( pBuf, nBufLen, &pBuf[i] ) - &pBuf[i];
	}
	*ppBufUnicode = pBufUnicode;
	return k;
}





/* Unicode•¶š—ñ‚ğASCII&SJIS‚É•ÏŠ· */
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





/* •¶š—ñ’uŠ· */
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



/* •¶š—ñ’uŠ·i“ú–{Œêl—¶”Åj */
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
				cmemWork.Append( &m_pData[nBgnOld], nBgn - nBgnOld );
			}
			cmemWork.Append( pszTo, nToLen );
			nBgn = nBgn + nFromLen;
			nBgnOld = nBgn;
		}else{
			if( _IS_SJIS_1( (unsigned char)m_pData[nBgn] ) ) nBgn++;
			nBgn++;
		}
	}
	if( 0  < m_nDataLen - nBgnOld ){
		cmemWork.Append( &m_pData[nBgnOld], m_nDataLen - nBgnOld );
	}
	SetData( &cmemWork );
	return;
}



/* “™‚µ‚¢“à—e‚© */
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





/* ”¼Šp¨‘SŠp */
void CMemory::ToZenkaku(
		int bHiragana,		/* 1== ‚Ğ‚ç‚ª‚È 0==ƒJƒ^ƒJƒi //2==‰p”ê—p 2001/07/30 Misaka ’Ç‰Á */
		int bHanKataOnly	/* 1== ”¼ŠpƒJƒ^ƒJƒi‚É‚Ì‚İì—p‚·‚é*/
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
	static unsigned char*	pszHanKataSet = (unsigned char*)"¡¢£¤¥¦§¨©ª«¬­®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏĞÑÒÓÔÕÖ×ØÙÚÛÜİŞß";
	static unsigned char*	pszDakuSet = (unsigned char*)"¶·¸¹º»¼½¾¿ÀÁÂÃÄÊËÌÍÎ";
	static unsigned char*	pszYouSet = (unsigned char*)"ÊËÌÍÎ";
	BOOL					bHenkanOK;

	pBufDes = new unsigned char[nBufLen * 2 + 1];
	if( NULL ==	pBufDes ){
		return;
	}
	nBufDesLen = 0;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 1){
			bHenkanOK = FALSE;
			if( bHanKataOnly ){	/* 1== ”¼ŠpƒJƒ^ƒJƒi‚É‚Ì‚İì—p‚·‚é */
				if( NULL != strchr( (const char *)pszHanKataSet, pBuf[i] ) ){
					bHenkanOK = TRUE;
				}
			}else{
				//! ‰p”•ÏŠ·—p‚ÉV‚½‚ÈğŒ‚ğ•t‰Á 2001/07/30 Misaka
				if( ( (unsigned char)0x20 <= pBuf[i] && pBuf[i] <= (unsigned char)0x7E ) ||
					( bHiragana != 2 && (unsigned char)0xA1 <= pBuf[i] && pBuf[i] <= (unsigned char)0xDF )
				){
					bHenkanOK = TRUE;
				}
			}
			if( bHenkanOK ){
				usSrc = pBuf[i];
				if( FALSE == bHiragana &&
					pBuf[i]		== (unsigned char)'³' &&
					pBuf[i + 1] == (unsigned char)'Ş' &&
					bHiragana != 2
				){
					usDes = (unsigned short)0x8394; /* ƒ” */
					nCharChars = 2;
				}else {
					usDes = _mbbtombc( usSrc );
					/* ‘÷‰¹ */
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'Ş' && NULL != strchr( (const char *)pszDakuSet, pBuf[i] ) ){
						usDes++;
						nCharChars = 2;
					}
					/* X‰¹ */
					//! ‰p”•ÏŠ·—p‚ÉV‚½‚ÈğŒ‚ğ•t‰Á 2001/07/30 Misaka
					//! bHiragana != 2 //‰p”•ÏŠ·ƒtƒ‰ƒO‚ªƒIƒ“‚Å‚Í‚È‚¢ê‡
					if( bHiragana != 2 && pBuf[i + 1] == (unsigned char)'ß' && NULL != strchr( (const char *)pszYouSet, pBuf[i] ) ){
						usDes += 2;
						nCharChars = 2;
					}
				}

				if( bHiragana == 1 ){
					/* ‚Ğ‚ç‚ª‚È‚É•ÏŠ·‰Â”\‚ÈƒJƒ^ƒJƒi‚È‚ç‚ÎA‚Ğ‚ç‚ª‚È‚É•ÏŠ·‚·‚é */
					if( (unsigned short)0x8340 <= usDes && usDes <= (unsigned short)0x837e ){	/* ƒ@`ƒ~ */
						usDes-= (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usDes && usDes <= (unsigned short)0x8393 ){	/* ƒ€`ƒ“ */
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
				if( bHiragana == 1 ){//‰p”•ÏŠ·‚ğ•t‰Á‚µ‚½‚½‚ß‚É”’l‚Åw’è‚µ‚½@2001/07/30 Misaka
					/* ‘SŠp‚Ğ‚ç‚ª‚È‚É•ÏŠ·‰Â”\‚È‘SŠpƒJƒ^ƒJƒi‚È‚ç‚ÎA‚Ğ‚ç‚ª‚È‚É•ÏŠ·‚·‚é */
					if( (unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ){	/* ƒ@`ƒ~ */
						usDes = usSrc - (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ){	/* ƒ€`ƒ“ */
						usDes = usSrc - (unsigned short)0x00a2;
					}
				}else if( bHiragana == 0 ){//‰p”•ÏŠ·‚ğ•t‰Á‚µ‚½‚½‚ß‚É”’l‚Åw’è‚µ‚½@2001/07/30 Misaka
					/* ‘SŠpƒJƒ^ƒJƒi‚É•ÏŠ·‰Â”\‚È‘SŠp‚Ğ‚ç‚ª‚È‚È‚ç‚ÎAƒJƒ^ƒJƒi‚É•ÏŠ·‚·‚é */
					if( (unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ){	/* ‚Ÿ`‚İ */
						usDes = usSrc + (unsigned short)0x00a1;
					}else
					if( (unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ){	/* ‚Ş`‚ñ */
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



/*!	‘SŠp¨”¼Šp
	nMode
		0x01	ƒJƒ^ƒJƒi‚É‰e‹¿ƒAƒŠ
		0x02	‚Ğ‚ç‚ª‚È‚É‰e‹¿ƒAƒŠ
		0x04	‰p”š‚É‰e‹¿ƒAƒŠ
*/
#define TO_KATAKANA	0x01
#define TO_HIRAGANA	0x02
#define TO_EISU		0x04
void CMemory::ToHankaku(
		int nMode		/* 0==ƒJƒ^ƒJƒi 1== ‚Ğ‚ç‚ª‚È 2==‰p”ê—p */
)
{
	static unsigned char*	pszZenDAKU = (unsigned char*)"‚ª‚¬‚®‚°‚²‚´‚¶‚¸‚º‚¼‚¾‚À‚Ã‚Å‚Ç‚Î‚Ñ‚Ô‚×‚ÚƒKƒMƒOƒQƒSƒUƒWƒYƒ[ƒ]ƒ_ƒaƒdƒfƒhƒoƒrƒuƒxƒ{ƒ”";
	static unsigned char*	pszZenYOU  = (unsigned char*)"‚Ï‚Ò‚Õ‚Ø‚Ûƒpƒsƒvƒyƒ|";

	/* “ü—Í */
	unsigned char*			pBuf = (unsigned char*)m_pData;
	int						nBufLen = m_nDataLen;
	unsigned int			uiSrc;

	/* o—Í */
	unsigned char*			pBufDes = new unsigned char[nBufLen + 1];
	int						nBufDesLen = 0;
	unsigned int			uiDes;
	if( NULL ==	pBufDes ){
		return;
	}

	/* ì‹Æ—p */
	int						nCharChars;
	unsigned char			pszZen[3];	//	‘SŠp•¶š—pƒoƒbƒtƒ@
	int i;
	BOOL bHenkanOK;
	for( i = 0; i < nBufLen; ++i ){
		nCharChars = CMemory::MemCharNext( (const char *)pBuf, nBufLen, (const char *)&(pBuf[i]) ) - (const char*)&(pBuf[i]);
		if( nCharChars == 2 ){
			uiSrc = pBuf[i + 1] | ( pBuf[i] << 8 );
			
			bHenkanOK = FALSE;
			if( nMode == 0x00 ){	//	‚Ç‚ñ‚È•¶š‚àokƒ‚[ƒh
				bHenkanOK = TRUE;
			}
			if( nMode & TO_KATAKANA ){	/* ƒJƒ^ƒJƒi‚Éì—p‚·‚é */
				if( 0x8340 <= uiSrc && uiSrc <= 0x8396){
					bHenkanOK = TRUE;
				}
			}
			if( nMode & TO_HIRAGANA ){	/* ‚Ğ‚ç‚ª‚È‚Éì—p‚·‚é */
				if( 0x829F <= uiSrc && uiSrc <= 0x82F1){
					bHenkanOK = TRUE;
				}
			}
			if ( nMode & TO_EISU ){		/* ‰p”‚Éì—p‚·‚é */
				if( 0x824F <= uiSrc && uiSrc <= 0x8258){	//	”š
					bHenkanOK = TRUE;
				}
				else if( 0x8260 <= uiSrc && uiSrc <= 0x8279){	//	‰p‘å•¶š
					bHenkanOK = TRUE;
				}
				else if( 0x8281 <= uiSrc && uiSrc <= 0x829A){	//	‰p‘å•¶š
					bHenkanOK = TRUE;
				}
			}
			if (bHenkanOK == TRUE){
				uiDes = _mbctombb( uiSrc );
				if( uiDes == uiSrc ){	//	•ÏŠ·•s‰Â”\
					memcpy( &pBufDes[nBufDesLen], &pBuf[i], nCharChars );
					nBufDesLen += nCharChars;
				}else{
					pBufDes[nBufDesLen] = (unsigned char)uiDes;
					nBufDesLen++;

					memcpy( pszZen, &pBuf[i], 2 );
					pszZen[2] = '\0';
					/* ‘÷‰¹’T‚µ */
					if( NULL != strstr( (const char *)pszZenDAKU, (const char *)pszZen ) ){
						pBufDes[nBufDesLen] = (unsigned char)'Ş';
						nBufDesLen++;
					}else
					/* X‰¹’T‚µ */
					if( NULL != strstr( (const char *)pszZenYOU,  (const char *)pszZen ) ){
						pBufDes[nBufDesLen] = (unsigned char)'ß';
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
	SetData( (const char *)pBufDes, nBufDesLen );
	delete [] pBufDes;

}


/*
|| ƒtƒ@ƒCƒ‹‚Ì“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê
||
|| y–ß‚è’lz
||	SJIS		0
||	JIS			1
||	EUC			2
||	Unicode		3
||	UTF-8		4
||	UTF-7		5
||	UnicodeBE	6
||	ƒGƒ‰[		-1
||	ƒGƒ‰[ˆÈŠO‚Í enumCodeType ‚ğg‚¤
*/
int CMemory::CheckKanjiCodeOfFile( const char* pszFile )
{
	HFILE					hFile;
	HGLOBAL					hgData;
	const unsigned char*	pBuf;
	int						nBufLen;
	int						nCodeType;

	/* ƒƒ‚ƒŠŠm•Û & ƒtƒ@ƒCƒ‹“Ç‚İ‚İ */
	hgData = NULL;
	hFile = _lopen( pszFile, OF_READ );
	if( HFILE_ERROR == hFile ){
		return -1;
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
		return -1;
	}
	pBuf = (const unsigned char*)::GlobalLock( hgData );
	_lread( hFile, (void *)pBuf, nBufLen );
	_lclose( hFile );

	/* “ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê */
	nCodeType = CheckKanjiCode( pBuf, nBufLen );

	if( NULL != hgData ){
		::GlobalUnlock( hgData );
		::GlobalFree( hgData );
		hgData = NULL;
	}
	return nCodeType;

}


/*
|| “ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê
||
|| y–ß‚è’lz
||	SJIS	0
||	JIS		1
||	EUC		2
||	Unicode	3
||	UTF-8	4
||	UTF-7	5
||	UnicodeBE	6
||	ƒGƒ‰[	-1
||	ƒGƒ‰[ˆÈŠO‚Í enumCodeType ‚ğg‚¤
*/
int CMemory::CheckKanjiCode( const unsigned char* pBuf, int nBufLen )
{
	int			nEUCMojiNum, nEUCCodeNum;
	int			nSJISMojiNum, nSJISCodeNum;
//	int			nUNICODEMojiNum, nUNICODECodeNum;
	int			nJISMojiNum, nJISCodeNum;
	int			nUTF8MojiNum, nUTF8CodeNum;
	int			nUTF7MojiNum, nUTF7CodeNum;
	int			nUnicodeBom;

	nUnicodeBom = IsUnicodeBom( pBuf, nBufLen );
	if( 0 != nUnicodeBom ){
		return nUnicodeBom; /* Unicode or UnicodeBE or UTF-8 */
	}

// IsUnicodeBom‚É‚ä‚¾‚Ë‚é
#if 0
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: Unicode‚©H
	*/
	CMemory::CheckKanjiCode_UNICODE( pBuf, nBufLen, &nUNICODEMojiNum, &nUNICODECodeNum );
		if( 0 < nUNICODEMojiNum && nUNICODEMojiNum == nUNICODECodeNum ){
			return CODE_UNICODE; /* Unicode */
		}
#endif
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: EUC‚©H
	*/
	CMemory::CheckKanjiCode_EUC( pBuf, nBufLen, &nEUCMojiNum, &nEUCCodeNum );
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: SJIS‚©H
	*/
	CMemory::CheckKanjiCode_SJIS( pBuf, nBufLen, &nSJISMojiNum, &nSJISCodeNum );
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: JIS‚©H
	*/
	CMemory::CheckKanjiCode_JIS( pBuf, nBufLen, &nJISMojiNum, &nJISCodeNum );
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: UTF-8‚©H
	*/
	CMemory::CheckKanjiCode_UTF8( pBuf, nBufLen, &nUTF8MojiNum, &nUTF8CodeNum );
	/*
	||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: UTF-7‚©H
	*/
	CMemory::CheckKanjiCode_UTF7( pBuf, nBufLen, &nUTF7MojiNum, &nUTF7CodeNum );

	if( nEUCCodeNum > 0
	 && nEUCCodeNum >= nSJISCodeNum
	 && nEUCCodeNum >= nJISCodeNum
	 && nEUCCodeNum >= nUTF8CodeNum
	 && nEUCCodeNum >= nUTF7CodeNum
	){
		return CODE_EUC; /* EUC */
	}
	if( nUTF7CodeNum > 0
	 && nUTF7CodeNum >= nSJISCodeNum
	 && nUTF7CodeNum >= nJISCodeNum
	 && nUTF7CodeNum >= nEUCCodeNum
	 && nUTF7CodeNum >= nUTF8CodeNum
	){
		return CODE_UTF7; /* UTF-7 */
	}
	if( nUTF8CodeNum > 0
	 && nUTF8CodeNum >= nSJISCodeNum
	 && nUTF8CodeNum >= nJISCodeNum
	 && nUTF8CodeNum >= nEUCCodeNum
	 && nUTF8CodeNum >= nUTF7CodeNum
	){
		return CODE_UTF8; /* UTF-8 */
	}
	if( nJISCodeNum > 0
	 && nJISCodeNum >= nEUCCodeNum
	 && nJISCodeNum >= nSJISCodeNum
	 && nJISCodeNum >= nUTF8CodeNum
	 && nJISCodeNum >= nUTF7CodeNum
	){
		return CODE_JIS; /* JIS */
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
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: Unicode‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_UNICODE( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUNICODECodeNum )
{
	int				nMojiNum;
	int				nUNICODECodeNum;

	*pnMojiNum = 0;
	*pnUNICODECodeNum = 0;
	nMojiNum = 0;
	nUNICODECodeNum = 0;

	if( nBufLen < 2 ){
		return TRUE;
	}

	if( (unsigned char)(pBuf[0	  ]) == (unsigned char)0xff &&
		(unsigned char)(pBuf[0 + 1]) == (unsigned char)0xfe ){
		nMojiNum = 1;
		nUNICODECodeNum = 1;
	}
#if 0
	int				i;
	// ’Ç‰Á‚Í‚µ‚½‚ª Œø—¦‚©‚çl‚¦‚Ä–¢g—p Moca, 2002/05/26
	// U+fffe,U+ffff‚ÍUnicode•¶š‚Å‚Í‚È‚¢
	for( i = 1; i < nBufLen ; i+= 2 ){
		if( (unsigned char)(pBuf[i    ]) == (unsigned char)0xff &&
			(unsigned char)(pBuf[i - 1]) == (unsigned char)0xff ||
			(unsigned char)(pBuf[i - 1]) == (unsigned char)0xfe ){
			nMojiNum = 0;
			nUNICODECodeNum = 0;
			break;
		}
		// ƒTƒƒQ[ƒg—Ìˆæ(UTF-16)‚ÌU+xxxxffff‚àUnicode•¶š‚Å‚Í‚È‚¢
		// UTF16LE = xx111111 110110xx 11111111 11011111
		else if( 4 <= i &&
			pBuf[i    ] == (unsigned char)0xdf && 
			pBuf[i - 1] == (unsigned char)0xff &&
			pBuf[i - 2] & (unsigned char)0xfc == (unsigned char)0xd8 &&
			pBuf[i - 3] & (unsigned char)0x3f == (unsigned char)0x3f
		){
			nMojiNum = 0;
			nUNICODECodeNum = 0;
			break;
		}
	}
#endif
	*pnMojiNum = nMojiNum;
	*pnUNICODECodeNum = nUNICODECodeNum;
	return TRUE;
}

/*
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: UnicodeBE‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/

int CMemory::CheckKanjiCode_UNICODEBE( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUNICODEBECodeNum ){
	int		nMojiNum;
	int		nUNICODEBECodeNum;

	*pnMojiNum = 0;
	*pnUNICODEBECodeNum = 0;
	nMojiNum = 0;
	nUNICODEBECodeNum = 0;

	if( nBufLen < 2 ){
		return TRUE;
	}

	if( pBuf[0    ] == (unsigned char)0xfe &&
		pBuf[0 + 1] == (unsigned char)0xff ){
		nMojiNum = 1;
		nUNICODEBECodeNum = 1;
	}
	*pnMojiNum = nMojiNum;
	*pnUNICODEBECodeNum = nUNICODEBECodeNum;
	return TRUE;
}

/*
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: EUC‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_EUC( const unsigned char* pBuf, int nBufLen, int*	pnMojiNum, int* pnEUCCodeNum )
{
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
		/* EUCŠ¿šƒR[ƒh‚©? */
		if( (unsigned char)pBuf[nPtr] == (unsigned char)0x8e ){
			/* ”¼ŠpƒJƒ^ƒJƒi */
//			pszDes[nPtrDes] = pBuf[nPtr + 1];
			nMojiNum++;
			nEUCCodeNum++;

//			nPtrDes++;
			nPtr += 2;
		}else
		if( nPtr < nBufLen - 1 && IsEUCKan1( pBuf[nPtr] ) && IsEUCKan2( pBuf[nPtr + 1L] ) ){
			/* ’Êí‚ÌJISƒR[ƒh‚É•ÏŠ· */
			pTest[0] = pBuf[nPtr	 ] & 0x7f;
			pTest[1] = pBuf[nPtr + 1L] & 0x7f;

			/* SJISƒR[ƒh‚É•ÏŠ· */	//Sept. 1, 2000 jepro 'ƒVƒtƒg'‚ğ'S'‚É•ÏX
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
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: SJIS‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_SJIS( const unsigned char* pBuf, int nBufLen, int*	pnMojiNum, int* pnSJISCodeNum )
{
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
//				/* ”¼ŠpƒJƒ^ƒJƒi */
//				nMojiNum++;
//				nSJISCodeNum++;
//			}else{
				nMojiNum++;
//			}
		}else
		if( nCharChars == 2 ){
			/* ‘SŠp•¶š */
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
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: JIS‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_JIS( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnJISCodeNum )

{
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
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: UTF-8‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_UTF8( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUTF8CodeNum )
{
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
		/* UTF-8‚Ì•¶š‚© */
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
||“ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê: UTF-7‚©H
|| ƒGƒ‰[‚Ìê‡AFALSE‚ğ•Ô‚·
*/
int CMemory::CheckKanjiCode_UTF7( const unsigned char* pBuf, int nBufLen, int* pnMojiNum, int* pnUTF7CodeNum )
{
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
				//	+‚ÌƒGƒ“ƒR[ƒh‚ÍUTF-7‚Ìƒ|ƒCƒ“ƒg‘ÎÛŠO‚Æ‚·‚é
				//	nUTF7CodeNum++;
				++i;
			}else
			if( i < nBufLen - 2
				&& '+' == pBuf[i]
				&& -1 != IsBASE64Char( pBuf[i + 1] )	/* •¶š‚ªBase64‚Ìƒf[ƒ^‚© */
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
					// Base64ƒfƒR[ƒh
					nWorkLen = MemBASE64_Decode( (unsigned char *)pszWork, nWorkLen );
					if( 0 == nWorkLen % 2 ){
						nMojiNum += (nWorkLen / 2);
						/* 2ƒoƒCƒg‚ÌUnicode‚ª‚ ‚é‚Æ‚¢‚¤‘O’ñ‚ÅLO/HIƒoƒCƒg‚ğŒğŠ· */
						for( j = 0; j < nWorkLen; j += 2 ){
							cWork = pszWork[j + 1];
							pszWork[j + 1] = pszWork[j];
							pszWork[j] = cWork;
							/* •ÏŠ·‰Â”\‚ÈUnicode‚© */
							nUniBytes = wctomb( (char*)NULL, *(wchar_t*)(&pszWork[j]) );
							if( -1 != nUniBytes ){
								nUTF7CodeNum++;
							}
						}
					}else{
#ifdef _DEBUG
						MYTRACE( "šššš2‚Ì”{”‚Å‚È‚¢’·‚³‚ÌUnicodeƒf[ƒ^—ñ nWorkLen=%d\n", nWorkLen );
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
				if( -1 == IsBASE64Char( pBuf[i] )  ){	/* •¶š‚ªBase64‚Ìƒf[ƒ^‚© */
					bBASE64 = FALSE;
					//	Oct. 10, 2000 genta
					if( pBuf[i] & 0x80 ){
						//	8bitƒR[ƒh‚ª“ü‚Á‚Ä‚¢‚½‚çUTF-7‚Å‚Í‚ ‚è“¾‚È‚¢‚Ì‚ÅƒJƒEƒ“ƒ^‚ğ0‚É–ß‚·
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



/* ©“®”»•Ê¨SJISƒR[ƒh•ÏŠ· */
void CMemory::AUTOToSJIS( void )
{
	int	nCodeType;
	/*
	|| “ú–{ŒêƒR[ƒhƒZƒbƒg”»•Ê
	||
	|| y–ß‚è’lz
	||	SJIS	0
	||	JIS		1
	||	EUC		2
	||	Unicode	3
	||	UTF-8	4
	||	UTF-7	5
	||	UnicodeBE 6
	||	ƒGƒ‰[	-1
	*/
	nCodeType = CheckKanjiCode( (const unsigned char*)m_pData, m_nDataLen );
	switch( nCodeType ){
	case CODE_JIS:			JIStoSJIS();break;		/* E-Mail(JIS¨SJIS)ƒR[ƒh•ÏŠ· */
	case CODE_EUC:			EUCToSJIS();break;		/* EUC¨SJISƒR[ƒh•ÏŠ· */
	case CODE_UNICODE:		UnicodeToSJIS();break;	/* Unicode¨SJISƒR[ƒh•ÏŠ· */
	case CODE_UTF8:			UTF8ToSJIS();break;		/* UTF-8¨SJISƒR[ƒh•ÏŠ· */
	case CODE_UTF7:			UTF7ToSJIS();break;		/* UTF-7¨SJISƒR[ƒh•ÏŠ· */
	case CODE_UNICODEBE:	UnicodeBEToSJIS();break;/* UnicodeBE¨SJISƒR[ƒh•ÏŠ· */
	};
	return;
}



int CMemory::IsZenHiraOrKata( unsigned short usSrc )
{
	/* ‘SŠpƒJƒ^ƒJƒi */
	if( ((unsigned short)0x8340 <= usSrc && usSrc <= (unsigned short)0x837e ) /* ƒ@`ƒ~ */
	 || ((unsigned short)0x8380 <= usSrc && usSrc <= (unsigned short)0x8393 ) /* ƒ€`ƒ“ */
	/* ‘SŠp‚Ğ‚ç‚ª‚È */
	 || ((unsigned short)0x829f <= usSrc && usSrc <= (unsigned short)0x82dd ) /* ‚Ÿ`‚İ */
	 || ((unsigned short)0x82de <= usSrc && usSrc <= (unsigned short)0x82f1 ) /* ‚Ş`‚ñ */
	){
		return TRUE;
	}else{
		return FALSE;
	}
}



/* ! •¶š—ñ‚Ìæ“ª‚ÉUnicodeŒnBOM‚ª•t‚¢‚Ä‚¢‚é‚©H

	@retval	0	‚È‚µ,–¢ŒŸo
	@retval	3	(CODE_UNICODE)	Unicode
	@retval	4	(CODE_UTF8)		UTF-8
	@retval	6	(CODE_UNICODEBE) UnicodeBE

*/
int CMemory::IsUnicodeBom( const unsigned char* pBuf, int nBufLen )
{
	if( NULL == pBuf ){
		return 0;
	}
#if 0
//	UTF-32 / UCS-4—p
	if( 4 <= nBufLen ){
		unsigned int* pUCS4Buf = (unsigned int*)pBuf;
		if( pUCS4Buf[0] == (unsigned int)0xfffe0000 ){
			return ??? // CODE_UTF32BE;
		}
		if( pUCS4Buf[0] == (unsigned int)0x0000feff ){
			return ??? // CODE_UTF32LE;
		}
	}
#endif
	if( 2 <= nBufLen ){
		if( pBuf[0] == (unsigned char)0xff &&
			pBuf[1] == (unsigned char)0xfe ){
			return CODE_UNICODE;
		}
		if( pBuf[0] == (unsigned char)0xfe &&
			pBuf[1] == (unsigned char)0xff ){
			return CODE_UNICODEBE;
		}
		if( 3 <= nBufLen ){
			if( pBuf[0] == (unsigned char)0xef &&
				pBuf[1] == (unsigned char)0xbb &&
				pBuf[2] == (unsigned char)0xbf ){
				return CODE_UTF8;
			}
		}
	}
	return 0;
}



/* TAB¨‹ó”’ */
void CMemory::TABToSPACE( int nTabSpace	/* TAB‚Ì•¶š” */ )
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
	/* CRLF‚Å‹æØ‚ç‚ê‚éusv‚ğ•Ô‚·BCRLF‚Ís’·‚É‰Á‚¦‚È‚¢ */
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
	/* CRLF‚Å‹æØ‚ç‚ê‚éusv‚ğ•Ô‚·BCRLF‚Ís’·‚É‰Á‚¦‚È‚¢ */
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


//!‹ó”’¨TAB•ÏŠ·
/*!
	@param nTabSpace TAB‚Ì•¶š”
	’P“Æ‚ÌƒXƒy[ƒX‚Í•ÏŠ·‚µ‚È‚¢

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

	BOOL		bSpace = FALSE;	//ƒXƒy[ƒX‚Ìˆ—’†‚©‚Ç‚¤‚©
	int		j;
	int		nStartPos;

	nBgn = 0;
	nPosDes = 0;
	/* •ÏŠ·Œã‚É•K—v‚ÈƒoƒCƒg”‚ğ’²‚×‚é */
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
	/* CRLF‚Å‹æØ‚ç‚ê‚éusv‚ğ•Ô‚·BCRLF‚Ís’·‚É‰Á‚¦‚È‚¢ */
	while( NULL != ( pLine = GetNextLine( m_pData, m_nDataLen, &nLineLen, &nBgn, &cEol ) ) ){
		if( 0 < nLineLen ){
			nPosX = 0;
			bSpace = FALSE;
			nStartPos = 0;
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
							}
							for( j = 0; j < (nPosX % nTabSpace); j++ ){
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
					}
					for( j = 0; j < nPosX % nTabSpace; j++ ){
						pDes[nPosDes] = SPACE;
						nPosDes++;
					}
				}
			}
		}

		/* s––‚Ìˆ— */
		memcpy( &pDes[nPosDes], cEol.GetValue(), cEol.GetLen() );
		nPosDes += cEol.GetLen();
	}
	pDes[nPosDes] = '\0';

	SetData( pDes, nPosDes );
	delete [] pDes;
	pDes = NULL;
	return;
}





/* !ãˆÊƒoƒCƒg‚Æ‰ºˆÊƒoƒCƒg‚ğŒğŠ·‚·‚é

	@author Moca
	@date 2002/5/27
	
	@note	nBufLen ‚ª2‚Ì”{”‚Å‚È‚¢‚Æ‚«‚ÍAÅŒã‚Ì1ƒoƒCƒg‚ÍŒğŠ·‚³‚ê‚È‚¢
*/
void CMemory::SwapHLByte( void ){
	unsigned char *p;
	unsigned char ctemp;
	unsigned char *p_end;
	unsigned int *pdwchar;
	unsigned int *pdw_end;
	unsigned char*	pBuf;
	int			nBufLen;

	pBuf = (unsigned char*)GetPtr( &nBufLen );

	if( nBufLen < 2){
		return;
	}
	// ‚‘¬‰»‚Ì‚½‚ß
	if( (unsigned int)pBuf % 2 == 0){
		if( (unsigned int)pBuf % 4 == 2 ){
			ctemp = pBuf[0];
			pBuf[0]  = pBuf[1];
			pBuf[1]  = ctemp;
			pdwchar = (unsigned int*)(pBuf + 2);
		}else{
			pdwchar = (unsigned int*)pBuf;
		}
		pdw_end = (unsigned int*)(pdwchar + nBufLen / sizeof(int)) - 1;

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


//	/* ƒoƒbƒtƒ@‚Ìæ“ª‚Éƒf[ƒ^‚ğ‘}“ü‚·‚é */
//	void CMemory::InsertTop( const char* pData, int nDataLen )
//	{
//		AllocBuffer( m_nDataLen + nDataLen );
//		memmove( m_pData + nDataLen, m_pData, m_nDataLen );
//		memcpy( m_pData, pData, nDataLen );
//		m_nDataLen += nDataLen;
//		m_pData[m_nDataLen] = '\0';
//	}





/*
|| ƒoƒbƒtƒ@ƒTƒCƒY‚Ì’²®
*/
void CMemory::AllocBuffer( int nNewDataLen )
{
	int		nWorkLen;
	char*	pWork = NULL;
	nWorkLen = nNewDataLen + 1;	/* 1ƒoƒCƒg‘½‚­ƒƒ‚ƒŠŠm•Û‚µ‚Ä‚¨‚­(\0‚ğ“ü‚ê‚é) */
	if( m_nDataBufSize == 0 ){
		/* –¢Šm•Û‚Ìó‘Ô */
		pWork = (char*)malloc( nWorkLen );
		m_nDataBufSize = nWorkLen;
	}else{
		/* Œ»İ‚Ìƒoƒbƒtƒ@ƒTƒCƒY‚æ‚è‘å‚«‚­‚È‚Á‚½ê‡‚Ì‚İÄŠm•Û‚·‚é */
		if( m_nDataBufSize < nWorkLen ){
			pWork = (char*)realloc( m_pData, nWorkLen );
			m_nDataBufSize = nWorkLen;
		}else{
			return;
		}
	}


	if( NULL == pWork ){
		::MYMESSAGEBOX(	NULL, MB_OKCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
			"CMemory::AllocBuffer(nNewDataLen==%d)\nƒƒ‚ƒŠŠm•Û‚É¸”s‚µ‚Ü‚µ‚½B\n", nNewDataLen
		);
		if( NULL != m_pData && 0 != nWorkLen ){
			/* ŒÃ‚¢ƒoƒbƒtƒ@‚ğ‰ğ•ú‚µ‚Ä‰Šú‰» */
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



/* ƒoƒbƒtƒ@‚Ì“à—e‚ğ’u‚«Š·‚¦‚é */
void CMemory::SetData( const char* pData, int nDataLen )
{
	Empty();
	AllocBuffer( nDataLen );
	AddData( pData, nDataLen );
	return;
}



/* ƒoƒbƒtƒ@‚Ì“à—e‚ğ’u‚«Š·‚¦‚é */
void CMemory::SetDataSz( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );

	Empty();
	AllocBuffer( nDataLen );
	AddData( pszData, nDataLen );
	return;
}


/* ƒoƒbƒtƒ@‚Ì“à—e‚ğ’u‚«Š·‚¦‚é */
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


/* ƒoƒbƒtƒ@‚ÌÅŒã‚Éƒf[ƒ^‚ğ’Ç‰Á‚·‚éipublicƒƒ“ƒoj*/
const char* CMemory::Append( const char* pData, int nDataLen )
{
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
	return m_pData;
}
/* ƒoƒbƒtƒ@‚ÌÅŒã‚Éƒf[ƒ^‚ğ’Ç‰Á‚·‚éipublicƒƒ“ƒoj*/
void CMemory::AppendSz( const char* pszData )
{
	int		nDataLen;
	nDataLen = strlen( pszData );
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pszData, nDataLen );
}
/* ƒoƒbƒtƒ@‚ÌÅŒã‚Éƒf[ƒ^‚ğ’Ç‰Á‚·‚éipublicƒƒ“ƒoj*/
void CMemory::Append( CMemory* pcmemData )
{
	char*	pData;
	int		nDataLen;
	pData = pcmemData->GetPtr( &nDataLen );
	AllocBuffer( m_nDataLen + nDataLen );
	AddData( pData, nDataLen );
}

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
