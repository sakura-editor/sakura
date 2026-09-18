/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */

#pragma once

#if defined(_MSC_VER) && defined(_DEBUG)

#include "cxx/ResourceHolder.hpp"

#include <crtdbg.h>

#include <memory>

namespace testing {

/*!
 * @brief MSVCのアサーションダイアログを抑制するためのクラス
 */
struct MsvcReportMode
{
	struct SReportMode
	{
		int prevMode = ::_CrtSetReportMode(_CRT_ASSERT, 0);
	};

	static void _Cleanup(const SReportMode* oldMode)
	{
		::_CrtSetReportMode(_CRT_ASSERT, oldMode->prevMode);
	}

	using Holder = cxx::ResourceHolder<&_Cleanup>;
	using Me = MsvcReportMode;

	MsvcReportMode() = default;

	MsvcReportMode(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	MsvcReportMode(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

private:
	Holder m_old{ std::make_unique<SReportMode>().release() };
};

} // end of namespace testing

#endif // if defined(_MSC_VER) && defined(_DEBUG)
