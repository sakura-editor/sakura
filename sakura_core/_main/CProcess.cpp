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
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_main/CProcess.h"

#include "util/module.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "config/app_constants.h"
#include "CSelectLang.h"

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE	hInstance,		//!< handle to process instance
	[[maybe_unused]] LPCWSTR		lpCmdLine		//!< pointer to command line
)
: m_hInstance( hInstance )
{
}

/*!
	@brief iniファイルパスを取得する
 */
std::filesystem::path CProcess::GetIniFileName() const
{
	if (m_cShareData.IsPrivateSettings()) {
		const DLLSHAREDATA *pShareData = &GetDllShareData();
		return pShareData->m_szPrivateIniFile.c_str();
	}
	return GetExeFileName().replace_extension(L".ini");
}

/*!
	@brief プロセスを初期化する

	共有メモリを初期化する
*/
bool CProcess::InitializeProcess(
	int nCmdShow [[maybe_unused]]
)
{
	/* 共有データ構造体のアドレスを返す */
	if( !GetShareData().InitShareData() ){
		//	適切なデータを得られなかった
		::MYMESSAGEBOX( nullptr, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, L"異なるバージョンのエディタを同時に起動することはできません。" );
		return false;
	}

	/* リソースから製品バージョンの取得 */
	//	2004.05.13 Moca 共有データのバージョン情報はコントロールプロセスだけが
	//	ShareDataで設定するように変更したのでここからは削除

	return true;
}

/*!
 * @brief プロセス実行
 *
 * @param[in] nCmdShow コマンドラインから渡されたウィンドウ表示フラグ
 * @returns アプリ終了コード
 *
 * @author aroka
 * @date 2002/01/16 aroka 新規作成（？）
 */
EAppExitCode CProcess::Run(
	int nCmdShow
)
{
	// 終了コード
	auto exitCode = EAppExitCode::Exited;

	//TODO: __try-__exceptで囲む

	if( InitializeProcess(nCmdShow) )
	{

			MainLoop() ;

		exitCode = EAppExitCode::Exited;	//TODO: MainLoopの戻り値を返すように変える
	}

	return exitCode;
}

/*!
	言語選択後に共有メモリ内の文字列を更新する
*/
void CProcess::RefreshString()
{
	m_cShareData.RefreshString();
}
