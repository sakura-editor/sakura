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
#include "TMockMainWindow.hpp"

#include "CEditorProcessInitTest.hpp"

#include "_main/CNormalProcess.h"

#include "apiwrap/kernel/format_message.hpp"

#include "view/colors/CColorStrategy.h"
#include "view/figures/CFigureManager.h"

/*!
 * @brief エディタープロセスの異常系テスト
 */
using CEditorProcessTest = TProcessTest<CEditorProcess>;

using MockCEditorProcess = TMockProcess<CEditorProcess>;

TEST_F(CEditorProcessTest, getEditorProcess)
{
	EXPECT_TRUE(CProcess::getInstance());
	EXPECT_TRUE(getEditorProcess());

	process.reset();
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(getEditorProcess());
}

// 多重起動制御ミューテクスの作成失敗
TEST_F(CEditorProcessTest, InitializeProcess001)
{
    EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Return(nullptr));
    EXPECT_CALL(*process, GetLastError()).Times(0);
    EXPECT_CALL(*process, WaitForSingleObject(_, _)).Times(0);

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGNRMPROC1));
}

// 多重起動制御ミューテクスの待機でタイムアウト
TEST_F(CEditorProcessTest, InitializeProcess002)
{
	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));
    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_ALREADY_EXISTS));
    EXPECT_CALL(*process, WaitForSingleObject(_, _)).WillOnce(Return(WAIT_TIMEOUT));

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGNRMPROC2));
}

// コントロールプロセスの起動に失敗
TEST_F(CEditorProcessTest, InitializeProcess003)
{
	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));

	EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Return(nullptr));

    EXPECT_CALL(*process, InitProcess()).WillOnce(Invoke(&*process, &MockCEditorProcess::OriginalInitProcess));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	EXPECT_CALL(*process, CreateProcessW(_, _, _, _, _)).WillOnce(Return(false));

	EXPECT_CALL(*process, GetLastError())
		.WillOnce(Return(ERROR_SUCCESS))
		.WillOnce(Return(ERROR_FILE_NOT_FOUND));

	std::wstring message;
	format_message(ERROR_FILE_NOT_FOUND, [&message](LPCWSTR pszMsg) {
		const auto exePath = GetExeFileName();
		message = fmt::format(L"\'{}\'\nプロセスの起動に失敗しました。\n{}", exePath.wstring(), pszMsg);
	});

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, message.c_str());
}

// コントロールプロセスの起動がタイムアウト
TEST_F(CEditorProcessTest, InitializeProcess004)
{
	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));

    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));
    EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillRepeatedly(Return(nullptr));

    EXPECT_CALL(*process, InitProcess()).WillOnce(Invoke(&*process, &MockCEditorProcess::OriginalInitProcess));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	// プロセス起動は成功させる
	EXPECT_CALL(*process, CreateProcessW(_, _, _, _, _)).WillOnce(Invoke(CreateDummyProcessW));

	// 初期化完了イベントの取得を失敗させる

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGNRMPROC2));
}

// コントロールプロセスの起動がタイムアウト
TEST_F(CEditorProcessTest, InitializeProcess005)
{
	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));

    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));

    EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED)))
		.WillOnce(Return(nullptr)) // 起動前チェックの分
		.WillOnce(Invoke(DefaultOpenEventW));

    EXPECT_CALL(*process, InitProcess()).WillOnce(Invoke(&*process, &MockCEditorProcess::OriginalInitProcess));
    EXPECT_CALL(*process, InitShareData()).Times(0);

	// プロセス起動は成功させる
	EXPECT_CALL(*process, CreateProcessW(_, _, _, _, _)).WillOnce(Invoke(CreateDummyProcessW));

	// 起動完了待機を失敗させる
    EXPECT_CALL(*process, WaitForSingleObject(_, _)).WillOnce(Return(WAIT_TIMEOUT));

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGNRMPROC2));
}

// 共有メモリの初期化失敗
TEST_F(CEditorProcessTest, InitializeProcess006)
{
	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));

    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));

    EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED)))
		.WillOnce(Return(nullptr)) // 起動前チェックの分
		.WillOnce(Invoke(DefaultOpenEventW));

    EXPECT_CALL(*process, InitProcess()).WillOnce(Invoke(&*process, &MockCEditorProcess::OriginalInitProcess));
    EXPECT_CALL(*process, InitShareData()).WillOnce(Return(false));

	// プロセス起動は成功させる
	EXPECT_CALL(*process, CreateProcessW(_, _, _, _, _)).WillOnce(Invoke(CreateDummyProcessW));

    EXPECT_CALL(*process, WaitForSingleObject(_, _)).WillOnce(Return(WAIT_OBJECT_0));

	ASSERT_THROW_MESSAGE(process->InitializeProcess(), process_init_failed, LS(STR_ERR_DLGPROCESS1));
}

