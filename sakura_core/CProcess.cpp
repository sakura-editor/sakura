//	$Id$
/*!	@file
	@brief �v���Z�X���N���X

	@author aroka
	@date 2002/01/07 �쐬
	@date 2002/01/17 �C��
	$Revision$
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CProcess.h"

/*!
	@brief �v���Z�X���N���X
	
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
	@brief �v���Z�X���s
	
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
