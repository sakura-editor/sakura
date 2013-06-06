/*!	@file
	@brief �����R�[�h�F���E���ʎx���֐����C�u����

	@author Sakura-Editor collaborators
	@date 1998/03/06 �V�K�쐬
	@date 2006/03/06 ���̕ύX�i�����F�����R�[�h�萔�̒�`�j
	@date 2007/03/19 ���̉���i�����F�����R�[�h�F�����C�u�����j
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2006, D. S. Koba, genta
	Copyright (C) 2007

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
#ifndef SAKURA_CODECHECKER_A8F89832_50E1_4F5F_9306_467062C9E3679_H_
#define SAKURA_CODECHECKER_A8F89832_50E1_4F5F_9306_467062C9E3679_H_

#include "_main/global.h"
#include "convert/convert_util2.h"



/*!
	�F�����镶���R�[�h���
*/
//enum ECodeType;     charset/charset.h �ɒ�`����Ă���
#if 0
enum ECodeType {
	CODE_SJIS,				// MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS)
	CODE_JIS,				// MS-CP5022x(ISO-2022-JP-MS)
	CODE_EUC,				// MS-CP51932, eucJP-ms(eucJP-open)
	CODE_UNICODE,			// UTF-16 LittleEndian(UCS-2)
	CODE_UTF8,				// UTF-8(UCS-2)
	CODE_UTF7,				// UTF-7(UCS-2)
	CODE_UNICODEBE,			// UTF-16 BigEndian(UCS-2)
	// ...
	CODE_CODEMAX,
	CODE_AUTODETECT = 99,	/* �����R�[�h�������� */
	CODE_DEFAULT    = CODE_SJIS,	/* �f�t�H���g�̕����R�[�h */

	/*
		- MS-CP50220
			Unicode ���� cp50220 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����� JIS X 0208 �̕Љ����ɒu�������
		- MS-CP50221
			Unicode ���� cp50221 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́AG0 �W���ւ̎w���̃G�X�P�[�v�V�[�P���X ESC ( I ��p���ăG���R�[�h�����
		- MS-CP50222
			Unicode ���� cp50222 �ւ̕ϊ����ɁA
			JIS X 0201 �Љ����́ASO/SI ��p���ăG���R�[�h�����

		�Q�l
		http://legacy-encoding.sourceforge.jp/wiki/
	*/
};
#endif




/*!
	�����I�ɔF�����镶���W��
*/
enum ECharSet {
	CHARSET_BINARY,					//!< �C�Ӓl
	CHARSET_ASCII7,					//!< 7�r�b�g ASCII ����  (ISO/IEC 646 IRV)
	CHARSET_JIS_HANKATA,			//!< ���{��̔��p�J�^�J�i����  (JIS X 0201)
	CHARSET_JIS_ZENKAKU,			//!< ���{��̑S�p����  (JIS X 0208 �{ MS �g������)
	CHARSET_JIS_SUPPLEMENTAL,		//!< ���{��̕⏕��������  (JIS X 0213)
	CHARSET_UNI_NORMAL,			//!< �T���Q�[�g�̈�����������j�R�[�h����
	CHARSET_UNI_SURROG,			//!< ���j�R�[�h�̃T���Q�[�g�̈�ɂ��镶��
	CHARSET_BINARY2,			//!< ������f��(�p���p)
	/*
		�� CHARSET_JIS_SUPPLEMENTAL �́AJIS_ZENKAKU �ƈꕔ�d�Ȃ荇������
		�@ ���܂育���v���Ȃ����������A�ꉞ�m���Ă�����́B
		���@CHARSET_BINARY �͕����W���ł͂Ȃ��W���B
	*/
};



/*!
	JIS �R�[�h�̃G�X�P�[�v�V�[�P���X����

	@note ���ԕς���Ɗ댯
*/
enum EJisEscseq {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,

