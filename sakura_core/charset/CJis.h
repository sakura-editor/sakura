#pragma once

#include "CCodeBase.h"

class CJis : public CCodeBase{
public:
	CJis(bool base64decode = true) : m_base64decode(base64decode) { }
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory* pSrc, CNativeW* pDst){ *pDst->_GetMemory()=*pSrc; return JISToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW* pSrc, CMemory* pDst){ *pDst=*pSrc->_GetMemory(); return UnicodeToJIS(pDst); }	//!< UNICODE    → 特定コード 変換

public:
	//実装
	static EConvertResult JISToUnicode(CMemory* pMem, bool base64decode = true);	// E-Mail(JIS→Unicode)コード変換	//2007.08.13 kobake 追加
	static EConvertResult UnicodeToJIS(CMemory* pMem);		// Unicode   → JISコード変換

	static void SJIStoJIS(CMemory* pMem);		// SJIS→JISコード変換
	static void JIStoSJIS( CMemory* pMem, bool base64decode = true);		// E-Mail(JIS→SJIS)コード変換	//Jul. 15, 2001 JEPRO

protected:
	static long MemJIStoSJIS(unsigned char*, long );	/* JIS→SJIS変換 */
	static long MemSJIStoJIS( unsigned char*, long );	/* SJIS→JIS変換 */
	static int StrSJIStoJIS( CMemory*, unsigned char*, int );	/* SJIS→JISで新メモリ確保 */
	static long QuotedPrintable_Decode(char*, long );	/* Quoted-Printableデコード */

private:
	//変換方針
	bool m_base64decode;
};
