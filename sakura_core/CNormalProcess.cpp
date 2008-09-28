/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X

	@author aroka
	@date 2002/01/07 Create
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2002, aroka CProcess��蕪��
	Copyright (C) 2002, YAZAKI, Moca, genta
	Copyright (C) 2003, genta, Moca, MIK
	Copyright (C) 2004, Moca, naoh
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "stdafx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "env/CShareData.h"
#include "debug/Debug.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "doc/CDocLine.h" // 2003/03/28 MIK
#include <tchar.h>
#include "debug/CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CNormalProcess::CNormalProcess( HINSTANCE hInstance, LPTSTR lpCmdLine )
: m_pcEditApp( NULL )
, CProcess( hInstance, lpCmdLine )
{
}

CNormalProcess::~CNormalProcess()
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     �v���Z�X�n���h��                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief �G�f�B�^�v���Z�X������������
	
	CEditWnd���쐬����B
	
	@author aroka
	@date 2002/01/07

	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@date 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ��false��Ԃ��悤�ɁD
	@date 2007.06.26 ryoji �O���[�vID���w�肵�ĕҏW�E�B���h�E���쐬����
*/
bool CNormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::Init" );

	/* �v���Z�X�������̖ڈ� */
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka ���ݓ����Ă����̂ŕ���
	if( NULL == hMutex ){
		return false;
	}

	if ( !CProcess::InitializeProcess() ){
		return false;
	}

	/* �R�}���h���C���I�v�V���� */
	bool			bViewMode;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	EditInfo		fi;
	
	/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�� */
	/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
	CCommandLine::Instance()->GetEditInfo(&fi); // 2002/2/8 aroka �����Ɉړ�
	if( 0 < _tcslen( fi.m_szPath ) ){
		//	Oct. 27, 2000 genta
		//	MRU����J�[�\���ʒu�𕜌����鑀���CEditDoc::FileLoad��
		//	�s����̂ł����ł͕K�v�Ȃ��D

		HWND hwndOwner;
		/* �w��t�@�C�����J����Ă��邩���ׂ� */
		// 2007.03.13 maru �����R�[�h���قȂ�Ƃ��̓��[�j���O���o���悤��
		if( GetShareData().ActiveAlreadyOpenedWindow( fi.m_szPath, &hwndOwner, fi.m_nCharCode ) ){
			//	From Here Oct. 19, 2001 genta
			//	�J�[�\���ʒu�������Ɏw�肳��Ă�����w��ʒu�ɃW�����v
			if( fi.m_ptCursor.y >= 0 ){	//	�s�̎w�肪���邩
				CLogicPoint& pt = *CProcess::GetDllShareData().m_sWorkBuffer.GetWorkBuffer<CLogicPoint>();
				if( fi.m_ptCursor.x < 0 ){
					//	���̎w�肪�����ꍇ
					::SendMessageAny( hwndOwner, MYWM_GETCARETPOS, 0, 0 );
				}
				else {
					pt.x = fi.m_ptCursor.x;
				}
				pt.y = fi.m_ptCursor.y;
				::SendMessageAny( hwndOwner, MYWM_SETCARETPOS, 0, 0 );
			}
			//	To Here Oct. 19, 2001 genta
			/* �A�N�e�B�u�ɂ��� */
			ActivateFrameWindow( hwndOwner );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			return false;
		}
	}


	// �G�f�B�^�A�v���P�[�V�������쐬�B2007.10.23 kobake
	m_pcEditApp = new CEditApp(GetProcessInstance());
	CEditWnd* pEditWnd = m_pcEditApp->GetWindow();

	/* �R�}���h���C���̉�� */	 // 2002/2/8 aroka �����Ɉړ�
	bDebugMode = CCommandLine::Instance()->IsDebugMode();
	bGrepMode  = CCommandLine::Instance()->IsGrepMode();
	bGrepDlg   = CCommandLine::Instance()->IsGrepDlg();

	MY_TRACETIME( cRunningTimer, "CheckFile" );
	if( bDebugMode ){
		/* �f�o�b�O���j�^���[�h�ɐݒ� */
		CAppMode::Instance()->SetDebugModeON();
		// 2004.09.20 naoh �A�E�g�v�b�g�p�^�C�v�ʐݒ�
		pEditWnd->GetDocument().m_cDocType.SetDocumentType( CDocTypeManager().GetDocumentTypeOfExt(_T("output")), true );
		// �����R�[�h��L���Ƃ��� Uchi 2008/6/8
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, CTypeConfig(-1));
	}
	else if( bGrepMode ){
		// 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ�̍l����ǉ�
		if( NULL == pEditWnd->GetHwnd() ){
			goto end_of_func;
		}
		/* GREP */
		CCommandLine::Instance()->GetGrepInfo(&gi); // 2002/2/8 aroka �����Ɉړ�
		if( !bGrepDlg ){
			TCHAR szWork[MAX_PATH];
			/* �����O�t�@�C�������擾���� */
			if( ::GetLongFileName( gi.cmGrepFolder.GetStringPtr(), szWork ) ){
				gi.cmGrepFolder.SetString( szWork, _tcslen( szWork ) );
			}
			// 2003.06.23 Moca GREP���s�O��Mutex���J��
			//	�������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			SetMainWindow( pEditWnd->GetHwnd() );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			this->m_pcEditApp->m_pcGrepAgent->DoGrep(
				pEditWnd->m_pcEditViewArr[0],
				&gi.cmGrepKey,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepSubFolder,
				gi.sGrepSearchOption,
				gi.nGrepCharSet,	//	2002/09/21 Moca
				gi.bGrepOutputLine,
				gi.nGrepOutputStyle
			);
			return true; // 2003.06.23 Moca
		}
		else{
			//-GREPDLG�Ń_�C�A���O���o���B�@���������f�i2002/03/24 YAZAKI�j
			CSearchKeywordManager().AddToSearchKeyArr( gi.cmGrepKey.GetStringPtr() );
			CSearchKeywordManager().AddToGrepFileArr( gi.cmGrepFile.GetStringPtr() );
			CSearchKeywordManager().AddToGrepFolderArr( gi.cmGrepFolder.GetStringPtr() );
			GetDllShareData().m_Common.m_sSearch.m_bGrepSubFolder = gi.bGrepSubFolder;
			GetDllShareData().m_Common.m_sSearch.m_sSearchOption = gi.sGrepSearchOption;
			GetDllShareData().m_Common.m_sSearch.m_nGrepCharSet = gi.nGrepCharSet;
			GetDllShareData().m_Common.m_sSearch.m_bGrepOutputLine = gi.bGrepOutputLine;
			GetDllShareData().m_Common.m_sSearch.m_nGrepOutputStyle = gi.nGrepOutputStyle;
			// 2003.06.23 Moca GREP�_�C�A���O�\���O��Mutex���J��
			//	�������Ȃ���Grep���I���܂ŐV�����E�B���h�E���J���Ȃ�
			SetMainWindow( pEditWnd->GetHwnd() );
			::ReleaseMutex( hMutex );
			::CloseHandle( hMutex );
			
			//	Oct. 9, 2003 genta �R�}���h���C������GERP�_�C�A���O��\���������ꍇ��
			//	�����̐ݒ肪BOX�ɔ��f����Ȃ�
			wcscpy( pEditWnd->m_cDlgGrep.m_szText, gi.cmGrepKey.GetStringPtr() );		/* ���������� */
			_tcscpy( pEditWnd->m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr() );	/* �����t�@�C�� */
			_tcscpy( pEditWnd->m_cDlgGrep.m_szFolder, gi.cmGrepFolder.GetStringPtr() );	/* �����t�H���_ */

			
			// Feb. 23, 2003 Moca Owner window���������w�肳��Ă��Ȃ�����
			int nRet = pEditWnd->m_cDlgGrep.DoModal( GetProcessInstance(), pEditWnd->GetHwnd(),  NULL);
			if( FALSE != nRet ){
				pEditWnd->m_pcEditViewArr[0]->GetCommander().HandleCommand(F_GREP, TRUE, 0, 0, 0, 0);
			}
			return true; // 2003.06.23 Moca
		}
	}
	else{
		// 2004.05.13 Moca �����if���̒�����O�Ɉړ�
		// �t�@�C�������^�����Ȃ��Ă�ReadOnly�w���L���ɂ��邽�߁D
		bViewMode = CCommandLine::Instance()->IsViewMode(); // 2002/2/8 aroka �����Ɉړ�
		if( 0 < _tcslen( fi.m_szPath ) ){
			//	Mar. 9, 2002 genta �����^�C�v�w��
			pEditWnd->OpenDocumentWhenStart(
				SLoadInfo(
					fi.m_szPath,
					fi.m_nCharCode,
					bViewMode
				)
			);
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode, // �r���[���[�h��
				fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt( fi.m_szDocType )
			);
			// 2004.05.13 Moca CEditWnd::Create()�Ɏ��s�����ꍇ�̍l����ǉ�
			if( NULL == pEditWnd->GetHwnd() ){
				goto end_of_func;
			}
			//	Nov. 6, 2000 genta
			//	�L�����b�g�ʒu�̕����̂���
			//	�I�v�V�����w�肪�Ȃ��Ƃ��͉�ʈړ����s��Ȃ��悤�ɂ���
			//	Oct. 19, 2001 genta
			//	���ݒ聁-1�ɂȂ�悤�ɂ����̂ŁC���S�̂��ߗ��҂��w�肳�ꂽ�Ƃ�����
			//	�ړ�����悤�ɂ���D || �� &&
			if( ( CLayoutInt(0) <= fi.m_nViewTopLine && CLayoutInt(0) <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < pEditWnd->GetDocument().m_cLayoutMgr.GetLineCount() ){
				pEditWnd->m_pcEditViewArr[0]->GetTextArea().SetViewTopLine( fi.m_nViewTopLine );
				pEditWnd->m_pcEditViewArr[0]->GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol );
			}

			//	�I�v�V�����w�肪�Ȃ��Ƃ��̓J�[�\���ʒu�ݒ���s��Ȃ��悤�ɂ���
			//	Oct. 19, 2001 genta
			//	0���ʒu�Ƃ��Ă͗L���Ȓl�Ȃ̂Ŕ���Ɋ܂߂Ȃ��Ă͂Ȃ�Ȃ�
			if( 0 <= fi.m_ptCursor.x || 0 <= fi.m_ptCursor.y ){
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				CLayoutPoint ptPos;
				pEditWnd->GetDocument().m_cLayoutMgr.LogicToLayout(
					fi.m_ptCursor,
					&ptPos
				);

				// From Here Mar. 28, 2003 MIK
				// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
				// 2008.08.20 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
				const CDocLine *pTmpDocLine = pEditWnd->GetDocument().m_cDocLineMgr.GetLine( fi.m_ptCursor.GetY2() );
				if( pTmpDocLine ){
					if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) ptPos.x--;
				}
				// To Here Mar. 28, 2003 MIK

				pEditWnd->m_pcEditViewArr[0]->GetCaret().MoveCursor( ptPos, TRUE );
				pEditWnd->m_pcEditViewArr[0]->GetCaret().m_nCaretPosX_Prev =
					pEditWnd->m_pcEditViewArr[0]->GetCaret().GetCaretLayoutPos().GetX2();
			}
			pEditWnd->m_pcEditViewArr[0]->RedrawAll();
		}
		else{
			// 2004.05.13 Moca �t�@�C�������^�����Ȃ��Ă�ReadOnly�ƃ^�C�v�w���L���ɂ���
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode,	// �r���[���[�h��
				fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt( fi.m_szDocType )
			);
		}
	}

