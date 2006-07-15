/*!	@file
	@brief �R���g���[���v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcess��蕪��, YAZAKI
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
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



//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	CEditApp���쐬����B
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI ���L������������������̂�CProcess�Ɉړ��B
	@date 2006/04/10 ryoji �����������C�x���g�̏�����ǉ��A�ُ펞�̌�n���̓f�X�g���N�^�ɔC����
*/
bool CControlProcess::Initialize()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::Initialize" );

	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA );
	if( NULL == m_hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}

	// �����������C�x���g���쐬����
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if( NULL == m_hEventCPInitialized )
	{
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateEvent()���s�B\n�I�����܂��B") );
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
		return false;
	}
	
	/* ���L�������������� */
	if ( CProcess::Initialize() == false ){
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
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME, _T("�E�B���h�E�̍쐬�Ɏ��s���܂����B\n�N���ł��܂���B") );
		return false;
	}
	m_pShareData->m_hwndTray = m_hWnd;

	// �����������C�x���g���V�O�i����Ԃɂ���
	if( !::SetEvent( m_hEventCPInitialized ) ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("SetEvent()���s�B\n�I�����܂��B") );
		return false;
	}

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
	@date 2006/07/02 ryoji ���L�f�[�^�ۑ��� CEditApp �ֈړ�
*/
void CControlProcess::Terminate()
{
	m_pShareData->m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	if( m_pcEditApp ){
		delete m_pcEditApp;
	}
	if( m_hEventCPInitialized ){
		::ResetEvent( m_hEventCPInitialized );
	}
	::CloseHandle( m_hEventCPInitialized );
	if( m_hMutexCP ){
		::ReleaseMutex( m_hMutexCP );
	}
	::CloseHandle( m_hMutexCP );
	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	if( m_hMutex ){
		::ReleaseMutex( m_hMutex );
	}
	::CloseHandle( m_hMutex );
};

/*[EOF]*/
