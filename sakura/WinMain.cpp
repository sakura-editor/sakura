//	$Id$
/*!	@file
	Entry Point
	
	@author Norio Nakatani
	@date	1998/03/13 �쐬
	@date	2001/06/26 genta ���[�h�P�ʂ�Grep�̂��߂̃R�}���h���C�������ǉ�
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <windows.h>
//#include <stdio.h>
//#include <io.h>
//#include <ras.h>
//#include <SHLWAPI.H>

//#include "sakura_rc.h"
//#include "CEditDoc.h"
//#include "debug.h"
//#include "CKeyBind.h"
//#include "CJre.h"
#include "CEditApp.h"
//#include "CEditWnd.h"
//#include "CShareData.h"
//#include "etc_uty.h"
//#include "global.h"
//#include "CProfile.h"
//#include "CRunningTimer.h"



BOOL CALLBACK ExitingDlgProc(
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


int WINAPI WinMain( HINSTANCE, HINSTANCE, LPSTR, int );

/////////////////////////////////////////////////////////////////////////


//struct VS_VERSION_INFO_HEAD {
//	WORD				wLength;
//	WORD				wValueLength;
//	WORD				bText;
//	WCHAR				szKey[16];
//	VS_FIXEDFILEINFO	Value;
//};
//char* m_pszAppName = "SakuraTrayClass";



/*!
	Windows Entry point
	
	1�ڂ̃G�f�B�^�v���Z�X�̏ꍇ�́A���̃v���Z�X�̓R���g���[���v���Z�X��
	�Ȃ�A�V�����G�f�B�^�v���Z�X���N������B�����łȂ��Ƃ��̓G�f�B�^�v���Z�X
	�ƂȂ�B
	
	�R���g���[���v���Z�X��CEditApp�N���X�̃C���X�^���X�����A�G�f�B�^
	�v���Z�X��CEditWnd�N���X�̃C���X�^���X�����B
*/
int WINAPI WinMain(
	HINSTANCE	hInstance,		//!< handle to current instance
	HINSTANCE	hPrevInstance,	//!< handle to previous instance
	LPSTR		lpCmdLine,		//!< pointer to command line
	int			nCmdShow		//!< show state of window
)
{

	
//	/* �e�X�g */
//	RASCONN			RasConn;
//	DWORD			dwStatus;
//	DWORD			dwSize;
//	DWORD			dwConnections;
//	RASCONNSTATUS	RasConnStatus;
//	RasConn.dwSize = sizeof( RASCONN );
//	dwSize = RasConn.dwSize;
//	/* RAS�ڑ��̊Ď� */
//	dwStatus = ::RasEnumConnections(
//	  &RasConn,			// buffer to receive connections data
//	  &dwSize,			// size in bytes of buffer
//	  &dwConnections	// number of connections written to buffer
//	);
//	if( 0 == dwStatus
//	 && NULL != RasConn.hrasconn
//	){
//		/* RAS�ڑ�����Ă��� */
//		RasConnStatus.dwSize = sizeof( RASCONNSTATUS );
//		::RasGetConnectStatus( RasConn.hrasconn, &RasConnStatus );
//		if( RASCS_Connected == RasConnStatus.rasconnstate
//		 && IDYES == ::MYMESSAGEBOX(
//			NULL,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			GSTR_APPNAME,
//			"%s  %s\n\nrime�����[�g�R���s���[�^�ɐڑ�����Ă��܂��B\n�ؒf���܂���?",
//			RasConn.szDeviceName, RasConn.szEntryName
//		)
//		){
//			/* RAS�ڑ���ؒf */
//			::RasHangUp( RasConn.hrasconn );
//
//		}
//	}




//	CRunningTimer	cRunningTimer;
//	CProfile		cProfile;

	HANDLE			hMutex;
//	HANDLE			hFileMap;
	CEditApp*		pcEditApp;
	CEditWnd*		pcEditWnd;
	HWND			hWnd;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	/* �R�}���h���C���I�v�V���� */
	BOOL			bGrepMode;
	CMemory			cmGrepKey;
	CMemory			cmGrepFile;
	CMemory			cmGrepFolder;
	CMemory			cmWork;
	BOOL			bGrepSubFolder;
	BOOL			bGrepLoHiCase;
	BOOL			bGrepRegularExp;
	BOOL			bGrepKanjiCode_AutoDetect;
	BOOL			bGrepOutputLine;
	BOOL			bGrepWordOnly;	//	Jun. 25, 2001 genta
	int				nGrepOutputStyle;
	BOOL			bDebugMode;
	BOOL			bNoWindow;
	FileInfo		fi;
	BOOL			bReadOnly;
	DWORD			dwRet;
	BOOL			bFindCTRLPROCESS;
//	BOOL			bInstanceAlreadyExist;

//	cRunningTimer.Reset();
//	cProfile.ReadProfile( "A:\\WINDOWS\\WIN.INI" );
//	MYTRACE( "A:\\WINDOWS\\WIN.INI  �ǂݍ��ݏ��� ���v����(�~���b) = %d\n", cRunningTimer.Read() );

//	cRunningTimer.Reset();
//	cProfile.WriteProfile( "a:\\tmp\\test.ini" );
//	MYTRACE( "a:\\tmp\\test.ini  �������ݏ��� ���v����(�~���b) = %d\n", cRunningTimer.Read() );


//	GetDllVersion( "Comctl32.dll" );
//	GetDllVersion( "Comdlg32.dll" );

	/* Windows�o�[�W�����̃`�F�b�N */
	if( FALSE == CheckWindowsVersion( GSTR_APPNAME ) ){
		return 0;
	}
	/* �V�X�e�����\�[�X�̃`�F�b�N */
	// Jul. 5, 2001 shoji masami NT�ł̓��\�[�X�`�F�b�N���s��Ȃ�
	if( !CheckWindowsVersionNT() ){
		if( !CheckSystemResources( GSTR_APPNAME ) ){
			return 0;
		}
	}

	/* �R�����R���g���[���̏����� */
	::InitCommonControls();

//	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	m_cShareData.Init();
//	m_pShareData = m_cShareData.GetShareData( NULL, NULL );


	/* �R�}���h���C���̉�� */
	CEditApp::ParseCommandLine(
		lpCmdLine,
		&bGrepMode,
		&cmGrepKey,
		&cmGrepFile,
		&cmGrepFolder,
		&bGrepSubFolder,
		&bGrepLoHiCase,
		&bGrepRegularExp,
		&bGrepKanjiCode_AutoDetect,
		&bGrepOutputLine,
		&bGrepWordOnly,	//	Jun. 25, 2001 genta
		&nGrepOutputStyle,
		&bDebugMode,
		&bNoWindow,
		&fi,
		&bReadOnly
	);

	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA );
	if( NULL == hMutex ){
		::MessageBeep( MB_ICONSTOP );
		::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "CreateMutex()���s�B\n�I�����܂��B" );
		return 0;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		/* �I�u�W�F�N�g���V�O�i����ԂɂȂ邩�A�܂��́A�^�C���A�E�g���Ԃ��o�߂���܂ő҂� */
		dwRet = ::WaitForSingleObject( hMutex, 20000 );
		if( WAIT_TIMEOUT == dwRet
		 || WAIT_ABANDONED == dwRet
		 || WAIT_FAILED == dwRet
		){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "�R���g���[���v���Z�X���������܂���B\n�I�����܂��B" );
			return 0;
		}
		::ReleaseMutex( hMutex );


		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
		if( !m_cShareData.Init() ){
			//	�K�؂ȃf�[�^�𓾂��Ȃ�����
			::MessageBox( NULL, "�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B", GSTR_APPNAME, MB_OK | MB_ICONERROR );
			return 0;
		}
		m_pShareData = m_cShareData.GetShareData( NULL, NULL );

		bFindCTRLPROCESS = TRUE;
		if( NULL == m_pShareData->m_hwndTray ){
			bFindCTRLPROCESS = FALSE;
		}else{
			if( FALSE == IsWindow( m_pShareData->m_hwndTray ) ){
				bFindCTRLPROCESS = FALSE;
			}else{
				char	szClassName[64];
				szClassName[0] = '\0';
				::GetClassName( m_pShareData->m_hwndTray, szClassName, sizeof( szClassName ) - 1 );
				if( 0 != strcmp( GSTR_CEDITAPP, szClassName ) ){
					bFindCTRLPROCESS = FALSE;
				}
			}
		}
		if( FALSE == bFindCTRLPROCESS ){
//����m�F�p	::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "�R���g���[���v���Z�X�����݂��Ă��܂���ł����B\n�V���ɃR���g���[���v���Z�X���N�����܂��B\n" );
//			bNoWindow = TRUE;
			goto CreateControlProcess;
//			return 0;
		}

		/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�� */
		/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
		if( 0 < strlen( fi.m_szPath ) ){
			//	Oct. 27, 2000 genta
			//	MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileRead��
			//	�s����̂ł����ł͕K�v�Ȃ��D
#if 0
			/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
			FileInfo fiWork;
			if( m_cShareData.IsExistInMRUList( (const char*)fi.m_szPath, &fiWork ) ){
//				MYTRACE( "MRU���X�g�ɑ��݂���[%s]\n", fi.m_szPath );
				if( 0 == fi.m_nX && 0 == fi.m_nY ){
					fi.m_nX = fiWork.m_nX;
					fi.m_nY = fiWork.m_nY;
				}
				if( 0 == fi.m_nViewTopLine && 0 == fi.m_nViewLeftCol ){
					fi.m_nViewTopLine = fiWork.m_nViewTopLine;
					fi.m_nViewLeftCol = fiWork.m_nViewLeftCol;
				}
				if( 99 == fi.m_nCharCode ){
					fi.m_nCharCode = fiWork.m_nCharCode;
				}

			}
#endif

			HWND hwndOwner;
			/* �w��t�@�C�����J����Ă��邩���ׂ� */
			if( TRUE == m_cShareData.IsPathOpened( fi.m_szPath, &hwndOwner ) ){
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( hwndOwner );
				return 0;
			}else{

			}
		}
