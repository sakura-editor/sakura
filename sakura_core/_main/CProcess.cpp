/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2004, Moca
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcess.h"

#include "apiwrap/kernel/format_message.hpp"
#include "apiwrap/kernel/system_path.hpp"

#include "util/module.h"
#include "config/system_constants.h"
#include "config/app_constants.h"

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
 */
CProcess::CProcess(
	HINSTANCE               hInstance,      //!< handle to process instance
	CCommandLineHolder&&    pCommandLine,   //!< pointer to command line
	int                     nCmdShow
) noexcept
	: m_hInstance(hInstance)
	, m_pCommandLine(std::move(pCommandLine))
	, m_nCmdShow(nCmdShow)
{
}

/*!
	@brief プロセスを初期化する

	共有メモリを初期化する
 */
bool CProcess::InitializeProcess()
{
	/* 共有データ構造体のアドレスを返す */
	m_cShareData.InitShareData();

	// 派生クラスでウインドウを作成する際に以下パラメーターを使用したい
	UNREFERENCED_PARAMETER(m_nCmdShow);

	/* リソースから製品バージョンの取得 */
	//	2004.05.13 Moca 共有データのバージョン情報はコントロールプロセスだけが
	//	ShareDataで設定するように変更したのでここからは削除

	return true;
}

bool CProcess::InitShareData()
{
	const auto result = m_cShareData.InitShareData();
	if (result)
	{
		m_FileNameManager = std::make_unique<CFileNameManager>();
		m_JackManager     = std::make_unique<CJackManager>();
	}
	return result;
}

[[nodiscard]] HANDLE CProcess::OpenInitEvent(std::optional<LPCWSTR> profileName) const
{
	std::wstring eventName = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	if (profileName.has_value() && profileName.value())
	{
		eventName += profileName.value();
	}
	return OpenEventW(SYNCHRONIZE, FALSE, eventName);
}

/*!
	コントロールプロセスの有無を調べる。
	
	@author aroka
	@author ryoji by assitance with karoto
	@date 2002/01/03
	@date 2006/04/10 ryoji
 */
bool CProcess::IsExistControlProcess(std::optional<LPCWSTR> profileName) const
{
	// 初期化完了イベントを開く
	const auto hEvent = OpenInitEvent(profileName);
	if (!hEvent) {
		return false;
	}

	// イベントハンドルをスマートポインタに入れる
	handleHolder eventHolder(hEvent, handle_closer());

	// イベントを待つ
	const auto waitResult = WaitForSingleObject(hEvent, 15 * 1000);
	return WAIT_OBJECT_0 == waitResult || WAIT_ABANDONED_0 == waitResult;
}

/*!
	@brief コントロールプロセスを起動する

	CProcessFactory::StartControlProcess()を移植した。

	@author genta
	@date Aug. 28, 2001
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
 */
bool CProcess::StartControlProcess(std::optional<LPCWSTR> profileName) const
{
	// 起動前に存在チェック
	if (const auto isControlProcessInitialized = IsExistControlProcess(profileName))
	{
		return isControlProcessInitialized;
	}

	// コマンドラインを用意する
	CCommandLine cCommandLine;
	cCommandLine.SetNoWindow(true);

	if (profileName.has_value()) // ここでは、空文字も有効値。
	{
		cCommandLine.SetProfileName(profileName.value());
	}

	// システムディレクトリパスを取得
	const auto systemPath = system_path();

	// コントロールプロセスを起動
	StartProcess(cCommandLine, systemPath.c_str());

	//初期化完了イベント
	handleHolder eventHolder(nullptr, handle_closer());

	// 初期化完了イベントが生成されるまで待つ
	for (int i = 0; i < 30 * 1000 / 200; ++i) {
		if (const auto hEvent = OpenInitEvent(profileName))
		{
			// イベントハンドルをスマートポインタに入れる
			eventHolder.reset(hEvent);

			// イベントを待つ
			if (const auto waitResult = WaitForSingleObject(hEvent, 10 * 1000);
				WAIT_FAILED == waitResult)
			{
				throw message_error( L"waitProcess has failed." );
			}
			else
			{
				return WAIT_TIMEOUT != waitResult;
			}
		}

		Sleep(200);
	}

	throw process_init_failed( LS(STR_ERR_DLGNRMPROC2) ); // L"エディタまたはシステムがビジー状態です。\nしばらく待って開きなおしてください。"
}

