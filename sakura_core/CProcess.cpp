//	$Id$
/*!	@file
	@brief プロセス基底クラス

	@author aroka
	@date 2002/01/07 作成
	@date 2002/01/17 修正
	$Revision$
*/
/*
	Copyright (C) 2002, aroka 新規作成

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CProcess.h"

/*!
	@brief プロセス基底クラス
	
	@author aroka
	@date 2002/01/07
*/
CProcess::CProcess(
	HINSTANCE	hInstance,		//!< handle to process instance
	LPSTR		lpCmdLine		//!< pointer to command line
) :
	m_hInstance( hInstance ),
	m_CommandLine( lpCmdLine ),
	m_hWnd( 0 )
{
}

/*!
	@brief プロセス実行
	
	@author aroka
	@date 2002/01/16
*/
bool CProcess::Run(void)
{
	if( true == Initialize() ){
		MainLoop() ;
		Terminate();
		return true;
	}
	return false;
}
/*[EOF]*/
