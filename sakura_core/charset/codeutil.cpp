#include "StdAfx.h"
#include <mbstring.h>
#include "charset/codeutil.h"
#include "charset/charcode.h"
#include "charset/codechecker.h"

#if 0
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
unsigned int _mbcjmstojis_ex( unsigned int nSrc, bool* pbNonroundtrip )
{
	unsigned int	tmpw;	/* �� int �� 16 bit �ȏ�ł��鎖�����҂��Ă��܂��B */
	bool bnonrt = false;
	
	unsigned char c0 = static_cast<unsigned char>((nSrc & 0x0000ff00) >> 8);
	unsigned char c1 = static_cast<unsigned char>(nSrc & 0x000000ff);

	if(	IsSjisZen1( static_cast<char>(c0) )	/* Shift_JIS �S�p������ 1�o�C�g�� */
	 && IsSjisZen2( static_cast<char>(c1) )	/* Shift_JIS �S�p������ 2�o�C�g�� */
	){	/* Shift_JIS�S�p�����ł��� */
		tmpw = static_cast<unsigned int>(c0 << 8) | static_cast<unsigned int>(c1);
		//tmpw = ( ((unsigned int)*pszSrc) << 8 ) | ( (unsigned int)*(pszSrc + 1) );
		if(
			( c0 == 0x0fa ) ||
			( c0 == 0x0fb ) ||
			( ( c0 == 0x0fc ) && ( c1 <= 0x04b ) )
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
		return _mbcjmstojis( tmpw );
	}
	return 0;
}
#endif





/*
	���ʃe�[�u��   WinAPI �֐� WideCharToMultiByte �̓���ȕϊ��i���ݕϊ��ł��Ȃ��ϊ��j��
	�Y�����̒�`��F0x00 �` 0x5f(=0x00ff - 0x00a0)
*/
static const bool bNA = false;
const bool TABLE_WctombSpec[] = {
	bNA,  true, true, true, bNA,  true, true, bNA,   // 00a0 - 00a7
	bNA,  true, true, true, true, true, true, true,  // 00a8 - 00af
	bNA,  bNA,  true, true, bNA,  true, bNA,  true,  // 00b0 - 00b7
	true, true, true, true, bNA,  bNA,  bNA,  bNA,   // 00b8 - 00bf
	true, true, true, true, true, true, true, true,  // 00c0 - 00c7
	true, true, true, true, true, true, true, true,  // 00c8 - 00cf
	true, true, true, true, true, true, true, bNA,   // 00d0 - 00d7
	true, true, true, true, true, true, true, true,  // 00d8 - 00df
	true, true, true, true, true, true, true, true,  // 00e0 - 00e7
	true, true, true, true, true, true, true, true,  // 00e8 - 00ef
	true, true, true, true, true, true, true, bNA,   // 00f0 - 00f7
	true, true, true, true, true, true, true, true,  // 00f8 - 00ff
};



//
//	���ʁE�ϊ��e�[�u��   SJIS �̏d������������Ă��镶������������⏕�I�ȕ\
//
//	�Q�l�����ꗗ�F
//		Windows-31J �̕����Z�b�g�C�X�R ���V
//		    http://www2d.biglobe.ne.jp/~msyk/charcode/cp932/Windows-31J-charset.html
//		JIS ��{�����CCyber Librarian
//		    http://www.asahi-net.or.jp/~ax2s-kmtn/ref/jisx0208.html
//		�g�������R�[�h�ϊ��}�N���C����
//		    http://sakura.qp.land.to/?Macro%2F%C5%EA%B9%C6%2F126
//		[PRB] SHIFT - JIS �� Unicode �Ԃ̕ϊ����CMicrosoft
//		    http://support.microsoft.com/default.aspx?scid=kb;ja;Q170559
//

//
//	2���13��ɂ���d�������̊���R�[�h�|�C���g��^����\
//
static const unsigned short TABLE_SjisPoorcodeDef[] = {
	0x81be, // ��
	0x81bf, // ��
	0x81ca, // ��  (0xeef9 ����ύX)
	0x81da, // ��
	0x81db, // ��
	0x81df, // ��
	0x81e0, // ��
	0x81e3, // ��
	0x81e6, // ��  (0x879a ����ύX)
	0x81e7, // ��
	0x8754, // �T
	0x8755, // �U
	0x8756, // �V
	0x8757, // �W
	0x8758, // �X
	0x8759, // �Y
	0x875a, // �Z
	0x875b, // �[
	0x875c, // �\
	0x875d, // �]
	0x8782, // ��
	0x8784, // ��
	0x878a, // ��
};

//
//	2���13��̏d������������R�[�h�|�C���g�֕ϊ�����\
//
static const unsigned  short TABLE_SjisPoorcodeIndex[][2] = {
//  { �d�������l, �Ή����� TABLE_SjisPoorcodeDef ���̈ʒu }
	{ 0x8790,  6 }, // ��
	{ 0x8791,  5 }, // ��
	{ 0x8792,  9 }, // ��
	{ 0x8795,  7 }, // ��
	{ 0x8796,  4 }, // ��
	{ 0x8797,  3 }, // ��
	{ 0x879a,  8 }, // ��
	{ 0x879b,  1 }, // ��
	{ 0x879c,  0 }, // ��
	{ 0xeef9,  2 }, // ��
	{ 0xfa4a, 10 }, // �T
	{ 0xfa4b, 11 }, // �U
	{ 0xfa4c, 12 }, // �V
	{ 0xfa4d, 13 }, // �W
	{ 0xfa4e, 14 }, // �X
	{ 0xfa4f, 15 }, // �Y
	{ 0xfa51, 16 }, // �Z
	{ 0xfa52, 17 }, // �[
	{ 0xfa53, 18 }, // �\
	{ 0xfa54,  2 }, // ��
	{ 0xfa54, 19 }, // �]
	{ 0xfa58, 22 }, // ��
	{ 0xfa59, 20 }, // ��
	{ 0xfa5a, 21 }, // ��
	{ 0xfa5b,  8 }, // ��
};
static const int TABLESIZE_SjisPoorcodeIndex = 23;

//  �m�F�̂��߂̃e�[�u���Q�Ɛ���

//	typedef struct tag_SjisPoorcodeResolvTable {
//		uint16_t codeDefined;  // ����R�[�h�|�C���g
//		uint16_t pcodeAlias[2];  // �G�C���A�X�R�[�h�|�C���g
//	} SJIS_POORCODE_RESOLV_TABLE;
//
//
//	JIS X 0208 �ɂ���2��ɂĒ�`���ꂽ�L���ɂ���
//
//	2��Œ�`����Ă��镶���ɑ΂��ẮA2��̃R�[�h�|�C���g���g���B
//
//	const SJIS_POORCODE_RESOLV_TABLE TABLE_cSPCR_block2[] = {
//		{ 0x81be, { 0x879c, 0      } }, // 00. ��
//		{ 0x81bf, { 0x879b, 0      } }, // 01. ��
//		{ 0x81ca, { 0xeef9, 0xfa54 } }, // 02. ��
//		{ 0x81da, { 0x8797, 0      } }, // 03. ��
//		{ 0x81db, { 0x8796, 0      } }, // 04. ��
//		{ 0x81df, { 0x8791, 0      } }, // 05. ��
//		{ 0x81e0, { 0x8790, 0      } }, // 06. ��
//		{ 0x81e3, { 0x8795, 0      } }, // 07. ��
//		{ 0x81e6, { 0x879a, 0xfa5b } }, // 08. ��
//		{ 0x81e7, { 0x8792, 0      } }, // 09. ��
//	};
//
//
//	NEC �̓s���ɂ���13��ɒ�`���ꂽ�����ɂ���
//
//	2��Œ�`�ς݂̕����������āA
//	�c���������ɑ΂��Ă���13��iNEC���ꕶ���j�̃R�[�h�|�C���g���g���B
//
//	const SJIS_POORCODE_RESOLV_TABLE TABLE_cSPCR_block13 = {
//		{ 0x8754, { 0xfa4a, 0 } }, // 10. �T
//		{ 0x8755, { 0xfa4b, 0 } }, // 11. �U
//		{ 0x8756, { 0xfa4c, 0 } }, // 12. �V
//		{ 0x8757, { 0xfa4d, 0 } }, // 13. �W
//		{ 0x8758, { 0xfa4e, 0 } }, // 14. �X
//		{ 0x8759, { 0xfa4f, 0 } }, // 15. �Y
//		{ 0x875a, { 0xfa51, 0 } }, // 16. �Z
//		{ 0x875b, { 0xfa52, 0 } }, // 17. �[
//		{ 0x875c, { 0xfa53, 0 } }, // 18. �\
//		{ 0x875d, { 0xfa54, 0 } }, // 19. �]
//		{ 0x8782, { 0xfa59, 0 } }, // 20. ��
//		{ 0x8784, { 0xfa5a, 0 } }, // 21. ��
//		{ 0x878a, { 0xfa58, 0 } }, // 22. ��
//	}
//
//
//
//	IBM �̓s���ɂ���115��`119��ɒ�`���ꂽ�����ɂ���
//
//	2��� 13��Œ�`�ς݂̕����������Ďc���������ɑ΂��ẮA
//	<del>89�悩��92��ɂ���A����Ɠ����� NEC�I��IBM�g�������R�[�h�|�C���g���g���B</del>
//  115�悩�� 119��ɂ��� IBM�g�������̃R�[�h�|�C���g���g���B
//	0xfa5c ���� 0xfc4b�A
//	0xfa40 ���� 0xfa49�A
//	0xfa55 ���� 0xfa57 �� SJIS �R�[�h�|�C���g���K��Ƃ���B
//	�����̃R�[�h�|�C���g�̃G�C���A�X�́A���ꂼ��A
//	0xed40 ���� 0xeeec�A
//	0xeeef ���� 0xeef8�A
//	0xeefa ���� 0xeefc �� SJIS �R�[�h�|�C���g�ł���B
//




/*!
	SJIS �t�B���^  2���13��ɂ��镶���̐��퉻

	2��Œ�`����Ă��镶����2��ֈړ����A13��Œ�`����Ă���2��ɂ͂Ȃ�������13��ֈړ�������B
*/
unsigned int __fastcall SjisFilter_basis( const unsigned int uCode )
{
	int ni, nleft, nright;
	unsigned int code_tmp;

	/* �񕪌����J�n */
	nleft = 0;
	nright = TABLESIZE_SjisPoorcodeIndex -1;
	while( nleft <= nright ){
		ni = (nright + nleft) / 2;
		code_tmp = TABLE_SjisPoorcodeIndex[ni][0];
		if( uCode < code_tmp ){
			nright = ni - 1;
		}else if( code_tmp < uCode ){
			nleft = ni + 1;
		}else{
			break; // ����
		}
	}

	if( nleft <= nright ){
		return TABLE_SjisPoorcodeDef[ TABLE_SjisPoorcodeIndex[ni][1] ];
	}
	return uCode;
}


/*!
	SJIS �t�B���^  IBM�g������ �� NEC �I��IBM�g������

	IBM �g�����������̕����Ɠ����� NEC �I�� IBM �g���̈�̃R�[�h�|�C���g��
	�\�Ȍ���ړ�������B
*/
unsigned int __fastcall SjisFilter_ibm2nec( const unsigned int uCode )
{
	uchar_t c1, c2;
	unsigned int code = (unsigned int) uCode;

	/*
		�z�肳�����͒l�F

		fa40 �` fa7e, fa80 �` fafc
		fb40 �` fb7e, fb80 �` fbfc
		fc40 �` fc4b
	*/
	c1 = static_cast<unsigned char>( (code >> 8) & 0x000000ff );
	c2 = static_cast<unsigned char>( code & 0x000000ff );
	if( c1 == 0xfa || c1 == 0xfb || (c1 == 0xfc && c2 <= 0x4b) ){
		if     ( code <= 0xfa49 ) { code -= 0x0b51; }	/* fa40�`fa49 �� eeef�`eef8 (�@�`�I) */
		else if( code <= 0xfa54 ) { ; }
		else if( code <= 0xfa57 ) { code -= 0x0b5b; }	/* fa55�`fa57 �� eefa�`eefc (�U�`�W) */
		else if( code <= 0xfa5b ) { ; }
		else if( code <= 0xfa7e ) { code -= 0x0d1c; }	/* fa5c�`fa7e �� ed40�`ed62 (�\�`�~) */
		else if( code <= 0xfa9b ) { code -= 0x0d1d; }	/* fa80�`fa9b �� ed63�`ed7e (���`��) */
		else if( code <= 0xfafc ) { code -= 0x0d1c; }	/* fa9c�`fafc �� ed80�`ede0 (���`��) */
		else if( code <= 0xfb5b ) { code -= 0x0d5f; }	/* fb40�`fb5b �� ede1�`edfc (�@�`�[) */
		else if( code <= 0xfb7e ) { code -= 0x0d1c; }	/* fb5c�`fb7e �� ee40�`ee62 (�\�`�~) */
		else if( code <= 0xfb9b ) { code -= 0x0d1d; }	/* fb80�`fb9b �� ee63�`ee7e (���`��) */
		else if( code <= 0xfbfc ) { code -= 0x0d1c; }	/* fb9c�`fbfc �� ee80�`eee0 (���`��) */
		else                      { code -= 0x0d5f; }	/* fc40�`fc4b �� eee1�`eeec (�@�`�K) */
	}

	return code;
}


/*!
	SJIS �t�B���^  NEC �I��IBM�g������ �� IBM�g������

	NEC �I�� IBM �g�����������̕����Ɠ����� IBM �g�������̈�̃R�[�h�|�C���g��
	�\�Ȍ���ړ�������B
*/
unsigned int __fastcall SjisFilter_nec2ibm( const unsigned int uCode )
{
	uchar_t c1, c2;
	unsigned int code = (unsigned int) uCode;

	/*
		�z�肳�ꂽ���͒l�F

		ed40 �` ed7e, ed80 �` edfc
		ee40 �` ee7e, ee80 �` eefc
	*/
	c1 = static_cast<unsigned char>( (code >> 8) & 0x000000ff );
	c2 = static_cast<unsigned char>( code & 0x000000ff );
	if( c1 == 0xed || c1 == 0xee ){
		if     ( code <= 0xed62 ) { code += 0x0d1c; }	/* ed40�`ed62 �� fa5c�`fa7e (�\�`�~) */
		else if( code <= 0xed7e ) { code += 0x0d1d; }	/* ed63�`ed7e �� fa80�`fa9b (���`��) */
		else if( code <= 0xede0 ) { code += 0x0d1c; }	/* ed80�`ede0 �� fa9c�`fafc (���`��) */
		else if( code <= 0xedfc ) { code += 0x0d5f; }	/* ede1�`edfc �� fb40�`fb5b (�@�`�[) */
		else if( code <= 0xee62 ) { code += 0x0d1c; }	/* ee40�`ee62 �� fb5c�`fb7e (�\�`�~) */
		else if( code <= 0xee7e ) { code += 0x0d1d; }	/* ee63�`ee7e �� fb80�`fb9b (���`��) */
		else if( code <= 0xeee0 ) { code += 0x0d1c; }	/* ee80�`eee0 �� fb9c�`fbf0 (���`��) */
		else if( code <= 0xeeec ) { code += 0x0d5f; }	/* eee1�`eeec �� fc40�`fc4b (�@�`�K) */
		else if( code <= 0xeeee ) { ; }
		else if( code <= 0xeef8 ) { code += 0x0b51; }	/* eeef�`eef8 �� fa40�`fa49 (�@�`�I) */
		else if( code == 0xeef9 ) { ; }
		else                      { code += 0x0b5b; }	/* eefa�`eefc �� fa55�`fa57 (�U�`�W) */
	}

	return code;
}

