#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CEuc : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       �� Unicode�R�[�h�ϊ�  //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   �� EUC�R�[�h�ϊ�
	static void EUCToSJIS(CMemory* pMem);			// EUC       �� SJIS�R�[�h�ϊ�
	static void SJISToEUC(CMemory* pMem);			// SJIS      �� EUC�R�[�h�ϊ�

public:
	//�e�픻��֐�
	static bool IsEucKan1( const uchar_t );  // EUCJP ������2 �̏ꍇ�� 0�o�C�g�ڃ`�F�b�N
	static bool IsEucKan2( const uchar_t );  // EUCJP ������2 �̏ꍇ�� 1�o�C�g�ڃ`�F�b�N
	static bool IsEucKan( const uchar_t* );  // IsEucKan1  + IsEucKan2
	static bool IsEucHanKata2( const uchar_t );  // EUCJP ���p�J�^�J�i 2�o�C�g�ڔ���  add by genta
};

inline bool CEuc::IsEucKan1( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xfe );
}
inline bool CEuc::IsEucKan2( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xfe );
}
inline bool CEuc::IsEucKan( const uchar_t* pC )
{
	return ( IsEucKan1(*pC) && IsEucKan2(*(pC+1)) );
}
inline bool CEuc::IsEucHanKata2( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xdf );
}
