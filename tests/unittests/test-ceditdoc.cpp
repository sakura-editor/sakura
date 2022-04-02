/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

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
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>

#include "CEditorProcessForTest.hpp"

/*!
 * CEditDocのためのフィクスチャクラス
 */
using CEditDocTest = TEditorProcessTest<CEditDoc>;

/*!
 * このウィンドウで新しいファイルを開けるか
 */
TEST_F(CEditDocTest, IsAcceptLoad)
{
	if (auto pcEditDoc = CEditDoc::getInstance()) {
		// 初期状態は再利用可能
		EXPECT_TRUE(pcEditDoc->IsAcceptLoad());

		// デバッグモニタモードは再利用不可
		CAppMode::getInstance()->SetDebugModeON();
		EXPECT_FALSE(pcEditDoc->IsAcceptLoad());

		// デバッグモニタモードを解除
		CAppMode::getInstance()->SetDebugModeOFF();
		EXPECT_TRUE(pcEditDoc->IsAcceptLoad());

		// GREPモードは再利用不可
		CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = true;
		EXPECT_FALSE(pcEditDoc->IsAcceptLoad());

		// GREPモードを解除
		CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode = false;
		EXPECT_TRUE(pcEditDoc->IsAcceptLoad());
	}
}

/*!
 * 編集可能かどうか
 */
TEST_F(CEditDocTest, IsEditable)
{
	if (auto pcEditDoc = CEditDoc::getInstance()) {
		// 初期状態は編集可能
		EXPECT_TRUE(pcEditDoc->IsEditable());

		// ビューモードは編集不可
		CAppMode::getInstance()->SetViewMode(true);
		EXPECT_FALSE(pcEditDoc->IsEditable());

		// ビューモードを解除
		CAppMode::getInstance()->SetViewMode(false);
		EXPECT_TRUE(pcEditDoc->IsEditable());
	}
}

/*!
 * 背景画像を読み込み
 */
TEST_F(CEditDocTest, SetBackgroundImage)
{
	if (auto pcEditDoc = CEditDoc::getInstance()) {
		// パスが空文字の状態で呼ぶ
		pcEditDoc->SetBackgroundImage();

		// パスを設定して呼ぶ
		auto& sTypeConfig = pcEditDoc->m_cDocType.GetDocumentAttributeWrite();
		sTypeConfig.m_szBackImgPath.Assign(LR"(..\..\resource\my_icons.bmp)");
		pcEditDoc->SetBackgroundImage();

		// とりあえずもう一回呼ぶ
		pcEditDoc->SetBackgroundImage();

		// 後処理が面倒なのでクリアしておく
		sTypeConfig.m_szBackImgPath.Assign(L"");
	}
}

/*!
 * セーブ情報を取得
 */
TEST_F(CEditDocTest, GetSaveInfo)
{
	SSaveInfo sSaveInfo;

	if (auto pcEditDoc = CEditDoc::getInstance()) {
		// セーブ情報を取得
		pcEditDoc->GetSaveInfo(&sSaveInfo);

		EXPECT_STREQ(L"", sSaveInfo.cFilePath.c_str());
		EXPECT_EQ(CODE_UTF8, sSaveInfo.eCharCode);
		EXPECT_FALSE(sSaveInfo.bBomExist);
		EXPECT_FALSE(sSaveInfo.bChgCodeSet);
		EXPECT_EQ(EEolType::cr_and_lf, sSaveInfo.cEol.GetType());
	}
}
