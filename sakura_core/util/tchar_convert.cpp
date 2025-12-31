/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/tchar_convert.h"
#include "mem/CRecycledBuffer.h"
#include "charset/charcode.h"

#include <cstdlib>

static CRecycledBuffer        g_bufSmall;
static CRecycledBufferDynamic g_bufBig;

const WCHAR* to_wchar(const ACHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_wchar(src, (int)strlen(src));
}

const WCHAR* to_wchar(const ACHAR* pSrc, int nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	//必要なサイズを計算
	int nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		nullptr,
		0
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	WCHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<WCHAR>()){
		pDst=g_bufSmall.GetBuffer<WCHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<WCHAR>(nDstCnt);
	}

	//変換
	nDstLen = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		pDst,
		nDstLen
	);
	pDst[nDstLen] = L'\0';

	return pDst;
}

const ACHAR* to_achar(const WCHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_achar(src, (int)wcslen(src));
}

const ACHAR* to_achar(const WCHAR* pSrc, int nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	//必要なサイズを計算
	int nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		nullptr,
		0,
		nullptr,
		nullptr
	);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	ACHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<ACHAR>()){
		pDst=g_bufSmall.GetBuffer<ACHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<ACHAR>(nDstCnt);
	}

	//変換
	nDstLen = WideCharToMultiByte(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLength,
		pDst,
		nDstLen,
		nullptr,
		nullptr
	);
	pDst[nDstLen] = '\0';

	return pDst;
}

namespace cxx {

/*!
 * ワイド文字列をナロー文字列に変換します。
 */
std::string to_string(std::wstring_view source) {
	const auto langId = ::GetThreadUILanguage();
	const auto lcid = MAKELCID(langId, SORT_DEFAULT);

	std::wstring localeName{ LOCALE_NAME_MAX_LENGTH, L'\0' };
	LCIDToLocaleName(lcid, std::data(localeName), LOCALE_NAME_MAX_LENGTH, 0);

	UINT codePage;
	if (2 != ::GetLocaleInfoEx(localeName.c_str(), LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, LPWSTR(&codePage), 2)) {
		codePage = CP_SJIS;
	}

	// 変換エラーを受け取るフラグ
	BOOL bUsedDefaultChar = FALSE;

	// 変換に必要な出力バッファサイズを求める
	const auto required = ::WideCharToMultiByte(
		codePage,
		0,
		std::data(source),
		int(std::size(source)),
		LPSTR(nullptr),
		0,
		nullptr,
		&bUsedDefaultChar
	);

	// 変換エラーがあったら例外を投げる
	if (bUsedDefaultChar) {
		throw std::invalid_argument("Invalid wide character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	// 変換を実行する
	const auto converted = ::WideCharToMultiByte(
		codePage,
		0,
		std::data(source),
		int(std::size(source)),
		std::data(buffer),
		int(std::size(buffer)),
		nullptr,
		nullptr
	);

	buffer.resize(converted); // WideCharToMultiByteの戻り値は終端NULを含まない

	return buffer;
}

} // namespace cxx
