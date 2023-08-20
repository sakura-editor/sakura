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
#include <vector>
#include "_main/CControlTray.h"

#include "_main/CCommandLine.h"
#include "_main/CControlProcess.h"
#include "doc/CEditDoc.h"

#include "MockShareDataAccessor.hpp"
#include "MockUser32Dll.hpp"

#include <array>

using ::testing::_;
using ::testing::Return;

/*!
 * KeyBindのダミーを作成します。
 *
 * キー割り当て「なし」だとCControlTray::CreateAccelTblが失敗する対策。
 */
void MakeDummyKeyBind(CommonSetting_KeyBind& sKeyBind)
{
	const KEYDATA keyData = {
		// m_nKeyCode
		VKEX_DBL_CLICK,

		// m_szKeyName[30]
		L"テスト用ダミー",

		// m_nFuncCodeArr[8]
		{
			F_FILENEW,
			F_FILEOPEN,
			F_FILESAVE,
			F_FILESAVEAS_DIALOG,
			F_FILESAVEAS,
			F_FILECLOSE,
			F_FILECLOSE_OPEN,
			F_FILEOPEN_DROPDOWN,
		}
	};

	sKeyBind.m_nKeyNameArrNum = 1;
	memcpy_s(sKeyBind.m_pKeyNameArr, sizeof(sKeyBind.m_pKeyNameArr), &keyData, sizeof(KEYDATA));
}

/*!
 * 構築するだけ。
 */
TEST(CControlTray, Construct)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	MakeDummyKeyBind(pDllShareData->m_Common.m_sKeyBind);
	EXPECT_NO_THROW({ CControlTray tray(std::move(pShareDataAccessor)); });
}

TEST(CControlTray, Create_fail)
{
	// コマンドラインのインスタンスを用意する
	CCommandLine cCommandLine;
	auto pCommandLine = &cCommandLine;
	pCommandLine->ParseCommandLine(LR"(-PROF="profile1")", false);

	// プロセスのインスタンスを用意する
	CControlProcess dummy(nullptr, LR"(-PROF="profile1")");

	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	MakeDummyKeyBind(pDllShareData->m_Common.m_sKeyBind);

	auto pUser32Dll = std::make_shared<MockUser32Dll>();
	EXPECT_CALL(*pUser32Dll, GetClassInfoExW(_, _, _)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, RegisterClassExW(_)).WillOnce(Return(FALSE));
	EXPECT_CALL(*pUser32Dll, CreateWindowExW(_, _, _, _, _, _, _, _, _, _, _, _)).Times(0);

	const auto hInstance = static_cast<HINSTANCE>(nullptr);

	CControlTray tray(std::move(pShareDataAccessor), std::move(pUser32Dll));
	EXPECT_FALSE(tray.Create(hInstance));
}

class CControlTrayForOnCreateTest : public CControlTray
{
public:
	explicit CControlTrayForOnCreateTest(std::shared_ptr<ShareDataAccessor> ShareDataAccessor_ = std::make_shared<ShareDataAccessor>())
		: CControlTray(std::move(ShareDataAccessor_))
	{
	}

	using CControlTray::OnCreate;
};

TEST(CControlTray, OnCreate)
{
	auto [pDllShareData, pShareDataAccessor] = MakeDummyShareData();
	MakeDummyKeyBind(pDllShareData->m_Common.m_sKeyBind);
	CControlTrayForOnCreateTest wnd(std::move(pShareDataAccessor));

	EXPECT_FALSE(wnd.OnCreate(nullptr, nullptr));

	const auto hWnd = (HWND)105;
	EXPECT_FALSE(wnd.OnCreate(hWnd, nullptr));

	//戻り値trueのパターンは起動テストでカバーする
}
