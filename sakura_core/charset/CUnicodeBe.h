#pragma once

#include "CCodeBase.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

public:
	//����
	static EConvertResult UnicodeBEToUnicode(CMemory* pMem);	// UnicodeBE �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUnicodeBE(CMemory* pMem);	// Unicode   �� UnicodeBE�R�[�h�ϊ�
};
