//	���̍s�͕��������΍��p�ł��D�����Ȃ��ł�������
/*!	@file
	@brief �v���Z�X�����N���X

	@author aroka
	@date 2002/01/03 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMain��蕪��
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CProcessFactory.h"
#include "CControlProcess.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CEditApp.h"
#include "Debug.h"
#include "etc_uty.h"
#include <tchar.h>
#include "COsVersionInfo.h"
#include "CRunningTimer.h"

class CProcess;


/*!
	@brief �v���Z�X�N���X�𐶐�����
	
	�R�}���h���C���A�R���g���[���v���Z�X�̗L���𔻒肵�A
	�K���ȃv���Z�X�N���X�𐶐�����B
	
	@param[in] hInstance �C���X�^���X�n���h��
	@param[in] lpCmdLine �R�}���h���C��������
	
	@author aroka
	@date 2002/01/08
	@date 2006/04/10 ryoji
*/
CProcess* CProcessFactory::Create( HINSTANCE hInstance, LPSTR lpCmdLine )
{
	CCommandLine::Instance(lpCmdLine);
	
	CProcess* process = 0;
	if( !IsValidVersion() ){
		return 0;
	}

	// �v���Z�X�N���X�𐶐�����
	//
	// Note: �ȉ��̏����ɂ����Ďg�p����� IsExistControlProcess() �́A�R���g���[���v���Z�X��
	// ���݂��Ȃ��ꍇ�����łȂ��A�R���g���[���v���Z�X���N������ ::CreateMutex() �����s����܂�
	// �̊Ԃ� false�i�R���g���[���v���Z�X�����j��Ԃ��B
	// �]���āA�����̃m�[�}���v���Z�X�������ɋN�������ꍇ�Ȃǂ͕����̃R���g���[���v���Z�X��
	// �N������邱�Ƃ�����B
	// �������A���̂悤�ȏꍇ�ł��~���[�e�b�N�X���ŏ��Ɋm�ۂ����R���g���[���v���Z�X���B�ꐶ���c��B
	//
	if( IsStartingControlProcess() ){
		if( !IsExistControlProcess() ){
			process = new CControlProcess( hInstance, lpCmdLine );
		}
	}else{
		if( !IsExistControlProcess() ){
			StartControlProcess();
		}
		if( WaitForInitializedControlProcess() ){	// 2006.04.10 ryoji �R���g���[���v���Z�X�̏����������҂�
			process = new CNormalProcess( hInstance, lpCmdLine );
		}
	}
	return process;
}


/*!
	@brief Windows�o�[�W�����̃`�F�b�N
	
	Windows 95�ȏ�CWindows NT4.0�ȏ�ł��邱�Ƃ��m�F����D
	Windows 95�n�ł͎c�胊�\�[�X�̃`�F�b�N���s���D
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsValidVersion()
{
	/* Windows�o�[�W�����̃`�F�b�N */
	COsVersionInfo	cOsVer;
	if( cOsVer.GetVersion() ){
		if( !cOsVer.OsIsEnableVersion() ){
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
				"���̃A�v���P�[�V���������s����ɂ́A\nWindows95�ȏ� �܂��� WindowsNT4.0�ȏ��OS���K�v�ł��B\n�A�v���P�[�V�������I�����܂��B"
			);
			return false;
		}
	}else{
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
			"OS�̃o�[�W�������擾�ł��܂���B\n�A�v���P�[�V�������I�����܂��B"
		);
		return false;
	}

	/* �V�X�e�����\�[�X�̃`�F�b�N */
	// Jul. 5, 2001 shoji masami NT�ł̓��\�[�X�`�F�b�N���s��Ȃ�
	if( !cOsVer.IsWin32NT() ){
		if( !CheckSystemResources( GSTR_APPNAME ) ){
			return false;
		}
	}
	return true;
}


/*!
	@brief �R�}���h���C���� -NOWIN �����邩�𔻒肷��B
	
	@author aroka
	@date 2002/01/03 �쐬 2002/01/18 �ύX
*/
bool CProcessFactory::IsStartingControlProcess()
{
	return CCommandLine::Instance()->IsNoWindow();
}

/*!
	�R���g���[���v���Z�X�̗L���𒲂ׂ�B
	
	@author aroka
	@date 2002/01/03
	@date 2006/04/10 ryoji
*/
bool CProcessFactory::IsExistControlProcess()
{
 	HANDLE hMutexCP;
	hMutexCP = ::OpenMutex( MUTEX_ALL_ACCESS, FALSE, GSTR_MUTEX_SAKURA_CP );	// 2006.04.10 ryoji ::CreateMutex() �� ::OpenMutex()�ɕύX
	if( NULL != hMutexCP ){
		::CloseHandle( hMutexCP );
		return true;	// �R���g���[���v���Z�X����������
	}

	return false;	// �R���g���[���v���Z�X�͑��݂��Ă��Ȃ����A�܂� CreateMutex() ���ĂȂ�
}

