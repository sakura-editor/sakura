#pragma once

#include "CCodeBase.h"

class CShiftJis : public CCodeBase{
public:

public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return SJISToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToSJIS(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult SJISToUnicode(CMemory* pMem);		// SJIS      → Unicodeコード変換
	static EConvertResult UnicodeToSJIS(CMemory* pMem);		// Unicode   → SJISコード変換
	static void S_GetEol(CMemory* pcmemEol, EEolType eEolType);	//!< 改行データ取得
	// 2005-09-02 D.S.Koba
	// 2007.08.14 kobake CMemoryからCShiftJisへ移動
	static int GetSizeOfChar( const char* pData, int nDataLen, int nIdx ); //!< 指定した位置の文字が何バイト文字かを返す

protected:
	static int MemSJISToUnicode( char**, const char*, int );	/* ASCII&SJIS文字列をUnicode に変換 */

public:
	//各種判定関数
	static bool IsSJisKan1( const uchar_t c );		//!< SJIS 文字長2 の場合の 0バイト目チェック
	static bool IsSJisKan2( const uchar_t c );		//!< SJIS 文字長2 の場合の 1バイト目チェック
	static bool IsSJisKan( const uchar_t* pC );		//!< IsSJisKan1 + IsSJisKan2
	static bool IsSJisHanKata( const uchar_t c );	//!< SJIS 半角カタカナ判別
};


inline bool CShiftJis::IsSJisKan1( const uchar_t c )
{
	// 参考 URL: http://www.st.rim.or.jp/~phinloda/cqa/cqa15.html#Q4
	//	return ( ((0x81 <= c) && (c <= 0x9f)) || ((0xe0 <= c) && (c <= 0xfc)) );
	return static_cast<unsigned int>(c ^ 0x20) - 0xa1 < 0x3c;
}

inline bool CShiftJis::IsSJisKan2( const uchar_t c )
{
	return ( 0x40 <= c && c <= 0xfc && c != 0x7f );
}

inline bool CShiftJis::IsSJisKan( const uchar_t* pC )
{
	return ( IsSJisKan1(*pC) && IsSJisKan2(*(pC+1)) );
}

inline bool CShiftJis::IsSJisHanKata( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xdf );
}

