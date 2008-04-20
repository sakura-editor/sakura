#pragma once

#include "CCodeBase.h"

class CUnicode : public CCodeBase{
public:
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UnicodeToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUnicode(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetBom(CMemory* pcmemBom);	//!< BOM�f�[�^�擾
	void GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult UnicodeToUnicode(CMemory* pMem);	// Unicode   �� Unicode (�������Ȃ�)

public:
	//�e�픻��֐�
	static bool IsUtf16SurrogHi( const uchar16_t );  // UTF16 ������4 �̏ꍇ�� 0-1�o�C�g�ڃ`�F�b�N
	static bool IsUtf16SurrogLow( const uchar16_t ); // UTF16 ������4 �̏ꍇ�� 2-3�o�C�g�ڃ`�F�b�N
};

inline bool CUnicode::IsUtf16SurrogHi( const uchar16_t wc )
{
//	return ( 0xd800 <= wc && wc <= 0xdbff );
	return ( (wc & 0xfc00) == 0xd800 );
}
inline bool CUnicode::IsUtf16SurrogLow( const uchar16_t wc )
{
//	return ( 0xdc00 <= wc && wc <= 0xdfff );
	return ( (wc & 0xfc00) == 0xdc00 );
}

