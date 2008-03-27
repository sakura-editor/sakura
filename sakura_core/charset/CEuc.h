#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CEuc : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       �� Unicode�R�[�h�ϊ�  //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   �� EUC�R�[�h�ϊ�
	static void EUCToSJIS(CMemory* pMem);			// EUC       �� SJIS�R�[�h�ϊ�
	static void SJISToEUC(CMemory* pMem);			// SJIS      �� EUC�R�[�h�ϊ�
};