// メインウインドウの作成に失敗
TEST_F(CEditorProcessTest, InitializeProcess007)
{
	auto mainWindow = std::make_unique<MockCMainWindow>(L"test");
    EXPECT_CALL(*mainWindow, CreateMainWnd(_)).WillOnce(Return(nullptr));

	EXPECT_CALL(*process, CreateMutexW(_, true, StrEq(GSTR_MUTEX_SAKURA_INIT))).WillOnce(Invoke(DefaultCreateMutexW));

    EXPECT_CALL(*process, GetLastError()).WillOnce(Return(ERROR_SUCCESS));

	EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).Times(0);
    EXPECT_CALL(*process, WaitForSingleObject(_, _)).Times(0);

    EXPECT_CALL(*process, InitProcess()).WillOnce(Invoke(DoNothing));
    EXPECT_CALL(*process, InitShareData()).Times(0);

    EXPECT_CALL(*process, GetMainWnd()).WillOnce(Return(mainWindow.get()));

	EXPECT_FALSE(process->InitializeProcess());
}


// コントロールプロセスが既に起動中
TEST_F(CEditorProcessTest, StartControlProcess001)
{
    EXPECT_CALL(*process, OpenEventW(_, _, StrEq(GSTR_EVENT_SAKURA_CP_INITIALIZED))).WillOnce(Invoke(DefaultOpenEventW));
    EXPECT_CALL(*process, WaitForSingleObject(_, _)).WillOnce(Return(WAIT_OBJECT_0));

	EXPECT_TRUE(process->StartControlProcess(L""));
}

// コントロールプロセスのプロセスIDを取得できなかった
TEST_F(CEditorProcessTest, TerminateControlProcess001)
{
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(nullptr));

	ASSERT_THROW_MESSAGE(process->TerminateControlProcess(), message_error, L"hWndTray can't be retrived.");
}

// コントロールプロセスのプロセスIDを取得できなかった
TEST_F(CEditorProcessTest, TerminateControlProcess002)
{
	const auto  hWndTray = HWND(1234);
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(hWndTray));
    EXPECT_CALL(*process, GetWindowThreadProcessId(hWndTray, _)).WillOnce(Invoke([](const HWND, DWORD* p) { *p = 0UL; return 0UL; }));

	ASSERT_THROW_MESSAGE(process->TerminateControlProcess(), message_error, L"dwControlProcessId can't be retrived.");
}

// コントロールプロセスを開けなかった
TEST_F(CEditorProcessTest, TerminateControlProcess003)
{
	const auto  hWndTray = HWND(1234);
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(hWndTray));
    EXPECT_CALL(*process, GetWindowThreadProcessId(hWndTray, _)).WillOnce(Invoke([](const HWND, DWORD* p) { *p = 1UL; return 0UL; }));
    EXPECT_CALL(*process, OpenProcess(_, _, _)).WillOnce(Return(nullptr));

	ASSERT_THROW_MESSAGE(process->TerminateControlProcess(), message_error, L"hControlProcess can't be opened.");
}

// コントロールプロセスの待機に失敗
TEST_F(CEditorProcessTest, TerminateControlProcess004)
{
	const auto  hWndTray = HWND(1234);
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(hWndTray));
    EXPECT_CALL(*process, GetWindowThreadProcessId(hWndTray, _)).WillOnce(Invoke([](const HWND, DWORD* p) { *p = 1UL; return 0UL; }));
    EXPECT_CALL(*process, OpenProcess(_, _, _)).WillOnce(Invoke(DefaultOpenProcess));
    EXPECT_CALL(*process, SendMessageW(hWndTray, WM_CLOSE, 0, 0)).WillOnce(Return(0L));
    EXPECT_CALL(*process, MsgWaitForMultipleObjects(_, _, _, _, _)).WillOnce(Return(WAIT_FAILED));

	ASSERT_THROW_MESSAGE(process->TerminateControlProcess(), message_error, L"waitProcess has failed.");
}

// コントロールプロセス終了コードの取得に失敗
TEST_F(CEditorProcessTest, TerminateControlProcess005)
{
	const auto  hWndTray = HWND(1234);
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(hWndTray));
    EXPECT_CALL(*process, GetWindowThreadProcessId(hWndTray, _)).WillOnce(Invoke([](const HWND, DWORD* p) { *p = 1UL; return 0UL; }));
    EXPECT_CALL(*process, OpenProcess(_, _, _)).WillOnce(Invoke(DefaultOpenProcess));
    EXPECT_CALL(*process, SendMessageW(hWndTray, WM_CLOSE, 0, 0)).WillOnce(Return(0L));
    EXPECT_CALL(*process, MsgWaitForMultipleObjects(_, _, _, _, _))
		.WillOnce(Return(WAIT_TIMEOUT))
		.WillOnce(Return(WAIT_TIMEOUT))
		.WillOnce(Return(WAIT_OBJECT_0));
    EXPECT_CALL(*process, GetExitCodeProcess(_, _)).WillOnce(Invoke([](const HANDLE, const DWORD* p) { return false; }));

	ASSERT_THROW_MESSAGE(process->TerminateControlProcess(), message_error, L"GetExitCodeProcess has failed.");
}

