/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include <Windows.h>

/*!
 * @brief コピー＆ペーストモード
 * 
 * モードはビットフラグ。
 */
enum class ECopyAndPasteMode : uint32_t {
	Default = 0,
	EnableEolConversion = 0x01,
	DisableEolConversion = 0x02,
	EnableLineModePaste = 0x04,
	DisableLineModePaste = 0x08,
	EnableColumnModePaste = 0x10,
	DisableColumnModePaste = 0x20,
};

// Windows SDKのマクロを使って演算子を定義する
DEFINE_ENUM_FLAG_OPERATORS(ECopyAndPasteMode);
