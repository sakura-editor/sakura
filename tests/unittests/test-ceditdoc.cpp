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
#include "pch.h"

#include "env/CSakuraEnvironment.h"

#include "_main/CProcessFactory.h"
#include "_main/CNormalProcess.h"

#include "CCodeChecker.h"
#include "window/CEditWnd.h"
#include "macro/CMacroFactory.h"
#include "extmodule/CMigemo.h"

#include "util/file.h"

struct CEditDocTest : public ::testing::Test
{
	static inline std::shared_ptr<CProcess> process = nullptr;

    static void SetUpTestSuite()
    {
		// エディタープロセスを生成する
		process = CProcessFactory().CreateInstance(LR"(-PROF="")");

		// エディタープロセスを初期化する
		process->InitProcess();
	}

    static void TearDownTestSuite() {
		process->TerminateControlProcess();

		process.reset();
	}
};

TEST_F(CEditDocTest, CCodeChecker)
{
	EXPECT_TRUE(CCodeChecker::getInstance());
}

TEST_F(CEditDocTest, CDiffManager)
{
	EXPECT_TRUE(CDiffManager::getInstance());
}

TEST_F(CEditDocTest, CEditWnd)
{
	EXPECT_TRUE(CEditWnd::getInstance());
}

TEST_F(CEditDocTest, CMacroFactory)
{
	EXPECT_TRUE(CMacroFactory::getInstance());
}

TEST_F(CEditDocTest, CMigemo)
{
	EXPECT_TRUE(CMigemo::getInstance());
}

/*!
 * @brief GREP検索キーの取得(設定なし)
 */
TEST_F(CEditDocTest, OnFileClose001)
{
	CAppMode::getInstance()->SetGrepKey( L"1234567890ABCDEF1234567890abcdef"sv );

	CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = true;

	process->GetShareData().m_Common.m_sSearch.m_bGrepExitConfirm = true;

	EXPECT_FALSE(CEditDoc::getInstance()->OnFileClose(false));
}