	/*
		�����������W��       16�i�\��            ������\��
		------------------------------------------------------------
		JIS C 6226-1978      1b 24 40            ESC $ @
		JIS X 0208-1983      1b 24 42            ESC $ B
		JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
		JIS X 0212-1990      1b 24 28 44         ESC $ ( D
		JIS X 0213:2000 1��  1b 24 28 4f         ESC $ ( O
		JIS X 0213:2004 1��  1b 24 28 51         ESC $ ( Q
		JIS X 0213:2000 2��  1b 24 28 50         ESC $ ( P
		JIS X 0201 ���e��    1b 28 4a            ESC ( J
		JIS X 0201 ���e��    1b 28 48            ESC ( H         ���j�I [*]
		JIS X 0201 �Љ���    1b 28 49            ESC ( I
		ISO/IEC 646 IRV      1b 28 42            ESC ( B

		  [*] ���j�I�ȗ��R�ɂ��o�������G�X�P�[�v�V�[�P���X�D
		      JIS X 0201�̎w���Ƃ��Ă͎g�p���ׂ��łȂ��D

		�o�W�Fhttp://www.asahi-net.or.jp/~wq6k-yn/code/
		�Q�l�Fhttp://homepage2.nifty.com/zaco/code/
	*/
};


/*!
	�����I�ɔF������ JIS �G�X�P�[�v�V�[�P���X�̎��
*/
enum EMyJisEscseq {
	MYJISESC_NONE,
	MYJISESC_ASCII7,
	MYJISESC_HANKATA,
	MYJISESC_ZENKAKU,
	MYJISESC_UNKNOWN,
};







/* ------------------------------------------------------------------------
      �f�[�^�\�ƕϊ��⏕
   ------------------------------------------------------------------------

*/

// UTF7 �����𔻕ʂ���e�[�u��
extern const char TABLE_IsUtf7Direct[];
#if 0  // ���g�p
// JIS �R�[�h�̃G�X�P�[�v�V�[�P���X������f�[�^
extern const int TABLE_JISESCLEN[];
extern const char* TABLE_JISESCDATA[];
#endif


// Unicode ���ʊ֌W

/*!
	Unicode �� Noncharacter ���m�F

	�Q�l�����Fhttp://unicode.org/versions/Unicode4.0.0/ch15.pdf
*/
inline bool IsUnicodeNoncharacter( const wchar32_t wc )
{
	wchar32_t wc_ = wc & 0xffff;

	if( wc_ == 0xfffe || wc_ == 0xffff ){
		return true;
	}
	if( wc >= 0xfdd0 && wc <= 0xfdef ){
		return true;
	}
	return false;
}




/* ------------------------------------------------------------------------
      �����R�[�h���ʎx��
   ------------------------------------------------------------------------

*/

#define CHARCODE__IS_SJIS_ZEN1(x) ((unsigned int)((x) ^ 0x20) - 0xa1 < 0x3c)
#define CHARCODE__IS_SJIS_ZEN2(x) (0x40 <= (x) && (x) <= 0xfc && (x) != 0x7f)
#define CHARCODE__IS_EUCJP_ZEN1(x) ( 0xa1 <= (x) && (x) <= 0xfe )
#define CHARCODE__IS_EUCJP_ZEN2(x) ( 0xa1 <= (x) && (x) <= 0xfe )
#define CHARCODE__IS_EUCJP_HANKATA2(x) ( 0xa1 <= (x) && (x) <= 0xdf )
#define CHARCODE__IS_JIS(x) ( 0x21 <= (x) && (x) <= 0x7e )

//! 7bit ASCII ��
template< typename Tchar >
inline bool IsAscii7( const Tchar c ){
	unsigned int c_ = c;
	return ( c_ < 0x80 );
}
//! SJIS �S�p���� 1 �o�C�g�ڂ�
inline bool IsSjisZen1( const char c ){
	return CHARCODE__IS_SJIS_ZEN1(static_cast<unsigned char>(c));
}
//! SJIS �S�p���� 2 �o�C�g�ڂ�
inline bool IsSjisZen2( const char c ){
	return CHARCODE__IS_SJIS_ZEN2(static_cast<unsigned char>(c));
}
//! SJIS �S�p������
inline bool IsSjisZen( const char* pC ){
	return ( CHARCODE__IS_SJIS_ZEN1(static_cast<unsigned char>(pC[0]))
		&& CHARCODE__IS_SJIS_ZEN2(static_cast<unsigned char>(pC[1])) );
}

