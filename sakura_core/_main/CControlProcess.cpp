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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CControlProcess.h"

#include "env/CShareData_IO.h"
#include "debug/CRunningTimer.h"
#include "config/system_constants.h"

#include "apiwrap/kernel/system_path.hpp"

CControlProcess::CControlProcess(HINSTANCE hInstance, CCommandLineHolder&& pCommandLine) noexcept
	: CProcess(hInstance, std::move(pCommandLine), SW_SHOWNA)
{
}

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
	const auto profileName = GetCCommandLine().GetProfileName();

	// ミューテックスを使って排他ロックをかける
	std::wstring mutexName = GSTR_MUTEX_SAKURA_CP;
	if (profileName && *profileName) {
		mutexName += profileName;
	}
	const auto hMutex = CreateMutexW(nullptr, true, mutexName);
	if (!hMutex)
	{
		throw process_init_failed( LS(STR_ERR_CTRLMTX1) ); // L"CreateMutex()失敗。\n終了します。"
	}

	// ミューテックスハンドルをスマートポインタに入れる
	handleHolder mutexHolder(hMutex, handle_closer());

	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		return false;
	}

	// 初期化完了イベントを作成する
	std::wstring eventName = GSTR_EVENT_SAKURA_CP_INITIALIZED;
	if (profileName && *profileName) {
		eventName += profileName;
	}
	const auto hEvent = CreateEventW(nullptr, true, false, eventName);
	if (!hEvent)
	{
		throw process_init_failed( LS(STR_ERR_CTRLMTX2) ); // L"CreateEvent()失敗。\n終了します。"
	}

	// イベントハンドルをスマートポインタに入れる
	m_InitEvent.reset(hEvent);

	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		return false;
	}

	InitProcess();

	if (!GetMainWnd()->CreateMainWnd(GetCmdShow())) {
		throw process_init_failed( LS(STR_ERR_CTRLMTX3) ); // L"ウィンドウの作成に失敗しました。\n起動できません。"
	}

	// 初期化完了イベントをシグナル状態にする
	if (!SetEvent(hEvent)) {
		throw process_init_failed( LS(STR_ERR_CTRLMTX4) ); // L"SetEvent()失敗。\n終了します。"
	}

	return true;
}

void CControlProcess::InitProcess()
{
	/* 共有メモリを初期化 */
	if (!InitShareData())
	{
		throw process_init_failed( LS(STR_ERR_DLGPROCESS1) ); // L"異なるバージョンのエディタを同時に起動することはできません。"
	}

	/* タスクトレイにアイコン作成 */
	SetMainWindow(std::make_unique<CControlTray>());
}

bool CControlProcess::InitShareData()
{
	const auto result = __super::InitShareData();
	if (result) {
		LoadShareData();
	}
	return result;
}

void CControlProcess::LoadShareData()
{
	// システムディレクトリパスを取得
	const auto systemPath = system_path();

	// コントロールプロセスのカレントディレクトリをシステムディレクトリに変更
	SetCurrentDirectoryW(systemPath.c_str());

	/* 共有データのロード */
	if (!CShareData_IO::LoadShareData()) {
		SaveShareData();
	}

	/* 言語を選択する */
	CSelectLang::ChangeLang(GetShareData().m_Common.m_sWindow.m_szLanguageDll);
	RefreshString();
}

void CControlProcess::SaveShareData() const
{
	CShareData_IO::SaveShareData();
}
