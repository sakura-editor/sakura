//	$Id$
/*!	@file
	@brief �R���g���[���v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcess��蕪��, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "CControlProcess.h"
#include "CCommandLine.h"
#include "CShareData.h"
#include "Debug.h"
#include "CEditApp.h"
#include "CMemory.h"
#include "etc_uty.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka �w�b�_����
#include <tchar.h>
#include "CRunningTimer.h"



/*!
	@brief �R���g���[���v���Z�X�I���_�C�A���O�p�v���V�[�W��
*/
BOOL CALLBACK CControlProcess::ExitingDlgProc(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam		// second message parameter
)
{
	switch( uMsg ){
	case WM_INITDIALOG:
		return TRUE;
	}
	return FALSE;
}
//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	CEditApp���쐬����B
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI ���L������������������̂�CProcess�Ɉړ��B
*/
bool CControlProcess::Initialize()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::Initialize" );

	// ���o�[�W�����Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}
	/* �R���g���[���v���Z�X�̖ڈ� */
	m_hMutexCP = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == m_hMutexCP ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		DWORD dwRet = ::WaitForSingleObject( m_hMutexCP, 0 );
		if( WAIT_TIMEOUT == dwRet ){// ���łɃ��b�N����Ă�
			::CloseHandle( m_hMutexCP );
			m_hMutexCP = NULL;
			return false;
		}
	}
	
	/* ���L�������������� */
	if ( CProcess::Initialize() == false ){
		::ReleaseMutex( m_hMutexCP );
		m_hMutexCP = NULL;
		return false;
	}

	/* ���L�f�[�^�̃��[�h */
	if( FALSE == m_cShareData.LoadShareData() ){
		/* ���W�X�g������ �쐬 */
		m_cShareData.SaveShareData();
	}

	MY_TRACETIME( cRunningTimer, "Before new CEditApp" );

	/* �^�X�N�g���C�ɃA�C�R���쐬 */
	m_pcEditApp = new CEditApp;

	MY_TRACETIME( cRunningTimer, "After new CEditApp" );

	if( NULL == ( m_hWnd = m_pcEditApp->Create( m_hInstance ) ) ){
		::ReleaseMutex( m_hMutexCP );
		m_hMutexCP = NULL;
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("�E�B���h�E�̍쐬�Ɏ��s���܂����B\n�N���ł��܂���B") );
		return false;
	}
	m_pShareData->m_hwndTray = m_hWnd;

	return true;
}

/*!
	@brief �R���g���[���v���Z�X�̃��b�Z�[�W���[�v
	
	@author aroka
	@date 2002/01/07
*/
bool CControlProcess::MainLoop()
{
	if( NULL != m_pcEditApp && NULL != m_hWnd ){
		m_pcEditApp->MessageLoop();	/* ���b�Z�[�W���[�v */
		return true;
	}
	return false;
}

/*!
	@brief �R���g���[���v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
*/
void CControlProcess::Terminate()
{
	/* �I���_�C�A���O��\������ */
	HWND hwndExitingDlg;
	if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
		/* �I�����_�C�A���O�̕\�� */
		hwndExitingDlg = ::CreateDialog(
			m_hInstance,
			MAKEINTRESOURCE( IDD_EXITING ),
			/*m_hWnd*/::GetDesktopWindow(),
			(DLGPROC)CControlProcess::ExitingDlgProc
		);
		::ShowWindow( hwndExitingDlg, SW_SHOW );
	}

	/* ���L�f�[�^�̕ۑ� */
	m_cShareData.SaveShareData();

	/* �I���_�C�A���O��\������ */
	if( FALSE != m_pShareData->m_Common.m_bDispExitingDialog ){
		/* �I�����_�C�A���O�̔j�� */
		::DestroyWindow( hwndExitingDlg );
	}

	m_pShareData->m_hwndTray = NULL;
	/* �A�N�Z�����[�^�e�[�u���̍폜 */
	if( m_pShareData->m_hAccel != NULL ){
		::DestroyAcceleratorTable( m_pShareData->m_hAccel );
		m_pShareData->m_hAccel = NULL;
	}

}

CControlProcess::~CControlProcess()
{
	if( m_pcEditApp ){
		delete m_pcEditApp;
	}
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// ���o�[�W�����Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

/*[EOF]*/
