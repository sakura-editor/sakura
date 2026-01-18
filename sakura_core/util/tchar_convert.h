/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#define SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_
#pragma once

#include "basis/primitive.h"

//WCHARに変換
const WCHAR* to_wchar(const ACHAR* src);
const WCHAR* to_wchar(const ACHAR* pSrcData, size_t nSrcLength);

//ACHARに変換
inline
const ACHAR* to_achar(const ACHAR* src){ return src; }
const ACHAR* to_achar(const WCHAR* src);
const ACHAR* to_achar(const WCHAR* pSrc, size_t nSrcLength);

size_t	to_mbs(std::wstring_view source, std::span<CHAR> buffer, _In_ UINT codePage) noexcept;
size_t	to_wcs(std::string_view source, std::span<WCHAR> buffer, _In_ UINT codePage) noexcept;

namespace cxx {

size_t			count_as_mbs(std::wstring_view source, _In_ UINT codePage);
size_t			count_as_wcs(std::string_view source, _In_ UINT codePage);
std::string		to_string(std::wstring_view source, _In_opt_ UINT codePage = CP_ACP);
std::wstring	to_wstring(std::string_view source, _In_opt_ UINT codePage = CP_ACP);

} // namespace cxx

#endif /* SAKURA_TCHAR_CONVERT_2F41312D_27C8_4366_89F5_046BF7ED3B32_H_ */
