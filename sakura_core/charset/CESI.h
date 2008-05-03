/*!	@file
	@brief �����R�[�h�������ێ��N���X

	@author Sakura-Editor collaborators
	@date 2006/12/10 �V�K�쐬
*/
/*
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

#ifndef _CESI_H_
#define _CESI_H_

#include "global.h"
#include "charset/charcode.h"

enum EJisESCSeqType;

/*
	�����R�[�h������ �\���̌Q
*/
struct MBCODE_INFO{
	ECodeType eCodeID;		// �����R�[�h���ʔԍ�
	int nSpecBytes;			// ���L�o�C�g��
	int nDiff;				// �|�C���g�� := ���L�o�C�g�� �| �s���o�C�g��
};
struct WCCODE_INFO{
	ECodeType eCodeID;	// �����R�[�h���ʔԍ�
	int nCRorLF;			// ���C�h�����̉��s�̌�
	int nLostBytes;			// �s���o�C�g��
};
struct UNICODE_INFO{
	WCCODE_INFO Uni;		// �����R�[�h������ for UNICODE
	WCCODE_INFO UniBe;		// �����R�[�h������ for UNICODE BE
	int nCRorLF_ascii;		// �}���`�o�C�g�����̉��s�̌�
};


/*
	Encoding and Scan and Information class.

	�g�p���@
		CESI( const char*, const int ) �ŃI�u�W�F�N�g���쐬���邩�C
		ScanEncoding() ���g���ăf�[�^���擾���Ă���C
		Detect �n�֐����g��.
*/
class CESI
{
public:
	
	// CODE_CODEMAX -2 := �}���`�o�C�g�n�����R�[�h�̐�
	enum enum_CESI_public_constants {
		NUM_OF_MBCODE = CODE_CODEMAX - 2,
	};
	
	CESI(){ }
	CESI( const char* pS, const int nLen ){ ScanEncoding( pS, nLen ); }
	bool ScanEncoding( const char*, const int );
	int DetectUnicode( WCCODE_INFO* );
	int DetectMultibyte( MBCODE_INFO* );

	//�f�o�b�O
public:
	static CNativeT GetDebugInfo(const char* pBuf, int nBufLen);

private:
	MBCODE_INFO		m_pEI[NUM_OF_MBCODE];	//!< �}���`�o�C�g�n�R�[�h���
	UNICODE_INFO	m_WEI;					//!< ���j�R�[�h�n�R�[�h���


	// -- -- -- -- static -- -- -- -- //
	/*
	|| ���L�����w���p�֐�
	*/
public:
	// --- ���j�R�[�h BOM ���o��
	static ECodeType DetectUnicodeBom( const char*, int );
protected:
	// --- �������\���D
	static int _GuessCharLen_utf8( const uchar_t*, const int nStrLen = 4 );
	static int _GuessCharLen_sjis( const uchar_t*, const int nStrLen = 2 );
	static int _GuessCharLen_eucjp( const uchar_t*, const int nStrLen = 3 );
	// --- �ꕶ���`�F�b�N
	static int _CheckSJisChar( const uchar_t*, const int );
	static int _CheckSJisCharR( const uchar_t*, const int );  // CheckCharLenAsSJis �̋t�������[�h
	static int _CheckEucJpChar( const uchar_t*, const int );
	static int _CheckUtf8Char( const uchar_t*, const int );
	static int _imp_CheckUtf16Char( const uchar_t*, const int, bool );
	static int _CheckUtf16Char( const uchar_t*, const int );
	static int _CheckUtf16BeChar( const uchar_t*, const int );
	// --- UTF-7 �������`�F�b�N
	static int _CheckUtf7SetDPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	static int _CheckUtf7SetBPart( const uchar_t*, const int, uchar_t*& ref_pNextC );
	// --- JIS �G�X�P�[�v�V�[�P���X���o
	static int _DetectJisESCSeq( const uchar_t* pS, const int nLen, EJisESCSeqType* pnEscType );

	/*
	|| ������̕����R�[�h���𓾂�D
	*/
protected:
	// --- ������`�F�b�N
	static void _GetEncdInf_SJis( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Jis( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_EucJp( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Utf8( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Utf7( const char*, const int, MBCODE_INFO* );
	static void _GetEncdInf_Uni( const char*, const int, UNICODE_INFO* );
};



#endif /*_CESI_H_*/

