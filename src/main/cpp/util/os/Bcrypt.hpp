/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/design_template.h"

#include <Windows.h>
#include <bcrypt.h>

//! Bcrypt.dll呼出をテスト可能にするDIっぽいもの
struct Bcrypt : public TSakuraSingleton<Bcrypt>
{
	~Bcrypt() override = default;

	virtual NTSTATUS BCryptGenRandom(
		_In_opt_                        BCRYPT_ALG_HANDLE   hAlgorithm,
		_Out_writes_bytes_(cbBuffer)    PUCHAR  pbBuffer,
		_In_                            ULONG   cbBuffer,
		_In_                            ULONG   dwFlags
	) const;
};
