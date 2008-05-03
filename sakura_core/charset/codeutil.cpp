#include "stdafx.h"
#include "codeutil.h"
#include "charset/charcode.h"
#include <mbstring.h>

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
unsigned short _mbcjmstojis_ex( unsigned char* pszSrc )
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
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40`fa49 ¨ eeef`eef8 (ú@`úI) */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a`fa53 ¨ 8754`875d (‡T`‡]) */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54`fa57 ¨ eef9`eefc (Ê`úW) */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* ‡Š */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* ‡‚ */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* ‡„ */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* æ */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c`fa7e ¨ ed40`ed62 (ú\`ú~) */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80`fa9b ¨ ed63`ed7e (ú€`ú›) */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c`fafc ¨ ed80`ede0 (úœ`úü) */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40`fb5b ¨ ede1`edfc (û@`û[) */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c`fb7e ¨ ee40`ee62 (û\`û~) */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80`fb9b ¨ ee63`ee7e (û€`û›) */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c`fbfc ¨ ee80`eee0 (ûœ`ûü) */
			else{							tmpw -= 0x0d5f;	}	/* fc40`fc4b ¨ eee1`eeec (ü@`üK) */
		}
		return (unsigned short) _mbcjmstojis( tmpw );
	}
	return 0;
}