//	From Here Aug. 28, 2001 genta
/*!
	@brief �R���g���[���v���Z�X���N������
	
	�������g�� -NOWIN �I�v�V������t���ċN������D
	���L���������`�F�b�N���Ă͂����Ȃ��̂ŁC�c�O�Ȃ���CEditApp::OpenNewEditor�͎g���Ȃ��D
	
	@author genta
	@date Aug. 28, 2001
*/
bool CProcessFactory::StartControlProcess()
{
	MY_RUNNINGTIMER(cRunningTimer,"StartControlProcess" );

	//	�v���Z�X�̋N��
	PROCESS_INFORMATION p;
	STARTUPINFO s;

	s.cb = sizeof( s );
	s.lpReserved = NULL;
	s.lpDesktop = NULL;
	s.lpTitle = NULL;

	s.dwFlags = STARTF_USESHOWWINDOW;
	s.wShowWindow = SW_SHOWDEFAULT;
	s.cbReserved2 = 0;
	s.lpReserved2 = NULL;

	TCHAR szCmdLineBuf[1024];	//	�R�}���h���C��
	TCHAR szEXE[MAX_PATH + 1];	//	�A�v���P�[�V�����p�X��
	TCHAR szDir[MAX_PATH + 1];	//	�f�B���N�g���p�X��

	::GetModuleFileName( ::GetModuleHandle( NULL ), szEXE, sizeof( szEXE ));
	::wsprintf( szCmdLineBuf, _T("%s -NOWIN"), szEXE );
	::GetSystemDirectory( szDir, sizeof( szDir ));

	if( 0 == ::CreateProcess( szEXE, szCmdLineBuf, NULL, NULL, FALSE,
		CREATE_DEFAULT_ERROR_MODE, NULL, szDir, &s, &p ) ){
		//	���s
		LPVOID pMsg;
		::FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_IGNORE_INSERTS |
						FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						::GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPTSTR) &pMsg,
						0,
						NULL
		);
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%s\'\n�v���Z�X�̋N���Ɏ��s���܂����B\n%s"), szEXE, (LPTSTR)pMsg );
		::LocalFree( (HLOCAL)pMsg );	//	�G���[���b�Z�[�W�o�b�t�@�����
		return false;
	}

	// �N�������v���Z�X�����S�ɗ����オ��܂ł�����Ƒ҂D
	//
	// Note: ���̑҂��ɂ��A�����ŋN�������R���g���[���v���Z�X�������ɐ����c��Ȃ������ꍇ�ł��A
	// �B�ꐶ���c�����R���g���[���v���Z�X�����d�N���h�~�p�~���[�e�b�N�X���쐬���Ă���͂��B
	//
	int nResult = ::WaitForInputIdle( p.hProcess, 10000 );	//	�ő�10�b�ԑ҂�
	if( 0 != nResult ){
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			_T("\'%s\'\n�R���g���[���v���Z�X�̋N���Ɏ��s���܂����B"), szEXE );
		::CloseHandle( p.hThread );
		::CloseHandle( p.hProcess );
		return false;
	}

	::CloseHandle( p.hThread );
	::CloseHandle( p.hProcess );
	
	return true;
}
//	To Here Aug. 28, 2001 genta

/*!
	@brief �R���g���[���v���Z�X�̏����������C�x���g��҂B

	@author ryoji by assitance with karoto
	@date 2006/04/10
*/
bool CProcessFactory::WaitForInitializedControlProcess()
{
	// �����������C�x���g��҂�
	//
	// Note: �R���g���[���v���Z�X���͑��d�N���h�~�p�~���[�e�b�N�X�� ::CreateMutex() ��
	// �쐬���������ɏ����������C�x���g�� ::CreateEvent() �ō쐬����B
	//
	if( !IsExistControlProcess() ){
		// �R���g���[���v���Z�X�����d�N���h�~�p�̃~���[�e�b�N�X�쐬�O�Ɉُ�I�������ꍇ�Ȃ�
		return false;
	}

	HANDLE hEvent;
	hEvent = ::OpenEvent( EVENT_ALL_ACCESS, FALSE, GSTR_EVENT_SAKURA_CP_INITIALIZED );
	if( NULL == hEvent ){
		// ���쒆�̃R���g���[���v���Z�X�����o�[�W�����Ƃ݂Ȃ��A�C�x���g��҂����ɏ�����i�߂�
		//
		// Note: Ver1.5.9.91�ȑO�̃o�[�W�����͏����������C�x���g�����Ȃ��B
		// ���̂��߁A�R���g���[���v���Z�X���풓���Ă��Ȃ��Ƃ��ɕ����E�B���h�E���ق�
		// �����ɋN������ƁA�����ɐ����c��Ȃ������R���g���[���v���Z�X�̐e�v���Z�X��A
		// �͂��ɏo�x��ăR���g���[���v���Z�X���쐬���Ȃ������v���Z�X�ł��A
		// �R���g���[���v���Z�X�̏�����������ǂ��z���Ă��܂��A�ُ�I��������A
		// �u�^�u�o�[���\������Ȃ��v�̂悤�Ȗ�肪�������Ă����B
		//
		return true;
	}
	DWORD dwRet = ::WaitForSingleObject( hEvent, 10000 );	// �ő�10�b�ԑ҂�
	if( WAIT_TIMEOUT == dwRet ){	// �R���g���[���v���Z�X�̏��������I�����Ȃ�
		::CloseHandle( hEvent );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B") );
		return false;
	}
	::CloseHandle( hEvent );
	return true;
}


/*[EOF]*/
