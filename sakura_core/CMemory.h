//	$Id$
/*!	@file
	@brief �������o�b�t�@�N���X

	@author Norio Nakatani
	@date 1998/03/06 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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
class CMemory;

#ifndef _CMEMORY_H_
#define _CMEMORY_H_

/*! �t�@�C�������R�[�h�Z�b�g���ʎ��̐�ǂݍő�T�C�Y */
#define CheckKanjiCode_MAXREADLENGTH 16384

#include "global.h"

//! �������o�b�t�@�N���X
class SAKURA_CORE_API CMemory
{
public:
	/*
	||  Constructors
	*/
	CMemory();
	CMemory( const char*, int );
	~CMemory();

	/*
	|| �֐�
	*/
//  int GetLength() const;
	int GetLength() const { return m_nDataLen; }
	void AllocBuffer( int );	/* �o�b�t�@�T�C�Y�̒��� */
	void SetData( const char*, int );	/* �o�b�t�@�̓��e��u�������� */
	void SetDataSz( const char* );	/* �o�b�t�@�̓��e��u�������� */
	void SetData( CMemory* );	/* �o�b�t�@�̓��e��u�������� */
	const char* Append( const char* pData, int nDataLen );/* �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����ipublic�����o�j*/
	void AppendSz( const char* pszData );/* �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����ipublic�����o�j*/
	void Append( CMemory* );/* �o�b�t�@�̍Ō�Ƀf�[�^��ǉ�����ipublic�����o�j*/
//	void InsertTop( const char*, int );	/* �o�b�t�@�̐擪�Ƀf�[�^��}������ */
	// 2005-09-02 D.S.Koba
	static int GetSizeOfChar( const char*, const int, const int );	//!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�

	static int IsEqual( CMemory&, CMemory& );	/* ���������e�� */

	/*
	|| �ϊ��֐�
	*/
	void Replace( char*, char* );	/* ������u�� */
	void Replace_j( char*, char* );	/* ������u���i���{��l���Łj */
	void ToLower( void );	/* �p�啶�����p������ */
	void ToUpper( void );	/* �p���������p�啶�� */

	void AUTOToSJIS( void );	/* �������ʁ�SJIS�R�[�h�ϊ� */
	void SJIStoJIS( void );		/* SJIS��JIS�R�[�h�ϊ� */
//	void JIStoSJIS( bool base64decode = false);		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	void JIStoSJIS( bool base64decode = true);		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */	//Jul. 15, 2001 JEPRO
	void SJISToUnicode( void );	/* SJIS��Unicode�R�[�h�ϊ� */
	void SJISToUnicodeBE( void );	/* SJIS��UnicodeBE�R�[�h�ϊ� */
	void SJISToEUC( void );		/* SJIS��EUC�R�[�h�ϊ� */
	void EUCToSJIS( void );		/* EUC��SJIS�R�[�h�ϊ� */
	void UnicodeToSJIS( void );	/* Unicode��SJIS�R�[�h�ϊ� */
	void UnicodeBEToSJIS( void );	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	void UTF8ToSJIS( void );	/* UTF-8��SJIS�R�[�h�ϊ� */
	void UTF7ToSJIS( void );	/* UTF-7��SJIS�R�[�h�ϊ� */
	void SJISToUTF8( void );	/* SJIS��UTF-8�R�[�h�ϊ� */
	void SJISToUTF7( void );	/* SJIS��UTF-7�R�[�h�ϊ�  */
	void UnicodeToUTF8( void );	/* Unicode��UTF-8�R�[�h�ϊ� */
	void UnicodeToUTF7( void );	/* Unicode��UTF-7�R�[�h�ϊ� */
	void TABToSPACE( int );	/* TAB���� */
	void SPACEToTAB( int );	/* �󔒁�TAB */  //#### Stonee, 2001/05/27
	void SwapHLByte( void ); /* Byte���������� */

