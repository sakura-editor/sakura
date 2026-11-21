/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/sync/ProcessHolder.hpp"

#include <type_traits>

#include <Windows.h>

namespace cxx {

/*!
 * @brief 起動したエディタープロセスをラップするクラス
 *
 * リソースホルダーを継承するスマートポインター。
 */
class EditorProcessHolder : public ProcessHolder
{
private:
	using Base = ProcessHolder;
	using Me = EditorProcessHolder;

public:
	explicit EditorProcessHolder(
		HANDLE hProcess,
		DWORD dwProcessId,
		DWORD dwThreadId,
		HWND hWnd_
	)
		: Base(hProcess, dwProcessId, dwThreadId)
		, hWnd(hWnd_)
	{
	}

	EditorProcessHolder(const Me&) = delete;
	Me& operator=(const Me&) = delete;

	EditorProcessHolder(Me&& other) noexcept = default;
	Me& operator=(Me&& rhs) noexcept = default;

	HWND hWnd;
};

} // namespace cxx