end_of_func:
	SetMainWindow( pEditWnd->GetHwnd() );

	//�E�B���h�E�L���v�V�����X�V
	pEditWnd->UpdateCaption();

	//	YAZAKI 2002/05/30 IME�E�B���h�E�̈ʒu�����������̂��C���B
	pEditWnd->GetActiveView().SetIMECompFormPos();

	//WM_SIZE���|�X�g
	{
		RECT rc;
		GetClientRect( pEditWnd->GetHwnd(), &rc);
		::PostMessageAny(
			pEditWnd->GetHwnd(),
			WM_SIZE,
			SIZE_RESTORED,
			MAKELPARAM(
				rc.right -rc.left,
				rc.bottom-rc.top
			)
		);
	}

	//�ĕ`��
	::InvalidateRect( pEditWnd->GetHwnd(), NULL, TRUE );

	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );


	return pEditWnd->GetHwnd() ? true : false;
}

/*!
	@brief �G�f�B�^�v���Z�X�̃��b�Z�[�W���[�v
	
	@author aroka
	@date 2002/01/07
*/
bool CNormalProcess::MainLoop()
{
	if( GetMainWindow() ){
		m_pcEditApp->GetWindow()->MessageLoop();	/* ���b�Z�[�W���[�v */
		return true;
	}
	return false;
}

