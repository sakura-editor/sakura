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
#include "dlg/CDlgPluginOption.h"

#include "doc/CEditDoc.h"
#include "plugin/CWSHPlugin.h"

#include <CommCtrl.h>
#include "prop/CPropCommon.h"

#include "MockShareDataAccessor.hpp"

/*!
 * プラグイン設定ダイアログ、構築するだけ。
 */
TEST(CDlgPluginOption, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	EXPECT_NO_THROW({ CDlgPluginOption dlg(std::move(pShareDataAccessor)); });
}

/*!
 * 表示テスト
 */
TEST(CDlgPluginOption, SimpleShowDialog)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	CEditDoc         doc(pShareDataAccessor);

	auto plugin = std::make_shared<CWSHPlugin>(L"");
	auto propPlugin = std::make_shared<CPropPlugin>(pShareDataAccessor);

	CDlgPluginOption dlg(std::move(pShareDataAccessor));
	dlg.SetPluginForTest(plugin, propPlugin);

	const auto hWndParent = static_cast<HWND>(nullptr);
	auto       hDlg       = dlg.Show(hWndParent, SW_SHOW, 0L);
	EXPECT_NE(nullptr, hDlg);
	dlg.CloseDialog(0);
}
