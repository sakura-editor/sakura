/*! @file */
/*
	Copyright (C) 2022, Sakura Editor Organization

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
#include "util/MessageBoxF.h"

#include "testing/GuiAwareTestSuite.hpp"

namespace message_box {

class MessageBoxTest : public testing::TGuiAware<::testing::Test> {
private:
	using Base = testing::TGuiAware<::testing::Test>;

public:
	static inline HWND hWnd = nullptr;

	/*!
	 * テストスイートの開始前に1回だけ呼ばれる関数
	 */
	static void SetUpTestSuite() {
		// OLEを初期化する
		EXPECT_TRUE(Base::SetUpGuiTestSuite());
	}

	/*!
	 * テストスイートの終了後に1回だけ呼ばれる関数
	 */
	static void TearDownTestSuite() {
		// OLEの初期化を解除する
		Base::TearDownGuiTestSuite();
	}
};

/*!
	MessageBoxFのテスト 
 */
TEST_F(MessageBoxTest, MessageBoxF001)
{
	EXPECT_MSGBOX(MessageBoxF(hWnd, MB_OK, L"caption", L"%d行をマージしました。", 2), L"caption", L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ErrorMessage001)
{
	//エラー：赤丸に「×」[OK]
	EXPECT_MSGBOX(ErrorMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ErrorMessage002)
{
	//エラー：赤丸に「×」[OK]
	EXPECT_MSGBOX(TopErrorMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, WarningMessage001)
{
	//警告：三角に「！」[OK]
	EXPECT_MSGBOX(WarningMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, WarningMessage002)
{
	//警告：三角に「！」[OK]
	EXPECT_MSGBOX(TopWarningMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, InfoMessage001)
{
	//情報：青丸に「i」[OK]
	EXPECT_MSGBOX(InfoMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, InfoMessage002)
{
	//情報：青丸に「i」[OK]
	EXPECT_MSGBOX(TopInfoMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ConfirmMessage001)
{
	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_MSGBOX(ConfirmMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, ConfirmMessage002)
{
	//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
	EXPECT_MSGBOX(TopConfirmMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, Select3Message001)
{
	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_MSGBOX(Select3Message(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, Select3Message002)
{
	//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
	EXPECT_MSGBOX(TopSelect3Message(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, OkMessage001)
{
	//その他メッセージ表示用ボックス[OK]
	EXPECT_MSGBOX(OkMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, OkMessage002)
{
	//その他メッセージ表示用ボックス[OK]
	EXPECT_MSGBOX(TopOkMessage(hWnd, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, CustomMessage001)
{
	//タイプ指定メッセージ表示用ボックス
	EXPECT_MSGBOX(CustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, CustomMessage002)
{
	//タイプ指定メッセージ表示用ボックス
	EXPECT_MSGBOX(TopCustomMessage(hWnd, MB_OK, L"%d行をマージしました。", 2), GSTR_APPNAME, L"2行をマージしました。");
}

/*!
	独自仕様メッセージボックス関数群のテスト
 */
TEST_F(MessageBoxTest, PleaseReportToAuthor001)
{
	//作者に教えて欲しいエラー
	EXPECT_MSGBOX(PleaseReportToAuthor(hWnd, L"%d行をマージしました。", 2), LS(STR_ERR_DLGDOCLMN1), L"2行をマージしました。");
}

} // namespace message_box
