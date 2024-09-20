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

#include "TMockProcess.hpp"

#include "eval_outputs.hpp"

#include "_main/CProcessFactory.h"
#include "_main/CControlProcess.h"

#include "config/system_constants.h"

#include "_main/CNormalProcess.h"

#include "apiwrap/window/COriginalWnd.hpp"

/*!
 * @brief コントロールプロセスの異常系テスト
 */
using CControlProcessTest = TProcessTest<CControlProcess>;

TEST_F(CControlProcessTest, getEditorProcess)
{
	EXPECT_TRUE(CProcess::getInstance());
	EXPECT_FALSE(getEditorProcess());
}

// 多重起動制御ミューテックスの作成失敗
TEST_F(CControlProcessTest, InitializeProcess001)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Return(nullptr));
    EXPECT_CALL(*process, GetLastError()).Times(0);
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).Times(0);
    EXPECT_CALL(*process, InitShareData()).Times(0);

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_CTRLMTX1));
}

// 多重起動制御ミューテックスの作成失敗(すでに存在している)
TEST_F(CControlProcessTest, InitializeProcess002)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).Times(0);
    EXPECT_CALL(*process, InitShareData()).Times(0);

	EXPECT_FALSE(process->InitializeProcess());
}

// 初期化完了イベントの作成失敗
TEST_F(CControlProcessTest, InitializeProcess011)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Return(nullptr));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_CTRLMTX2));
}

// 初期化完了イベントがすでに存在している
TEST_F(CControlProcessTest, InitializeProcess012)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_ALREADY_EXISTS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	EXPECT_FALSE(process->InitializeProcess());
}

// 共有メモリの初期化失敗
TEST_F(CControlProcessTest, InitializeProcess021)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).WillOnce(Return(false));

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGPROCESS1));
}

// 初期化処理で発生したエラーメッセージを表示する
TEST_F(CControlProcessTest, Run001)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_CP))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError())
        .WillOnce(Return(ERROR_SUCCESS))
        .WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, CreateEventW(_, true, false, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultCreateEventW));
    EXPECT_CALL(*process, InitShareData()).WillOnce(Return(false));

	EXPECT_ERROUT(process->Run(), LS(STR_ERR_DLGPROCESS1));
}

TEST(CProcess, CUxTheme)
{
	EXPECT_FALSE(CUxTheme::getInstance());
}

TEST(CProcess, CAppNodeManager)
{
	EXPECT_FALSE(CAppNodeManager::getInstance());
}

TEST(CProcess, CFileNameManager)
{
	EXPECT_FALSE(CFileNameManager::getInstance());
}

TEST(CProcess, CPluginManager)
{
	EXPECT_FALSE(CPluginManager::getInstance());
}

TEST(CProcess, CJackManager)
{
	EXPECT_FALSE(CJackManager::getInstance());
}

namespace apiwrap::window
{

// クラス名が空
TEST(CWndClass, RegisterWndClass001)
{
	CWndClass wc(L""sv);

	EXPECT_FALSE(wc.RegisterWndClass());
}

// 正常+二度呼び
TEST(CWndClass, RegisterWndClass002)
{
	constexpr auto& className = L"test";

	const auto hInstance = GetModuleHandleW(nullptr);

	CWndClass wc(className, hInstance, &COriginalWnd::WndProc);
	EXPECT_STREQ(className, wc.GetOriginalClassName().data());
	EXPECT_STREQ(className, wc.GetClassNameW());

	EXPECT_TRUE(wc.RegisterWndClass());
	EXPECT_STREQ(className, wc.GetOriginalClassName().data());
	EXPECT_NE(className, wc.GetClassNameW());

	EXPECT_TRUE(wc.RegisterWndClass());

	UnregisterClassW(className, hInstance);
}

struct STestWnd : public apiwrap::window::COriginalWnd
{
    using COriginalWnd::WndProc;
    using COriginalWnd::OnCreate;

	STestWnd() : COriginalWnd(L"test") {}
};

struct COriginalWndTest : public ::testing::Test
{
	std::unique_ptr<COriginalWnd> pWnd = nullptr;

	void SetUp() override {
		pWnd = std::make_unique<STestWnd>();
	}
};

TEST_F(COriginalWndTest, WndProc001)
{
	EXPECT_FALSE(STestWnd::WndProc(nullptr, WM_NULL, 0, 0));
}

TEST_F(COriginalWndTest, OnCreate001)
{
	EXPECT_FALSE(pWnd->OnCreate(nullptr, nullptr));
}

TEST_F(COriginalWndTest, OnCreate002)
{
    const auto hWnd = HWND(0x1234);
	EXPECT_FALSE(pWnd->OnCreate(hWnd, nullptr));
}

TEST_F(COriginalWndTest, OnCreate003)
{
    const auto hWnd = HWND(0x1234);
    const auto createStruct = std::make_unique<CREATESTRUCT>();
	EXPECT_TRUE(pWnd->OnCreate(hWnd, &*createStruct));
}

TEST_F(COriginalWndTest, CreateWnd001)
{
	EXPECT_FALSE(pWnd->CreateWnd(HWND(nullptr)));
}

} // end of namespace apiwrap::window

using ConvertHotKeyModsTestParamType = std::tuple<WORD, WORD>;
class ConvertHotKeyModsTest : public ::testing::TestWithParam<ConvertHotKeyModsTestParamType> {};

TEST_P(ConvertHotKeyModsTest, convert)
{
	const auto arg      = std::get<0>(GetParam());
	const auto expected = std::get<1>(GetParam());
	EXPECT_EQ(expected, convertHotKeyMods(arg));
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(Parameterized
	, ConvertHotKeyModsTest
	, ::testing::Values(
		ConvertHotKeyModsTestParamType{ HOTKEYF_CONTROL | HOTKEYF_SHIFT | HOTKEYF_ALT,   MOD_CONTROL | MOD_SHIFT | MOD_ALT },
		ConvertHotKeyModsTestParamType{ HOTKEYF_CONTROL | HOTKEYF_SHIFT,                 MOD_CONTROL | MOD_SHIFT           },
		ConvertHotKeyModsTestParamType{ HOTKEYF_CONTROL                 | HOTKEYF_ALT,   MOD_CONTROL             | MOD_ALT },
		ConvertHotKeyModsTestParamType{ HOTKEYF_CONTROL,                                 MOD_CONTROL                       },
		ConvertHotKeyModsTestParamType{                   HOTKEYF_SHIFT | HOTKEYF_ALT,                 MOD_SHIFT | MOD_ALT },
		ConvertHotKeyModsTestParamType{                   HOTKEYF_SHIFT,                               MOD_SHIFT           },
		ConvertHotKeyModsTestParamType{                                   HOTKEYF_ALT,                             MOD_ALT },
		ConvertHotKeyModsTestParamType{ 0,                                               0                                 }
	)
);
// clang-format on
