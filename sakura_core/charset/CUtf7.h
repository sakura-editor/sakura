#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CUtf7 : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return UTF7ToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToUTF7(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得

public:
	//実装
	static EConvertResult UTF7ToUnicode(CMemory* pMem);		// UTF-7     → Unicodeコード変換 //2007.08.13 kobake 追加
	static EConvertResult UnicodeToUTF7(CMemory* pMem);		// Unicode   → UTF-7コード変換
//	static int MemBASE64_Encode( const char*, int, char**, int, int );/* Base64エンコード */  // convert/convert_util2.h へ移動

protected:

	// 2008.11.10 変換ロジックを書き直す
	static int _Utf7SetDToUni_block( const char*, const int, wchar_t* );
	static int _Utf7SetBToUni_block( const char*, const int, wchar_t* );
	static int Utf7ToUni( const char*, const int, wchar_t*, bool* pbError );

	static int _UniToUtf7SetD_block( const wchar_t* pSrc, const int nSrcLen, char* pDst );
	static int _UniToUtf7SetB_block( const wchar_t* pSrc, const int nSrcLen, char* pDst );
	static int UniToUtf7( const wchar_t* pSrc, const int nSrcLen, char* pDst );


};

