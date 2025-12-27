/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/tchar_convert.h"
#include "mem/CRecycledBuffer.h"
#include "charset/charcode.h"

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
	// 変換に必要な出力バッファサイズを求める
	size_t required = 0;
	if (const auto ret = ::wcstombs_s(&required, nullptr, 0, std::data(source), 0); EILSEQ == ret) {
		throw std::invalid_argument("Invalid wide character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	size_t converted = 0;
	::wcstombs_s(&converted, std::data(buffer), required, std::data(source), _TRUNCATE);

	buffer.resize(converted - 1); // wcstombs_sの戻り値は終端NULを含むので -1 する

	return buffer;
}

} // namespace cxx
