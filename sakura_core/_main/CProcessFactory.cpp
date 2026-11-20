/*!	@file
	@brief プロセス生成クラス

	@author aroka
	@date 2002/01/03 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMainより分離
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"

#include "_main/CControlProcess.h"
#include "_main/CControlTray.h"
#include "_main/CNormalProcess.h"
#include "basis/primitive.h"

#include "dlg/CDlgProfileMgr.h"
#include "debug/CRunningTimer.h"
#include "util/os.h"
#include "util/sync/ProcessHolder.hpp"
#include <tchar.h>
#include "CSelectLang.h"
#include "config/system_constants.h"

namespace cxx {

HWND	FindWindowW(std::wstring_view className, const std::optional<std::wstring>& optWindowName = std::nullopt);

} // namespace cxx

CProcessFactory::CProcessFactory(
	_In_opt_ HINSTANCE hInstance
)
	: m_hInstance(hInstance ? hInstance : G_AppInstance())
{
}

/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@param[in] hInstance インスタンスハンドル
	@param[in] commandLine コマンドライン文字列
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
*/
std::unique_ptr<CProcess> CProcessFactory::CreateInstance(
	std::wstring_view commandLine
) const
{
	// 引数はNUL終端文字列として扱う
	cxx::NullTerminatedString<WCHAR> _CommandLine{ commandLine };

	// NUL終端文字列から文字列ポインターを取り出す
	const auto lpCmdLine = _CommandLine.c_str();

	//コマンドラインクラスのインスタンスを生成する
	auto cmdLineHolder = std::make_unique<CCommandLine>();

	auto& cCommandLine = *cmdLineHolder;

	// 実行ファイル名を取得する
	std::wstring exeFileName{ GetExeFileName() };
	auto szExeFileName = exeFileName.data();
	const auto cchExeFileName = static_cast<int>(exeFileName.length());

	// 実行ファイル名をもとに漢字コードを固定する．
	cCommandLine.ParseKanjiCodeFromFileName(szExeFileName, cchExeFileName);

	// コマンドラインを解析する
	cCommandLine.ParseCommandLine(lpCmdLine);

	if (!ProfileSelect(cCommandLine))
	{
		return nullptr;
	}

	if( !IsValidVersion() ){
		return nullptr;
	}

	// プロセスクラスを生成する
	//
	// Note: 以下の処理において使用される IsExistControlProcess() は、コントロールプロセスが
	// 存在しない場合だけでなく、コントロールプロセスが起動して ::CreateMutex() を実行するまで
	// の間も false（コントロールプロセス無し）を返す。
	// 従って、複数のノーマルプロセスが同時に起動した場合などは複数のコントロールプロセスが
	// 起動されることもある。
	// しかし、そのような場合でもミューテックスを最初に確保したコントロールプロセスが唯一生き残る。
	//
	if( IsStartingControlProcess() ){
		return std::make_unique<CControlProcess>(m_hInstance, std::move(cmdLineHolder));
	}
	else{
		return std::make_unique<CNormalProcess>(m_hInstance,  std::move(cmdLineHolder));
	}
}

bool CProcessFactory::ProfileSelect(
	CCommandLine& cCommandLine
) const
{
	auto* pCommandLine = &cCommandLine;

	// コマンドラインオプションから起動プロファイルを判定する
	if (const auto profileSelected = CDlgProfileMgr::TrySelectProfile(pCommandLine);
		!profileSelected)	// プロファイルが決まらなかった
	{
		// プロファイルマネージャを表示して選択させる
		CDlgProfileMgr dlgProf;
		if (HWND hWndParent = nullptr;
			!dlgProf.DoModal(hWndParent, pCommandLine->GetProfileNameRef())) {
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}

/*!
	@brief Windowsバージョンのチェック
	
	Windows 95以上，Windows NT4.0以上であることを確認する．
	Windows 95系では残りリソースのチェックも行う．
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsValidVersion() const
{
	// Windowsバージョンは廃止。
	// 動作可能バージョン(=windows7以降)でなければ起動できない。
	return true;
}

/*!
	@brief コマンドラインに -NOWIN があるかを判定する。
	
	@author aroka
	@date 2002/01/03 作成 2002/01/18 変更
*/
bool CProcessFactory::IsStartingControlProcess() const
{
	return CCommandLine::getInstance()->IsNoWindow();
}

/*!
	コントロールプロセスの有無を調べる。
	
	@author aroka
	@date 2002/01/03
	@date 2006/04/10 ryoji
*/
/* static */ bool CProcessFactory::IsExistControlProcess()
{
	const auto pszProfileName = GetProfileName();
	std::wstring strMutexSakuraCp = GSTR_MUTEX_SAKURA_CP;
	strMutexSakuraCp += pszProfileName;
 	HANDLE hMutexCP;
	hMutexCP = ::OpenMutex( MUTEX_ALL_ACCESS, FALSE, strMutexSakuraCp.c_str() );	// 2006.04.10 ryoji ::CreateMutex() を ::OpenMutex()に変更
	if( nullptr != hMutexCP ){
		::CloseHandle( hMutexCP );
		return true;	// コントロールプロセスが見つかった
	}

	return false;	// コントロールプロセスは存在していないか、まだ CreateMutex() してない
}

/*!
	@brief コントロールプロセスを起動する
	
	自分自身に -NOWIN オプションを付けて起動する．
	共有メモリをチェックしてはいけないので，残念ながらCControlTray::OpenNewEditorは使えない．
	
	@author genta
	@date Aug. 28, 2001
	@date 2008.05.05 novice GetModuleHandle(NULL)→NULLに変更
*/
/* static */ bool CProcessFactory::StartControlProcess()
{
	MY_RUNNINGTIMER(cRunningTimer,L"StartControlProcess" );

	std::wstring trayWndClassName = GSTR_CEDITAPP;
	trayWndClassName += GetProfileName();

	const auto isTrayWndReady = [&trayWndClassName] () {
		const auto hWndTray = cxx::FindWindowW(trayWndClassName, trayWndClassName);
		return
			hWndTray &&
			::SendMessageTimeoutW(hWndTray, WM_NULL, 0L, 0L, SMTO_ABORTIFHUNG | SMTO_ERRORONEXIT, 2000, nullptr);
	};

	// 起動中ミューテックスが有効、かつ、
	// トレイウィンドウが応答可能なら起動済みとみなす
	if (IsExistControlProcess() &&
		isTrayWndReady())
	{
		return true;	// 起動済み
	}

	bool bRet = false;
	try {
		// コントロールプロセスを起動する
		const auto cp = cxx::ProcessHolder::CreateControlProcess();

		bRet = true;	// 起動できた
	}
	// エラーが発生した場合
	catch (const std::system_error&) {
		// 別プロセスによって起動されたトレイウィンドウがあるはず。
		if (!isTrayWndReady()) throw;

		bRet = true;	// 起動済み
	}

	return bRet;
}
