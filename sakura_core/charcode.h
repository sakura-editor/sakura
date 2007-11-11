/*!	@file
	@brief �����R�[�h�F�����C�u����

	@author Sakura-Editor collaborators
	@date 1998/03/06 �V�K�쐬 by ����� Norio Nakatani (C) 1998-2001
	@date 2006/03/06 ���́E���C�Z���X���� [�����R�[�h�萔�̒�`] �� [�����R�[�h�F�����C�u����]
*/
/*
	Copyright (C) 2006, D. S. Koba, rastiv

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


#ifndef _CHARCODE_H_
#define _CHARCODE_H_



enum enumCodeType;
typedef enumCodeType ECodeType;
#if 0
/*! �����R�[�h�Z�b�g��� */
enum enumCodeType {
	CODE_SJIS,				// MS-CP932(Windows-31J), �V�t�gJIS(Shift_JIS)
	CODE_JIS,				// MS-CP5022x(ISO-2022-JP-MS)
	CODE_EUC,				// MS-CP51932, eucJP-ms(eucJP-open)
	CODE_UNICODE,			// UTF-16 LittleEndian(UCS-2)
	CODE_UTF8,				// UTF-8(UCS-2)
	CODE_UTF7,				// UTF-7(UCS-2)
	CODE_UNICODEBE,			// UTF-16 BigEndian(UCS-2)
	// ...
	CODE_CODEMAX,
	CODE_AUTODETECT	= 99	/* �����R�[�h�������� */
	
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

/*! JIS �R�[�h�̃G�X�P�[�v�V�[�P���X���� */
/*
	�����������W��       16�i�\��            ������\��[*1]
	------------------------------------------------------------
	JIS C 6226-1978      1b 24 40            ESC $ @
	JIS X 0208-1983      1b 24 42            ESC $ B
	JIS X 0208-1990      1b 26 40 1b 24 42   ESC & @ ESC $ B
	JIS X 0212-1990      1b 24 28 44         ESC $ ( D
	JIS X 0213:2000 1��  1b 24 28 4f         ESC $ ( O
	JIS X 0213:2004 1��  1b 24 28 51         ESC $ ( Q
	JIS X 0213:2000 2��  1b 24 28 50         ESC $ ( P
	JIS X 0201 ���e��    1b 28 4a            ESC ( J
	JIS X 0201 ���e��    1b 28 48            ESC ( H         (���j�I[*2])
	JIS X 0201 �Љ���    1b 28 49            ESC ( I
	ISO/IEC 646 IRV      1b 28 42            ESC ( B
	
	
	  [*1] �e�o�C�g��֋X�I��ISO/IEC 646 IRV�̕����ŕ\�������́B
	       ������ESC�̓o�C�g�l1b��\���B
	
	  [*2] JIS X 0201�̎w���Ƃ��Ă͎g�p���ׂ��łȂ����A�Â��f�[�^�ł�
	       �g���Ă���\��������B
	
	�o�W�Fhttp://www.asahi-net.or.jp/~wq6k-yn/code/
	�Q�l�Fhttp://homepage2.nifty.com/zaco/code/
*/
enum enumJisESCSeqType {
	JISESC_UNKNOWN,
	JISESC_ASCII,
	JISESC_JISX0201Latin,
	JISESC_JISX0201Latin_OLD,
	JISESC_JISX0201Katakana,
	JISESC_JISX0208_1978,
	JISESC_JISX0208_1983,
	JISESC_JISX0208_1990,
};

/*
	�����R�[�h������ �\���̌Q
*/
typedef struct EncodingInfo_t {
	ECodeType eCodeID;		// �����R�[�h���ʔԍ�
	int nSpecBytes;			// ���L�o�C�g��
	int nDiff;				// �|�C���g�� := ���L�o�C�g�� �| �s���o�C�g��
} MBCODE_INFO;
typedef struct WC_EncodingInfo_t {
	enumCodeType eCodeID;	// �����R�[�h���ʔԍ�
	int nCRorLF;			// ���C�h�����̉��s�̌�
	int nLostBytes;			// �s���o�C�g��
} WCCODE_INFO;
typedef struct UnicodeInfo_t {
	WCCODE_INFO Uni;		// �����R�[�h������ for UNICODE
	WCCODE_INFO UniBe;		// �����R�[�h������ for UNICODE BE
	int nCRorLF_ascii;		// �}���`�o�C�g�����̉��s�̌�
} UNICODE_INFO;

/*
	�֐��̃G�~�����[�V����
*/




namespace Charcode
{
	// BASE64�G���R�[�h��Ɏg�p���镶��
	extern const uchar_t BASE64CHAR[];
	// BASE64�f�R�[�h����ۂɎg���o�C�i���l
	extern const uchar_t BASE64VAL[];
	// UTF7SetD ����������ۂɎg���u�[���l
	extern const bool UTF7SetD[];
	// JIS �R�[�h�̃G�X�P�[�v�V�[�P���X������f�[�^
	extern const char JISESCDATA_ASCII[];
	extern const char JISESCDATA_JISX0201Latin[];
	extern const char JISESCDATA_JISX0201Latin_OLD[];
	extern const char JISESCDATA_JISX0201Katakana[];
	extern const char JISESCDATA_JISX0208_1978[];
	extern const char JISESCDATA_JISX0208_1983[];
	extern const char JISESCDATA_JISX0208_1990[];
	extern const int TABLE_JISESCLEN[];
	extern const char* TABLE_JISESCDATA[];
	
