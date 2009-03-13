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
//	static int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64�G���R�[�h */  // convert/convert_util2.h �ֈړ�

protected:

	// 2008.11.10 �ϊ����W�b�N����������
	static int _Utf7SetDToUni_block( const char*, const int, wchar_t* );
	static int _Utf7SetBToUni_block( const char*, const int, wchar_t* );
	static int Utf7ToUni( const char*, const int, wchar_t*, bool* pbError );

	static int _UniToUtf7SetD_block( const wchar_t* pSrc, const int nSrcLen, char* pDst );
	static int _UniToUtf7SetB_block( const wchar_t* pSrc, const int nSrcLen, char* pDst );
	static int UniToUtf7( const wchar_t* pSrc, const int nSrcLen, char* pDst );


};