/*!
	@brief �G�f�B�^�v���Z�X���I������
	
	@author aroka
	@date 2002/01/07
	�����͂Ȃɂ����Ȃ��B��n����dtor�ŁB
*/
void CNormalProcess::OnExitProcess()
{
	SAFE_DELETE(m_pcEditApp);
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �����⏕                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@brief Mutex(�v���Z�X�������̖ڈ�)���擾����

	���������ɋN������ƃE�B���h�E���\�ɏo�Ă��Ȃ����Ƃ�����B
	
	@date 2002/2/8 aroka InitializeProcess����ړ�
	@retval Mutex �̃n���h����Ԃ�
	@retval ���s�������̓����[�X���Ă��� NULL ��Ԃ�
*/
HANDLE CNormalProcess::_GetInitializeMutex() const
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::_GetInitializeMutex" );
	HANDLE hMutex;
	hMutex = ::CreateMutex( NULL, TRUE, GSTR_MUTEX_SAKURA_INIT );
	if( NULL == hMutex ){
		ErrorBeep();
		TopErrorMessage( NULL, _T("CreateMutex()���s�B\n�I�����܂��B") );
		return NULL;
	}
	if( ::GetLastError() == ERROR_ALREADY_EXISTS ){
		DWORD dwRet = ::WaitForSingleObject( hMutex, 15000 );	// 2002/2/8 aroka ������������
		if( WAIT_TIMEOUT == dwRet ){// �ʂ̒N�����N����
			TopErrorMessage( NULL, _T("�G�f�B�^�܂��̓V�X�e�����r�W�[��Ԃł��B\n���΂炭�҂��ĊJ���Ȃ����Ă��������B") );
			::CloseHandle( hMutex );
			return NULL;
		}
	}
	return hMutex;
}


