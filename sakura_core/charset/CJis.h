#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return JISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< ���s�f�[�^�擾

public:
	//����
	static EConvertResult JISToUnicode(CMemory* pMem, bool base64decode = true);	// E-Mail(JIS��Unicode)�R�[�h�ϊ�	//2007.08.13 kobake �ǉ�
	static EConvertResult UnicodeToJIS(CMemory* pMem);		// Unicode   �� JIS�R�[�h�ϊ�

	static void SJIStoJIS(CMemory* pMem);		// SJIS��JIS�R�[�h�ϊ�
	static void JIStoSJIS( CMemory* pMem, bool base64decode = true);		// E-Mail(JIS��SJIS)�R�[�h�ϊ�	//Jul. 15, 2001 JEPRO

protected:
	static long MemJIStoSJIS(unsigned char*, long );	/* JIS��SJIS�ϊ� */
	static long MemSJIStoJIS( unsigned char*, long );	/* SJIS��JIS�ϊ� */
	static int StrSJIStoJIS( CMemory*, unsigned char*, int );	/* SJIS��JIS�ŐV�������m�� */
	static long QuotedPrintable_Decode(char*, long );	/* Quoted-Printable�f�R�[�h */

private:
	//�ϊ����j
	bool m_base64decode;
};
