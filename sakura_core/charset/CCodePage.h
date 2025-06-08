/*!	@file
	@brief コードページ
	
	@author Sakura-Editor collaborators
*/
/*
	Copyright (C) 2010-2012 Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCODEPAGE_2FB24C14_37A2_4D64_BF97_973E456462CE_H_
#define SAKURA_CCODEPAGE_2FB24C14_37A2_4D64_BF97_973E456462CE_H_
#pragma once

#include "CCodeBase.h"
#include <vector>
#include <utility>
#include <string>
#include "CShiftJis.h"

enum EEncodingTrait
{
	ENCODING_TRAIT_ERROR, // error
	ENCODING_TRAIT_ASCII,// ASCII comportible 1byte
	ENCODING_TRAIT_UTF16LE,// UTF-16LE
	ENCODING_TRAIT_UTF16BE,// UTF-16BE
	ENCODING_TRAIT_UTF32LE,// UTF-32LE 0123
	ENCODING_TRAIT_UTF32BE,// UTF-32BE 3210
	ENCODING_TRAIT_EBCDIC_CRLF,// EBCDIC/CR,LF
	ENCODING_TRAIT_EBCDIC,// EBCDIC/CR,LF,NEL
};

/*
	システムコードページによる文字コード変換
*/
class CCodePage : public CCodeBase{
public:
	CCodePage(int codepageEx) : m_nCodePageEx(codepageEx) { }
	
	//CCodeBaseインターフェース
	EConvertResult CodeToUnicode(const CMemory& cSrc, CNativeW* pDst) override{ return CPToUnicode(cSrc, pDst, m_nCodePageEx); }	//!< 特定コード → UNICODE    変換
	EConvertResult UnicodeToCode(const CNativeW& cSrc, CMemory* pDst) override{ return UnicodeToCP(cSrc, pDst, m_nCodePageEx); }	//!< UNICODE    → 特定コード 変換
	EConvertResult UnicodeToHex(const wchar_t* cSrc, const int iSLen, WCHAR* pDst, const CommonSetting_Statusbar* psStatusbar) override;			//!< UNICODE → Hex 変換

public:
	//実装
	static EConvertResult CPToUnicode(const CMemory& cSrc, CNativeW* pDst, int codepageEx);		// CodePage  → Unicodeコード変換 
	static EConvertResult UnicodeToCP(const CNativeW& cSrc, CMemory* pDst, int codepageEx);		// Unicode   → CodePageコード変換

	typedef std::vector<std::pair<int, std::wstring> > CodePageList;
	
	//GUI用補助関数
	static CCodePage::CodePageList& GetCodePageList();
	static int GetNameNormal(LPWSTR outName, int charcodeEx);
	static int GetNameShort(LPWSTR outName, int charcodeEx);
	static int GetNameLong(LPWSTR outName, int charcodeEx);
	static int GetNameBracket(LPWSTR outName, int charcodeEx);
	static int AddComboCodePages(HWND hwnd, HWND combo, int nSelCode);
	
	//CP補助情報
	static EEncodingTrait GetEncodingTrait(int charcodeEx);
	
protected:
	// 実装
	static EConvertResult CPToUni( const char* pSrc, const int nSrcLen, wchar_t* pDst, int nDstCchLen, int& nRetLen, UINT codepage );
	static EConvertResult UniToCP( const wchar_t* pSrc, const int nSrcLen, char* pDst, int nDstByteLen, int& nRetLen, UINT codepage );
	
	int m_nCodePageEx;
	
	static BOOL CALLBACK CallBackEnumCodePages( LPCWSTR );

	static int MultiByteToWideChar2(UINT codepage, int flags, const char* pSrc, int nSrcLen, wchar_t* pDst, int nDstLen);
	static int WideCharToMultiByte2(UINT codepage, int flags, const wchar_t* pSrc, int nSrcLen, char* pDst, int nDstLen);
	static int S_UTF32LEToUnicode(const char* pSrc, int nSrcLen, wchar_t* pDst, int nDstLen);
	static int S_UTF32BEToUnicode(const char* pSrc, int nSrcLen, wchar_t* pDst, int nDstLen);
	static int S_UnicodeToUTF32LE(const wchar_t* pSrc, int nSrcLen, char* pDst, int nDstLen);
	static int S_UnicodeToUTF32BE(const wchar_t* pSrc, int nSrcLen, char* pDst, int nDstLen);
};
#endif /* SAKURA_CCODEPAGE_2FB24C14_37A2_4D64_BF97_973E456462CE_H_ */
