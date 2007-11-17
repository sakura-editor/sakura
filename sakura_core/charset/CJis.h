#pragma once

#include "CCodeBase.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBase�C���^�[�t�F�[�X
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return JISToUnicode(pDst->_GetMemory()); }	//!< ����R�[�h �� UNICODE    �ϊ�
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToJIS(pDst); }	//!< UNICODE    �� ����R�[�h �ϊ�

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
