#pragma once

#include "CCodeBase.h"

class CUtf8 : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW2* pDst){ return UTF8ToUnicode(pSrc,pDst); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW2* pSrc, CMemory* pDst){ return UnicodeToUTF8(pSrc,pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�


public:
	//����
	static EConvertResult UTF8ToUnicode(const CMemory* pSrcMem, CNativeW2* pDstMem);		// UTF-8     �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUTF8(const CNativeW2* pSrcMem, CMemory* pDstMem);		// Unicode   �� UTF-8�R�[�h�ϊ�
};