DWORD CProcess::StartProcess(
	const CCommandLine& cCommandLine,
	std::optional<std::wstring_view> workingDirectory
) const
{
	// 実行ファイル名を取得
	const auto exePath = GetExeFileName();

	// コマンドライン文字列を作成
	auto strCommandLine = fmt::format(LR"("{}"{})", exePath.wstring(), cCommandLine.ToCommandArgs());

	const auto pszWorkingDirectory = workingDirectory.has_value() ? workingDirectory.value().data() : nullptr;

	// スタートアップ情報
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.dwFlags     = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWDEFAULT;

	if (cCommandLine.IsNoWindow())
	{
		si.lpTitle = LPWSTR(L"sakura control process");
	}

	PROCESS_INFORMATION pi = {};

	if (!CreateProcessW(
		exePath.c_str(),		// 実行可能モジュールの名前
		strCommandLine,			// コマンドラインの文字列
		pszWorkingDirectory,	// カレントディレクトリの名前
		&si,					// スタートアップ情報
		&pi						// プロセス情報
	))
	{
		format_message(GetLastError(), [exePath](LPCWSTR pszMsg) {
			const auto message = fmt::format(L"\'{}\'\nプロセスの起動に失敗しました。\n{}", exePath.wstring(), pszMsg);
			throw process_init_failed(message);
		});
	}

	// ハンドルは使わないので、即座に閉じてしまう。
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return pi.dwProcessId;
}

/*!
 * @brief コントロールプロセスに終了指示を出して終了を待つ
 */
void CProcess::TerminateControlProcess(std::optional<LPCWSTR> profileName) const
{
	// トレイウインドウを検索する
	std::wstring strCEditAppName(GSTR_CEDITAPP);
	if (profileName.has_value() && profileName.value()) {
		strCEditAppName += profileName.value();
	}
	std::wstring_view className = strCEditAppName;
	std::wstring_view windowName = className;
	const auto hWndTray = FindWindowW(className.data(), windowName.data() );
	if (!hWndTray)
	{
		throw message_error( L"hWndTray can't be retrived.");
	}

	// トレイウインドウからプロセスIDを取得する
	DWORD dwControlProcessId = 0;
	GetWindowThreadProcessId(hWndTray, &dwControlProcessId);
	if (!dwControlProcessId)
	{
		throw message_error( L"dwControlProcessId can't be retrived.");
	}

	// プロセス情報の問い合せを行うためのハンドルを開く
	const auto hControlProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, dwControlProcessId);
	if (!hControlProcess)
	{
		throw message_error( L"hControlProcess can't be opened.");
	}

	// プロセスハンドルをスマートポインタに入れる
	handleHolder processHolder(hControlProcess, handle_closer());

	// トレイウインドウを閉じる
	SendMessageW(hWndTray, WM_CLOSE, 0, 0);

	// プロセス終了を待つ
	for (;;)
	{
		if (const auto waitResult = MsgWaitForMultipleObjects(1, &hControlProcess, FALSE, 1000, 0);
			WAIT_FAILED == waitResult)
		{
			throw message_error( L"waitProcess has failed." );
		}
		else if (WAIT_TIMEOUT == waitResult)
		{
			continue;
		}

		if (DWORD exitCode = 0UL;
			!GetExitCodeProcess(hControlProcess, &exitCode))
		{
			throw message_error( L"GetExitCodeProcess has failed." );
		}
		else if (STILL_ACTIVE != exitCode)
		{
			break;
		}

		Sleep(100);
	}
}

/*!
	@brief プロセス実行
	
	@author aroka
	@date 2002/01/16
 */
int CProcess::Run() noexcept
{
	bool initialized = false;
	try
	{
		initialized = InitializeProcess();
	}
	catch (const process_init_failed& e)
	{
		ErrorBeep();
		TopErrorMessage(nullptr, e.message());
		return 1;
	}
	catch (const message_error& e)
	{
		TopErrorMessage(nullptr, e.message());
	}
	if (initialized)
	{
		GetMainWnd()->MessageLoop();
	}
	return 0;
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
