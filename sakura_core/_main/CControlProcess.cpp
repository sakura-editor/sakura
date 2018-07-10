/*!	@file
	@brief コントロールプロセスクラス

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcessより分離, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CControlProcess.h"
#include "CControlTray.h"
#include "env/DLLSHAREDATA.h"
#include "CCommandLine.h"
#include "env/CShareData_IO.h"
#include "debug/CRunningTimer.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka ヘッダ整理

#include "_os\ProcessEntryIterator.h"

#include <Psapi.h>
#if _WIN32_WINNT < _WIN32_WINNT_WIN7
	// リンカ指定
	#if PSAPI_VERSION < 2
	#pragma comment(lib, "Psapi.lib")
	#endif
#endif

//-------------------------------------------------


/*!
	@brief コントロールプロセスを初期化する
	
	MutexCPを作成・ロックする。
	CControlTrayを作成する。
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI 共有メモリを初期化するのはCProcessに移動。
	@date 2006/04/10 ryoji 初期化完了イベントの処理を追加、異常時の後始末はデストラクタに任せる
	@date 2013.03.20 novice コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::InitializeProcess" );

	// アプリケーション実行検出用(インストーラで使用)
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()失敗。\n終了します。") );
		return false;
	}

	std::tstring strProfileName = to_tchar(CCommandLine::getInstance()->GetProfileName());

	// 初期化完了イベントを作成する
	std::tstring strInitEvent = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	strInitEvent += strProfileName;
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, strInitEvent.c_str() );
	if( NULL == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateEvent()失敗。\n終了します。") );
		return false;
	}

	/* コントロールプロセスの目印 */
	std::tstring strCtrlProcEvent = GSTR_MUTEX_SAKURA_CP;
	strCtrlProcEvent += strProfileName;
	m_hMutexCP = ::CreateMutex( NULL, TRUE, strCtrlProcEvent.c_str() );
	if( NULL == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()失敗。\n終了します。") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* 共有メモリを初期化 */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	TCHAR szDir[_MAX_PATH];
	::GetSystemDirectory( szDir, _countof(szDir) );
	::SetCurrentDirectory( szDir );

	/* 共有データのロード */
	// 2007.05.19 ryoji 「設定を保存して終了する」オプション処理（sakuext連携用）を追加
	TCHAR szIniFile[_MAX_PATH];
	CShareData_IO::LoadShareData();
	CFileNameManager::getInstance()->GetIniFileName( szIniFile, strProfileName.c_str() );	// 出力iniファイル名
	if( !fexist(szIniFile) || CCommandLine::getInstance()->IsWriteQuit() ){
		/* レジストリ項目 作成 */
		CShareData_IO::SaveShareData();
		if( CCommandLine::getInstance()->IsWriteQuit() ){
			return false;
		}
	}

	// 常駐しない設定の場合
	if (!GetDllShareData().m_Common.m_sGeneral.m_bStayTaskTray) {
		// 先行して起動したエディタプロセスを検索する
		if (!IsEditorProcess(strProfileName)) {
			return false;	// エディタプロセスが見つからなければ終了
		}
	}

	/* 言語を選択する */
	CSelectLang::ChangeLang( GetDllShareData().m_Common.m_sWindow.m_szLanguageDll );
	RefreshString();

	MY_TRACETIME( cRunningTimer, "Before new CControlTray" );

	/* タスクトレイにアイコン作成 */
	m_pcTray = new CControlTray;

	MY_TRACETIME( cRunningTimer, "After new CControlTray" );

	HWND hwnd = m_pcTray->Create( GetProcessInstance() );
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX3) );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// 初期化完了イベントをシグナル状態にする
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( NULL, LS(STR_ERR_CTRLMTX4) );
		return false;
	}

	return true;
}

/*!
	@brief コントロールプロセスのメッセージループ
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( m_pcTray && GetMainWindow() ){
		m_pcTray->MessageLoop();	/* メッセージループ */
		return true;
	}
	return false;
}

