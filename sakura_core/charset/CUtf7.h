#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CUtf7 : public CCodeBase{
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UTF7ToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUTF7(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult UTF7ToUnicode(CMemory* pMem);		// UTF-7     �� Unicode�R�[�h�ϊ� //2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToUTF7(CMemory* pMem);		// Unicode   �� UTF-7�R�[�h�ϊ�
	static int IsUTF7Direct( wchar_t ); /* Unicode������UTF7�Œ��ڃG���R�[�h�ł��邩 */ // 2002.10.25 Moca
	static int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64�G���R�[�h */


public:
	//�e�픻��萔
	static const bool UTF7SetD[];	// UTF7SetD ����������ۂɎg���u�[���l
public:
	//�e�픻��֐�
	static bool IsUtf7SetDChar( const uchar_t ); // UTF-7 Set D �̕����𔻕�
};

inline bool CUtf7::IsUtf7SetDChar( const uchar_t c )
{
	return ( !(c & 0x80) && UTF7SetD[c] );
}
