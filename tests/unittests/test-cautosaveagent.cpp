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

#include "CEditorProcessInitTest.hpp"

#include "_main/CNormalProcess.h"

struct CAutoSaveAgentTest : public CEditorProcessInitTest {};

TEST_F(CAutoSaveAgentTest, CheckAutoSave)
{
	auto pcDoc = CEditDoc::getInstance();

	auto pAutoSaveAgent = pcDoc->GetAutoSaveAgent();
	EXPECT_TRUE(pAutoSaveAgent);

	auto& sBackup = GetDllShareData().m_Common.m_sBackup;
	sBackup.SetAutoBackupInterval(1);
	sBackup.EnableAutoBackup(true);
	pAutoSaveAgent->ReloadAutoSaveParam();

	Sleep(60 * 1000 + 100);

	pcDoc->m_cDocEditor.SetModified(true, false);

	pAutoSaveAgent->CheckAutoSave();

	pcDoc->m_cDocEditor.SetModified(false, false);
}
