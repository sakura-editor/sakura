#pragma once

#include "CCodeBase.h"
#include "CShiftJis.h"

class CEuc : public CCodeBase{
public:
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst){ *pDst->_GetMemory()=cSrc; return EUCToUnicode(pDst->_GetMemory()); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst){ *pDst=*cSrc._GetMemory(); return UnicodeToEUC(pDst); }	//!< UNICODE    → 特定コード 変換
	void GetEol(CMemory* pcmemEol, EEolType eEolType){ CShiftJis::S_GetEol(pcmemEol,eEolType); }	//!< 改行データ取得
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, TCHAR* pDst);			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult EUCToUnicode(CMemory* pMem);		// EUC       → Unicodeコード変換  //2007.08.13 kobake 追加
	static EConvertResult UnicodeToEUC(CMemory* pMem);		// Unicode   → EUCコード変換
	static void EUCToSJIS(CMemory* pMem);			// EUC       → SJISコード変換
	static void SJISToEUC(CMemory* pMem);			// SJIS      → EUCコード変換

public:
	//各種判定関数
	static bool IsEucKan1( const uchar_t );  // EUCJP 文字長2 の場合の 0バイト目チェック
	static bool IsEucKan2( const uchar_t );  // EUCJP 文字長2 の場合の 1バイト目チェック
	static bool IsEucKan( const uchar_t* );  // IsEucKan1  + IsEucKan2
	static bool IsEucHanKata2( const uchar_t );  // EUCJP 半角カタカナ 2バイト目判別  add by genta
};

inline bool CEuc::IsEucKan1( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xfe );
}
inline bool CEuc::IsEucKan2( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xfe );
}
inline bool CEuc::IsEucKan( const uchar_t* pC )
{
	return ( IsEucKan1(*pC) && IsEucKan2(*(pC+1)) );
}
inline bool CEuc::IsEucHanKata2( const uchar_t c )
{
	return ( 0xa1 <= c && c <= 0xdf );
}
