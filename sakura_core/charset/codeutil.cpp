#include "stdafx.h"
#include "codeutil.h"
#include "charcode.h"
#include <mbstring.h>

/*!
	@brief g’£”Ε SJIS¨JIS•Ο·

	SJISƒR[ƒh‚πJIS‚Ι•Ο·‚·‚ιD‚»‚ΜΫCJIS‚Ι‘Ξ‰—Μζ‚Μ‚Θ‚ΆIBMg’£•¶‚π
	NEC‘I’θIBMg’£•¶‚Ι•Ο·‚·‚ιD

	Shift_JIS fa40`fc4b ‚Μ”ΝΝ‚Μ•¶‚Ν 8754`879a ‚ά‚½‚Ν ed40`eefc ‚Ι
	Uέ‚·‚ι•¶‚Ι•Ο·‚³‚κ‚½γ‚ΙCJIS‚Ι•Ο·‚³‚κ‚ά‚·D
	
	@param pszSrc [in] •Ο·‚·‚ι•¶—ρ‚Φ‚Μƒ|ƒCƒ“ƒ^ (Shift JIS)
	
	@author ‚·‚Ά
	@date 2002.10.03 1•¶‚Μ‚έµ‚ΆC•Ο·‚ά‚Εs‚¤‚ζ‚¤‚Ι•ΟX genta
*/
unsigned short _mbcjmstojis_ex( unsigned char* pszSrc )
{
	unsigned int	tmpw;	/* © int ‚ª 16 bit Θγ‚Ε‚ ‚ι–‚πϊ‘Ò‚µ‚Δ‚Ά‚ά‚·B */
	
	if(	_IS_SJIS_1(* pszSrc    ) &&	/* Shift_JIS ‘Sp•¶‚Μ 1ƒoƒCƒg–Ϊ */
		_IS_SJIS_2(*(pszSrc+1) )	/* Shift_JIS ‘Sp•¶‚Μ 2ƒoƒCƒg–Ϊ */
	){	/* Shift_JIS‘Sp•¶‚Ε‚ ‚ι */
		tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( *pszSrc == 0x0fa ) ||
			( *pszSrc == 0x0fb ) ||
			( ( *pszSrc == 0x0fc ) && ( *(pszSrc+1) <= 0x04b ) )
		) {		/* fa40`fc4b ‚Μ•¶‚Ε‚ ‚ιB */
			/* •¶ƒR[ƒh•Ο·— */
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40`fa49 ¨ eeef`eef8 (ξο`ξψ) */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a`fa53 ¨ 8754`875d (‡T`‡]) */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54`fa57 ¨ eef9`eefc (Κ`ξό) */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* ‡ */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* ‡‚ */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* ‡„ */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* ζ */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c`fa7e ¨ ed40`ed62 (ν@`νb) */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80`fa9b ¨ ed63`ed7e (νc`ν~) */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c`fafc ¨ ed80`ede0 (ν€`νΰ) */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40`fb5b ¨ ede1`edfc (να`νό) */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c`fb7e ¨ ee40`ee62 (ξ@`ξb) */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80`fb9b ¨ ee63`ee7e (ξc`ξ~) */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c`fbfc ¨ ee80`eee0 (ξ€`ξΰ) */
			else{							tmpw -= 0x0d5f;	}	/* fc40`fc4b ¨ eee1`eeec (ξα`ξμ) */
		}
		return (unsigned short) _mbcjmstojis( tmpw );
	}
	return 0;
}

