#pragma once

#include "CCodeBase.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult UnicodeBEToUnicode(CMemory* pMem);	// UnicodeBE �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUnicodeBE(CMemory* pMem);	// Unicode   �� UnicodeBE�R�[�h�ϊ�
};

class CEol;
SAKURA_CORE_API const wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)
