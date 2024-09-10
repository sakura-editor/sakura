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
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcessFactory.h"

#include "_main/CControlProcess.h"
#include "_main/CNormalProcess.h"

#include "dlg/CDlgProfileMgr.h"

#include "config/system_constants.h"

CProcessFactory::CProcessFactory(
	_In_ HINSTANCE hInstance,
	_In_ int nCmdShow
) noexcept
	: m_hInstance(hInstance)
	, m_nCmdShow(nCmdShow)
{
}

/*!
	@brief プロセスクラスを生成する
	
	コマンドライン、コントロールプロセスの有無を判定し、
	適当なプロセスクラスを生成する。
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
 */
std::unique_ptr<CProcess> CProcessFactory::CreateInstance(
	std::wstring_view commandLine     //!< [in] コマンドライン文字列
)
{
	// 言語環境を初期化する
	CSelectLang::InitializeLanguageEnvironment();

	//コマンドラインクラスのインスタンスを確保する
	m_pCommandLine = std::make_unique<CCommandLine>();

	//実行ファイル名をもとに漢字コードを固定する．
	auto exeFilePath = GetExeFileName().wstring();
	m_pCommandLine->ParseKanjiCodeFromFileName(exeFilePath.data(), int(exeFilePath.length()));
	m_pCommandLine->ParseCommandLine(commandLine.data(), true);

	if( !ProfileSelect(m_hInstance)){
		return 0;
	}

	// プロセスクラスを生成する
	if (m_pCommandLine->IsNoWindow()) {
		return std::make_unique<CControlProcess>(m_hInstance, std::move(m_pCommandLine));
	} else {
		return std::make_unique<CEditorProcess>(m_hInstance, std::move(m_pCommandLine), m_nCmdShow);
	}
}

bool CProcessFactory::ProfileSelect(HINSTANCE hInstance) const
{
	auto commandLine = m_pCommandLine.get();

	std::wstring strProfileName;
	bool hasProfileName = commandLine->IsSetProfile();
	bool showProfileMgr = commandLine->IsProfileMgr();

	// コマンドラインオプションから起動プロファイルを判定する
	if (const auto profileSelected = CDlgProfileMgr::TrySelectProfile(strProfileName, hasProfileName, showProfileMgr);
		!profileSelected)
	{
		if (CDlgProfileMgr dlgProf;
			dlgProf.DoModal(hInstance, HWND(nullptr), 0))
		{
			commandLine->SetProfileName(dlgProf.m_strProfileName);
		}else{
			return false; // プロファイルマネージャで「閉じる」を選んだ。プロセス終了
		}
	}
	return true;
}
