/*!	@file
	@brief �R���g���[���v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka CProcess��蕪��, YAZAKI
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CControlProcess.h"
#include "CCommandLine.h"
#include "env/CShareData.h"
#include "debug/Debug.h"
#include "CControlTray.h"
#include "mem/CMemory.h"
#include "sakura_rc.h"/// IDD_EXITTING 2002/2/10 aroka �w�b�_����
#include <io.h>
#include <tchar.h>
#include "debug/CRunningTimer.h"
#include "env/CShareData_IO.h"


//-------------------------------------------------


/*!
	@brief �R���g���[���v���Z�X������������
	
	MutexCP���쐬�E���b�N����B
	CControlTray���쐬����B
	
	@author aroka
	@date 2002/01/07
	@date 2002/02/17 YAZAKI ���L������������������̂�CProcess�Ɉړ��B
	@date 2006/04/10 ryoji �����������C�x���g�̏�����ǉ��A�ُ펞�̌�n���̓f�X�g���N�^�ɔC����
*/
bool CControlProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "CControlProcess::InitializeProcess" );

	// ���o�[�W�����i1.2.104.1�ȑO�j�Ƃ̌݊����F�u�قȂ�o�[�W����...�v�����o�Ȃ��悤��
	m_hMutex = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA_OLD );
	if( NULL == m_hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}

	// �����������C�x���g���쐬����
	m_hEventCPInitialized = ::CreateEvent( NULL, TRUE, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if( NULL == m_hEventCPInitialized )
	{
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateEvent()���s�B\n�I�����܂��B") );
		return false;
	}

	/* �R���g���[���v���Z�X�̖ڈ� */
	m_hMutexCP = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == m_hMutexCP ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return false;
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		return false;
	}
	
	/* ���L�������������� */
	if( !CProcess::InitializeProcess() ){
		return false;
	}

	/* ���L�f�[�^�̃��[�h */
	// 2007.05.19 ryoji �u�ݒ��ۑ����ďI������v�I�v�V���������isakuext�A�g�p�j��ǉ�
	TCHAR szIniFile[_MAX_PATH];
	CShareData_IO::LoadShareData();
	CFileNameManager::Instance()->GetIniFileName( szIniFile );	// �o��ini�t�@�C����
	if( !fexist(szIniFile) || CCommandLine::Instance()->IsWriteQuit() ){
		/* ���W�X�g������ �쐬 */
		CShareData_IO::SaveShareData();
		if( CCommandLine::Instance()->IsWriteQuit() ){
			return false;
		}
	}

	MY_TRACETIME( cRunningTimer, "Before new CControlTray" );

	/* �^�X�N�g���C�ɃA�C�R���쐬 */
	m_pcTray = new CControlTray;

	MY_TRACETIME( cRunningTimer, "After new CControlTray" );

	HWND hwnd = m_pcTray->Create( GetProcessInstance() );
	if( !hwnd ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("�E�B���h�E�̍쐬�Ɏ��s���܂����B\n�N���ł��܂���B") );
		return false;
	}
	SetMainWindow(hwnd);
	GetDllShareData().m_sHandles.m_hwndTray = hwnd;

	// �����������C�x���g���V�O�i����Ԃɂ���
	if( !::SetEvent( m_hEventCPInitialized ) ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("SetEvent()���s�B\n�I�����܂��B") );
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
	if( m_pcTray && GetMainWindow() ){
		m_pcTray->MessageLoop();	/* ���b�Z�[�W���[�v */
		return true;
	}
	return false;
}

/*!
	@brief �R���g���[���v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	@date 2006/07/02 ryoji ���L�f�[�^�ۑ��� CControlTray �ֈړ�
*/
void CControlProcess::OnExitProcess()
{
	GetDllShareData().m_sHandles.m_hwndTray = NULL;
}

CControlProcess::~CControlProcess()
{
	if( m_pcTray ){
		delete m_pcTray;
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


