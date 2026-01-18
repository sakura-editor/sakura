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

/*!
 * ワイド文字列をナロー文字列に変換します。
 *
 * @param [in] source 変換元のワイド文字列
 * @param [in] buffer 変換先のナロー文字列を格納するバッファー。
 * @param [in] codePage 変換に使用するコードページ。
 */
size_t to_mbs(std::wstring_view source, std::span<CHAR> buffer, _In_ UINT codePage) noexcept
{
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
 * @param [in] source 変換元のナロー文字列
 * @param [in] buffer 変換先のワイド文字列を格納するバッファー。
 * @param [in] codePage 変換に使用するコードページ。
 */
size_t to_wcs(std::string_view source, std::span<WCHAR> buffer, _In_ UINT codePage) noexcept
{
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

const WCHAR* to_wchar(const ACHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_wchar(src, (int)strlen(src));
}

const WCHAR* to_wchar(const ACHAR* pSrc, size_t nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	std::string_view source{ pSrc, nSrcLength };

	//必要なサイズを計算
	auto nDstCnt = cxx::count_as_wcs(source, CP_SJIS);

	//バッファ取得
	WCHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<WCHAR>()){
		pDst=g_bufSmall.GetBuffer<WCHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<WCHAR>(nDstCnt);
	}

	//変換
	to_wcs(source, std::span(pDst, nDstCnt), CP_SJIS);

	return pDst;
}

const ACHAR* to_achar(const WCHAR* src)
{
	if(src==nullptr)return nullptr;

	return to_achar(src, (int)wcslen(src));
}

const ACHAR* to_achar(const WCHAR* pSrc, size_t nSrcLength)
{
	if(pSrc==nullptr)return nullptr;

	std::wstring_view source{ pSrc, nSrcLength };

	//必要なサイズを計算
	auto nDstCnt = cxx::count_as_mbs(source, CP_SJIS);

	//バッファ取得
	ACHAR* pDst;
	if(nDstCnt < g_bufSmall.GetMaxCount<ACHAR>()){
		pDst=g_bufSmall.GetBuffer<ACHAR>(&nDstCnt);
	}
	else{
		pDst=g_bufBig.GetBuffer<ACHAR>(nDstCnt);
	}

	//変換
	to_mbs(source, std::span( pDst, nDstCnt ), CP_SJIS);

	return pDst;
}

namespace cxx {

/*!
 * ワイド文字列をナロー文字列に変換するのに必要なバッファサイズを数えます。
 *
 * @param [in] source 変換元のワイド文字列
 * @param [in] codePage 変換に使用するコードページ。
 */
size_t count_as_mbs(std::wstring_view source, _In_ UINT codePage)
{
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

	return required;
}

/*!
 * ナロー文字列をワイド文字列に変換するのに必要なバッファサイズを数えます。
 *
 * @param [in] source 変換元のナロー文字列
 * @param [in] codePage 変換に使用するコードページ。
 */
size_t count_as_wcs(std::string_view source, _In_ UINT codePage)
{
	// 変換に必要な出力バッファサイズを求める
	size_t required = ::MultiByteToWideChar(
		codePage,
		MB_ERR_INVALID_CHARS,
		std::data(source),
		int(std::size(source)),
		LPWSTR(nullptr),
		0
	);

	// 変換エラーがあったら例外を投げる
	if (0 == required) {
		throw std::invalid_argument("Invalid character sequence.");
	}

	return required;
}

/*!
 * ワイド文字列をナロー文字列に変換します。
 *
 * @param [in] source 変換元のワイド文字列
 * @param [in, opt] codePage 変換に使用するコードページ。
 */
std::string to_string(std::wstring_view source, _In_opt_ UINT codePage)
{
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

	// 変換に必要な出力バッファサイズを求める
	const auto required = count_as_mbs(source, codePage);

	// 変換に必要な出力バッファを確保する
	std::string buffer(required, '\0');

	// 変換を実行する
	const auto converted = to_mbs(source, buffer, codePage);

	buffer.resize(converted); // WideCharToMultiByteの戻り値は終端NULを含まない

	return buffer;
}

/*!
 * ナロー文字列をワイド文字列に変換します。
 *
 * @param [in] source 変換元のナロー文字列
 * @param [in, opt] codePage 変換に使用するコードページ。
 */
std::wstring to_wstring(std::string_view source, _In_opt_ UINT codePage)
{
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
	const auto required = count_as_wcs(source, codePage);

	// 変換に必要な出力バッファを確保する
	std::wstring buffer(required, '\0');

	// 変換を実行する
	const auto converted = to_wcs(source, buffer, codePage);

	buffer.resize(converted); // MultiByteToWideCharの戻り値は終端NULを含まない

	return buffer;
}

} // namespace cxx