inline bool _IS_SJIS_1(unsigned char c)
{
	return IsSjisZen1(static_cast<char>(c));
}
inline bool _IS_SJIS_2(unsigned char c)
{
	return IsSjisZen2(static_cast<char>(c));
}
inline bool _IS_SJIS_1(char c)
{
	return IsSjisZen1(c);
}
inline bool _IS_SJIS_2(char c)
{
	return IsSjisZen2(c);
}
inline int my_iskanji1( int c )
{
	return IsSjisZen1(static_cast<char>(c & 0x00ff));
}
inline int my_iskanji2( int c )
{
	return IsSjisZen2(static_cast<char>(c & 0x00ff));
}

//! SJIS ���p�J�^�J�i��
inline bool IsSjisHankata( const char c ){
	return ( 0xa1 <= static_cast<unsigned char>(c) && static_cast<unsigned char>(c) <= 0xdf );
}
//! EUCJP �S�p���� 1 �o�C�g�ڂ�
inline bool IsEucjpZen1( const char c ){
	return CHARCODE__IS_EUCJP_ZEN1(static_cast<unsigned char>(c));
}
//! EUCJP �S�p���� 2 �o�C�g�ڂ�
inline bool IsEucjpZen2( const char c ){
	return CHARCODE__IS_EUCJP_ZEN2(static_cast<unsigned char>(c));
}
//! EUCJP �S�p������
inline bool IsEucjpZen( const char* pC ){
	return ( CHARCODE__IS_EUCJP_ZEN1(static_cast<unsigned char>(pC[0]))
		&& CHARCODE__IS_EUCJP_ZEN2(static_cast<unsigned char>(pC[1])) );
}
inline bool IsEucZen_hirakata( const char* pC ){
	unsigned char c0 = pC[0];
	unsigned char c1 = pC[1];
	return ( (c0 == 0xa4 && (c1 >= 0xa1 && c1 <= 0xf3))
	      || (c0 == 0xa5 && (c1 >= 0xa1 && c1 <= 0xf6)) );
}
//! EUCJP �S�p�����@�⏕������
inline bool IsEucjpSupplemtal( const char* pC){
	return ( static_cast<unsigned char>(pC[0]) == 0x8f
	      && CHARCODE__IS_EUCJP_ZEN1(static_cast<unsigned char>(pC[1]))
	      && CHARCODE__IS_EUCJP_ZEN2(static_cast<unsigned char>(pC[2])) );
}
//! EUCJP ���p�J�^�J�i���� 2 �o�C�g�ڂ�  added by genta
inline bool IsEucjpHankata2( const char c ){
	return CHARCODE__IS_EUCJP_HANKATA2(static_cast<unsigned char>(c));
}
//! EUCJP ���p�J�^�J�i������
inline bool IsEucjpHankata( const char *pC ){
	return ( static_cast<unsigned char>(pC[0]) == 0x8e && CHARCODE__IS_EUCJP_HANKATA2(static_cast<unsigned char>(pC[1])) );
}
//! ISO-2022-JP(JIS) ��
inline bool IsJis( const char c ){
	return CHARCODE__IS_JIS(static_cast<unsigned char>(c));
}
//! ISO-2022-JP(JIS) ���p�J�^�J�i������
inline bool IsJisHankata( const char c ){
	return ( 0x21 <= static_cast<unsigned char>(c) && static_cast<unsigned char>(c) <= 0x7e );
}
//! ISO-2022-JP(JIS) �S�p������
inline bool IsJisZen( const char* pC ){
	return ( CHARCODE__IS_JIS(static_cast<unsigned char>(pC[0]))
		&& CHARCODE__IS_JIS(static_cast<unsigned char>(pC[1])) );
}


#undef CHARCODE__IS_SJIS_ZEN1
#undef CHARCODE__IS_SJIS_ZEN2
#undef CHARCODE__IS_EUCJP_ZEN1
#undef CHARCODE__IS_EUCJP_ZEN2
#undef CHARCODE__IS_EUCJP_HANKATA2
#undef CHARCODE__IS_JIS


