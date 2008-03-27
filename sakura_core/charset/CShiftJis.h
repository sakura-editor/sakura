#pragma once

#include "CCodeBase.h"

class CShiftJis : public CCodeBase{
public:

public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾

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
};
