/*!	@file
	@brief �v���Z�X���N���X

	@author aroka
	@date 2002/01/07 �쐬
	@date 2002/01/17 �C��
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬
	Copyright (C) 2004, Moca
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CProcess.h"
#include "debug/Debug.h"
#include "util/module.h"

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
, m_pfnMiniDumpWriteDump(NULL)
{
}

/*!
	@brief �v���Z�X������������

	���L������������������
*/
bool CProcess::InitializeProcess()
{
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	if( !m_cShareData.InitShareData() ){
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
		HMODULE hDllDbgHelp = LoadLibraryExedir( _T("dbghelp.dll") );
		m_pfnMiniDumpWriteDump = NULL;
		if( hDllDbgHelp ){
			*(FARPROC*)&m_pfnMiniDumpWriteDump = ::GetProcAddress( hDllDbgHelp, "MiniDumpWriteDump" );
		}

		__try {
			MainLoop() ;
			OnExitProcess();
		}
		__except( WriteDump( GetExceptionInformation() ) ){
		}

		if( hDllDbgHelp ){
			::FreeLibrary( hDllDbgHelp );
			m_pfnMiniDumpWriteDump = NULL;
		}
		return true;
	}
	return false;
}

/*!
	@brief �N���b�V���_���v
	
	@author ryoji
	@date 2009.01.21
*/
int CProcess::WriteDump( PEXCEPTION_POINTERS pExceptPtrs )
{
	if( !m_pfnMiniDumpWriteDump )
		return EXCEPTION_CONTINUE_SEARCH;

	static TCHAR szFile[MAX_PATH];
	GetInidirOrExedir( szFile, _APP_NAME_(_T) _T(".dmp") );	// �o�͐��ini�Ɠ����iInitializeProcess()��Ɋm��j

	HANDLE hFile = ::CreateFile(
		szFile,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
		NULL);

	if( hFile != INVALID_HANDLE_VALUE ){
		MINIDUMP_EXCEPTION_INFORMATION eInfo;
		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = pExceptPtrs;
		eInfo.ClientPointers = FALSE;

		m_pfnMiniDumpWriteDump(
			::GetCurrentProcess(),
			::GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,
			pExceptPtrs ? &eInfo : NULL,
			NULL,
			NULL);

		::CloseHandle(hFile);
	}

	return EXCEPTION_CONTINUE_SEARCH;
}