/*!
	@brief コントロールプロセスを終了する
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji 共有データ保存を CControlTray へ移動
*/
void CControlProcess::OnExitProcess()
{
	GetDllShareData().m_sHandles.m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	delete m_pcTray;

	if( m_hEventCPInitialized ){
		::ResetEvent( m_hEventCPInitialized );
	}
	::CloseHandle( m_hEventCPInitialized );
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// 旧バージョン（1.2.104.1以前）との互換性：「異なるバージョン...」が二回出ないように
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

//他プロセスのコマンドラインを取得する
std::wstring getCommandLine(_In_ HANDLE hProcess);

/*!
 * @brief コントロールプロセスを起動したエディタプロセスを取得する
 *
 * @param [in] profileName プロファイル名
 * @date 2017/06/25 berryzplus		新規作成
 */
bool CControlProcess::IsEditorProcess(const std::wstring &profileName) const
{
	// 自プロセスのフルパスを取得する
	TCHAR szMyPath[MAX_PATH];
	::GetModuleFileName(NULL, szMyPath, _countof(szMyPath) - 1);

	// 自プロセスのファイル名を取得する
	TCHAR szMyFilename[MAX_PATH];
	::SplitPath_FolderAndFile(szMyPath, NULL, szMyFilename);

	// 自プロセスのプロセスIDを取得する
	DWORD myProcessId = ::GetCurrentProcessId();

	// エディタプロセスを見付けるか、全プロセスのチェックが終わるまでループ
	bool editorExists = false;
	ProcessEntryIterator processIter(myProcessId);
	ProcessEntryIterator processEnd;
	for (; processIter != processEnd; ++processIter) {
		// プロセス情報を取得
		PROCESSENTRY32 &pe = *processIter;

		// 自プロセスはスキップ
		if (myProcessId == pe.th32ProcessID)continue;

		// ファイル名が異なるものはスキップ
		if (szMyFilename[0] != pe.szExeFile[0] || 0 != _tcsicmp(szMyFilename, pe.szExeFile)) continue;

		// プロセスIDを使ってプロセスハンドルを開く。
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);

		// プロセスを開けなければスキップ(おそらく権限エラー)
		if (!hProcess) continue;

		// プロセスのフルパスを取得
		TCHAR szFullname[MAX_PATH];
		if (!::GetModuleFileNameEx(hProcess, NULL, szFullname, _countof(szFullname) - 1)) {
			::CloseHandle(hProcess);
			continue;
		}

		// パスが一致しなければスキップ
		if (0 != _tcsicmp(szMyPath, szFullname))
		{
			::CloseHandle(hProcess);
			continue;
		}

		// コマンドラインを取得
		std::wstring cmdline;
		try {
			cmdline = getCommandLine(hProcess);
			::CloseHandle(hProcess);
		}
		catch (const std::exception& /* ex */) {
			::CloseHandle(hProcess);
			return false;
		}

		// コマンドラインを解析
		CCommandLine cmdlineObj;
		cmdlineObj.ParseCommandLine(cmdline.c_str(), false);

		// プロファイル指定が一致
		if (0 == _tcsicmp(profileName.c_str(), cmdlineObj.GetProfileName())) {
			return true; // エディタプロセスを見付けた！
		}
	}
	return false; // エディタプロセスは見つからなかった
}

#include "_os\NtQueryProcessInformation.h"


//他プロセスの生コマンドラインを取得する
std::wstring getRawCommandLine(_In_ HANDLE hProcess);

