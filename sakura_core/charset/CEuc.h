#pragma once

#include "CCodeBase.h"

class CEuc : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

public:
	//����
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       �� Unicode�R�[�h�ϊ�  //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   �� EUC�R�[�h�ϊ�
	static void EUCToSJIS(CMemory* pMem);			// EUC       �� SJIS�R�[�h�ϊ�
	static void SJISToEUC(CMemory* pMem);			// SJIS      �� EUC�R�[�h�ϊ�
};
