#pragma once

#include "CCodeBase.h"
#include "CUnicode.h"

class CUnicodeBe : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeBEToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicodeBE(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:

	inline static EConvertResult UnicodeBEToUnicode(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_in(pMem,true); }	// UnicodeBE �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	inline static EConvertResult UnicodeToUnicodeBE(CMemory* pMem)
		{ return CUnicode::_UnicodeToUnicode_out(pMem,true); }	// Unicode   �� UnicodeBE�R�[�h�ϊ�

};


// util_string_ex2.h �ֈړ�
//class CEol;
//SAKURA_CORE_API const wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLine��wchar_t��(�r�b�N�G���f�B�A���p)
