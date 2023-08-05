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
#include "typeprop/CDlgSameColor.h"

#include <gtest/gtest.h>

/*!
 * 文字色／背景色統一ダイアログ、構築するだけ。
 */
TEST(CDlgSameColor, Construct)
{
	EXPECT_NO_THROW({ CDlgSameColor dlg; });
}

/*!
 * 表示テスト
 */
TEST(CDlgSameColor, SimpleShowDialog)
{
	const auto cr = RGB(255, 0, 0);
	STypeConfig typeConfig = {};
	CDlgSameColor dlg;
	dlg.SetNanikaForTest(IDC_BUTTON_SAMETEXTCOLOR, &typeConfig, cr);

	const auto hWndParent = static_cast<HWND>(nullptr);
	const auto hDlg       = dlg.Show(hWndParent, SW_SHOW, 0L);
	EXPECT_NE(nullptr, hDlg);
	dlg.CloseDialog(0);
}
