#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"
#include "CUtf8.h"


class CCesu8 : public CCodeBase {
public:

	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){	//!< ����R�[�h �� UNICODE    �ϊ�
		*pDst->_GetMemory()=cSrc; return CUtf8::CESU8ToUnicode(pDst->_GetMemory());
	}
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){	//!< UNICODE    �� ����R�[�h �ϊ�
		*pDst=*cSrc._GetMemory(); return CUtf8::UnicodeToCESU8(pDst);
	}
	void GetBom(CMemory* pcmemBom);																			//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	inline EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst){			//!< UNICODE �� Hex �ϊ�
		return CUtf8()._UnicodeToHex( cSrc, iSLen, pDst, true );
	}

};
