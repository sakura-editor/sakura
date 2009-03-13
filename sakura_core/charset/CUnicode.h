#pragma once
// IsUtf16SurrogHi()�AIsUtf16SurrogLow() �֐���charset/codechecker.h �Ɉړ�

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< ����R�[�h �� UNICODE    �ϊ�
		*pDst->_GetMemory() = cSrc;
		return UnicodeToUnicode_in(pDst->_GetMemory());
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    �� ����R�[�h �ϊ�
		*pDst=*cSrc._GetMemory();
		return UnicodeToUnicode_out(pDst);
	}
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult _UnicodeToUnicode_in(CMemory* pMem, const bool bBigEndian);		// Unicode   �� Unicode (���͑�)
	static EConvertResult _UnicodeToUnicode_out(CMemory* pMem, const bool bBigEndian);	// Unicode   �� Unicode (�o�͑�)
	inline static EConvertResult UnicodeToUnicode_in(CMemory* pMem){ return _UnicodeToUnicode_in(pMem,false); }
	inline static EConvertResult UnicodeToUnicode_out(CMemory* pMem){ return _UnicodeToUnicode_out(pMem,false); }

};
