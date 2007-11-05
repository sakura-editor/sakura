#pragma once

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW2* pDst){ *pDst->_GetMemory()=*pSrc; return UnicodeToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW2* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToUnicode(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

public:
	//����
	static EConvertResult UnicodeToUnicode(CMemory* pMem);	// Unicode   �� Unicode (�������Ȃ�)
};

