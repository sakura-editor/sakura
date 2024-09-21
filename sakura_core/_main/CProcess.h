/*!	@file
	@brief プロセス基底クラスヘッダーファイル

	@author aroka
	@date	2002/01/08 作成
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#define SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_
#pragma once

#include "_main/CCommandLine.h"
#include "_main/CMainWindow.hpp"

#include "env/CShareData.h"

#include "util/design_template.h"

#include "apiwrap/kernel/handle_closer.hpp"

#include "_main/process_init_failed.hpp"

#include "CSelectLang.h"
#include "sakura_rc.h"
#include "String_define.h"

#include "env/CAppNodeManager.h"
#include "env/CFileNameManager.h"
#include "plugin/CJackManager.h"
#include "plugin/CPluginManager.h"
#include "extmodule/CUxTheme.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief プロセス基底クラス
 */
class CProcess : public TSingleInstance<CProcess> {

	using Me = CProcess;
	using CCommandLineHolder = std::unique_ptr<CCommandLine>;
	using CMainWindowHolder = std::unique_ptr<CMainWindow>;
	using CAppNodeManagerHolder = std::unique_ptr<CAppNodeManager>;
	using CFileNameManagerHolder = std::unique_ptr<CFileNameManager>;
	using CPluginManagerHolder = std::unique_ptr<CPluginManager>;
	using CJackManagerHolder = std::unique_ptr<CJackManager>;
	using CUxThemeHolder = std::unique_ptr<CUxTheme>;

public:
	explicit CProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine, int nCmdShow) noexcept;
	~CProcess() override = default;

	int Run() noexcept;

	[[nodiscard]] HANDLE OpenInitEvent(std::optional<LPCWSTR> profileName) const;

	bool    IsExistControlProcess(std::optional<LPCWSTR> profileName = std::nullopt) const;
	bool    StartControlProcess(std::optional<LPCWSTR> profileName = std::nullopt) const;
	DWORD   StartProcess(const CCommandLine& cCommandLine, std::optional<std::wstring_view> workingDirectory = std::nullopt) const;
	void    TerminateControlProcess(std::optional<LPCWSTR> profileName = std::nullopt) const;

	virtual void RefreshString();
	virtual void    InitProcess() = 0;
	virtual CMainWindow*    GetMainWnd() const { return m_MainWindow.get(); }

	CPluginManager*   GetPluginManager() const { return m_PluginManager.get(); }

