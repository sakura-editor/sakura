/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "cxx/ResourceHolder.hpp"
#include "extmodule/DbgHelp.hpp"

#include <Windows.h>
#include <DbgHelp.h>

#include <ostream>

namespace cxx {

/*!
 * @brief スタックトレースを出力するクラス
 */
class StackTraceWriter final : private DbgHelp
{
private:
	using Base = DbgHelp;
	using Me = StackTraceWriter;

	static void _CleanUp(
		const StackTraceWriter* pThis
	) noexcept;
	using Holder = cxx::ResourceHolder<&_CleanUp>;

public:
	StackTraceWriter();

	void WriteStackTrace(
		CONTEXT& context,
		std::ostream* out
	) const;

private:
	Holder m_Holder{ this };
};

}	// namespace cxx
