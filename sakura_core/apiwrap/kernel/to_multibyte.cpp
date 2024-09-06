/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
 */
#include "StdAfx.h"
#include "apiwrap/kernel/to_multibyte.hpp"

std::string to_multibyte(std::wstring_view src, UINT codePage) {
	constexpr auto lpDefaultChar = LPCCH(nullptr);
	constexpr auto lpUsedDefaultChar = LPBOOL(nullptr);
	std::string dst;
	if (const auto required = WideCharToMultiByte(codePage, 0, &src[0], int(src.size()), nullptr, 0, lpDefaultChar, lpUsedDefaultChar))
	{
		dst.resize(required, wchar_t());
		WideCharToMultiByte(codePage, 0, &src[0], int(src.size()), dst.data(), required, lpDefaultChar, lpUsedDefaultChar);
	}
	return dst;
}
