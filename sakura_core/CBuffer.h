//	$Id$
/************************************************************************
	CBuffer.h
    �������o�b�t�@�N���X
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
class CBuffer;

#ifndef _CBUFFER_H_
#define _CBUFFER_H_

class CBuffer
{
public:
	/*
	||  Constructors
	*/
	CBuffer();
	CBuffer( const char* , int  );
	~CBuffer();

    /*
    || �֐�
    */
    int GetLength() const;
	void SetData( const char*, int );	/* �o�b�t�@�̓��e��u�������� */
	void Append( const char* pData, int nDataLen );
	static int IsEqual( CBuffer&, CBuffer& );	/* ���������e�� */

	/*
	|| �ϊ��֐�
	*/
//	void BASE64Decode( void );	// Base64�f�R�[�h
//	void UUDECODE( char* );	/* uudecode(�f�R�[�h) */
	void Replace( char*, char* );	/* ������u�� */
//	void ToLower( void );	/* �p�啶�����p������ */
//	void ToUpper( void );	/* �p���������p�啶�� */
//	void SJIStoJIS( void );		/* �R�[�h�ϊ� �@SJIS��JIS�@*/
//	void JIStoSJIS( void );		/* �R�[�h�ϊ� �@JIS��SJIS�@*/
//	void SJISToUnicode( void );	/* �R�[�h�ϊ�   SJIS��Unicode */
//	void UnicodeToSJIS( void );	/* �R�[�h�ϊ�   Unicode��SJIS */
//	void SJISToEUC( void );		/* �R�[�h�ϊ�   SJIS��EUC */
//	void EUCToSJIS( void );		/* �R�[�h�ϊ�   EUC��SJIS */
	static const char* MemCharNext( const char*, int, const char* );	/* �|�C���^�Ŏ����������̎��ɂ��镶���̈ʒu��Ԃ��܂� */
	static const char* MemCharPrev( const char*, int, const char* );	/* �|�C���^�Ŏ����������̒��O�ɂ��镶���̈ʒu��Ԃ��܂� */
//	static int MemSJISToUnicode( char** , const char*, int );	/* ASCII&SJIS�������Unicode�ɕϊ� */
//	static int MemUnicodeToSJIS( char** , const char*, int );	/* Unicode�������ASCII&SJIS�ɕϊ� */

//	void ToZenkaku( int, int );	/* ���p���S�p */
//	void ToHankaku( void );	/* �S�p�����p */

//	/* �t�@�C���̓��{��R�[�h�Z�b�g���� */
//	static int CheckKanjiCode( const char* );
//	/* �t�@�C���̓��{��R�[�h�Z�b�g����:�@EUC���H */
//	static int CheckKanjiCode_EUC( const char* , int*, int* );
//	/* �t�@�C���̓��{��R�[�h�Z�b�g����:�@SJIS���H */
//	static int CheckKanjiCode_SJIS( const char* , int* , int* );
//	/* �t�@�C���̓��{��R�[�h�Z�b�g����:�@Unicode���H */
//	static CBuffer::CheckKanjiCode_UNICODE( const char*, int*, int* );
//	/* �t�@�C���̓��{��R�[�h�Z�b�g����:�@JIS���H */
//	static int CBuffer::CheckKanjiCode_JIS( const char*, int*, int* );
							
	/*
	|| ���Z�q
	*/
	const CBuffer& operator=( char );
//	const CBuffer& operator=( const char* );
	const CBuffer& operator=( const CBuffer& );
//	const CBuffer& operator+=( const char* );
	const CBuffer& operator+=( const CBuffer& );
	const CBuffer& operator+=( char );
//	operator char*() const;
//	operator const char*() const;
//	operator unsigned char*() const;
//	operator const unsigned char*() const;
//	operator void*() const;
//	operator const void*() const;
	const char operator[](int nIndex) const;
	char* GetPtr( int* ) const;	/* �f�[�^�ւ̃|�C���^�ƗL������Ԃ� */
//	void Append( const char* , int );	/* �f�[�^�̍Ō�ɒǉ� public�����o */
//protected:
	/*
	|| �����o�ϐ�
	*/
	int		m_nDataBufSize;
	char*	m_pData;
	int		m_nDataLen;

	/*
	||  �����w���p�֐�
	*/
	void Init( void );
	void Empty( void );
	void AllocBuffer( int );
	void AddData( const char*, int );
//	static int IsEUCKan1(unsigned char );	/* EUC�S�p�R�[�h�̂P�o�C�g�ڂ� */
//	static int IsEUCKan2(unsigned char );	/*  EUC�S�p�R�[�h�̂Q�o�C�g�ڂ� */
//	long MemBASE64_Decode(char* , long );	/* Base64�f�R�[�h */
//	long QuotedPrintable_Decode(char* , long );	/* Quoted-Printable�f�R�[�h	*/
//	long MemJIStoSJIS(unsigned char* , long );	/* JIS��SJIS�ϊ� */
//	int StrSJIStoJIS( CBuffer*, unsigned char*, int );	/* SJIS��JIS�ŐV�������m�ہ@*/
//	long MemSJIStoJIS( unsigned char* , long );	/* SJIS��JIS�ϊ� */
//	int IsBASE64Char( char );	/* ������Base64�̃f�[�^�� */

};


///////////////////////////////////////////////////////////////////////
#endif /* _CBUFFER_H_ */

/*[EOF]*/
