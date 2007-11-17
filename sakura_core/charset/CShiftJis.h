#pragma once

#include "CCodeBase.h"

class CShiftJis : public CCodeBase{
public:

public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

public:
	//����
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      �� Unicode�R�[�h�ϊ�
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   �� SJIS�R�[�h�ϊ�
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemory����CShiftJis�ֈړ�
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< �w�肵���ʒu�̕��������o�C�g��������Ԃ�

protected:
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS�������Unicode �ɕϊ� */
};
