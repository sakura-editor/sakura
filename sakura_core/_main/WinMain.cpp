/*!	@file
	@brief Entry Point

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date	2001/06/26 genta ���[�h�P�ʂ�Grep�̂��߂̃R�}���h���C�������ǉ�
	@date	2002/01/08 aroka �����̗���𐮗��A���g�p�R�[�h���폜
	@date	2002/01/18 aroka ����聕�����[�X
	@date	2009/01/07 ryoji WinMain��OleInitialize/OleUninitialize��ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta
	Copyright (C) 2002, aroka
	Copyright (C) 2007, kobake
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <Ole2.h>
#include "CProcessFactory.h"
#include "CProcess.h"
#include "util/os.h"
#include "util/module.h"
#include "debug/CRunningTimer.h"

/*!
	Windows Entry point

	1�ڂ̃G�f�B�^�v���Z�X�̏ꍇ�́A���̃v���Z�X�̓R���g���[���v���Z�X��
	�Ȃ�A�V�����G�f�B�^�v���Z�X���N������B�����łȂ��Ƃ��̓G�f�B�^�v���Z�X
	�ƂȂ�B

	�R���g���[���v���Z�X��CControlProcess�N���X�̃C���X�^���X�����A
	�G�f�B�^�v���Z�X��CNormalProcess�N���X�̃C���X�^���X�����B
*/
#ifdef __MINGW32__
int WINAPI WinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPSTR		lpCmdLineA,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
#else
int WINAPI _tWinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPTSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
#endif
{
#ifdef USE_LEAK_CHECK_WITH_CRTDBG
	// 2009.9.10 syat ���������[�N�`�F�b�N��ǉ�
	::_CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF);
#endif

	MY_RUNNINGTIMER(cRunningTimer, "WinMain" );
	{
		// 2010.08.28 Moca DLL�C���W�F�N�V�����΍�
		CCurrentDirectoryBackupPoint dirBack;
		ChangeCurrentDirectoryToExeDir();

		setlocale( LC_ALL, "Japanese" ); //2007.08.16 kobake �ǉ�
		::OleInitialize( NULL );	// 2009.01.07 ryoji �ǉ�
	}
	
	//�J�����
	DEBUG_TRACE(_T("-- -- WinMain -- --\n"));
	DEBUG_TRACE(_T("sizeof(DLLSHAREDATA) = %d\n"),sizeof(DLLSHAREDATA));

	//�v���Z�X�̐����ƃ��b�Z�[�W���[�v
	CProcessFactory aFactory;
	CProcess *process = 0;
	try{
#ifdef __MINGW32__
		LPTSTR pszCommandLine;
		pszCommandLine = ::GetCommandLine();
		// ���s�t�@�C�������X�L�b�v����
		if( _T('\"') == *pszCommandLine ){
			pszCommandLine++;
			while( _T('\"') != *pszCommandLine && _T('\0') != *pszCommandLine ){
				pszCommandLine++;
			}
			if( _T('\"') == *pszCommandLine ){
				pszCommandLine++;
			}
		}else{
			while( _T(' ') != *pszCommandLine && _T('\t') != *pszCommandLine
				&& _T('\0') != *pszCommandLine ){
				pszCommandLine++;
			}
		}
		// ���̃g�[�N���܂Ői�߂�
		while( _T(' ') == *pszCommandLine || _T('\t') == *pszCommandLine ){
			pszCommandLine++;
		}
		process = aFactory.Create( hInstance, pszCommandLine );
#else
		process = aFactory.Create( hInstance, lpCmdLine );
#endif
		MY_TRACETIME( cRunningTimer, "ProcessObject Created" );
	}
	catch(...){
	}
	if( 0 != process ){
		process->Run();
		delete process;
	}

	::OleUninitialize();	// 2009.01.07 ryoji �ǉ�
	return 0;
}


