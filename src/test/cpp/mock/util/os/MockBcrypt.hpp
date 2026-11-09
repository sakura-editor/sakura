/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/os/Bcrypt.hpp"

#include <gmock/gmock.h>

struct MockBcrypt final : public Bcrypt
{
	MOCK_CONST_METHOD4(BCryptGenRandom, NTSTATUS(
		_In_opt_                        BCRYPT_ALG_HANDLE   hAlgorithm,
		_Out_writes_bytes_(cbBuffer)    PUCHAR  pbBuffer,
		_In_                            ULONG   cbBuffer,
		_In_                            ULONG   dwFlags
	));
};
