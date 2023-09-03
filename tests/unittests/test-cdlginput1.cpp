/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include <gmock/gmock.h>

#include "dlg/CDlgInput1.h"

#include "apiwrap/StdControl.h"

#include "tests1_rc.h"
#include "sakura_rc.h"

#include "TAutoCloseDialog.hpp"
#include "CSelectLang.h"

/*
 * ダイアログクラステンプレートをテストするためのクラス
 
 * 自動テストで実行できるように、初期表示後、勝手に閉じる仕様。
 */
class CDlgInput1ForTest : public TAutoCloseDialog<CDlgInput1, IDC_EDIT_INPUT1>
{
public:
	explicit CDlgInput1ForTest(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>()) noexcept
		: TAutoCloseDialog(IDD_INPUT1, std::move(User32Dll_))
	{
	}
	~CDlgInput1ForTest() override = default;

	using CDlgInput1::GetHelpIdTable;
};

using ::testing::Return;

/*!
 * モーダルダイアログ表示、正常系テスト
 */
TEST(CDlgInput1, SimpleDoModal)
{
	auto buffer = std::wstring(L"test");
	buffer.resize(256);

	CDlgInput1ForTest dlg;
	HINSTANCE hInstance  = nullptr;
	const auto hWndParent = static_cast<HWND>(nullptr);
	EXPECT_EQ(IDOK, dlg.DoModal(hInstance, hWndParent, L"title", L"message", static_cast<int>(buffer.length()), buffer.data()));
	EXPECT_STREQ(L"test", buffer.c_str());
}

/*!
 * 入力テキストがトリムされる確認
 */
TEST(CDlgInput1, TrimTest)
{
	auto buffer = std::wstring(L"test");
	buffer.resize(256);

	CDlgInput1ForTest dlg;
	HINSTANCE hInstance  = nullptr;
	const auto hWndParent = static_cast<HWND>(nullptr);
	EXPECT_EQ(IDOK, dlg.DoModal(hInstance, hWndParent, L"title", L"message", 2, buffer.data()));
	EXPECT_STREQ(L"te", buffer.c_str());
}

TEST(CDlgInput1, GetHelpIdTable)
{
	CDlgInput1ForTest dlg;
	EXPECT_TRUE(dlg.GetHelpIdTable());
}
