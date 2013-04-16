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
	Copyright (C) 2009, syat, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/


#include "StdAfx.h"
#include "CNormalProcess.h"
#include "CCommandLine.h"
#include "CControlTray.h"
#include "window/CEditWnd.h" // 2002/2/3 aroka
#include "CGrepAgent.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLine.h" // 2003/03/28 MIK
#include "debug/CRunningTimer.h"
#include "util/window.h"
#include "util/file.h"
#include "plugin/CPluginManager.h"
#include "plugin/CJackManager.h"
#include "CAppMode.h"
#include "env/CDocTypeManager.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CNormalProcess::CNormalProcess( HINSTANCE hInstance, LPCTSTR lpCmdLine )
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
	@date 2012.02.25 novice �����t�@�C���ǂݍ���
*/
bool CNormalProcess::InitializeProcess()
{
	MY_RUNNINGTIMER( cRunningTimer, "NormalProcess::Init" );

	/* �v���Z�X�������̖ڈ� */
	HANDLE	hMutex = _GetInitializeMutex();	// 2002/2/8 aroka ���ݓ����Ă����̂ŕ���
	if( NULL == hMutex ){
		return false;
	}

	/* ���L������������������ */
	if ( !CProcess::InitializeProcess() ){
		return false;
	}

	/* �R�}���h���C���I�v�V���� */
	bool			bViewMode = false;
	bool			bDebugMode;
	bool			bGrepMode;
	bool			bGrepDlg;
	GrepInfo		gi;
	EditInfo		fi;
	
	/* �R�}���h���C���Ŏ󂯎�����t�@�C�����J����Ă���ꍇ�� */
	/* ���̕ҏW�E�B���h�E���A�N�e�B�u�ɂ��� */
	CCommandLine::getInstance()->GetEditInfo(&fi); // 2002/2/8 aroka �����Ɉړ�
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


	// �v���O�C���ǂݍ���
	MY_TRACETIME( cRunningTimer, "Before Init Jack" );
	/* �W���b�N������ */
	CJackManager::getInstance();
	MY_TRACETIME( cRunningTimer, "After Init Jack" );

	MY_TRACETIME( cRunningTimer, "Before Load Plugins" );
	/* �v���O�C���ǂݍ��� */
	CPluginManager::getInstance()->LoadAllPlugin();
	MY_TRACETIME( cRunningTimer, "After Load Plugins" );

	// �G�f�B�^�A�v���P�[�V�������쐬�B2007.10.23 kobake
	// �O���[�vID���擾
	int nGroupId = CCommandLine::getInstance()->GetGroupId();
	if( GetDllShareData().m_Common.m_sTabBar.m_bNewWindow && nGroupId == -1 ){
		nGroupId = CAppNodeManager::getInstance()->GetFreeGroupId();
	}
	// CEditApp���쐬
	m_pcEditApp = new CEditApp(GetProcessInstance(), nGroupId);
	CEditWnd* pEditWnd = m_pcEditApp->GetWindow();
	if( NULL == pEditWnd->GetHwnd() ){
		::ReleaseMutex( hMutex );
		::CloseHandle( hMutex );
		return false;	// 2009.06.23 ryoji CEditWnd::Create()���s�̂��ߏI��
	}

	/* �R�}���h���C���̉�� */	 // 2002/2/8 aroka �����Ɉړ�
	bDebugMode = CCommandLine::getInstance()->IsDebugMode();
	bGrepMode  = CCommandLine::getInstance()->IsGrepMode();
	bGrepDlg   = CCommandLine::getInstance()->IsGrepDlg();

	MY_TRACETIME( cRunningTimer, "CheckFile" );

	// -1: SetDocumentTypeWhenCreate �ł̋����w��Ȃ�
	const CTypeConfig nType = (fi.m_szDocType[0] == '\0' ? CTypeConfig(-1) : CDocTypeManager().GetDocumentTypeOfExt(fi.m_szDocType));

	if( bDebugMode ){
		/* �f�o�b�O���j�^���[�h�ɐݒ� */
		CAppMode::getInstance()->SetDebugModeON();
		// 2004.09.20 naoh �A�E�g�v�b�g�p�^�C�v�ʐݒ�
		// �����R�[�h��L���Ƃ��� Uchi 2008/6/8
		// 2010.06.16 Moca �A�E�g�v�b�g�� CCommnadLine�� -TYPE=output �����Ƃ���
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// �A�E�g���C����\������
	}
	else if( bGrepMode ){
		/* GREP */
		// 2010.06.16 Moca Grep�ł��I�v�V�����w���K�p
		pEditWnd->SetDocumentTypeWhenCreate( fi.m_nCharCode, false, nType );
		pEditWnd->m_cDlgFuncList.Refresh();	// �A�E�g���C����\�ߕ\�����Ă���
		HWND hEditWnd = pEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) && pEditWnd->m_cDlgFuncList.GetHwnd() ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::SendMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
		CCommandLine::getInstance()->GetGrepInfo(&gi); // 2002/2/8 aroka �����Ɉړ�
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
				&pEditWnd->GetActiveView(),
				&gi.cmGrepKey,
				&gi.cmGrepFile,
				&gi.cmGrepFolder,
				gi.bGrepSubFolder,
				gi.sGrepSearchOption,
				gi.nGrepCharSet,	//	2002/09/21 Moca
				gi.bGrepOutputLine,
				gi.nGrepOutputStyle
			);
			pEditWnd->m_cDlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
			return true; // 2003.06.23 Moca
		}
		else{
			//-GREPDLG�Ń_�C�A���O���o���B�@���������f�i2002/03/24 YAZAKI�j
			if( gi.cmGrepKey.GetStringLength() < _MAX_PATH ){
				CSearchKeywordManager().AddToSearchKeyArr( gi.cmGrepKey.GetStringPtr() );
			}
			if( gi.cmGrepFile.GetStringLength() < _MAX_PATH ){
				CSearchKeywordManager().AddToGrepFileArr( gi.cmGrepFile.GetStringPtr() );
			}
			if( gi.cmGrepFolder.GetStringLength() < _MAX_PATH ){
				CSearchKeywordManager().AddToGrepFolderArr( gi.cmGrepFolder.GetStringPtr() );
			}
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
			pEditWnd->m_cDlgGrep.m_strText = gi.cmGrepKey.GetStringPtr();		/* ���������� */
			int nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFile);
			_tcsncpy( pEditWnd->m_cDlgGrep.m_szFile, gi.cmGrepFile.GetStringPtr(), nSize );	/* �����t�@�C�� */
			pEditWnd->m_cDlgGrep.m_szFile[nSize-1] = _T('\0');
			nSize = _countof2(pEditWnd->m_cDlgGrep.m_szFolder);
			_tcsncpy( pEditWnd->m_cDlgGrep.m_szFolder, gi.cmGrepFolder.GetStringPtr(), nSize );	/* �����t�H���_ */
			pEditWnd->m_cDlgGrep.m_szFolder[nSize-1] = _T('\0');

			
			// Feb. 23, 2003 Moca Owner window���������w�肳��Ă��Ȃ�����
			int nRet = pEditWnd->m_cDlgGrep.DoModal( GetProcessInstance(), pEditWnd->GetHwnd(),  NULL);
			if( FALSE != nRet ){
				pEditWnd->GetActiveView().GetCommander().HandleCommand(F_GREP, true, 0, 0, 0, 0);
			}
			pEditWnd->m_cDlgFuncList.Refresh();	// �A�E�g���C�����ĉ�͂���
			return true; // 2003.06.23 Moca
		}
	}
	else{
		// 2004.05.13 Moca �����if���̒�����O�Ɉړ�
		// �t�@�C�������^�����Ȃ��Ă�ReadOnly�w���L���ɂ��邽�߁D
		bViewMode = CCommandLine::getInstance()->IsViewMode(); // 2002/2/8 aroka �����Ɉړ�
		if( 0 < _tcslen( fi.m_szPath ) ){
			//	Mar. 9, 2002 genta �����^�C�v�w��
			pEditWnd->OpenDocumentWhenStart(
				SLoadInfo(
					fi.m_szPath,
					fi.m_nCharCode,
					bViewMode,
					nType
				)
			);
			// �ǂݍ��ݒ��f���āu(����)�v�ɂȂ������i���v���Z�X����̃��b�N�Ȃǁj���I�v�V�����w���L���ɂ���
			// Note. fi.m_nCharCode �ŕ����R�[�h�������w�肳��Ă��Ă��A�ǂݍ��ݒ��f���Ȃ��ꍇ�͕ʂ̕����R�[�h���I������邱�Ƃ�����B
			//       �ȑO�́u(����)�v�ɂȂ�Ȃ��ꍇ�ł��������� SetDocumentTypeWhenCreate() ���Ă�ł������A
			//       �u�O��ƈقȂ镶���R�[�h�v�̖₢���킹�őO��̕����R�[�h���I�����ꂽ�ꍇ�ɂ��������Ȃ��Ă����B
			if( !pEditWnd->GetDocument().m_cDocFile.GetFilePathClass().IsValidPath() ){
				// �ǂݍ��ݒ��f���āu(����)�v�ɂȂ���
				// ---> �����ɂȂ����I�v�V�����w���L���ɂ���
				pEditWnd->SetDocumentTypeWhenCreate(
					fi.m_nCharCode,
					bViewMode,
					nType
				);
			}
			//	Nov. 6, 2000 genta
			//	�L�����b�g�ʒu�̕����̂���
			//	�I�v�V�����w�肪�Ȃ��Ƃ��͉�ʈړ����s��Ȃ��悤�ɂ���
			//	Oct. 19, 2001 genta
			//	���ݒ聁-1�ɂȂ�悤�ɂ����̂ŁC���S�̂��ߗ��҂��w�肳�ꂽ�Ƃ�����
			//	�ړ�����悤�ɂ���D || �� &&
			if( ( CLayoutInt(0) <= fi.m_nViewTopLine && CLayoutInt(0) <= fi.m_nViewLeftCol )
				&& fi.m_nViewTopLine < pEditWnd->GetDocument().m_cLayoutMgr.GetLineCount() ){
				pEditWnd->GetActiveView().GetTextArea().SetViewTopLine( fi.m_nViewTopLine );
				pEditWnd->GetActiveView().GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol );
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

				pEditWnd->GetActiveView().GetCaret().MoveCursor( ptPos, true );
				pEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev =
					pEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
			}
			pEditWnd->GetActiveView().RedrawAll();
		}
		else{
			// 2004.05.13 Moca �t�@�C�������^�����Ȃ��Ă�ReadOnly�ƃ^�C�v�w���L���ɂ���
			pEditWnd->SetDocumentTypeWhenCreate(
				fi.m_nCharCode,
				bViewMode,	// �r���[���[�h��
				nType
			);
		}
	}

	SetMainWindow( pEditWnd->GetHwnd() );

	//	YAZAKI 2002/05/30 IME�E�B���h�E�̈ʒu�����������̂��C���B
	pEditWnd->GetActiveView().SetIMECompFormPos();

	//WM_SIZE���|�X�g
	{	// �t�@�C���ǂݍ��݂��Ȃ������ꍇ�ɂ͂��� WM_SIZE ���A�E�g���C����ʂ�z�u����
		HWND hEditWnd = pEditWnd->GetHwnd();
		if( !::IsIconic( hEditWnd ) ){
			RECT rc;
			::GetClientRect( hEditWnd, &rc );
			::PostMessageAny( hEditWnd, WM_SIZE, ::IsZoomed( hEditWnd )? SIZE_MAXIMIZED: SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
		}
	}

	//�ĕ`��
	::InvalidateRect( pEditWnd->GetHwnd(), NULL, TRUE );

	::ReleaseMutex( hMutex );
	::CloseHandle( hMutex );

	//�v���O�C���FEditorStart�C�x���g���s
	CPlug::Array plugs;
	CWSHIfObj::List params;
	CJackManager::getInstance()->GetUsablePlug(
			PP_EDITOR_START,
			0,
			&plugs
		);
	for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
		(*it)->Invoke(&pEditWnd->GetActiveView(), params);
	}

	// 2006.09.03 ryoji �I�[�v���㎩�����s�}�N�������s����
	if( !( bDebugMode || bGrepMode ) )
		pEditWnd->GetDocument().RunAutoMacro( GetDllShareData().m_Common.m_sMacro.m_nMacroOnOpened );

	// �N�����}�N���I�v�V����
	LPCWSTR pszMacro = CCommandLine::getInstance()->GetMacro();
	if( pEditWnd->GetHwnd()  &&  pszMacro  &&  pszMacro[0] != L'\0' ){
		LPCWSTR pszMacroType = CCommandLine::getInstance()->GetMacroType();
		if( pszMacroType == NULL || pszMacroType[0] == L'\0' || wcsicmp(pszMacroType, L"file") == 0 ){
			pszMacroType = NULL;
		}
		CEditView& view = pEditWnd->GetActiveView();
		view.GetCommander().HandleCommand( F_EXECEXTMACRO, true, (LPARAM)pszMacro, (LPARAM)pszMacroType, 0, 0 );
	}

	// �����t�@�C���ǂݍ���
	int fileNum = CCommandLine::getInstance()->GetFileNum();
	if( fileNum > 0 ){
		int nDropFileNumMax = GetDllShareData().m_Common.m_sFile.m_nDropFileNumMax - 1;
		// �t�@�C���h���b�v���̏���ɍ��킹��
		if( fileNum > nDropFileNumMax ){
			fileNum = nDropFileNumMax;
		}
		EditInfo openFileInfo = fi;
		int i;
		for( i = 0; i < fileNum; i++ ){
			// �t�@�C���������ւ�
			_tcscpy(openFileInfo.m_szPath, CCommandLine::getInstance()->GetFileName(i));
			bool ret = CControlTray::OpenNewEditor2( GetProcessInstance(), pEditWnd->GetHwnd(), &openFileInfo, bViewMode );
			if( ret == false ){
				break;
			}
		}
		// �p�ς݂Ȃ̂ō폜
		CCommandLine::getInstance()->ClearFile();
	}

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
	/* �v���O�C����� */
	SAFE_DELETE(m_pcEditApp);
	CPluginManager::getInstance()->UnloadAllPlugin();		// Mpve here	2010/7/11 Uchi
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


