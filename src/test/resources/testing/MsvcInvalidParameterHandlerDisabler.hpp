/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)

#include "cxx/ResourceHolder.hpp"

#include <cstdlib>
#include <cstdint>

namespace testing {

/*!
 * @brief MSVCの無効なパラメーターハンドラーを無効化するためのクラス
 */
struct MsvcInvalidParameterHandlerDisabler
{
	using Holder = cxx::ResourceHolder<&::_set_invalid_parameter_handler>;
	using Me = MsvcInvalidParameterHandlerDisabler;

	static void __cdecl empty_handler(
		wchar_t const*,
		wchar_t const*,
		wchar_t const*,
		unsigned int,
		uintptr_t
	)
	{
		return;	// 何もしない
	}

	MsvcInvalidParameterHandlerDisabler() = default;

	Holder prev{ ::_set_invalid_parameter_handler(&empty_handler) };
};

} // end of namespace testing

#endif // if defined(_MSC_VER) && defined(_DEBUG)
