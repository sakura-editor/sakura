//	$Id$
/*!	@file
	@brief �v���Z�X�����N���X

	@author aroka
	@date 2002/01/03 Create
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, masami shoji
	Copyright (C) 2002, aroka WinMain��蕪��

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

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
	
	@author aroka
	@date 2002/01/08
*/
CProcess* CProcessFactory::Create( HINSTANCE hInstance, LPSTR lpCmdLine )
{
	CCommandLine::Instance(lpCmdLine);
	
	CProcess* process = 0;
	if( !IsValidVersion() ){
		return 0;
	}
	if( IsStartingControlProcess() ){
		if( !IsExistControlProcess() ){
			process = new CControlProcess( hInstance, lpCmdLine );
		}
	}else{
		if( !IsExistControlProcess() ){
			StartControlProcess();
		}
		if( IsExistControlProcess() ){
			process = new CNormalProcess( hInstance, lpCmdLine );
		}
	}
	return process;
}


/*!
	@brief Windows�o�[�W�����̃`�F�b�N
	
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
	�~���[�e�b�N�X���擾���A�R���g���[���v���Z�X�̗L���𒲂ׂ�B
	
	@author aroka
	@date 2002/01/03
*/
bool CProcessFactory::IsExistControlProcess()
{
	HANDLE hMutexCP;
	hMutexCP = ::CreateMutex( NULL, FALSE, GSTR_MUTEX_SAKURA_CP );
	if( NULL == hMutexCP ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			_T("CreateMutex()���s�B\n�I�����܂��B"));
		::ExitProcess(1);
	}
	if( ERROR_ALREADY_EXISTS == ::GetLastError() ){
		DWORD dwRet = ::WaitForSingleObject( hMutexCP, 0 );
		if( WAIT_TIMEOUT == dwRet ){// ���邯�ǃ��b�N����Ă�
			::CloseHandle( hMutexCP );
			return true;
		}
		// ���b�N����Ă��Ȃ�����
		::ReleaseMutex( hMutexCP );
		::CloseHandle( hMutexCP );
		return false;
	}
	// �Ȃ�����
	::CloseHandle( hMutexCP );
	return false;
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

	//	�N�������v���Z�X�����S�ɗ����オ��܂ł�����Ƒ҂D
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


/*[EOF]*/
