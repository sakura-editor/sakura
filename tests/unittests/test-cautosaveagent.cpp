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
#include "CAutoSaveAgent.h"

#include "_main/CProcessFactory.h"
#include "_main/CNormalProcess.h"

struct CAutoSaveAgentTest : public ::testing::Test
{
	static inline std::shared_ptr<CProcess> process = nullptr;

    static void SetUpTestSuite()
    {
		EXPECT_FALSE(CProcess::getInstance());

		// エディタープロセスを生成する
		process = CProcessFactory().CreateInstance(LR"(-PROF="")");

		// コントロールプロセスを起動する
		if (const auto profileName = process->GetCCommandLine().GetProfileName();
			!process->StartControlProcess(profileName))
		{
			if (const auto hEvent = process->OpenInitEvent(profileName))
			{
				// イベントハンドルをスマートポインタに入れる
				handleHolder eventHolder(hEvent, handle_closer());

				// イベントを待つ
				const auto waitResult = WaitForSingleObject(hEvent, 10 * 1000);
				if (WAIT_FAILED == waitResult)
				{
					throw message_error( L"waitProcess has failed." );
				}
			}
		}

		// エディタープロセスを初期化する
		process->InitProcess();
	}

    static void TearDownTestSuite() {
		process->TerminateControlProcess();

        process.reset();

		EXPECT_FALSE(CProcess::getInstance());
	}
};

TEST_F(CAutoSaveAgentTest, CheckAutoSave)
{
	auto pAutoSaveAgent = CEditDoc::getInstance()->GetAutoSaveAgent();
	EXPECT_TRUE(pAutoSaveAgent);

	auto& sBackup = GetDllShareData().m_Common.m_sBackup;
	sBackup.SetAutoBackupInterval(1);
	sBackup.EnableAutoBackup(true);
	pAutoSaveAgent->ReloadAutoSaveParam();

	Sleep(60 * 1000 + 100);

	pAutoSaveAgent->CheckAutoSave();
}
