/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "util/os/Shell32.hpp"

BOOL Shell32::ShellExecuteExW(
	SHELLEXECUTEINFOW* pExecInfo
) const
{
	return ::ShellExecuteExW(pExecInfo);
}
