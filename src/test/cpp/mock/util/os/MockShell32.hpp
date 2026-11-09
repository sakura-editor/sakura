/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "util/os/Shell32.hpp"

#include <gmock/gmock.h>

struct MockShell32 final : public Shell32
{
	MOCK_CONST_METHOD1(ShellExecuteExW, BOOL (SHELLEXECUTEINFOW*));
};