//�}���`�X���b�h��
//
//		/* �V�����ҏW�E�B���h�E�̍쐬�˗�(�R�}���h���C����n��) */
//		::strcpy( pShareData->m_szWork, lpCmdLine );
//		hwndNew = (HWND)::SendMessage( pShareData->m_hwndTray, MYWM_OPENNEWEDITOR, 0, 0 );
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( hwndNew );


//�����v���Z�X��
		/* �G�f�B�^�E�B���h�E�I�u�W�F�N�g���쐬 */
		pcEditWnd = new CEditWnd;
		if( bDebugMode ){
			hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );

//	#ifdef _DEBUG/////////////////////////////////////////////
			/* �f�o�b�O���j�^���[�h�ɐݒ� */
			pcEditWnd->SetDebugModeON();
//	#endif////////////////////////////////////////////////////
		}else
		if( bGrepMode ){
			hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
			/* GREP */
			/*nHitCount = */

			char szWork[MAX_PATH];
			/* �����O�t�@�C�������擾���� */
			if( TRUE == ::GetLongFileName( cmGrepFolder.GetPtr( NULL ), szWork ) ){
				cmGrepFolder.SetData( szWork, strlen( szWork ) );
			}
			pcEditWnd->m_cEditDoc.m_cEditViewArr[0].DoGrep(
				&cmGrepKey,
				&cmGrepFile,
				&cmGrepFolder,
				bGrepSubFolder,
				bGrepLoHiCase,
				bGrepRegularExp,
				bGrepKanjiCode_AutoDetect,
				bGrepOutputLine,
				bGrepWordOnly,	//	Jun. 26, 2001 genta
				nGrepOutputStyle
			);
		}else{
			if( 0 < (int)strlen( fi.m_szPath ) ){
				hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, fi.m_szPath, fi.m_nCharCode, bReadOnly/* �ǂݎ���p�� */ );
				//	Nov. 6, 2000 genta
				//	�L�����b�g�ʒu�̕����̂���
				//	�I�v�V�����w�肪�Ȃ��Ƃ��͉�ʈړ����s��Ȃ��悤�ɂ���
				if( ( 0 <= fi.m_nViewTopLine || 0 <= fi.m_nViewLeftCol ) && fi.m_nViewTopLine < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewTopLine = fi.m_nViewTopLine;
					pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nViewLeftCol = fi.m_nViewLeftCol;
				}

				//	Nov. 6, 2000 genta
				//	�L�����b�g�ʒu�̕����̂���
				//	m_nCaretPosX_Prev�̗p�r�͕s�������C�����ݒ肵�Ȃ��̂͂܂����̂łƂ肠���� 0
				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
				//	�I�v�V�����w�肪�Ȃ��Ƃ��̓J�[�\���ʒu�ݒ���s��Ȃ��悤�ɂ���
				if( fi.m_nX > 0 || fi.m_nY > 0 ){
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  ��
					  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					*/
					int		nPosX;
					int		nPosY;
					pcEditWnd->m_cEditDoc.m_cLayoutMgr.CaretPos_Phys2Log(
						fi.m_nX,
						fi.m_nY,
						&nPosX,
						&nPosY
					);
					if( nPosY < pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() ){
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( nPosX, nPosY, TRUE );
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = nPosX;
					}else{
						int		i;
						i = pcEditWnd->m_cEditDoc.m_cLayoutMgr.GetLineCount() - 1;
						if( i < 0 ){
							i = 0;
						}
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].MoveCursor( 0, i, TRUE );
						pcEditWnd->m_cEditDoc.m_cEditViewArr[0].m_nCaretPosX_Prev = 0;
					}
				}
				pcEditWnd->m_cEditDoc.m_cEditViewArr[0].RedrawAll();
			}else{
				hWnd = pcEditWnd->Create( hInstance, m_pShareData->m_hwndTray, NULL, 0, FALSE );
			}
		}
		if( NULL != hWnd ){
			pcEditWnd->MessageLoop();	/* ���b�Z�[�W���[�v */
		}
		delete pcEditWnd;
	}else{
CreateControlProcess:;


		/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
		if( !m_cShareData.Init() ){
			//	�K�؂ȃf�[�^�𓾂��Ȃ�����
			::MessageBox( NULL, "�قȂ�o�[�W�����̃G�f�B�^�𓯎��ɋN�����邱�Ƃ͂ł��܂���B", GSTR_APPNAME, MB_OK | MB_ICONERROR );
			return 0;
		}
		m_pShareData = m_cShareData.GetShareData( NULL, NULL );

		/* ���\�[�X���琻�i�o�[�W�����̎擾 */
		GetAppVersionInfo( hInstance, VS_VERSION_INFO, &m_pShareData->m_dwProductVersionMS, &m_pShareData->m_dwProductVersionLS );
//		MYTRACE( "���i�o�[�W����=%d.%d.%d.%d\n",
//			HIWORD( m_pShareData->m_dwProductVersionMS ), LOWORD( m_pShareData->m_dwProductVersionMS ),
//			HIWORD( m_pShareData->m_dwProductVersionLS ), LOWORD( m_pShareData->m_dwProductVersionLS )
//		);

		/* ���L�f�[�^�̃��[�h */
		if( FALSE == m_cShareData.LoadShareData() ){
//			int	i;
			/*	���W�X�g������ݒ�f�[�^�����݂��Ȃ������ꍇ�͏���N�����Ƃ݂Ȃ��S���W�X�g���f�[�^������Ă��� */

//			/* �ύX�t���O(���ʐݒ�̑S��)�̃Z�b�g */
//			m_pShareData->m_nCommonModify = TRUE;
//
//			/* �ύX�t���O �t�H���g */
//			m_pShareData->m_bFontModify = TRUE;
//
//			m_pShareData->m_bKeyBindModify = TRUE;	/* �ύX�t���O �L�[�o�C���h */
//			for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//				m_pShareData->m_bKeyBindModifyArr[i] = TRUE;	/* �ύX�t���O �L�[�o�C���h(�L�[����) */
//			}
//			/* �ύX�t���O(����̑S��)�̃Z�b�g */
//			m_pShareData->m_bPrintSettingModify = TRUE;
//			/* �ύX�t���O(����ݒ育��)�̃Z�b�g */
//			for( i = 0; i < MAX_PRINTSETTINGARR; ++i ){
//				m_pShareData->m_bPrintSettingModifyArr[i] = TRUE;
//			}
//			m_pShareData->m_bKeyWordSetModify = TRUE;	/* �ύX�t���O(�L�[���[�h�̑S��) */
//			for( i = 0; i < MAX_SETNUM; ++i ){
//				m_pShareData->m_bKeyWordSetModifyArr[i] = TRUE;	/* �ύX�t���O(�L�[���[�h�̃Z�b�g����) */
//			}
//			/* �ύX�t���O(�^�C�v�ʐݒ�) �̃Z�b�g */
//			for( i = 0; i < MAX_TYPES; ++i ){
//				m_pShareData->m_nTypesModifyArr[i] = TRUE;
//			}

			/* ���W�X�g������ �쐬 */
			m_cShareData.SaveShareData();
		}

		/* �^�X�N�g���C�ɃA�C�R���쐬 */
		pcEditApp = new CEditApp;
		HWND	hwndTray;

		if( NULL == ( hwndTray = pcEditApp->Create( hInstance ) ) ){
			::MessageBeep( MB_ICONSTOP );
			::MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME, "�E�B���h�E�̍쐬�Ɏ��s���܂����B\n�N���ł��܂���B" );
			return 0;
		}
		m_pShareData->m_hwndTray = hwndTray;

		::ReleaseMutex( hMutex );

		/* ��̕ҏW�E�B���h�E���쐬 */
		if( !bNoWindow ){
			CEditApp::OpenNewEditor(
				hInstance,
				m_pShareData->m_hwndTray,
				lpCmdLine,
				//	May 30, 2000 genta
				fi.m_nCharCode,	/* �����R�[�h�͈����̐ݒ�������p�� */
				FALSE				/* �ǂݎ���p�� */
			);
		}
		pcEditApp->MessageLoop();

		dwRet = ::WaitForSingleObject( hMutex, 20000 );

		/* �I���_�C�A���O��\������ */
		HWND hwndExitingDlg;
		if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
			/* �I�����_�C�A���O�̕\�� */
			hwndExitingDlg = ::CreateDialog(
				hInstance,
				MAKEINTRESOURCE( IDD_EXITING ),
				/*m_hWnd*/::GetDesktopWindow(),
				(DLGPROC)ExitingDlgProc
			);
			::ShowWindow( hwndExitingDlg, SW_SHOW );
		}

		/* ���L�f�[�^�̕ۑ� */
		m_cShareData.SaveShareData();

		/* �I���_�C�A���O��\������ */
		if( TRUE == m_pShareData->m_Common.m_bDispExitingDialog ){
			/* �I�����_�C�A���O�̔j�� */
			::DestroyWindow( hwndExitingDlg );
		}
		
		m_pShareData->m_hwndTray = NULL;
		delete pcEditApp;
		/* �A�N�Z�����[�^�e�[�u���̍폜 */
		if( m_pShareData->m_hAccel != NULL ){
			::DestroyAcceleratorTable( m_pShareData->m_hAccel );
			m_pShareData->m_hAccel = NULL; 
		}

//		Sleep( 3000 );
		::ReleaseMutex( hMutex );
	}
	return 0;
}


/*[EOF]*/
