#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CUtf8 : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ return UTF8ToUnicode(cSrc,pDst); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ return UnicodeToUTF8(cSrc,pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetBom(CMemory* pcmemBom);																			//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE �� Hex �ϊ�

public:
	//����
	static EConvertResult UTF8ToUnicode(const CMemory& cSrcMem, CNativeW* pDstMem);		// UTF-8     �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUTF8(const CNativeW& cSrcMem, CMemory* pDstMem);		// Unicode   �� UTF-8�R�[�h�ϊ�
};
