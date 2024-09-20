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
#pragma once

#include <gmock/gmock.h>

#include "_main/CProcess.h"

#include "config/system_constants.h"

#define ASSERT_THROW_MESSAGE(statement, expected_exception, expected_message) \
	try { \
		statement; \
		FAIL() << "Expected " << #expected_exception << "."; \
	} \
	catch (const expected_exception& e) { \
		EXPECT_STREQ(expected_message, e.message()); \
	} \
	(void)0


using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::StrEq;

HANDLE DefaultOpenEventW(DWORD, bool, std::wstring_view name);
HANDLE DefaultOpenProcess(DWORD, BOOL, DWORD);
HANDLE DefaultCreateMutexW(const SECURITY_ATTRIBUTES*, bool, std::wstring_view name);
HANDLE DefaultCreateEventW(const SECURITY_ATTRIBUTES*, bool, bool, std::wstring_view name);
bool CreateDummyProcessW(
	std::wstring_view,
	const std::wstring&,
	LPCWSTR,
	const STARTUPINFOW*,
	LPPROCESS_INFORMATION lpProcessInformation
);

template<typename Base, std::enable_if_t<std::is_base_of_v<CProcess, Base>, int> = 0>
struct TMockProcess : public Base
{
	TMockProcess()
		: Base(nullptr, std::make_unique<CCommandLine>())
	{
		GetCCommandLine().SetProfileName(L"");
	}

	using Base::InitializeProcess;
	using Base::SetMainWindow;

	MOCK_CONST_METHOD4(CreateEventW, HANDLE(
		_In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
		bool bManualReset,
		bool bInitialState,
		std::wstring_view name
	));

	MOCK_CONST_METHOD3(CreateMutexW, HANDLE(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		bool bInitialOwner,
		std::wstring_view name));

	MOCK_CONST_METHOD5(CreateProcessW, bool(
		std::wstring_view exePath,
		std::wstring& commandLine,
		_In_opt_ LPCWSTR lpCurrentDirectory,
		_In_ LPSTARTUPINFOW lpStartupInfo,
		_Out_ LPPROCESS_INFORMATION lpProcessInformation
	));

	MOCK_CONST_METHOD2(FindWindowW, HWND(
		_In_opt_ LPCWSTR lpClassName,
		_In_opt_ LPCWSTR lpWindowName
	));

	MOCK_CONST_METHOD2(GetExitCodeProcess, bool(
		_In_ HANDLE hProcess,
		_Out_ LPDWORD lpExitCode
	));

	MOCK_CONST_METHOD0(GetLastError, DWORD());

	MOCK_CONST_METHOD2(GetWindowThreadProcessId, DWORD(
		_In_ HWND hWnd,
		_Out_opt_ LPDWORD lpdwProcessId
	));

	MOCK_CONST_METHOD5(MsgWaitForMultipleObjects, DWORD(
		_In_ DWORD nCount,
		_In_reads_opt_(nCount) CONST HANDLE* pHandles,
		_In_ BOOL fWaitAll,
		_In_ DWORD dwMilliseconds,
		_In_ DWORD dwWakeMask
	));

	MOCK_CONST_METHOD3(OpenEventW, HANDLE(
		_In_ DWORD dwDesiredAccess,
		bool bInheritHandle,
		std::wstring_view name
	));

	MOCK_CONST_METHOD3(OpenProcess, HANDLE(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ DWORD dwProcessId
	));

	MOCK_CONST_METHOD4(SendMessageW, LRESULT(
		_In_ HWND hWnd,
		_In_ UINT Msg,
		_Pre_maybenull_ _Post_valid_ WPARAM wParam,
		_Pre_maybenull_ _Post_valid_ LPARAM lParam
	));

	MOCK_CONST_METHOD1(SetEvent, bool(
		_In_ HANDLE hEvent
	));

	MOCK_CONST_METHOD2(WaitForSingleObject, DWORD(
		_In_ HANDLE hHandle,
		_In_ DWORD dwMilliseconds
	));

	MOCK_METHOD0(InitShareData, bool());

	bool OriginalInitShareData()
	{
		return Base::InitShareData();
	}

	MOCK_METHOD0(InitProcess, void());

	void OriginalInitProcess()
	{
		Base::InitProcess();
	}

	MOCK_CONST_METHOD0(GetMainWnd, CMainWindow*());

	CMainWindow* OriginalGetMainWnd() const
	{
		return Base::GetMainWnd();
	}
};

/*!
 * @brief プロセスの異常系テスト
 */
template<typename Base, std::enable_if_t<std::is_base_of_v<CProcess, Base>, int> = 0>
struct TProcessTest : public ::testing::Test
{
	using MockedProcess = TMockProcess<Base>;

	static inline std::unique_ptr<MockedProcess> process = nullptr;

    static void SetUpTestSuite()
    {
		ASSERT_FALSE(CProcess::getInstance());

		EXPECT_FALSE(::OpenEventW(SYNCHRONIZE, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED));
	}

    static void TearDownTestSuite() {
		ASSERT_FALSE(CProcess::getInstance());
	}

	void SetUp() {
        process = std::make_unique<MockedProcess>();
	}

	void TearDown() {
		process.reset();

		EXPECT_FALSE(::OpenEventW(SYNCHRONIZE, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED));
	}
};
