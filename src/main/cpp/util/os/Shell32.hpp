/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>
#include <shellapi.h>

 //! Shell32.dll呼出をテスト可能にするDIっぽいもの
struct Shell32 : public TSakuraSingleton<Shell32>
{
	using Me = Shell32;

	~Shell32() override = default;

	virtual BOOL ShellExecuteExW(
		SHELLEXECUTEINFOW* pExecInfo
	) const;
};
