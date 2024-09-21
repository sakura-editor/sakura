/*! @file */
/*
	Copyright (C) 2024, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#pragma once

#include <gmock/gmock.h>

#include "_main/CProcessFactory.h"

#include "util/os.h" // CCurrentDirectoryBackupPoint

struct CControlProcessInitTest : public ::testing::Test
{
	static constexpr auto& profileName = L"";

	/*!
	 * テストスイートの開始時に一度だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		EXPECT_FALSE(CProcess::getInstance());
	}

	std::unique_ptr<CProcess> process = nullptr;

	CCurrentDirectoryBackupPoint cCurDirBackup;

	/*!
	 * テストが起動される直前に毎回呼ばれる関数
	 */
	void SetUp() override {
		ASSERT_FALSE(CProcess::getInstance());

		// プロセスのインスタンスを用意する
		process = CProcessFactory().CreateInstance(fmt::format(LR"(-NOWIN -PROF="{}")", profileName));

		// コントロールプロセスを初期化する
		process->InitProcess();
	}

	/*!
	 * テストが実行された直後に毎回呼ばれる関数
	 */
	void TearDown() override {
		process.reset();

		EXPECT_FALSE(CProcess::getInstance());
	}

	/*!
	 * テストスイートの終了時に一度だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		EXPECT_FALSE(CProcess::getInstance());
	}
};