//他プロセスのコマンドラインを取得する
std::wstring getCommandLine(_In_ HANDLE hProcess)
{
	// CommandLine
	std::wstring rawCmdline(getRawCommandLine(hProcess));

	// WinMainのMinGW向けコードからパクってきたコード
	LPCWSTR pszCommandLine = rawCmdline.c_str();
	{
		// 実行ファイル名をスキップする
		if (_T('\"') == *pszCommandLine) {
			pszCommandLine++;
			while (_T('\"') != *pszCommandLine && _T('\0') != *pszCommandLine) {
				pszCommandLine++;
			}
			if (_T('\"') == *pszCommandLine) {
				pszCommandLine++;
			}
		}
		else {
			while (_T(' ') != *pszCommandLine && _T('\t') != *pszCommandLine
				&& _T('\0') != *pszCommandLine) {
				pszCommandLine++;
			}
		}
		// 次のトークンまで進める
		while (_T(' ') == *pszCommandLine || _T('\t') == *pszCommandLine) {
			pszCommandLine++;
		}
	}
	return std::wstring(pszCommandLine);
}


//他プロセスの環境ブロックを取得する
void readProcessEnvironmentBlock(_In_ HANDLE hProcess, _Out_ PPEB ppeb);

//他プロセスのプロセスパラメータを取得する
void readUserProcessParameters(_In_ HANDLE hProcess, _Out_ PRTL_USER_PROCESS_PARAMETERS pupp);

//他プロセスの生コマンドラインを取得する
std::wstring getRawCommandLine(_In_ HANDLE hProcess)
{
	// 他プロセスのコマンドラインにアクセスするには、
	// プロセスの環境ブロック(PEB)にあるパラメータを参照する必要がある。

	// User Process Parameters
	RTL_USER_PROCESS_PARAMETERS upp;

	// read User Process Parameters
	readUserProcessParameters(hProcess, &upp);

	if (!upp.CommandLine.Length)
	{
		throw std::exception("upp.CommandLine is empty.");
	}

	// CommandLine
	std::wstring rawCmdline(upp.CommandLine.Length, '\0');

	// Try to read CommandLine
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, upp.CommandLine.Buffer, &*rawCmdline.begin(), upp.CommandLine.Length, &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for CommandLine.");
	}

	return std::move(rawCmdline);
}


//他プロセスのプロセスパラメータを取得する
void readUserProcessParameters(_In_ HANDLE hProcess, _Out_ PRTL_USER_PROCESS_PARAMETERS pupp)
{
	// Process Environment Block(PEB)
	PEB peb;

	// read Process Environment Block (PEB)
	readProcessEnvironmentBlock(hProcess, &peb);

	// try to read User Process Parameters
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, peb.ProcessParameters, pupp, sizeof(pupp), &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for USER_PROCESS_PARAMETERS.");
	}
}


//他プロセスの環境ブロックを取得する
void readProcessEnvironmentBlock(_In_ HANDLE hProcess, _Out_ PPEB ppeb)
{
	NtQueryInformationProcessT NtQueryInformationProcess;

	std::unique_ptr<BYTE> pbiBuf(new BYTE[sizeof(PROCESS_BASIC_INFORMATION)]);
	for (DWORD size = sizeof(PROCESS_BASIC_INFORMATION);;) {
		ULONG sizeNeeded = 0;
		NTSTATUS status = NtQueryInformationProcess(hProcess, ProcessBasicInformation, (PPROCESS_BASIC_INFORMATION) &*pbiBuf, size, &sizeNeeded);
		if (status)
		{
			if (size < sizeNeeded) {
				size = sizeNeeded;
				pbiBuf = std::unique_ptr<BYTE>(new BYTE[size]);
				continue;
			}
			throw std::exception("failed NtQueryInformationProcess.");
		}
		break;
	}

	PPROCESS_BASIC_INFORMATION pbi = (PPROCESS_BASIC_INFORMATION) &*pbiBuf;
	if (pbi->PebBaseAddress == NULL)
	{
		throw std::exception("pbi->PebBaseAddress is NULL.");
	}

	// try to read Process Environment Block (PEB)
	SIZE_T cbRead = 0;
	if (!ReadProcessMemory(hProcess, pbi->PebBaseAddress, ppeb, sizeof(*ppeb), &cbRead))
	{
		throw std::exception("failed to ReadProcessMemory for PEB.");
	}
}
