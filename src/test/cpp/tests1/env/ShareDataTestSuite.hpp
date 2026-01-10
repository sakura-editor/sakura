/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/CShareData.h"

namespace env {

struct ShareDataTestSuite
{
	using CShareDataHolder = std::unique_ptr<CShareData>;

	static inline CShareDataHolder pcShareData = nullptr;

	static void SetUpShareData();
	static void TearDownShareData();
};

} // namespace env
