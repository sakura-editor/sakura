/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "pch.h"
#include "env/ShareDataTestSuite.hpp"

namespace env {

/*!
 * テストスイートの開始前に1回だけ呼ばれる関数
 */
/* static */ void ShareDataTestSuite::SetUpShareData()
{
	pcShareData = std::make_unique<CShareData>();

	EXPECT_THAT(pcShareData->InitShareData(), IsTrue());
}

/*!
 * テストスイートの終了後に1回だけ呼ばれる関数
 */
/* static */ void ShareDataTestSuite::TearDownShareData()
{
	pcShareData = nullptr;
}

} // namespace env
