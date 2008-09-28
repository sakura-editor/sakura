#pragma once

#include "CCodeBase.h"

class CShiftJis : public CCodeBase{
public:

public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      �� Unicode�R�[�h�ϊ�
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   �� SJIS�R�[�h�ϊ�
	static void S_GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemory����CShiftJis�ֈړ�
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS�������Unicode �ɕϊ� */

public:
	//�e�픻��֐�
	static bool IsSJisKan1( const uchar_t c );		//!< SJIS ������2 �̏ꍇ�� 0�o�C�g�ڃ`�F�b�N
	static bool IsSJisKan2( const uchar_t c );		//!< SJIS ������2 �̏ꍇ�� 1�o�C�g�ڃ`�F�b�N
	static bool IsSJisKan( const uchar_t* pC );		//!< IsSJisKan1 + IsSJisKan2
	static bool IsSJisHanKata( const uchar_t c );	//!< SJIS ���p�J�^�J�i����
};


inline bool CShiftJis::IsSJisKan1( const uchar_t c )
{
	// �Q�l URL: http://www.st.rim.or.jp/~phinloda/cqa/cqa15.html#Q4
	//	return ( ((0x81 <= c) && (c <= 0x9f)) || ((0xe0 <= c) && (c <= 0xfc)) );
	return static_cast<unsigned int>(c ^ 0x20) - 0xa1 < 0x3c;
}

inline bool CShiftJis::IsSJisKan2( const uchar_t c )
{
	return ( 0x40 <= c && c <= 0xfc && c != 0x7f );
}

inline bool CShiftJis::IsSJisKan( const uchar_t* pC )
{
	return ( IsSJisKan1(*pC) && IsSJisKan2(*(pC+1)) );
}

inline bool CShiftJis::IsSJisHanKata( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xdf );
}

