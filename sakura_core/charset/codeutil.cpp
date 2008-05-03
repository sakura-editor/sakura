#include "stdafx.h"
#include "codeutil.h"
#include "charset/charcode.h"
#include <mbstring.h>

/*!
	@brief �g���� SJIS��JIS�ϊ�

	SJIS�R�[�h��JIS�ɕϊ�����D���̍ہCJIS�ɑΉ��̈�̂Ȃ�IBM�g��������
	NEC�I��IBM�g�������ɕϊ�����D

	Shift_JIS fa40�`fc4b �͈̔͂̕����� 8754�`879a �܂��� ed40�`eefc ��
	�U�݂��镶���ɕϊ����ꂽ��ɁCJIS�ɕϊ�����܂��D
	
	@param pszSrc [in] �ϊ����镶����ւ̃|�C���^ (Shift JIS)
	
	@author ����
	@date 2002.10.03 1�����݈̂����C�ϊ��܂ōs���悤�ɕύX genta
*/
unsigned short _mbcjmstojis_ex( unsigned char* pszSrc )
{
	unsigned int	tmpw;	/* �� int �� 16 bit �ȏ�ł��鎖�����҂��Ă��܂��B */
	
	if(	_IS_SJIS_1(* pszSrc    ) &&	/* Shift_JIS �S�p������ 1�o�C�g�� */
		_IS_SJIS_2(*(pszSrc+1) )	/* Shift_JIS �S�p������ 2�o�C�g�� */
	){	/* Shift_JIS�S�p�����ł��� */
		tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( *pszSrc == 0x0fa ) ||
			( *pszSrc == 0x0fb ) ||
			( ( *pszSrc == 0x0fc ) && ( *(pszSrc+1) <= 0x04b ) )
		) {		/* fa40�`fc4b �̕����ł���B */
			/* �����R�[�h�ϊ����� */
			if		  ( tmpw <= 0xfa49 ) {	tmpw -= 0x0b51;	}	/* fa40�`fa49 �� eeef�`eef8 (�@�`�I) */
			else	if( tmpw <= 0xfa53 ) {	tmpw -= 0x72f6;	}	/* fa4a�`fa53 �� 8754�`875d (�T�`�]) */
			else	if( tmpw <= 0xfa57 ) {	tmpw -= 0x0b5b;	}	/* fa54�`fa57 �� eef9�`eefc (�ʁ`�W) */
			else	if( tmpw == 0xfa58 ) {	tmpw  = 0x878a;	}	/* �� */
			else	if( tmpw == 0xfa59 ) {	tmpw  = 0x8782;	}	/* �� */
			else	if( tmpw == 0xfa5a ) {	tmpw  = 0x8784;	}	/* �� */
			else	if( tmpw == 0xfa5b ) {	tmpw  = 0x879a;	}	/* �� */
			else	if( tmpw <= 0xfa7e ) {	tmpw -= 0x0d1c;	}	/* fa5c�`fa7e �� ed40�`ed62 (�\�`�~) */
			else	if( tmpw <= 0xfa9b ) {	tmpw -= 0x0d1d;	}	/* fa80�`fa9b �� ed63�`ed7e (���`��) */
			else	if( tmpw <= 0xfafc ) {	tmpw -= 0x0d1c;	}	/* fa9c�`fafc �� ed80�`ede0 (���`��) */
			else	if( tmpw <= 0xfb5b ) {	tmpw -= 0x0d5f;	}	/* fb40�`fb5b �� ede1�`edfc (�@�`�[) */
			else	if( tmpw <= 0xfb7e ) {	tmpw -= 0x0d1c;	}	/* fb5c�`fb7e �� ee40�`ee62 (�\�`�~) */
			else	if( tmpw <= 0xfb9b ) {	tmpw -= 0x0d1d;	}	/* fb80�`fb9b �� ee63�`ee7e (���`��) */
			else	if( tmpw <= 0xfbfc ) {	tmpw -= 0x0d1c;	}	/* fb9c�`fbfc �� ee80�`eee0 (���`��) */
			else{							tmpw -= 0x0d5f;	}	/* fc40�`fc4b �� eee1�`eeec (�@�`�K) */
		}
		return (unsigned short) _mbcjmstojis( tmpw );
	}
	return 0;
}