	uchar_t __fastcall Base64_CharToVal( const uchar_t );
	uchar_t __fastcall Base64_ValToChar( const uchar_t );
	int __fastcall GetJisESCSeqLen( const enumJisESCSeqType );
	const char* __fastcall GetJisESCSeqData( const enumJisESCSeqType );

	/*
	|| ���L�����w���p�֐�
	*/
	
	// --- �����R�[�h���ʎx��
	bool __fastcall IsSJisKan1( const uchar_t );  // SJIS ������2 �̏ꍇ�� 0�o�C�g�ڃ`�F�b�N
	bool __fastcall IsSJisKan2( const uchar_t );  // SJIS ������2 �̏ꍇ�� 1�o�C�g�ڃ`�F�b�N
	bool __fastcall IsSJisKan( const uchar_t* ); // IsSJisKan1 + IsSJisKan2
	bool __fastcall IsSJisHanKata( const uchar_t );  // SJIS ���p�J�^�J�i����
	bool __fastcall IsEucKan1( const uchar_t );  // EUCJP ������2 �̏ꍇ�� 0�o�C�g�ڃ`�F�b�N
	bool __fastcall IsEucKan2( const uchar_t );  // EUCJP ������2 �̏ꍇ�� 1�o�C�g�ڃ`�F�b�N
	bool __fastcall IsEucKan( const uchar_t* );  // IsEucKan1  + IsEucKan2
	bool __fastcall IsEucHanKata2( const uchar_t );  // EUCJP ���p�J�^�J�i 2�o�C�g�ڔ���  add by genta
	bool __fastcall IsUtf16SurrogHi( const uchar16_t );  // UTF16 ������4 �̏ꍇ�� 0-1�o�C�g�ڃ`�F�b�N
	bool __fastcall IsUtf16SurrogLow( const uchar16_t ); // UTF16 ������4 �̏ꍇ�� 2-3�o�C�g�ڃ`�F�b�N
#if 0
	bool __fastcall IsUtf16Surrogates( const uchar16_t* ); // UTF16 �̃T���Q�[�g�y�A����
	bool __fastcall IsUtf16SurrogHiOrLow( const uchar16_t );  // UTF16 �̃T���Q�[�g�Д���
#endif
	bool __fastcall IsBase64Char( const uchar_t );  // UTF-7 �Ŏg���� Modified BASE64 �𔻕�
	bool __fastcall IsUtf7SetDChar( const uchar_t ); // UTF-7 Set D �̕����𔻕�
	// --- �������\���D
	int GuessCharLen_utf8( const uchar_t*, const int nStrLen = 4 );
	int GuessCharLen_sjis( const uchar_t*, const int nStrLen = 2 );
	int GuessCharLen_eucjp( const uchar_t*, const int nStrLen = 3 );
#if 0
	int GuessCharLenAsUtf16_imp( const uchar_t*, const int, bool bBigEndian );
	int GuessCharLenAsUtf16( const uchar_t*, const int nStrLen = 4 );
	int GuessCharLenAsUtf16Be( const uchar_t*, const int nStrLen = 4 );
#endif
	// --- �ꕶ���`�F�b�N
	int CheckSJisChar( const uchar_t*, const int );
	int CheckSJisCharR( const uchar_t*, const int );  // CheckCharLenAsSJis �̋t�������[�h
	int CheckEucJpChar( const uchar_t*, const int );
	int CheckUtf8Char( const uchar_t*, const int );
	int imp_CheckUtf16Char( const uchar_t*, const int, bool );
	int CheckUtf16Char( const uchar_t*, const int );
	int CheckUtf16BeChar( const uchar_t*, const int );
#if 0
	int CheckJisChar_JISX0208( const uchar_t*, const int );
#endif
	// --- �������`�F�b�N
	int GetCharLen_sjis( const uchar_t*, const int );
	int GetCharLenR_sjis( const uchar_t*, const int );  // GetCharLenAsSJis �̋t�������[�h
	int GetCharLen_eucjp( const uchar_t*, const int );
	int GetCharLen_utf8( const uchar_t*, const int );
	// --- UTF-7 �������`�F�b�N
	int CheckUtf7SetDPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	int CheckUtf7SetBPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	// --- JIS �G�X�P�[�v�V�[�P���X���o
	int DetectJisESCSeq( const uchar_t* pS, const int nLen, int* pnEscType );
	// --- ���j�R�[�h BOM ���o��
	ECodeType DetectUnicodeBom( const char*, int );
	
	/*
	|| ������̕����R�[�h���𓾂�D
	*/
	
	// --- ������`�F�b�N
	void GetEncdInf_SJis( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Jis( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_EucJp( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Utf8( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Utf7( const char*, const int, MBCODE_INFO* );
	void GetEncdInf_Uni( const char*, const int, UNICODE_INFO* );
} // ends namespace Charcode.

#include "charcode2.h"


#endif /* _CHARCODE_H_ */

/*[EOF]*/
