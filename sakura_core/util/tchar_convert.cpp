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

namespace cxx {

/*!
 * ワイド文字列をナロー文字列に変換します。
 *
 * @param [in] codePage 変換に使用するコードページ。
 * @param [in] source 変換元のワイド文字列
 * @param [out] bUsedDefaultChar 変換に失敗した文字があったかどうかを受け取るフラグ
 */
int CountAsMultiByte(UINT codePage, std::wstring_view source, BOOL& bUsedDefaultChar) {
	// 変換に必要な出力バッファサイズを求める
	return ::WideCharToMultiByte(
		codePage,
		0,
		std::data(source),
		int(std::size(source)),
		LPSTR(nullptr),
		0,
		nullptr,
		&bUsedDefaultChar
	);
}

/*!
 * ワイド文字列をナロー文字列に変換します。
 *
 * @param [in] codePage 変換に使用するコードページ。
 * @param [in] source 変換元のワイド文字列
 * @param [out] buffer 変換後のナロー文字列を受け取るバッファ
 */
int WideCharToMultiByte(UINT codePage, std::wstring_view source, std::span<CHAR> buffer) {
	// 変換を実行する
	return ::WideCharToMultiByte(
		codePage,
		0,
		std::data(source),
		int(std::size(source)),
		std::data(buffer),
		int(std::size(buffer)),
		nullptr,
		nullptr
	);
}

/*!
 * ナロー文字列をワイド文字列に変換します。
 *
 * @param [in] codePage 変換に使用するコードページ。
 * @param [in] source 変換元のナロー文字列
 */
int CountAsWideChar(UINT codePage, std::string_view source) {
	// 変換に必要な出力バッファサイズを求める
	return ::MultiByteToWideChar(
		codePage,
		MB_ERR_INVALID_CHARS,
		std::data(source),
		int(std::size(source)),
		LPWSTR(nullptr),
		0
	);
}

/*!
 * ナロー文字列をワイド文字列に変換します。
 *
 * @param [in] codePage 変換に使用するコードページ。
 * @param [in] source 変換元のナロー文字列
 * @param [out] buffer 変換後のワイド文字列を受け取るバッファ
 */
int MultiByteToWideChar(UINT codePage, std::string_view source, std::span<WCHAR> buffer) {
	// 変換を実行する
	return ::MultiByteToWideChar(
		codePage,
		0,
		std::data(source),
		int(std::size(source)),
		std::data(buffer),
		int(std::size(buffer))
	);
}

} // namespace cxx

const WCHAR* to_wchar(const ACHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_wchar(std::string_view(src));
}

const WCHAR* to_wchar(const ACHAR* pSrc, size_t nSrcLength)
{
	if (!pSrc || !nSrcLength) return nullptr;

	return to_wchar(std::string_view(pSrc, nSrcLength));
}

const WCHAR* to_wchar(std::string_view source)
{
	if (source.empty()) return nullptr;

	//必要なサイズを計算
	int nDstLen = cxx::CountAsWideChar(CP_SJIS, source);

	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	WCHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<WCHAR>()){
		pDst=g_bufSmall.GetBuffer<WCHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<WCHAR>(nDstCnt);
	}

	auto buffer = std::span(pDst, nDstCnt);

	//変換
	nDstLen = cxx::MultiByteToWideChar(CP_SJIS, source, buffer);

	pDst[nDstLen] = L'\0';

	return pDst;
}

const ACHAR* to_achar(const WCHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_achar(std::wstring_view(src));
}

const ACHAR* to_achar(const WCHAR* pSrc, size_t nSrcLength)
{
	if (!pSrc || !nSrcLength) return nullptr;

	return to_achar(std::wstring_view(pSrc, nSrcLength));
}


const ACHAR* to_achar(std::wstring_view source)
{
	if (source.empty()) return nullptr;

	//必要なサイズを計算
	BOOL bUsedDefaultChar = FALSE;
	auto nDstLen = cxx::CountAsMultiByte(CP_SJIS, source, bUsedDefaultChar);
	size_t nDstCnt = (size_t)nDstLen + 1;

	//バッファ取得
	ACHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<ACHAR>()){
		pDst=g_bufSmall.GetBuffer<ACHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<ACHAR>(nDstCnt);
	}

	auto buffer = std::span(pDst, nDstCnt);

	//変換
	nDstLen = cxx::WideCharToMultiByte(CP_SJIS, source, buffer);

	pDst[nDstLen] = '\0';

	return pDst;
}

namespace cxx {

/*!
 * ワイド文字列をナロー文字列に変換します。
 *
 * @param [in] source 変換元のワイド文字列
 * @param [in, opt] codePage 変換に使用するコードページ。
 */
std::string to_string(std::wstring_view source, _In_opt_ UINT codePage) {
	if (source.empty()) {
		return "";
	}

	if (CP_ACP == codePage) {
		const auto langId = ::GetThreadUILanguage();
		const auto lcid = MAKELCID(langId, SORT_DEFAULT);

		std::wstring localeName{ LOCALE_NAME_MAX_LENGTH, L'\0' };
		LCIDToLocaleName(lcid, std::data(localeName), LOCALE_NAME_MAX_LENGTH, 0);

		if (2 != ::GetLocaleInfoEx(localeName.c_str(), LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, LPWSTR(&codePage), 2)) {
			codePage = CP_SJIS;
		}
	}

	// 変換エラーを受け取るフラグ
	BOOL bUsedDefaultChar = FALSE;

	// 変換に必要な出力バッファサイズを求める
	const auto required = cxx::CountAsMultiByte(codePage, source, bUsedDefaultChar);

	// 変換エラーがあったら例外を投げる
	if (bUsedDefaultChar) {
		throw std::invalid_argument("Invalid wide character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	// 変換を実行する
	const auto converted = cxx::WideCharToMultiByte(codePage, source, buffer);

	buffer.resize(converted); // WideCharToMultiByteの戻り値は終端NULを含まない

	return buffer;
}

/*!
 * ナロー文字列をワイド文字列に変換します。
 *
 * @param [in] source 変換元のナロー文字列
 * @param [in, opt] codePage 変換に使用するコードページ。
 */
std::wstring to_wstring(std::string_view source, _In_opt_ UINT codePage) {
	if (source.empty()) {
		return L"";
	}

	if (CP_ACP == codePage) {
		const auto langId = ::GetThreadUILanguage();
		const auto lcid = MAKELCID(langId, SORT_DEFAULT);

		std::wstring localeName{ LOCALE_NAME_MAX_LENGTH, L'\0' };
		LCIDToLocaleName(lcid, std::data(localeName), LOCALE_NAME_MAX_LENGTH, 0);

		if (2 != ::GetLocaleInfoEx(localeName.c_str(), LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER, LPWSTR(&codePage), 2)) {
			codePage = CP_SJIS;
		}
	}

	// 変換に必要な出力バッファサイズを求める
	const auto required = cxx::CountAsWideChar(codePage, source);

	// 変換エラーがあったら例外を投げる
	if (0 == required) {
		throw std::invalid_argument("Invalid character sequence.");
	}

	// 変換に必要な出力バッファを確保する
	std::wstring buffer(required, '\0');

	// 変換を実行する
	const auto converted = cxx::MultiByteToWideChar(codePage, source, buffer);

	buffer.resize(converted); // MultiByteToWideCharの戻り値は終端NULを含まない

	return buffer;
}

} // namespace cxx
