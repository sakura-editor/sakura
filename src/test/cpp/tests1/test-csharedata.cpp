/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "env/ShareDataTestSuite.hpp"

#include "dlg/CDialog.h"	//仮置き、本来は不要。

namespace env {

struct CShareDataTest : public ::testing::Test, public env::ShareDataTestSuite {
	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite()
	{
		// CanBeMoveリージョンをテストケースに分割する。（すぐ対応できないのでコメント残し）

		// 共有データがなくてもエラーにならない
		EXPECT_THAT(GetDllShareDataPtr(), IsNull());

		// 共有データがないのでエラー
		EXPECT_ANY_THROW(GetDllShareData());

#pragma region CanBeMove
		// 共有データがないのでエラー
		EXPECT_ANY_THROW(CDialog(false, true));	//2つ目の引数がtrueなら共有データ必須

		// 共有データがなくてもエラーにならない
		EXPECT_NO_THROW([] { CDialog(false, false).GetHwnd(); });

#pragma endregion CanBeMove

		// 共有データを生成して初期化する
		SetUpShareData();

		// 共有データがあるので値を返す
		EXPECT_THAT(GetDllShareDataPtr(), pcShareData->GetDllShareDataPtr());

		// 共有データがあるのでエラーにならない
		EXPECT_NO_THROW([] { GetDllShareData(); });

#pragma region CanBeMove
		// 共有データがあるのでエラーにならない
		EXPECT_NO_THROW([] { CDialog(false, true).GetHwnd(); });

#pragma endregion CanBeMove
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite()
	{
		// 共有データを破棄する
		TearDownShareData();

		// 共有データがなくなってもエラーにならない
		EXPECT_THAT(GetDllShareDataPtr(), IsNull());

		// 共有データがなくなったのでエラー
		EXPECT_ANY_THROW(GetDllShareData());
	}
};

/*!
 * @brief CShareData初期化と言語切替のテスト
 */
TEST_F(CShareDataTest, initShareData001)
{
	// 言語切り替えのテストを実施する
	std::vector<std::wstring> values;
	pcShareData->ConvertLangValues(values, true);
	CSelectLang::ChangeLang(L"sakura_lang_en_US.dll");
	pcShareData->ConvertLangValues(values, false);
	pcShareData->RefreshString();

	// 言語を元に戻す
	CSelectLang::ChangeLang(L"");
}

} // namespace env
