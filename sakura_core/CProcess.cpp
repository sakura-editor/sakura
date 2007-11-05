/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
*/
/*
	Copyright (C) 2002, aroka 新規作成
	Copyright (C) 2004, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CProcess.h"
#include "debug.h"

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE	hInstance,		//!< handle to process instance
	LPTSTR		lpCmdLine		//!< pointer to command line
)
: m_hInstance( hInstance )
, m_CommandLine( lpCmdLine )
, m_hWnd( 0 )
{
}

/*!
	@brief プロセスを初期化する

	共有メモリを初期化する
*/
bool CProcess::Initialize()
{
	/* 共有データ構造体のアドレスを返す */
	if( !m_cShareData.Init() ){
		//	適切なデータを得られなかった
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, _T("異なるバージョンのエディタを同時に起動することはできません。") );
		return false;
	}

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
bool CProcess::Run(void)
{
	if( Initialize() )
	{
		MainLoop() ;
		Terminate();
		return true;
	}
	return false;
}
/*[EOF]*/
