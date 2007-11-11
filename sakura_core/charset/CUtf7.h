#pragma once

#include "CCodeBase.h"

class CUtf7 : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return UTF7ToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToUTF7(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

public:
	//����
	static EConvertResult UTF7ToUnicode(CMemory* pMem);		// UTF-7     �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUTF7(CMemory* pMem);		// Unicode   �� UTF-7�R�[�h�ϊ�
	static int IsUTF7Direct( wchar_t ); /* Unicode������UTF7�Œ��ڃG���R�[�h�ł��邩 */ // 2002.10.25 Moca
	static int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64�G���R�[�h */
};
