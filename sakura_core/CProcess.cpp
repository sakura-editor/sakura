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
#include "debug.h"
#include "etc_uty.h"

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
	@brief �v���Z�X������������

	���L������������������
*/
bool CProcess::Initialize()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	if( !m_cShareData.Init() ){
		//	�K�؂ȃf�[�^�𓾂��Ȃ�����
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, _T("�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B") );
		return false;
	}
	m_pShareData = m_cShareData.GetShareData();

	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
	GetAppVersionInfo( m_hInstance, VS_VERSION_INFO,
		&m_pShareData->m_dwProductVersionMS, &m_pShareData->m_dwProductVersionLS );

	return true;
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
