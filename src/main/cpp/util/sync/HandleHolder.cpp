/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/sync/HandleHolder.hpp"

#include "util/os/Kernel32.hpp"
#include "util/os/make_system_error.hpp"
#include "util/os/ntstatus_category.hpp"

#include <system_error>

namespace cxx {

DWORD WaitForSingleObject(
	_In_ HANDLE hHandle,
	_In_ DWORD dwMilliseconds
)
{
	const auto dwRet = Kernel32::getInstance()->WaitForSingleObject(
		hHandle,
		dwMilliseconds
	);

	if (const auto bRet =
		dwRet == WAIT_OBJECT_0 ||
		dwRet == WAIT_ABANDONED;
		!bRet)
	{
		// システムエラー例外を発生させる
		throw std::system_error(
			int(dwRet),
			ntstatus_category(),
			"WaitForSingleObject() failed"
		);
	}

	return dwRet;
}

bool HandleHolder::Lock(
	DWORD dwTimeout
)
{
	bool bRet;
	try {
		// ロック取得を試行
		const auto dwRet = cxx::WaitForSingleObject(get(), dwTimeout);

		bRet =
			WAIT_OBJECT_0 == dwRet ||
			WAIT_ABANDONED == dwRet;
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		bRet = false;
	}

	return bRet;
}

bool HandleHolder::Unlock()
{
	return true;
}

} // namespace cxx