//! UTF16 ��ʃT���Q�[�g��
inline bool IsUtf16SurrogHi( const wchar_t wc ){
//	return ( 0xd800 <= wc && wc <= 0xdbff );
	return ( (static_cast<unsigned short>(wc) & 0xfc00) == 0xd800 );
}
//! UTF16 ���ʃT���Q�[�g������
inline bool IsUtf16SurrogLow( const wchar_t wc ){
//	return ( 0xdc00 <= wc && wc <= 0xdfff );
	return ( (static_cast<unsigned short>(wc) & 0xfc00) == 0xdc00 );
}
//! UtF-8�� ��ʃT���Q�[�g��
inline bool IsUtf8SurrogHi( const char* pS ) {
	const unsigned char* ps = reinterpret_cast<const unsigned char*>( pS );
	if( (ps[0] & 0xff) == 0xed && (ps[1] & 0xf0) == 0xa0 ){
		return true;
	}
	return false;
}
//! UtF-8�� ���ʃT���Q�[�g��
inline bool IsUtf8SurrogLow( const char* pS ) {
	const unsigned char* ps = reinterpret_cast<const unsigned char*>( pS );
	if( (ps[0] & 0xff) == 0xed && (ps[1] & 0xf0) == 0xb0 ){
		return true;
	}
	return false;
}
//! UTF-7 Set D �̕�����
template< typename CHAR_TYPE >
inline bool IsUtf7SetD( const CHAR_TYPE c ){
	unsigned int c_ = c;
	return ( c_ < 0x80 && TABLE_IsUtf7Direct[c_] == 1 );
}
//! UTF-7 Set O �̕�����
template< typename CHAR_TYPE >
inline bool IsUtf7SetO( const CHAR_TYPE c ){
	unsigned int c_ = c;
	return ( c_ < 0x80 && TABLE_IsUtf7Direct[c_] == 2 );
}
//! UTF-7 �Œ��ڃG���R�[�h���꓾�镶����
template< typename CHAR_TYPE >
inline bool IsUtf7Direct( const CHAR_TYPE c ){
	return IsUtf7SetD( c ) || IsUtf7SetO( c );
	// 2012.11.08 Set O ���ǂݍ��߂�悤��
}

//! UTF-7 Set B (Modified BASE64) �̕�����
template< class CHAR_TYPE >
inline bool IsBase64( const CHAR_TYPE c ){
	unsigned int c_ = c;
	return (c_ < 0x80 && (int)TABLE_BASE64CharToValue[c_] < 64)? true : false;
}

inline bool IsBinaryOnSurrogate( const wchar_t wc ){
	int wc_ = wc;
	return ( 0xdc00 <= wc_ && wc_ <= 0xdcff );
}

//! ���ʃT���Q�[�g�G���A���H	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
inline bool IsUTF16High( wchar_t c ){
	return IsUtf16SurrogHi(c);
}
//! ���ʃT���Q�[�g�G���A���H	from ssrc_2004-06-05wchar00703b	2008/5/15 Uchi
inline bool IsUTF16Low( wchar_t c ){
	return IsUtf16SurrogLow(c);
}


//! ��ʃo�C�g�Ɖ��ʃo�C�g������ (��� UTF-16 LE/BE ����)
inline unsigned short _SwapHLByte( const unsigned short wc ){
	unsigned short wc1 = static_cast<unsigned short>( (static_cast<unsigned int>(wc) << 8) & 0x0000ffff );
	unsigned short wc2 = static_cast<unsigned short>( (static_cast<unsigned int>(wc) >> 8) & 0x0000ffff );
	return (wc1 | wc2);
}

/*
	�������̐���
*/
//
// ---- �f�[�^���͗p
//
//! SJIS �̕������𐄑�
inline int GuessSjisCharsz( const char uc ){
	if( IsSjisZen1(uc) ){ return 2; }
	return 1;
}
//! UTF-16 �̕������𐄑��i�g�ݍ��킹������̍l���Ȃ��j
inline int GuessUtf16Charsz( const wchar_t wc ){
	if( IsUtf16SurrogHi(wc) ){ return 2; }
	return 1;
}
//! UTF-8 �̕������𐄑��i�g�ݍ��킹������̍l���Ȃ��j
inline int GuessUtf8Charsz( const char uc_ ){
	unsigned char uc = uc_;
	if( (uc & 0xe0) == 0xc0 ){ return 2; }
	if( (uc & 0xf0) == 0xe0 ){ return 3; }
	if( (uc & 0xf8) == 0xf0 ){ return 4; }
	return 1;
}
//! CESU-8 �̕������𐄑�
inline int GuessCesu8Charsz( const char uc_ ){
	unsigned char uc = uc_;
	if( (uc & 0xe0) == 0xc0 ){ return 2; }
	if( (uc & 0xf0) == 0xe0 ){ return 6; }
	return 1;
}
//! EUCJP �̕������𐄑�
inline int GuessEucjpCharsz( const char uc_ ){
	unsigned char uc = uc_;
	if( uc == 0x8f ){ return 3; }
	if( uc == 0x8e || IsEucjpZen1(static_cast<char>(uc)) ){ return 2; }
	return 1;
}