	void BASE64Decode( void );	// Base64�f�R�[�h
	void UUDECODE( char* );		/* uudecode(�f�R�[�h) */
//	static const char* MemCharNext( const char*, int, const char* );	/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
//	static const char* MemCharNext( /*const char*, int,*/ const char* );	/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
	static const char* MemCharNext( const char*, int, const char* );	/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
	static const char* MemCharPrev( const char*, int, const char* );	/* �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS�������Unicode �ɕϊ� */
	static int MemUnicodeToSJIS( char**, const char*, int );	/* Unicode�������ASCII&SJIS �ɕϊ� */
	static int DecodeUTF8toUnicode( const unsigned char*, int, unsigned char* );
	static int IsUTF8( const unsigned char*, int ); /* UTF-8�̕����� */
	void ToZenkaku( int, int );	/* ���p���S�p */
	void ToHankaku( int nMode );	/* �S�p�����p */

	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
	static int CheckKanjiCodeOfFile( const char* );
	/* ���{��R�[�h�Z�b�g���� */
	static int CheckKanjiCode( const unsigned char*, int );
	/* ���{��R�[�h�Z�b�g����: EUC���H */
	static int CheckKanjiCode_EUC( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: SJIS���H */
	static int CheckKanjiCode_SJIS( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: Unicode���H */
	static int CheckKanjiCode_UNICODE( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: UnicodeBE���H */
	static int CheckKanjiCode_UNICODEBE( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: JIS���H */
	static int CMemory::CheckKanjiCode_JIS( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: UTF-8S���H */
	static int CheckKanjiCode_UTF8( const unsigned char*, int, int*, int* );
	/* ���{��R�[�h�Z�b�g����: UTF-7S���H */
	static int CheckKanjiCode_UTF7( const unsigned char*, int, int*, int* );
	static int IsZenHiraOrKata( unsigned short );
	/* Unicode�nBOM�`�F�b�N */
	static int IsUnicodeBom( const unsigned char*, int );


	/*
	|| ���Z�q
	*/
	const CMemory& operator=( char );
//	const CMemory& operator=( const char* );
	const CMemory& operator=( const CMemory& );
//	const CMemory& operator+=( const char* );
	const CMemory& operator+=( const CMemory& );
	const CMemory& operator+=( char );
//	operator char*() const;
//	operator const char*() const;
//	operator unsigned char*() const;
//	operator const unsigned char*() const;
//	operator void*() const;
//	operator const void*() const;
	const char operator[](int nIndex) const;

	/* �f�[�^�ւ̃|�C���^�ƒ����Ԃ� */
	__forceinline char* GetPtr( int* pnLength ) const
	{
		if( NULL != pnLength ){
			*pnLength = GetLength();
		}
		return (char*)m_pData;
	}

	__forceinline char* GetPtr( void ) const
	{
		return (char*)m_pData;
	}


//	void Append( const char*, int );	/* �f�[�^�̍Ō�ɒǉ� public�����o */
protected: // 2002/2/10 aroka �A�N�Z�X���ύX
	/*
	|| �����o�ϐ�
	*/
	int		m_nDataBufSize;
	char*	m_pData;
	int		m_nDataLen;

	/*
	||  �����w���p�֐�
	*/
//	void Init( void );
	void Empty( void );
	void AddData( const char*, int );
	static int IsEUCKan1(unsigned char );	/* EUC�S�p�R�[�h�̂P�o�C�g�ڂ� */
	static int IsEUCKan2(unsigned char );	/* EUC�S�p�R�[�h�̂Q�o�C�g�ڂ� */
	static long MemBASE64_Decode( unsigned char*, long );	/* Base64�f�R�[�h */
	int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64�G���R�[�h */
	long QuotedPrintable_Decode(char*, long );	/* Quoted-Printable�f�R�[�h */
	long MemJIStoSJIS(unsigned char*, long );	/* JIS��SJIS�ϊ� */
	int StrSJIStoJIS( CMemory*, unsigned char*, int );	/* SJIS��JIS�ŐV�������m�� */
	long MemSJIStoJIS( unsigned char*, long );	/* SJIS��JIS�ϊ� */
	static int IsBASE64Char( char );	/* ������BaseE64�̃f�[�^�� */
	static int IsUTF7Direct( wchar_t ); /* Unicode������UTF7�Œ��ڃG���R�[�h�ł��邩 */ // 2002.10.25 Moca
	//	Oct. 3, 2002 genta
	static unsigned short _mbcjmstojis_ex( unsigned char* pszSrc );

};


///////////////////////////////////////////////////////////////////////
#endif /* _CMEMORY_H_ */


/*[EOF]*/
