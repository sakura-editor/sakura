/*!	@file
	@brief �v���Z�X���N���X

	@author aroka
	@date 2002/01/07 �쐬
	@date 2002/01/17 �C��
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬
	Copyright (C) 2004, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CProcess.h"
#include "debug/Debug.h"

/*!
	@brief �v���Z�X���N���X
	
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
	@brief �v���Z�X������������

	���L������������������
*/
bool CProcess::InitializeProcess()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	if( !m_cShareData.Init() ){
		//	�K�؂ȃf�[�^�𓾂��Ȃ�����
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONERROR,
			GSTR_APPNAME, _T("�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B") );
		return false;
	}

	/* ���\�[�X���琻�i�o�[�W�����̎擾 */
	//	2004.05.13 Moca ���L�f�[�^�̃o�[�W�������̓R���g���[���v���Z�X������
	//	ShareData�Őݒ肷��悤�ɕύX�����̂ł�������͍폜

	return true;
}

/*!
	@brief �v���Z�X���s
	
	@author aroka
	@date 2002/01/16
*/
bool CProcess::Run()
{
	if( InitializeProcess() )
	{
		MainLoop() ;
		OnExitProcess();
		return true;
	}
	return false;
}