protected:
	virtual bool InitializeProcess() = 0;

	void			SetMainWindow(std::unique_ptr<CMainWindow>&& mainWindow) { m_MainWindow = std::move(mainWindow); }

	virtual bool InitShareData();

	virtual HANDLE CreateEventW(
		_In_opt_ LPSECURITY_ATTRIBUTES lpEventAttributes,
		bool bManualReset,
		bool bInitialState,
		std::wstring_view name
	) const
	{
		return ::CreateEventW(lpEventAttributes, bManualReset, bInitialState, name.data());
	}
	virtual HANDLE CreateMutexW(
		_In_opt_ LPSECURITY_ATTRIBUTES lpMutexAttributes,
		bool bInitialOwner,
		std::wstring_view name
	) const
	{
		return ::CreateMutexW(lpMutexAttributes, bInitialOwner, name.data());
	}
	virtual bool CreateProcessW(
		std::wstring_view exePath,
		std::wstring& commandLine,
		_In_opt_ LPCWSTR lpCurrentDirectory,
		_In_ LPSTARTUPINFOW lpStartupInfo,
		_Out_ LPPROCESS_INFORMATION lpProcessInformation
	) const
	{
		const auto bInheritHandles = false;
		const auto dwCreationFlag = CREATE_DEFAULT_ERROR_MODE;
		const auto lpEnvironment = nullptr;
		return ::CreateProcessW(
			exePath.data(),			// 実行可能モジュールの名前
			commandLine.data(),		// コマンドラインの文字列
			nullptr,				// セキュリティ記述子
			nullptr,				// セキュリティ記述子
			bInheritHandles,		// ハンドルの継承オプション
			dwCreationFlag,			// 作成のフラグ
			lpEnvironment,			// 新しい環境ブロック
			lpCurrentDirectory,		// カレントディレクトリの名前
			lpStartupInfo,			// スタートアップ情報
			lpProcessInformation	// プロセス情報
		);
	}
	virtual HWND FindWindowW(
		_In_opt_ LPCWSTR lpClassName,
		_In_opt_ LPCWSTR lpWindowName
	) const
	{
		return ::FindWindowW(lpClassName, lpWindowName);
	}
	virtual bool GetExitCodeProcess(
		_In_ HANDLE hProcess,
		_Out_ LPDWORD lpExitCode
	) const
	{
		return ::GetExitCodeProcess(hProcess, lpExitCode);
	}
	virtual DWORD GetLastError() const
	{
		return ::GetLastError();
	}
	virtual DWORD GetWindowThreadProcessId(
		_In_ HWND hWnd,
		_Out_opt_ LPDWORD lpdwProcessId
	) const
	{
		return ::GetWindowThreadProcessId(hWnd, lpdwProcessId);
	}
	virtual DWORD MsgWaitForMultipleObjects(
		_In_ DWORD nCount,
		_In_reads_opt_(nCount) CONST HANDLE* pHandles,
		_In_ BOOL fWaitAll,
		_In_ DWORD dwMilliseconds,
		_In_ DWORD dwWakeMask
	) const
	{
		return ::MsgWaitForMultipleObjects(nCount, pHandles, fWaitAll, dwMilliseconds, dwWakeMask);
	}
	virtual HANDLE OpenEventW(
		_In_ DWORD dwDesiredAccess,
		bool bInheritHandle,
		std::wstring_view name
	) const
	{
		return ::OpenEventW(dwDesiredAccess, bInheritHandle, name.data());
	}
	virtual HANDLE OpenProcess(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ DWORD dwProcessId
	) const
	{
		return ::OpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	}
	virtual LRESULT SendMessageW(
		_In_ HWND hWnd,
		_In_ UINT Msg,
		_Pre_maybenull_ _Post_valid_ WPARAM wParam,
		_Pre_maybenull_ _Post_valid_ LPARAM lParam
	) const
	{
		return ::SendMessageW(hWnd, Msg, wParam, lParam);
	}
	virtual bool SetEvent(
		_In_ HANDLE hEvent
	) const
	{
		return ::SetEvent(hEvent);
	}
	virtual DWORD WaitForSingleObject(
		_In_ HANDLE hHandle,
		_In_ DWORD dwMilliseconds
	) const
	{
		return ::WaitForSingleObject(hHandle, dwMilliseconds);
	}

public:
	HINSTANCE		GetProcessInstance() const{ return m_hInstance; }
	int             GetCmdShow() const{ return m_nCmdShow; }
	CCommandLine&   GetCCommandLine() const { return *m_pCommandLine; }
	CShareData&	    GetCShareData() { return m_cShareData; }
	DLLSHAREDATA&   GetShareData() const { return m_cShareData.GetShareData(); }
	HWND            GetMainWindow() const{ return m_MainWindow ? GetMainWnd()->GetHwnd() : nullptr; }

private:
	HINSTANCE           m_hInstance;
	CCommandLineHolder  m_pCommandLine;
	int                 m_nCmdShow;
	CShareData          m_cShareData;

	CMainWindowHolder       m_MainWindow      = nullptr;

	CUxThemeHolder          m_UxTheme         = std::make_unique<CUxTheme>();
	CAppNodeManagerHolder   m_AppNodeManager  = std::make_unique<CAppNodeManager>();
	CPluginManagerHolder    m_PluginManager   = std::make_unique<CPluginManager>();
	CFileNameManagerHolder  m_FileNameManager = nullptr;
	CJackManagerHolder      m_JackManager     = nullptr;
};

#endif /* SAKURA_CPROCESS_FECC5450_9096_4EAD_A6DA_C8B12C3A31B5_H_ */