/*
	����������
*/
/* --- ���[�J�������R�[�h�`�F�b�N */
int CheckSjisChar( const char*, const int, ECharSet* );
int CheckEucjpChar( const char*, const int, ECharSet* );
int DetectJisEscseq( const char*, const int, EMyJisEscseq* ); // JIS �G�X�P�[�v�V�[�P���X���o��
int _CheckJisAnyPart( const char*, const int, const char **ppNextChar, EMyJisEscseq *peNextEsc, int *pnErrorCount, const int nType );
enum EJisChecker{
	JISCHECK_ASCII7,
	JISCHECK_HANKATA,
	JISCHECK_ZENKAKU,
	JISCHECK_UNKNOWN = -1,
};

inline int CheckJisAscii7Part( const char *pS, const int nLen,
		const char **ppNextChar, EMyJisEscseq *peNextEsc, int *pnErrorCount )
	{ return _CheckJisAnyPart( pS, nLen, ppNextChar, peNextEsc, pnErrorCount, JISCHECK_ASCII7 ); }
inline int CheckJisHankataPart( const char *pS, const int nLen,
		const char **ppNextChar, EMyJisEscseq *peNextEsc, int *pnErrorCount )
	{ return _CheckJisAnyPart( pS, nLen, ppNextChar, peNextEsc, pnErrorCount, JISCHECK_HANKATA ); }
inline int CheckJisZenkakuPart( const char *pS, const int nLen,
		const char **ppNextChar, EMyJisEscseq *peNextEsc, int *pnErrorCount )
	{ return _CheckJisAnyPart( pS, nLen, ppNextChar, peNextEsc, pnErrorCount, JISCHECK_ZENKAKU ); }
inline int CheckJisUnknownPart( const char *pS, const int nLen,
		const char **ppNextChar, EMyJisEscseq *peNextEsc, int *pnErrorCount )
	{ return _CheckJisAnyPart( pS, nLen, ppNextChar, peNextEsc, pnErrorCount, JISCHECK_UNKNOWN ); }


// _CheckUtf16Char �̃I�v�V������`
#define UC_NONCHARACTER 1  //!< �񕶎���s�������Ƃ���
// CheckUtf7BPart �̃I�v�V������`
#define UC_LOOSE 0x02


/* --- Unicode �n�R�[�h�`�F�b�N */
int _CheckUtf16Char( const wchar_t*, const int, ECharSet*, const int nOption, const bool bBigEndian );
inline int CheckUtf16leChar( const wchar_t* p, const int n, ECharSet* e, const int o ) { return _CheckUtf16Char( p, n, e, o, false ); }
inline int CheckUtf16beChar( const wchar_t* p, const int n, ECharSet* e, const int o ) { return _CheckUtf16Char( p, n, e, o, true ); }

int CheckUtf8Char( const char*, const int, ECharSet*, const bool bAllow4byteCode, const int nOption );
int CheckUtf8Char2( const char*, const int, ECharSet*, const bool bAllow4byteCode, const int nOption );
int CheckCesu8Char( const char*, const int, ECharSet*, const int nOption );
// UTF-7 �t�H�[�}�b�g�`�F�b�N
int CheckUtf7DPart( const char*, const int, char **ppNextChar, bool *pbError );
int CheckUtf7BPart( const char*, const int, char **ppNextChar, bool *pbError, const int nOption );

#endif /* SAKURA_CODECHECKER_A8F89832_50E1_4F5F_9306_467062C9E3679_H_ */
/*[EOF]*/