// コントロールプロセス終了コードの取得に成功
TEST_F(CEditorProcessTest, TerminateControlProcess006)
{
	const auto  hWndTray = HWND(1234);
    EXPECT_CALL(*process, FindWindowW(_, _)).WillOnce(Return(hWndTray));
    EXPECT_CALL(*process, GetWindowThreadProcessId(hWndTray, _)).WillOnce(Invoke([](const HWND, DWORD* p) { *p = 1UL; return 0UL; }));
    EXPECT_CALL(*process, OpenProcess(_, _, _)).WillOnce(Invoke(DefaultOpenProcess));
    EXPECT_CALL(*process, SendMessageW(hWndTray, WM_CLOSE, 0, 0)).WillOnce(Return(0L));
    EXPECT_CALL(*process, MsgWaitForMultipleObjects(_, _, _, _, _))
		.WillOnce(Return(WAIT_OBJECT_0))
		.WillOnce(Return(WAIT_OBJECT_0));
    EXPECT_CALL(*process, GetExitCodeProcess(_, _))
		.WillOnce(Invoke([](const HANDLE, DWORD* p) { *p = STILL_ACTIVE; return true; }))
		.WillOnce(Invoke([](const HANDLE, DWORD* p) { *p = 0; return true; }));

	process->TerminateControlProcess();
}

TEST(CProcess, CCodeChecker)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CCodeChecker::getInstance());
}

TEST(CProcess, CDiffManager)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CDiffManager::getInstance());
}

TEST(CProcess, CEditWnd)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CEditWnd::getInstance());
}

TEST(CProcess, CMacroFactory)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CMacroFactory::getInstance());
}

TEST(CProcess, CMigemo)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CMigemo::getInstance());
}

TEST(CProcess, CFigureManager)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CFigureManager::getInstance());
}

TEST(CProcess, CColorStrategyPool)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CColorStrategyPool::getInstance());
}

TEST(CProcess, CAppMode)
{
	EXPECT_FALSE(CProcess::getInstance());
	EXPECT_FALSE(CAppMode::getInstance());
}

TEST_F(CEditorProcessInitTest, CCommandLine)
{
	EXPECT_TRUE(CCommandLine::getInstance());
}

TEST_F(CEditorProcessInitTest, CCodeChecker)
{
	EXPECT_TRUE(CCodeChecker::getInstance());
}

TEST_F(CEditorProcessInitTest, CDiffManager)
{
	EXPECT_TRUE(CDiffManager::getInstance());
}

TEST_F(CEditorProcessInitTest, CEditWnd)
{
	EXPECT_TRUE(CEditWnd::getInstance());
}

TEST_F(CEditorProcessInitTest, CMacroFactory)
{
	EXPECT_TRUE(CMacroFactory::getInstance());
}

TEST_F(CEditorProcessInitTest, CMigemo)
{
	EXPECT_TRUE(CMigemo::getInstance());
}

TEST_F(CEditorProcessInitTest, OnCreate)
{
	EXPECT_FALSE(process->GetMainWnd()->OnCreate(nullptr, nullptr));

	auto pcView = &GetEditWnd().GetActiveView();
	EXPECT_FALSE(pcView->OnCreate(nullptr, nullptr));

	const auto hWnd = HWND(0x1234);
	pcView->DispatchEvent(hWnd, WM_COMMAND, 0, 0);
}

TEST_F(CEditorProcessInitTest, GetGroupId)
{
	EXPECT_EQ(1, GetGroupId(-1, true));
}

HANDLE DefaultOpenEventW(DWORD, bool, std::wstring_view name)
{
	return CreateEventW(nullptr, true, false, name.data());
}

HANDLE DefaultOpenProcess(DWORD, BOOL, DWORD)
{
	const auto processId = GetProcessIdOfThread(GetCurrentThread());
	return OpenProcess(SYNCHRONIZE, FALSE, processId);
}

HANDLE DefaultCreateMutexW(const SECURITY_ATTRIBUTES*, bool, std::wstring_view name)
{
	return CreateMutexW(nullptr, true, name.data());
}

HANDLE DefaultCreateEventW(const SECURITY_ATTRIBUTES*, bool, bool, std::wstring_view name)
{
	return CreateEventW(nullptr, true, false, name.data());
}

bool CreateDummyProcessW(
	std::wstring_view,
	const std::wstring&,
	LPCWSTR,
	const STARTUPINFOW*,
	LPPROCESS_INFORMATION lpProcessInformation
)
{
	auto& pi = *lpProcessInformation;
	const auto processId = GetProcessIdOfThread(GetCurrentThread());
	pi.hProcess = OpenProcess(SYNCHRONIZE, FALSE, processId);
	const auto threadId = GetCurrentThreadId();
	pi.hThread = OpenThread(SYNCHRONIZE, FALSE, threadId);
	return true;
}
