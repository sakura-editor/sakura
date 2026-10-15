/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "util/os/Bcrypt.hpp"

#pragma comment(lib, "Bcrypt.lib")

NTSTATUS Bcrypt::BCryptGenRandom(
	_In_opt_                        BCRYPT_ALG_HANDLE   hAlgorithm,
	_Out_writes_bytes_(cbBuffer)    PUCHAR  pbBuffer,
	_In_                            ULONG   cbBuffer,
	_In_                            ULONG   dwFlags
) const
{
	return ::BCryptGenRandom(
		hAlgorithm,
		pbBuffer,
		cbBuffer,
		dwFlags
	);
}
