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
	catch (const message_error& e)
	{
		TopErrorMessage(nullptr, e.message());
	}
	if (initialized)
	{
			MainLoop() ;
			OnExitProcess();
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
