/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001, genta, YAZAKI, jepro, novice, asa-o, MIK,
	Copyright (C) 2002, YAZAKI, hor, genta, aroka, frozen, Moca, MIK
	Copyright (C) 2003, MIK, genta, ryoji, Moca, zenryaku, naoh, wmlhq
	Copyright (C) 2004, genta, novice, Moca, MIK, zenryaku
	Copyright (C) 2005, genta, naoh, FILE, Moca, ryoji, D.S.Koba, aroka
	Copyright (C) 2006, genta, ryoji, aroka
	Copyright (C) 2007, ryoji, maru

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>	// Apr. 03, 2003 genta
#include <io.h>
#include <Cderr.h> // Nov. 3, 2005 genta
#include "CEditDoc.h"
#include "debug.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include <DLGS.H>
#include "CShareData.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"
#include "CDlgFileUpdateQuery.h"
#include <assert.h> /// 2002/11/2 frozen
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�
#include "my_sp.h" // 2005/11/22 aroka �ǉ�
#include "CClipboard.h"
#include "CLayout.h"	// 2007.08.22 ryoji �ǉ�
#include "CMemoryIterator.h"	// 2007.08.22 ryoji �ǉ�
#include "charset/CCodeMediator.h"
#include "io/io_util.h"
#include "util/file.h"
#include "util/window.h"
#include "util/string_ex2.h"
#include "util/format.h"
#include "util/module.h"
#include "CEditApp.h"
#include "util/other_util.h"

#define IDT_ROLLMOUSE	1

/*!
	May 12, 2000 genta ���������@�ύX
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
	@note m_pcEditWnd �̓R���X�g���N�^���ł͎g�p���Ȃ����ƁD
*/
CEditDoc::CEditDoc(CEditApp* pcApp)
: m_pcEditWnd(pcApp->m_pcEditWnd)
, m_cNewLineCode( EOL_CRLF )		//	New Line Type
, m_cSaveLineCode( EOL_NONE )		//	�ۑ�����Line Type
, m_bGrepRunning( FALSE )		/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
, m_nCommandExecNum( 0 )			/* �R�}���h���s�� */
// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
, m_bReadOnly( false )			/* �ǂݎ���p���[�h */
, m_bDebugMode( FALSE )			/* �f�o�b�O���j�^���[�h */
, m_bGrepMode( FALSE )			/* Grep���[�h�� */
, m_nCharCode( CODE_DEFAULT )	/* �����R�[�h��� */
, m_bBomExist( FALSE )			//	Jul. 26, 2003 ryoji BOM
, m_bDoing_UndoRedo( FALSE )		/* �A���h�D�E���h�D�̎��s���� */
, m_nFileShareModeOld( SHAREMODE_NOT_EXCLUSIVE )		/* �t�@�C���̔r�����䃂�[�h */
, m_hLockedFile( NULL )			/* ���b�N���Ă���t�@�C���̃n���h�� */
, m_pszAppName( _T("EditorClient") )
, m_hInstance( NULL )
, m_eWatchUpdate( CEditDoc::WU_QUERY )
, m_nSettingTypeLocked( false )	//	�ݒ�l�ύX�\�t���O
, m_nSettingType( 0 )	// Sep. 11, 2002 genta
, m_bInsMode( true )	// Oct. 2, 2005 genta
, m_bIsModified( false )	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditDoc::CEditDoc" );
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* ���ݕҏW���̃t�@�C���̃p�X */
	wcscpy( m_szGrepKey, L"" );
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */

	m_pShareData = CShareData::getInstance()->GetShareData();
	//	Sep. 11, 2002 genta �폜
	//	SetDocumentType�̓R���X�g���N�^���ł͎g��Ȃ��D
	//int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	//SetDocumentType( doctype, true );


	/* ���C�A�E�g�Ǘ����̏����� */
	m_cLayoutMgr.Create( this, &m_cDocLineMgr );
	/* ���C�A�E�g���̕ύX */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);
//	MYTRACE_A( "CEditDoc::CEditDoc()�����\n" );

	//	Aug, 21, 2000 genta
	//	�����ۑ��̐ݒ�
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	�}�N��
	m_pcSMacroMgr = new CSMacroMgr;
	//wcscpy(m_pszCaption, L"sakura");	//@@@	YAZAKI
	
	//	m_FileTime�̏�����
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;

	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode != FALSE );

	return;
}


CEditDoc::~CEditDoc()
{
//	delete (CDialog*)m_pcDlgTest;
//	m_pcDlgTest = NULL;

	if( GetSplitterHwnd() != NULL ){
		DestroyWindow( GetSplitterHwnd() );
	}
	/* �t�@�C���̔r�����b�N���� */
	delete m_pcSMacroMgr;
	DoFileUnLock();
	return;
}





/////////////////////////////////////////////////////////////////////////////
//
//	CEditDoc::Create
//	BOOL Create(HINSTANCE hInstance, HWND hwndParent)
//
//	����
//	  �E�B���h�E�̍쐬��
//
//	@date Sep. 29, 2001 genta �}�N���N���X��n���悤��
//	@date 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	CImageListMgr* pcIcons
 )
{
	MY_RUNNINGTIMER( cRunningTimer, L"CEditDoc::Create" );

	m_hInstance = hInstance;


	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pShareData->m_MacroTable, &m_pShareData->m_Common );


	MY_TRACETIME( cRunningTimer, L"End: PropSheet" );

	return TRUE;
}












BOOL CEditDoc::SelectFont( LOGFONT* plf )
{
	// 2004.02.16 Moca CHOOSEFONT�������o����O��
	CHOOSEFONT cf;
	/* CHOOSEFONT�̏����� */
	::ZeroMemory( &cf, sizeof( cf ) );
	cf.lStructSize = sizeof( cf );
	cf.hwndOwner = GetSplitterHwnd();
	cf.hDC = NULL;
//	cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;

	//FIXED�t�H���g�ȊO
	#ifdef USE_UNFIXED_FONT
		cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
	#endif

	cf.lpLogFont = plf;
	if( !ChooseFont( &cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE_A( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE_A( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE_A( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE_A( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE_A( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE_A( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE_A( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE_A( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE_A( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE_A( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE_A( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE_A( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE_A( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}

	return TRUE;
}




/*! �t�@�C�����J��

	@return ����: TRUE/pbOpened==FALSE,
			���ɊJ����Ă���: FALSE/pbOpened==TRUE
			���s: FALSE/pbOpened==FALSE

	@note genta �߂������Ɍ����������������ȁD

	@date 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
	@date 2000.05,12 genta ���s�R�[�h�̐ݒ�
	@date 2000.10.25 genta �����R�[�h�ُ̈�Ȓl���`�F�b�N
	@date 2000.11.20 genta IME��Ԃ̐ݒ�
	@date 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	@date 2002.01.16 hor �u�b�N�}�[�N����
	@date 2002.10.19 genta �ǂݎ��s�̃t�@�C���͕����R�[�h���ʂŎ��s����
	@date 2003.03.28 MIK ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤��
	@date 2003.07.26 ryoji BOM�����ǉ�
	@date 2002.05.26 Moca gm_pszCodeNameArr_Normal ���g���悤�ɕύX
	@date 2004.06.18 moca �t�@�C�����J���Ȃ������ꍇ��pbOpened��FALSE�ɏ���������Ă��Ȃ������D
	@date 2004.10.09 genta ���݂��Ȃ��t�@�C�����J�����Ƃ����Ƃ���
					�t���O�ɉ����Čx�����o���i�ȑO�̓���j�悤��
	@date 2006.12.16 ���イ�� �O��̕����R�[�h��D�悷��
	@date 2007.03.12 maru �t�@�C�������݂��Ȃ��Ă��O��̕����R�[�h���p��
						���d�I�[�v��������CEditDoc::IsPathOpened�Ɉړ�
*/
bool CEditDoc::FileRead(
	TCHAR*		pszPath,			//!< [in/out]
	bool*		pbOpened,			//!< [out] ���łɊJ����Ă�����
	ECodeType	nCharCode,			/*!< [in] �����R�[�h��� */
	bool		bReadOnly,			/*!< [in] �ǂݎ���p�� */
	bool		bConfirmCodeChange	/*!< [in] �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
)
{
	HWND			hWndOwner;
	bool			bRet;
	FileInfo		fi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( GetSplitterHwnd() );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	*pbOpened = FALSE;	// 2004.06.18 Moca �������~�X
	m_bReadOnly = bReadOnly;	/* �ǂݎ���p���[�h */

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU			cMRU;

	/* �t�@�C���̑��݃`�F�b�N */
	bFileIsExist = FALSE;
	if( -1 == _taccess( pszPath, 0 ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );

		bFileIsExist = TRUE;

		/* �t�H���_���w�肳�ꂽ�ꍇ */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* �w��t�H���_�Łu�J���_�C�A���O�v��\�� */
			{
				TCHAR pszPathNew[_MAX_PATH]=_T("");

				/* �u�t�@�C�����J���v�_�C�A���O */
				nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
				bReadOnly = FALSE;
//				::ShowWindow( GetSplitterHwnd(), SW_SHOW );
				if( !OpenFileDialog( GetSplitterHwnd(), pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					return FALSE;
				}
				_tcscpy( pszPath, pszPathNew );

				if( -1 == _taccess( pszPath, 0 ) ){
					bFileIsExist = FALSE;
				}else{
					bFileIsExist = TRUE;
				}
			}
		}

	}

	//	From Here Oct. 19, 2002 genta
	//	�ǂݍ��݃A�N�Z�X���������ꍇ�ɂ͊����R�[�h����Ńt�@�C����
	//	�J���Ȃ��̂ŕ����R�[�h���ʃG���[�Əo�Ă��܂��D
	//	���K�؂ȃ��b�Z�[�W���o�����߁C�ǂ߂Ȃ��t�@�C����
	//	���O�ɔ���E�r������
	//
	//	_access�ł̓��b�N���ꂽ�t�@�C���̏�Ԃ��擾�ł��Ȃ��̂�
	//	���ۂɃt�@�C�����J���Ċm�F����
	if( bFileIsExist){
		HANDLE hTest = 	CreateFile( pszPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hTest == INVALID_HANDLE_VALUE ){
			// �ǂݍ��݃A�N�Z�X�����Ȃ�
			::MYMESSAGEBOX(
				GetSplitterHwnd(), MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				_T("\'%ls\'\n")
				_T("�Ƃ����t�@�C�����J���܂���B\n")
				_T("�ǂݍ��݃A�N�Z�X��������܂���B"),
				pszPath
			 );
			return FALSE;
		}
		else {
			CloseHandle( hTest );
		}
	}
	//	To Here Oct. 19, 2002 genta

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
	}else{
		hwndProgress = NULL;
	}
	bRet = true;
	if( NULL == pszPath ){
		MYMESSAGEBOX_A(
			GetSplitterHwnd(),
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			"�o�O���႟�������I�I�I",
			"CEditDoc::FileRead()\n\nNULL == pszPath\n�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������ˁB"
		);
		return FALSE;
	}
	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	if( CShareData::getInstance()->IsPathOpened(pszPath, &hWndOwner, nCharCode) ){	/* 2007.03.12 maru ���d�I�[�v��������IsPathOpened�ɂ܂Ƃ߂� */
		*pbOpened = TRUE;
		bRet = false;
		goto end_of_func;
	}

	//�r���[�̃e�L�X�g�I������
	m_pcEditWnd->Views_DisableSelectArea(true);

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */


	/* �w�肳�ꂽ�����R�[�h��ʂɕύX���� */
	//	Oct. 25, 2000 genta
	//	�����R�[�h�Ƃ��Ĉُ�Ȓl���ݒ肳�ꂽ�ꍇ�̑Ή�
	//	-1�ȏ�CODE_MAX�����̂ݎ󂯕t����
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECT�͂��͈̔͂���O��Ă��邩��ʂɃ`�F�b�N
	//  Aug. 14, 2007 kobake
	//  nCharCode�̃`�F�b�N���p�̊֐��ɈϏ�
	if( IsInECodeType(nCharCode) )
		m_nCharCode = nCharCode;
	
	/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	if ( cMRU.GetFileInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

		if( -1 == m_nCharCode ){
			/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
			m_nCharCode = fi.m_nCharCode;
		}
		
		if( !bConfirmCodeChange && ( CODE_AUTODETECT == m_nCharCode ) ){	// �����R�[�h�w��̍ăI�[�v���Ȃ�O��𖳎�
			m_nCharCode = fi.m_nCharCode;
		}
		if( (FALSE == bFileIsExist) && (CODE_AUTODETECT == m_nCharCode) ){
			/* ���݂��Ȃ��t�@�C���̕����R�[�h�w��Ȃ��Ȃ�O����p�� */
			m_nCharCode = fi.m_nCharCode;
		}
	} else {
		bIsExistInMRU = FALSE;
	}

	/* �����R�[�h�������� */
	if( CODE_AUTODETECT == m_nCharCode ) {
		if( FALSE == bFileIsExist ){	/* �t�@�C�������݂��Ȃ� */
			m_nCharCode = CODE_DEFAULT;
		} else {
			m_nCharCode = CCodeMediator::CheckKanjiCodeOfFile( pszPath );
			if( CODE_ERROR == m_nCharCode ){
				::MYMESSAGEBOX( GetSplitterHwnd(), MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					_T("%ls\n")
					_T("�����R�[�h�̔��ʏ����ŃG���[���������܂����B"),
					pszPath
				);
				//	Sep. 10, 2002 genta
				SetFilePath( _T("") );
				bRet = false;
				goto end_of_func;
			}
		}
	}
	/* �����R�[�h���قȂ�Ƃ��Ɋm�F���� */
	if( bConfirmCodeChange && bIsExistInMRU ){
		if (m_nCharCode != fi.m_nCharCode ) {	// MRU �̕����R�[�h�Ɣ��ʂ��قȂ�
			const TCHAR* pszCodeName = NULL;
			const TCHAR* pszCodeNameNew = NULL;

			// gm_pszCodeNameArr_Normal ���g���悤�ɕύX Moca. 2002/05/26
			if(IsValidCodeType(fi.m_nCharCode)){
				pszCodeName = gm_pszCodeNameArr_Normal[fi.m_nCharCode];
			}
			if(IsValidCodeType(m_nCharCode)){
				pszCodeNameNew = gm_pszCodeNameArr_Normal[m_nCharCode];
			}
			if( pszCodeName != NULL ){
				::MessageBeep( MB_ICONQUESTION );
				nRet = MYMESSAGEBOX_A(
					GetSplitterHwnd(),
					MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
					"�����R�[�h���",
					"%ts\n\n���̃t�@�C���́A�O��͕ʂ̕����R�[�h %ts �ŊJ����Ă��܂��B\n"
					"�O��Ɠ��������R�[�h���g���܂����H\n"
					"\n"
					"�E[�͂�(Y)]  ��%ts\n"
					"�E[������(N)]��%ts\n"
					"�E[�L�����Z��]���J���܂���",
					GetFilePath(),
					pszCodeName,
					pszCodeName,
					pszCodeNameNew
				);
				if( IDYES == nRet ){
					/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
					m_nCharCode = fi.m_nCharCode;
				}
				else if( IDCANCEL == nRet ){
					m_nCharCode = CODE_DEFAULT;
					//	Sep. 10, 2002 genta
					SetFilePath( _T("") );
					bRet = false;
					goto end_of_func;
				}
			}else{
				MYMESSAGEBOX_A(
					GetSplitterHwnd(),
					MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
					"�o�O���႟�������I�I�I",
					"�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������B"
				);
				//	Sep. 10, 2002 genta
				SetFilePath( _T("") );
				bRet = false;
				goto end_of_func;
			}
		}
	}
	if( -1 == m_nCharCode ){
		m_nCharCode = CODE_DEFAULT;
	}

	//	Nov. 12, 2000 genta �����O�t�@�C�����̎擾��O���Ɉړ�
	TCHAR szWork[MAX_PATH];
	/* �����O�t�@�C�������擾���� */
	if( ::GetLongFileName( pszPath, szWork ) ){
		//	Sep. 10, 2002 genta
		SetFilePath( szWork );
	}

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	//	From Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�
	switch( m_nCharCode ){
	case CODE_UNICODE:
	case CODE_UNICODEBE:
		m_bBomExist = TRUE;
		break;
	case CODE_UTF8:
	default:
		m_bBomExist = FALSE;
		break;
	}
	//	To Here Jul. 26, 2003 ryoji BOM�̗L���̏�����Ԃ�ݒ�

	/* �t�@�C�������݂��Ȃ� */
	if( FALSE == bFileIsExist ){

		//	Oct. 09, 2004 genta �t���O�ɉ����Čx�����o���i�ȑO�̓���j�悤��
		if( m_pShareData->m_Common.m_sFile.GetAlertIfFileNotExist() ){
			::MessageBeep( MB_ICONINFORMATION );

			//	Feb. 15, 2003 genta Popup�E�B���h�E��\�����Ȃ��悤�ɁD
			//	�����ŃX�e�[�^�X���b�Z�[�W���g���Ă���ʂɕ\������Ȃ��D
			::MYMESSAGEBOX(
				GetOwnerHwnd(),
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%ls\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B"),	//Mar. 24, 2001 jepro �኱�C��
				pszPath
			);
		}
	}else{
		/* �t�@�C����ǂ� */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		//	Jul. 26, 2003 ryoji BOM�����ǉ�
		if( FALSE == m_cDocLineMgr.ReadFile( GetFilePath(), GetSplitterHwnd(), hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.m_sFile.GetAutoMIMEdecode(), &m_bBomExist ) ){
			//	Sep. 10, 2002 genta
			SetFilePath( _T("") );
			bRet = false;
			goto end_of_func;
		}
//#ifdef _DEBUG
//		m_cDocLineMgr.DUMP();
//#endif

	}

	/* ���C�A�E�g���̕ύX */
	{
		Types& ref = GetDocumentAttribute();
		m_cLayoutMgr.SetLayoutInfo(
			TRUE,
			hwndProgress,
			ref
		);
	}

	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.m_sFile.GetRestoreCurPosition() ){
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptCaretPos;
		m_cLayoutMgr.LogicToLayout(
			fi.m_ptCursor,
			&ptCaretPos
		);
		if( ptCaretPos.GetY2() >= m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
			m_pcEditWnd->GetActiveView().GetCommander().HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine( fi.m_nViewTopLine ); // 2001/10/20 novice
			m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol( fi.m_nViewLeftCol ); // 2001/10/20 novice
			// From Here Mar. 28, 2003 MIK
			// ���s�̐^�񒆂ɃJ�[�\�������Ȃ��悤�ɁB
			const CDocLine *pTmpDocLine = m_cDocLineMgr.GetLineInfo( fi.m_ptCursor.GetY2() );	// 2008.08.22 ryoji ���s�P�ʂ̍s�ԍ���n���悤�ɏC��
			if( pTmpDocLine ){
				if( pTmpDocLine->GetLengthWithoutEOL() < fi.m_ptCursor.x ) ptCaretPos.x--;
			}
			// To Here Mar. 28, 2003 MIK
			m_pcEditWnd->GetActiveView().GetCaret().MoveCursor( ptCaretPos, TRUE );
			m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev =
				m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();
		}
	}
	// 2002.01.16 hor �u�b�N�}�[�N����
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.m_sFile.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		wcscpy(fi.m_szMarkLines,L"");
	}
	GetFileInfo( &fi );

	//	May 12, 2000 genta
	//	���s�R�[�h�̐ݒ�
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( CLogicInt(0) );
		if( pFirstlineinfo != NULL ){
			enumEOLType t = (enumEOLType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN ){
				SetNewLineCode( t );
			}
		}
	}

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );
	
	/* �J�����g�f�B���N�g���̕ύX */
	{
		TCHAR	szCurDir[_MAX_PATH];
		TCHAR	szDrive[_MAX_DRIVE];
		TCHAR	szDir[_MAX_DIR];
		_tsplitpath( GetFilePath(), szDrive, szDir, NULL, NULL );
		_tcscpy( szCurDir, szDrive);
		_tcscat( szCurDir, szDir );
		::SetCurrentDirectory( szCurDir );
	}

end_of_func:;
	//	2004.05.13 Moca ���s�R�[�h�̐ݒ�����炱���Ɉړ�
	m_pcEditWnd->GetActiveView().GetCaret().DrawCaretPosInfo();

	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( bRet && IsFilePathAvailable() ){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	//	From Here Jul. 26, 2003 ryoji �G���[�̎��͋K���BOM�ݒ�Ƃ���
	if( !bRet ){
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
	}
	//	To Here Jul. 26, 2003 ryoji
	return bRet;
}


/*!	@brief �t�@�C���̕ۑ�
	
	@param pszPath [in] �ۑ��t�@�C����
	@param cEolType [in] ���s�R�[�h���
	
	pszPath��NULL�ł����Ă͂Ȃ�Ȃ��B
	
	@date Feb. 9, 2001 genta ���s�R�[�h�p�����ǉ�
*/
BOOL CEditDoc::FileWrite( const TCHAR* pszPath, enumEOLType cEolType )
{
	BOOL		bRet = TRUE;

	//	Sep. 7, 2003 genta
	//	�ۑ�����������܂ł̓t�@�C���X�V�̒ʒm��}������
	WatchUpdate wuSave = m_eWatchUpdate;
	m_eWatchUpdate = WU_NONE;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	//�v���O���X�o�[
	HWND		hwndProgress;
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	// �t�@�C���̔r�����b�N����
	DoFileUnLock();

	if( m_pShareData->m_Common.m_sBackup.m_bBackUp ){	/* �o�b�N�A�b�v�̍쐬 */
		//	Jun.  5, 2004 genta �t�@�C������^����悤�ɁD�߂�l�ɉ�����������ǉ��D
		switch( MakeBackUp( pszPath )){
		case 2:	//	���f�w��
			return FALSE;
		case 3: //	�t�@�C���G���[
			if( IDYES != ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"�t�@�C���ۑ�",
				"�o�b�N�A�b�v�̍쐬�Ɏ��s���܂����D���t�@�C���ւ̏㏑�����p�����čs���܂����D"
			)){
				return FALSE;
			}
		break;
		}
	}

	//�����v
	CWaitCursor cWaitCursor( GetSplitterHwnd() );

	//�J�L�R
	EConvertResult nWriteResult = m_cDocLineMgr.WriteFile(
		pszPath,
		GetSplitterHwnd(),
		hwndProgress,
		m_nCharCode,
		&m_FileTime,
		CEOL(cEolType),
		m_bBomExist
	);

	//�J�L�R����
	if( nWriteResult == RESULT_FAILURE ){
		bRet = FALSE;
		goto end_of_func;
	}
	else if(nWriteResult == RESULT_LOSESOME){
		ErrorMessage(NULL, _T("�ꕔ�̃e�L�X�g�f�[�^���A�ϊ��ɂ�莸���܂���"));
	}

	/* �s�ύX��Ԃ����ׂă��Z�b�g */
	m_cDocLineMgr.ResetAllModifyFlag();
	
	//�r���[�ĕ`��
	m_pcEditWnd->Views_RedrawAll();

	//	Sep. 10, 2002 genta
	SetFilePath( pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */

	SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = false;	/* �ǂݎ���p���[�h */

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();
end_of_func:;

	if( IsFilePathAvailable() &&
		!m_bReadOnly && /* �ǂݎ���p���[�h �ł͂Ȃ� */
		TRUE == bRet
	){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	//	Sep. 7, 2003 genta
	//	�t�@�C���X�V�̒ʒm�����ɖ߂�
	m_eWatchUpdate = wuSave;


	return bRet;
}



/* �u�t�@�C�����J���v�_�C�A���O */
//	Mar. 30, 2003 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
BOOL CEditDoc::OpenFileDialog(
	HWND				hwndParent,
	const TCHAR*		pszOpenFolder,	//<! [in]  NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	TCHAR*				pszPath,		//<! [out] �J���t�@�C���̃p�X���󂯎��A�h���X
	ECodeType*			pnCharCode,		//<! [out] �w�肳�ꂽ�����R�[�h��ʂ��󂯎��A�h���X
	bool*				pbReadOnly		//<! [out] �ǂݎ���p��
)
{
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndParent );

	TCHAR**	ppszMRU;
	TCHAR**	ppszOPENFOLDER;
	BOOL	bRet;

	/* MRU���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new TCHAR*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);


	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new TCHAR*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �����t�H���_�̐ݒ� */
	// pszFolder�̓t�H���_�������A�t�@�C�����t���p�X��n���Ă�CDlgOpenFile���ŏ������Ă����
	SFilePath pszCurDir;
	const TCHAR* pszDefFolder;
	if( pszOpenFolder!=NULL ){
		pszDefFolder = pszOpenFolder;
	}else{
		if( IsFilePathAvailable() ){
			pszDefFolder = GetFilePath();
		// Mar. 28, 2003 genta �J�����g�f�B���N�g����MRU���D�悳����
		//}else if( ppszMRU[0] != NULL && ppszMRU[0][0] != '\0' ){ // Sep. 9, 2002 genta
		//	pszDefFolder = ppszMRU[0];
		}else{ // 2002.10.25 Moca
			int nCurDir;
			nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
			if( 0 == nCurDir || _MAX_PATH < nCurDir ){
				pszDefFolder = _T("");
			}else{
				pszDefFolder = pszCurDir;
			}
		}
	}
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	m_pcEditWnd->m_cDlgOpenFile.Create2(
		m_hInstance,
		hwndParent,
		_T("*.*"),
		pszDefFolder,
		ppszMRU,
		ppszOPENFOLDER
	);
	
	bRet = m_pcEditWnd->m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
//	delete [] pszCurDir;
	return bRet;
}


//pszOpenFolder pszOpenFolder


/*! �u�t�@�C������t���ĕۑ��v�_�C�A���O

	@param pszPath [out]	�ۑ��t�@�C����
	@param pnCharCode [out]	�ۑ������R�[�h�Z�b�g
	@param pcEol [out]		�ۑ����s�R�[�h

	@date 2001.02.09 genta	���s�R�[�h�����������ǉ�
	@date 2003.03.30 genta	�t�@�C�������莞�̏����f�B���N�g�����J�����g�t�H���_��
	@date 2003.07.20 ryoji	BOM�̗L�������������ǉ�
	@date 2006.11.10 ryoji	���[�U�[�w��̊g���q���󋵈ˑ��ŕω�������
*/
BOOL CEditDoc::SaveFileDialog( TCHAR* pszPath, ECodeType* pnCharCode, CEOL* pcEol, BOOL* pbBomExist )
{
	TCHAR**	ppszMRU;		//	�ŋ߂̃t�@�C��
	TCHAR**	ppszOPENFOLDER;	//	�ŋ߂̃t�H���_
	TCHAR*	pszCurDir = NULL;
	TCHAR	szDefaultWildCard[_MAX_PATH + 10];	// ���[�U�[�w��g���q
	BOOL	bret;

	/* MRU���X�g�̃t�@�C���̃��X�g */
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new TCHAR*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);

	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new TCHAR*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �t�@�C���ۑ��_�C�A���O�̏����� */
	/* �t�@�C�����̖����t�@�C����������AppszMRU[0]���f�t�H���g�t�@�C�����Ƃ��āHppszOPENFOLDER����Ȃ��H */
	// �t�@�C�����̖����Ƃ��̓J�����g�t�H���_���f�t�H���g�ɂ��܂��BMar. 30, 2003 genta
	// �f���v�] No.2699 (2003/02/05)
	const TCHAR*	pszDefFolder; // �f�t�H���g�t�H���_
	if( !IsFilePathAvailable() ){
		// 2002.10.25 Moca ����̃R�[�h�𗬗p Mar. 23, 2003 genta
		int nCurDir;
		pszCurDir = new TCHAR[_MAX_PATH];
		nCurDir = ::GetCurrentDirectory( _MAX_PATH, pszCurDir );
		if( 0 == nCurDir || _MAX_PATH < nCurDir ){
			pszDefFolder = _T("");
		}else{
			pszDefFolder = pszCurDir;
		}
		_tcscpy(szDefaultWildCard, _T("*.txt"));
		if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveNew )
			_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
	}else{
		pszDefFolder = GetFilePath();
		TCHAR	szExt[_MAX_EXT];
		_tsplitpath(GetFilePath(), NULL, NULL, NULL, szExt);
		if( szExt[0] == _T('.') && szExt[1] != _T('\0') ){
			_tcscpy(szDefaultWildCard, _T("*"));
			_tcscat(szDefaultWildCard, szExt);
			if( m_pShareData->m_Common.m_sFile.m_bNoFilterSaveFile )
				_tcscat(szDefaultWildCard, _T(";*.*"));	// �S�t�@�C���\��
		}else{
			_tcscpy(szDefaultWildCard, _T("*.*"));
		}
	}
	m_pcEditWnd->m_cDlgOpenFile.Create2( m_hInstance, GetSplitterHwnd(), szDefaultWildCard, pszDefFolder,
		ppszMRU, ppszOPENFOLDER );

	/* �_�C�A���O��\�� */
	//	Jul. 26, 2003 ryoji pbBomExist�ǉ�
	bret = m_pcEditWnd->m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol, pbBomExist );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	delete [] pszCurDir;
	return bret;
}





/*! ���ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int		i;
//	BOOL	bModify;
	
	// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
	// ���ʐݒ�̈ꎞ�ݒ�̈��SharaData���R�s�[����
	m_pcEditWnd->m_cPropCommon.InitData();
	
	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_pcEditWnd->m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){

		// 2002.12.11 Moca ���̕����ōs���Ă����f�[�^�̃R�s�[��CPropCommon�Ɉړ��E�֐���
		// ShareData �� �ݒ��K�p�E�R�s�[����
		// 2007.06.20 ryoji �O���[�v���ɕύX���������Ƃ��̓O���[�vID�����Z�b�g����
		BOOL bGroup = (m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin);
		m_pcEditWnd->m_cPropCommon.ApplyData();
		m_pcSMacroMgr->UnloadAll();	// 2007.10.19 genta �}�N���o�^�ύX�𔽉f���邽�߁C�ǂݍ��ݍς݂̃}�N����j������
		if( bGroup != (m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin ) ){
			CShareData::getInstance()->ResetGroupId();
		}

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �t�H���g���ς���� */
		for( i = 0; i < 4; ++i ){
			m_pcEditWnd->m_pcEditViewArr[i]->m_cTipWnd.ChangeFont( &(m_pShareData->m_Common.m_sHelper.m_lf_kh) );
		}

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)GetOwnerHwnd(),
			GetOwnerHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	m_pcEditWnd->m_cPropTypes.SetTypeData( m_pShareData->m_Types[nSettingType] );
	// Mar. 31, 2003 genta �������팸�̂��߃|�C���^�ɕύX��ProperySheet���Ŏ擾����悤��
	//m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_pcEditWnd->m_cPropTypes.DoPropertySheet( nPageNum ) ){
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		m_pcEditWnd->m_cPropTypes.GetTypeData( m_pShareData->m_Types[nSettingType] );

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessageAny( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
		CShareData::getInstance()->SendMessageToAllEditors(
			MYWM_CHANGESETTING,
			0,
			(LPARAM)GetOwnerHwnd(),
			GetOwnerHwnd()
		);

		return TRUE;
	}else{
		return FALSE;
	}
}



/* Undo(���ɖ߂�)�\�ȏ�Ԃ��H */
BOOL CEditDoc::IsEnableUndo( void )
{
	return m_cOpeBuf.IsEnableUndo();
}



/*! Redo(��蒼��)�\�ȏ�Ԃ��H */
BOOL CEditDoc::IsEnableRedo( void )
{
	return m_cOpeBuf.IsEnableRedo();
}




/*! �N���b�v�{�[�h����\��t���\���H */
BOOL CEditDoc::IsEnablePaste( void )
{
	return CClipboard::HasValidData()?TRUE:FALSE;
}





/*! �e�E�B���h�E�̃^�C�g�����X�V

	@date 2007.03.08 ryoji bKillFocus�p�����[�^������
*/
void CEditDoc::SetParentCaption( void )
{
	if( NULL == GetSplitterHwnd() ){
		return;
	}
	if( !m_pcEditWnd->GetActiveView().GetDrawSwitch() ){
		return;
	}

//	/* �A�C�R��������Ă��Ȃ����̓t���p�X */
//	/* �A�C�R��������Ă��鎞�̓t�@�C�����̂� */
//	if( ::IsIconic( GetSplitterHwnd() ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}

	wchar_t	pszCap[1024];	//	Nov. 6, 2000 genta �I�[�o�[�w�b�h�y���̂���Heap��Stack�ɕύX

	// From Here Apr. 04, 2003 genta / Apr.05 ShareData�̃p�����[�^���p��
	if( !m_pcEditWnd->IsActiveApp() ){	// 2007.03.08 ryoji bKillFocus��IsActiveApp()�ɕύX
		ExpandParameter(
			to_wchar(m_pShareData->m_Common.m_sWindow.m_szWindowCaptionInactive),
			pszCap,
			_countof( pszCap )
		);
	}
	else {
		ExpandParameter(
			to_wchar(m_pShareData->m_Common.m_sWindow.m_szWindowCaptionActive),
			pszCap,
			_countof( pszCap )
		);
	}
	// To Here Apr. 04, 2003 genta

	TCHAR tszBuf[256];
	_wcstotcs(tszBuf, pszCap, _countof(tszBuf));
	::SetWindowText( GetOwnerHwnd(), tszBuf );

	//@@@ From Here 2003.06.13 MIK
	//�^�u�E�C���h�E�̃t�@�C������ʒm
	ExpandParameter( m_pShareData->m_Common.m_sTabBar.m_szTabWndCaption, pszCap, _countof( pszCap ));
	m_pcEditWnd->ChangeFileNameNotify( to_tchar(pszCap), GetFilePath(), m_bGrepMode );	// 2006.01.28 ryoji �t�@�C�����AGrep���[�h�p�����[�^��ǉ�
	//@@@ To Here 2003.06.13 MIK

	return;
}




/*! �o�b�N�A�b�v�̍쐬
	@author genta
	@date 2001.06.12 asa-o
		�t�@�C���̎��������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\
	@date 2001.12.11 MIK �o�b�N�A�b�v�t�@�C�����S�~���ɓ����@�\
	@date 2004.06.05 genta �o�b�N�A�b�v�Ώۃt�@�C���������ŗ^����悤�ɁD
		���O��t���ĕۑ��̎��͎����̃o�b�N�A�b�v������Ă����Ӗ��Ȃ̂ŁD
		�܂��C�o�b�N�A�b�v���ۑ����s��Ȃ��I������ǉ��D
	@date 2005.11.26 aroka �t�@�C����������FormatBackUpPath�ɕ���

	@retval 0 �o�b�N�A�b�v�쐬���s�D
	@retval 1 �o�b�N�A�b�v�쐬�����D
	@retval 2 �o�b�N�A�b�v�쐬���s�D�ۑ����f�w���D
	@retval 3 �t�@�C������G���[�ɂ��o�b�N�A�b�v�쐬���s�D
	
	@todo Advanced mode�ł̐���Ǘ�
*/
int CEditDoc::MakeBackUp( const TCHAR* target_file )
{
	int		nRet;

	/* �o�b�N�A�b�v�\�[�X�̑��݃`�F�b�N */
	//	Aug. 21, 2005 genta �������݃A�N�Z�X�����Ȃ��ꍇ��
	//	�t�@�C�����Ȃ��ꍇ�Ɠ��l�ɉ������Ȃ�
	if( (_taccess( target_file, 2 )) == -1 ){
		return 0;
	}

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpFolder ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		//	Aug. 21, 2005 genta �w��t�H���_���Ȃ��ꍇ�Ɍx��
		if( (_taccess( m_pShareData->m_Common.m_sBackup.m_szBackUpFolder, 0 )) == -1 ){

			int nMsgResult = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�G���[",
				"�ȉ��̃o�b�N�A�b�v�t�H���_��������܂���D\n%ts\n"
				"�o�b�N�A�b�v���쐬�����ɏ㏑���ۑ����Ă�낵���ł����D",
				m_pShareData->m_Common.m_sBackup.m_szBackUpFolder
			);
			if( nMsgResult == IDYES ){
				return 0;//	�ۑ��p��
			}
			else {
				return 2;// �ۑ����f
			}
		}
	}

	TCHAR	szPath[_MAX_PATH];
	FormatBackUpPath( szPath, target_file );

	//@@@ 2002.03.23 start �l�b�g���[�N�E�����[�o�u���h���C�u�̏ꍇ�͂��ݔ��ɕ��荞�܂Ȃ�
	bool dustflag = false;
	if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox ){
		dustflag = !IsLocalDrive( szPath );
	}
	//@@@ 2002.03.23 end

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpDialog ){	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		::MessageBeep( MB_ICONQUESTION );
//From Here Feb. 27, 2001 JEPROtest �L�����Z�����ł���悤�ɂ��A���b�Z�[�W��ǉ�����
//		if( IDYES != MYMESSAGEBOX(
//			GetSplitterHwnd(),
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"�o�b�N�A�b�v�쐬�̊m�F",
//			"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H\n\n%ls\n    ��\n%ls\n\n",
//			IsFilePathAvailable() ? GetFilePath() : "�i����j",
//			szPath
//		) ){
//			return FALSE;
//		}
		if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox && dustflag == false ){	//@@@ 2001.12.11 add start MIK	//2002.03.23
			nRet = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%ls\n    ��\n%ls\n\n�쐬�����o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�݂܂��B\n",
				target_file,
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX_A(
				GetSplitterHwnd(),
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%ls\n    ��\n%ls\n\n",
				//IsFilePathAvailable() ? GetFilePath() : "�i����j",
				//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
				target_file,
				szPath
			);	//Jul. 06, 2001 jepro [���O��t���ĕۑ�] �̏ꍇ������̂Ń��b�Z�[�W���C��
		}	//@@@ 2001.12.11 add MIK
		//	Jun.  5, 2005 genta �߂�l�ύX
		if( IDNO == nRet ){
			return 0;//	�ۑ��p��
		}else if( IDCANCEL == nRet ){
			return 2;// �ۑ����f
		}
//To Here Feb. 27, 2001
	}

	//	From Here Aug. 16, 2000 genta
	//	Jun.  5, 2005 genta 1�̊g���q���c���ł�ǉ�
	if( m_pShareData->m_Common.m_sBackup.GetBackupType() == 3 ||
		m_pShareData->m_Common.m_sBackup.GetBackupType() == 6 ){
		//	���ɑ��݂���Backup�����炷����
		int				i;

		//	�t�@�C�������p
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		TCHAR*	pBase = szPath + _tcslen( szPath ) - 2;	//	2: �g���q�̍Ō��2���̈Ӗ�
		//::MessageBoxA( NULL, pBase, _T("���������ꏊ"), MB_OK );

		//------------------------------------------------------------------
		//	1. �Y���f�B���N�g������backup�t�@�C����1���T��
		for( i = 0; i <= 99; i++ ){	//	�ő�l�Ɋւ�炸�C99�i2���̍ő�l�j�܂ŒT��
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );

			hFind = ::FindFirstFile( szPath, &fData );
			if( hFind == INVALID_HANDLE_VALUE ){
				//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
				break;
			}
			::FindClose( hFind );
			//	���������t�@�C���̑������`�F�b�N
			//	�͖ʓ|���������炵�Ȃ��D
			//	�������O�̃f�B���N�g������������ǂ��Ȃ�̂��낤...
		}
		--i;

		//------------------------------------------------------------------
		//	2. �ő�l���琧����-1�Ԃ܂ł��폜
		int boundary = m_pShareData->m_Common.m_sBackup.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	�ŏ��l��0
		//::MessageBoxA( NULL, pBase, _T("���������ꏊ"), MB_OK );

		for( ; i >= boundary; --i ){
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( GetSplitterHwnd(), szPath, _T("�폜���s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
				//	���s�����ꍇ
				//	��ōl����
			}
		}

		//	���̈ʒu��i�͑��݂���o�b�N�A�b�v�t�@�C���̍ő�ԍ���\���Ă���D

		//	3. ��������0�Ԃ܂ł̓R�s�[���Ȃ���ړ�
		TCHAR szNewPath[MAX_PATH];
		TCHAR *pNewNrBase;

		_tcscpy( szNewPath, szPath );
		pNewNrBase = szNewPath + _tcslen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	�t�@�C�������Z�b�g
			auto_sprintf( pBase, _T("%02d"), i );
			auto_sprintf( pNewNrBase, _T("%02d"), i + 1 );

			//	�t�@�C���̈ړ�
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	���s�����ꍇ
				//	��ōl����
				::MessageBox( GetSplitterHwnd(), szPath, _T("�ړ����s"), MB_OK );
				//	Jun.  5, 2005 genta �߂�l�ύX
				//	���s���Ă��ۑ��͌p��
				return 0;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBoxA( NULL, szPath, _T("���O�̃o�b�N�A�b�v�t�@�C��"), MB_OK );
	/* �o�b�N�A�b�v�̍쐬 */
	//	Aug. 21, 2005 genta ���݂̃t�@�C���ł͂Ȃ��^�[�Q�b�g�t�@�C�����o�b�N�A�b�v����悤��
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];
	_tsplitpath( szPath, szDrive, szDir, szFname, szExt );
	TCHAR	szPath2[MAX_PATH];
	auto_sprintf( szPath2, _T("%ts%ts"), szDrive, szDir );

	HANDLE			hFind;
	WIN32_FIND_DATA	fData;

	hFind = ::FindFirstFile( szPath2, &fData );
	if( hFind == INVALID_HANDLE_VALUE ){
		//	�����Ɏ��s���� == �t�@�C���͑��݂��Ȃ�
		::CreateDirectory( szPath2, NULL );
	}
	::FindClose( hFind );

	if( ::CopyFile( target_file, szPath, FALSE ) ){
		/* ����I�� */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_sBackup.m_bBackUpDustBox && dustflag == false ){	//@@@ 2002.03.23 �l�b�g���[�N�E�����[�o�u���h���C�u�łȂ�
			TCHAR	szDustPath[_MAX_PATH+1];
			_tcscpy(szDustPath, szPath);
			szDustPath[_tcslen(szDustPath) + 1] = _T('\0');
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = GetSplitterHwnd();
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//�_�C�A���O�Ȃ�
			fos.fAnyOperationsAborted = true; //false;
			fos.hNameMappings = NULL;
			fos.lpszProgressTitle = NULL; //"�o�b�N�A�b�v�t�@�C�������ݔ��Ɉړ����Ă��܂�...";
			if( ::SHFileOperation(&fos) == 0 ){
				/* ����I�� */
			}else{
				/* �G���[�I�� */
			}
		}
		//@@@ 2001.12.11 end MIK
	}else{
		/* �G���[�I�� */
		//	Jun.  5, 2005 genta �߂�l�ύX
		return 3;
	}
	//	Jun.  5, 2005 genta �߂�l�ύX
	return 1;
}

/*! �o�b�N�A�b�v�̍쐬

	@param[out] szNewPath �o�b�N�A�b�v��p�X��
	@param[in]  target_file �o�b�N�A�b�v���p�X��

	@author aroka
	@date 2005.11.29 aroka
		MakeBackUp���番���D���������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\�ǉ�

	@retval true
	
	@todo Advanced mode�ł̐���Ǘ�
*/
bool CEditDoc::FormatBackUpPath( TCHAR* szNewPath, const TCHAR* target_file )
{
	TCHAR	szDrive[_MAX_DIR];
	TCHAR	szDir[_MAX_DIR];
	TCHAR	szFname[_MAX_FNAME];
	TCHAR	szExt[_MAX_EXT];

	/* �p�X�̕��� */
	_tsplitpath( target_file, szDrive, szDir, szFname, szExt );

	if( m_pShareData->m_Common.m_sBackup.m_bBackUpFolder ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		_tcscpy( szNewPath, m_pShareData->m_Common.m_sBackup.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		AddLastYenFromDirectoryPath( szNewPath );
	}
	else{
		auto_sprintf( szNewPath, _T("%ts%ts"), szDrive, szDir );
	}

	/* ���΃t�H���_��}�� */
	if( !m_pShareData->m_Common.m_sBackup.m_bBackUpPathAdvanced ){
		time_t	ltime;
		struct	tm *today, *gmt;
		wchar_t	szTime[64];
		wchar_t	szForm[64];

		TCHAR*	pBase;
		pBase = szNewPath + _tcslen( szNewPath );

		/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
		switch( m_pShareData->m_Common.m_sBackup.GetBackupType() ){
		case 1:
			auto_sprintf( pBase, _T("%ls.bak"), szFname );
			break;
		case 5: //	Jun.  5, 2005 genta 1�̊g���q���c����
			auto_sprintf( pBase, _T("%ts%ts.bak"), szFname, szExt );
			break;
		case 2:	//	���t�C����
			_tzset();
			_wstrdate( szTime );
			time( &ltime );				/* �V�X�e�������𓾂܂� */
			gmt = gmtime( &ltime );		/* �����W�����ɕϊ����� */
			today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

			wcscpy( szForm, L"" );
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				wcscat( szForm, L"%Y" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				wcscat( szForm, L"%m" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				wcscat( szForm, L"%d" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				wcscat( szForm, L"%H" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				wcscat( szForm, L"%M" );
			}
			if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				wcscat( szForm, L"%S" );
			}
			/* YYYYMMDD�����b �`���ɕϊ� */
			wcsftime( szTime, _countof( szTime ) - 1, szForm, today );
			auto_sprintf( pBase, _T("%ts_%ls%ls"), szFname, szTime, szExt );
			break;
	//	2001/06/12 Start by asa-o: �t�@�C���ɕt������t��O��̕ۑ���(�X�V����)�ɂ���
		case 4:	//	���t�C����
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// ���n�����ɕϊ�
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// �V�X�e���^�C���ɕϊ�

				wcscpy( szTime, L"" );
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
					auto_sprintf(szTime,L"%d",SystemTime.wYear);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wMonth);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wDay);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wHour);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wMinute);
				}
				if( m_pShareData->m_Common.m_sBackup.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
					auto_sprintf(szTime,L"%ls%02d",szTime,SystemTime.wSecond);
				}
				auto_sprintf( pBase, _T("%ts_%ls%ts"), szFname, szTime, szExt );
			}
			break;
	// 2001/06/12 End

		case 3: //	?xx : xx = 00~99, ?�͔C�ӂ̕���
		case 6: //	Jun.  5, 2005 genta 3�̊g���q���c����
			//	Aug. 15, 2000 genta
			//	�����ł͍쐬����o�b�N�A�b�v�t�@�C�����̂ݐ�������D
			//	�t�@�C������Rotation�͊m�F�_�C�A���O�̌�ōs���D
			{
				//	Jun.  5, 2005 genta �g���q���c����悤�ɏ����N�_�𑀍삷��
				TCHAR* ptr;
				if( m_pShareData->m_Common.m_sBackup.GetBackupType() == 3 ){
					ptr = szExt;
				}
				else {
					ptr = szExt + _tcslen( szExt );
				}
				*ptr   = _T('.');
				*++ptr = m_pShareData->m_Common.m_sBackup.GetBackupExtChar();
				*++ptr = _T('0');
				*++ptr = _T('0');
				*++ptr = _T('\0');
			}
			auto_sprintf( pBase, _T("%ts%ts"), szFname, szExt );
			break;
		}

	}else{ // �ڍאݒ�g�p����
		TCHAR szFormat[1024];

		switch( m_pShareData->m_Common.m_sBackup.GetBackupTypeAdv() ){
		case 4:	//	�t�@�C���̓��t�C����
			{
				FILETIME	LastWriteTime,
							LocalTime;
				SYSTEMTIME	SystemTime;

				// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX
				if( ! GetLastWriteTimestamp( target_file, LastWriteTime )){
					LastWriteTime.dwHighDateTime = LastWriteTime.dwLowDateTime = 0;
				}
				::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// ���n�����ɕϊ�
				::FileTimeToSystemTime(&LocalTime,&SystemTime);			// �V�X�e���^�C���ɕϊ�

				GetDateTimeFormat( szFormat, _countof(szFormat), m_pShareData->m_Common.m_sBackup.m_szBackUpPathAdvanced , SystemTime );
			}
			break;
		case 2:	//	���݂̓��t�C����
		default:
			{
				time_t	ltime;
				struct	tm *today;

				time( &ltime );				/* �V�X�e�������𓾂܂� */
				today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

				/* YYYYMMDD�����b �`���ɕϊ� */
				_tcsftime( szFormat, _countof( szFormat ) - 1, m_pShareData->m_Common.m_sBackup.m_szBackUpPathAdvanced , today );
			}
			break;
		}

		{
			// make keys
			// $0-$9�ɑΉ�����t�H���_����؂�o��
			TCHAR keybuff[1024];
			_tcscpy( keybuff, szDir );
			CutLastYenFromDirectoryPath( keybuff );

			TCHAR *folders[10];
			{
				//	Jan. 9, 2006 genta VC6�΍�
				int idx;
				for( idx=0; idx<10; ++idx ){
					folders[idx] = 0;
				}
				folders[0] = szFname;

				for( idx=1; idx<10; ++idx ){
					TCHAR *cp;
					cp = auto_strchr(keybuff, _T('\\'));
					if( cp != NULL ){
						folders[idx] = cp+1;
						*cp = _T('\0');
					}
					else{
						break;
					}
				}
			}
			{
				// $0-$9��u��
				//wcscpy( szNewPath, L"" );
				TCHAR *q= szFormat;
				TCHAR *q2 = szFormat;
				while( *q ){
					if( *q==_T('$') ){
						++q;
						if( isdigit(*q) ){
							q[-1] = _T('\0');
							_tcscat( szNewPath, q2 );
							if( folders[*q-_T('0')] != 0 ){
								_tcscat( szNewPath, folders[*q-_T('0')] );
							}
							q2 = q+1;
						}
					}
					++q;
				}
				_tcscat( szNewPath, q2 );
			}
		}
		{
			TCHAR temp[1024];
			TCHAR *cp;
			//	2006.03.25 Aroka szExt[0] == '\0'�̂Ƃ��̃I�[�o���������C��
			TCHAR *ep = (szExt[0]!=0) ? &szExt[1] : &szExt[0];

			while( _tcschr( szNewPath, _T('*') ) ){
				_tcscpy( temp, szNewPath );
				cp = _tcschr( temp, _T('*') );
				*cp = 0;
				auto_sprintf( szNewPath, _T("%ts%ts%ts"), temp, ep, cp+1 );
			}
			//	??�̓o�b�N�A�b�v�A�Ԃɂ������Ƃ���ł͂��邪�C
			//	�A�ԏ����͖�����2���ɂ����Ή����Ă��Ȃ��̂�
			//	�g�p�ł��Ȃ�����?��_�ɕϊ����Ă��������
			while(( cp = _tcschr( szNewPath, _T('?') ) ) != NULL){
				*cp = _T('_');
			}
		}
	}
	return true;
}

/* �t�@�C���̔r�����b�N */
void CEditDoc::DoFileLock( void )
{
	BOOL	bCheckOnly;

	/* ���b�N���Ă��� */
	if( NULL != m_hLockedFile ){
		/* ���b�N���� */
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = NULL;
	}

	/* �t�@�C�������݂��Ȃ� */
	if( -1 == _taccess( GetFilePath(), 0 ) ){
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
		return;
	}else{
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = m_pShareData->m_Common.m_sFile.m_nFileShareMode;
	}


	/* �t�@�C�����J���Ă��Ȃ� */
	if( ! IsFilePathAvailable() ){
		return;
	}
	/* �ǂݎ���p���[�h */
	if( m_bReadOnly ){
		return;
	}


	if( m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_WRITE ||
		m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_DENY_READWRITE ){
		bCheckOnly = FALSE;
	}
	else{
		/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���̂�return���Ȃ� */
//		return;
		bCheckOnly = TRUE;
	}
	/* �����݋֎~���ǂ������ׂ� */
	if( -1 == _taccess( GetFilePath(), 2 ) ){	/* �A�N�Z�X���F�������݋��� */
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}

	//�������߂邩����
	if( !IsFileWritable(GetFilePath()) ){
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			GetSplitterHwnd(),
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ts\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B"),
			IsFilePathAvailable() ? GetFilePath() : _T("�i����j")
		);
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}

	//������kobake��: ���̊Ԃ̔����Ȏ��ԍ��ɒ��ӁB�����Ɨǂ�����������͂��B

	//�I�[�v��
	m_hLockedFile = OpenFile_InShareMode( GetFilePath(), m_pShareData->m_Common.m_sFile.m_nFileShareMode );
	if( INVALID_HANDLE_VALUE == m_hLockedFile ){
		const TCHAR*	pszMode;
		switch( m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
		case SHAREMODE_DENY_READWRITE:	/* �ǂݏ��� */
			pszMode = _T("�ǂݏ����֎~���[�h");
			break;
		case SHAREMODE_DENY_WRITE:	/* ���� */
			pszMode = _T("�������݋֎~���[�h");
			break;
		default:
			pszMode = _T("����`�̃��[�h�i��肪����܂��j");
			break;
		}
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			GetSplitterHwnd(),
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("%ls\n��%ls�Ń��b�N�ł��܂���ł����B\n���݂��̃t�@�C���ɑ΂���r������͖����ƂȂ�܂��B"),
			IsFilePathAvailable() ? GetFilePath() : _T("�i����j"),
			pszMode
		);
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}
	/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���ꍇ */
	if( bCheckOnly ){
		/* ���b�N���������� */
		DoFileUnLock();

	}
	return;
}


/* �t�@�C���̔r�����b�N���� */
void CEditDoc::DoFileUnLock( void )
{
	if( NULL != m_hLockedFile ){
		/* ���b�N���� */
		::CloseHandle( m_hLockedFile );
		m_hLockedFile = NULL;

		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;
	}
	return;
}

/*
	C�֐����X�g�쐬��CEditDoc_FuncList1.cpp�ֈړ�
*/

/*! PL/SQL�֐����X�g�쐬 */
void CEditDoc::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int			i;
	int			nCharChars;
	wchar_t		szWordPrev[100];
	wchar_t		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	wchar_t		szFuncName[80];
	CLogicInt	nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = L'\0';
	szWord[nWordIdx] = L'\0';
	nMode = 0;
	nFuncNum = 0;
	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			// 2005-09-02 D.S.Koba GetSizeOfChar
			nCharChars = CNativeW::GetSizeOfChar( pLine, nLineLen, i );
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( L'\'' == pLine[i] ){
					if( i + 1 < nLineLen && L'\'' == pLine[i + 1] ){
						++i;
					}else{
						nMode = 0;
						continue;
					}
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i + 1 < nLineLen && L'*' == pLine[i] &&  L'/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
					) )
				 || 2 == nCharChars
				){
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
				else{
					if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 0 == nParseCnt && 0 == wcsicmp( szWord, L"PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + CLogicInt(1);
					}
					else if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == wcsicmp( szWord, L"BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}
					else if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							wcscpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == wcsicmp( szWord, L"IS" ) ){
							if( 1 == nFuncOrProc ){
								nFuncId = 11;	/* �t�@���N�V�����{�� */
							}else
							if( 2 == nFuncOrProc ){
								nFuncId = 21;	/* �v���V�[�W���{�� */
							}else
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
							}
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							CLayoutPoint ptPos;
							m_cLayoutMgr.LogicToLayout(
								CLogicPoint(0, nFuncLine - 1),
								&ptPos
							);
							pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1), szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == wcsicmp( szWord, L"AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
							else if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								CLayoutPoint ptPos;
								m_cLayoutMgr.LogicToLayout(
									CLogicPoint(0, nFuncLine - 1),
									&ptPos
								);
								pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( L'_' == pLine[i] ||
					L'~' == pLine[i] ||
					(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
					(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
					(L'0' <= pLine[i] &&	pLine[i] <= L'9' )||
					L'\t' == pLine[i] ||
					 L' ' == pLine[i] ||
					 WCODE::CR == pLine[i] ||
					 WCODE::LF == pLine[i] ||
					 L'{' == pLine[i] ||
					 L'}' == pLine[i] ||
					 L'(' == pLine[i] ||
					 L')' == pLine[i] ||
					 L';' == pLine[i] ||
					L'\'' == pLine[i] ||
					 L'/' == pLine[i] ||
					 L'-' == pLine[i]
				){
					wcscpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = L'\0';
					nMode = 0;
					i--;
					continue;
				}else{
//					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( L'\t' == pLine[i] ||
					L' ' == pLine[i] ||
					WCODE::CR == pLine[i] ||
					WCODE::LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && L'-' == pLine[i] &&  L'-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && L'/' == pLine[i] &&  L'*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( L'\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( L';' == pLine[i] ){
					if( 2 == nParseCnt ){
						if( 1 == nFuncOrProc ){
							nFuncId = 10;	/* �t�@���N�V�����錾 */
						}else{
							nFuncId = 20;	/* �v���V�[�W���錾 */
						}
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						CLayoutPoint ptPos;
						m_cLayoutMgr.LogicToLayout(
							CLogicPoint(0, nFuncLine - 1),
							&ptPos
						);
						pcFuncInfoArr->AppendData( nFuncLine, ptPos.GetY2() + CLayoutInt(1) , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						L'_' == pLine[i] ||
						L'~' == pLine[i] ||
						(L'a' <= pLine[i] &&	pLine[i] <= L'z' )||
						(L'A' <= pLine[i] &&	pLine[i] <= L'Z' )||
						(L'0' <= pLine[i] &&	pLine[i] <= L'9' )
						) )
					 || 2 == nCharChars
					){
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						wcscpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						wmemcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = L'\0';
						nWordIdx += (nCharChars);

						nMode = 2;
					}
				}
			}
			i += (nCharChars - 1);
		}
	}
	return;
}





/*!	�e�L�X�g�E�g�s�b�N���X�g�쐬
	
	@date 2002.04.01 YAZAKI CDlgFuncList::SetText()���g�p����悤�ɉ����B
	@date 2002.11.03 Moca	�K�w���ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
							�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.8��   kobake �@�B�I��UNICODE��
	@date 2007.11.29 kobake UNICODE�Ή��ł��ĂȂ������̂ŏC��
*/
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	using namespace WCODE;

	//���o���L��
	const wchar_t*	pszStarts = m_pShareData->m_Common.m_sFormat.m_szMidashiKigou;
	int				nStartsLen = wcslen( pszStarts );

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	CLogicInt				nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine )break;

		//�s���̋󔒔�΂�
		int i;
		for( i = 0; i < nLineLen; ++i ){
			if( WCODE::isBlank(pLine[i]) ){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		if(NULL==wcschr(pszStarts,pLine[0]))continue;

		//���o����ނ̔��� -> szTitle
		if( pLine[i] == L'(' ){
			if(0){}
			else if ( IsInRange(pLine[i + 1], L'0', L'9') ) wcscpy( szTitle, L"(0)" ); //����
			else if ( IsInRange(pLine[i + 1], L'A', L'Z') ) wcscpy( szTitle, L"(A)" ); //�p�啶��
			else if ( IsInRange(pLine[i + 1], L'a', L'z') ) wcscpy( szTitle, L"(a)" ); //�p������
			else continue; //���u(�v�̎����p�����Ŗ����ꍇ�A���o���Ƃ݂Ȃ��Ȃ�
		}
		else if( IsInRange(pLine[i], L'�O', L'�X') ) wcscpy( szTitle, L"�O" ); // �S�p����
		else if( IsInRange(pLine[i], L'�@', L'�S') ) wcscpy( szTitle, L"�@" ); // �@�`�S
		else if( IsInRange(pLine[i], L'�T', L'�]') ) wcscpy( szTitle, L"�T" ); // �T�`�]
		else if( wcschr(L"�Z���O�l�ܘZ������\�S����Q��", pLine[i]) ) wcscpy( szTitle, L"��" ); //������
		else{
			szTitle[0]=pLine[i];
			szTitle[1]=L'\0';
		}

		/*	�u���o���L���v�Ɋ܂܂�镶���Ŏn�܂邩�A
			(0�A(1�A...(9�A(A�A(B�A...(Z�A(a�A(b�A...(z
			�Ŏn�܂�s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		wchar_t*	pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		for( i = 0; i < (int)wcslen(pszText); ++i ){
			if( pszText[i] == CR || pszText[i] == LF )pszText[i] = L'\0';
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepth���v�Z */
		int k;
		bool bAppend = true;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack > k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}
		else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[����
			// nDepth = nMaxStack;
			bAppend = false;
		}
		
		if( bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}


/*! ���[���t�@�C����1�s���Ǘ�����\����

	@date 2002.04.01 YAZAKI
	@date 2007.11.29 kobake ���O�ύX: oneRule��SOneRule
*/
struct SOneRule {
	wchar_t szMatch[256];
	int		nLength;
	wchar_t szGroupName[256];
};

/*! ���[���t�@�C����ǂݍ��݁A���[���\���̂̔z����쐬����

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca ����nMaxCount��ǉ��B�o�b�t�@���`�F�b�N������悤�ɕύX
*/
int CEditDoc::ReadRuleFile( const TCHAR* pszFilename, SOneRule* pcOneRule, int nMaxCount )
{
	long	i;
	// 2003.06.23 Moca ���΃p�X�͎��s�t�@�C������̃p�X�Ƃ��ĊJ��
	// 2007.05.19 ryoji ���΃p�X�͐ݒ�t�@�C������̃p�X��D��
	FILE*	pFile = _tfopen_absini( pszFilename, _T("r") );
	if( NULL == pFile ){
		return 0;
	}
	wchar_t	szLine[LINEREADBUFSIZE];
	const wchar_t*	pszDelimit = L" /// ";
	const wchar_t*	pszKeySeps = L",\0";
	wchar_t*	pszWork;
	int nDelimitLen = wcslen( pszDelimit );
	int nCount = 0;
	while( NULL != fgetws( szLine, _countof(szLine), pFile ) && nCount < nMaxCount ){
		pszWork = wcsstr( szLine, pszDelimit );
		if( NULL != pszWork && szLine[0] != L';' ){
			*pszWork = L'\0';
			pszWork += nDelimitLen;

			/* �ŏ��̃g�[�N�����擾���܂��B */
			wchar_t* pszToken = wcstok( szLine, pszKeySeps );
			while( NULL != pszToken ){
//				nRes = wcsicmp( pszKey, pszToken );
				for( i = 0; i < (int)wcslen(pszWork); ++i ){
					if( pszWork[i] == L'\r' ||
						pszWork[i] == L'\n' ){
						pszWork[i] = L'\0';
						break;
					}
				}
				wcsncpy( pcOneRule[nCount].szMatch, pszToken, 255 );
				wcsncpy( pcOneRule[nCount].szGroupName, pszWork, 255 );
				pcOneRule[nCount].szMatch[255] = L'\0';
				pcOneRule[nCount].szGroupName[255] = L'\0';
				pcOneRule[nCount].nLength = wcslen(pcOneRule[nCount].szMatch);
				nCount++;
				pszToken = wcstok( NULL, pszKeySeps );
			}
		}
	}
	fclose( pFile );
	return nCount;
}

/*! ���[���t�@�C�������ɁA�g�s�b�N���X�g���쐬

	@date 2002.04.01 YAZAKI
	@date 2002.11.03 Moca �l�X�g�̐[�����ő�l�𒴂���ƃo�b�t�@�I�[�o�[��������̂��C��
		�ő�l�ȏ�͒ǉ������ɖ�������
	@date 2007.11.29 kobake SOneRule test[1024] �ŃX�^�b�N�����Ă����̂��C��
*/
void CEditDoc::MakeFuncList_RuleFile( CFuncInfoArr* pcFuncInfoArr )
{
	wchar_t*		pszText;

	/* ���[���t�@�C���̓��e���o�b�t�@�ɓǂݍ��� */
	auto_array_ptr<SOneRule> test = new SOneRule[1024];	// 1024���B 2007.11.29 kobake �X�^�b�N�g�������Ȃ̂ŁA�q�[�v�Ɋm�ۂ���悤�ɏC���B
	int nCount = ReadRuleFile(GetDocumentAttribute().m_szOutlineRuleFilename, test.get(), 1024 );
	if ( nCount < 1 ){
		return;
	}

	/*	�l�X�g�̐[���́A32���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�256�����܂ŋ��
		�i256�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int	nMaxStack = 32;	//	�l�X�g�̍Ő[
	int			nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t		pszStack[nMaxStack][256];
	wchar_t		szTitle[256];			//	�ꎞ�̈�
	for( CLogicInt nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount )
	{
		//�s�擾
		CLogicInt		nLineLen;
		const wchar_t*	pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}

		//�s���̋󔒔�΂�
		int		i;
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == L' ' || pLine[i] == L'\t' || pLine[i] == L'�@'){
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}

		//�擪���������o���L���̂����ꂩ�ł���΁A���֐i��
		int		j;
		for( j = 0; j < nCount; j++ ){
			if ( 0 == wcsncmp( &pLine[i], test[j].szMatch, test[j].nLength ) ){
				wcscpy( szTitle, test[j].szGroupName );
				break;
			}
		}
		if( j >= nCount ){
			continue;
		}

		/*	���[���Ƀ}�b�`�����s�́A�A�E�g���C�����ʂɕ\������B
		*/

		//�s�����񂩂���s����菜�� pLine -> pszText
		pszText = new wchar_t[nLineLen + 1];
		wmemcpy( pszText, &pLine[i], nLineLen );
		pszText[nLineLen] = L'\0';
		int nTextLen = wcslen( pszText );
		for( i = 0; i < nTextLen; ++i ){
			if( pszText[i] == WCODE::CR || pszText[i] == WCODE::LF ){
				pszText[i] = L'\0';
				break;
			}
		}

		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		CLayoutPoint ptPos;
		m_cLayoutMgr.LogicToLayout(
			CLogicPoint(0, nLineCount),
			&ptPos
		);

		/* nDepth���v�Z */
		int k;
		BOOL bAppend;
		bAppend = TRUE;
		for ( k = 0; k < nDepth; k++ ){
			int nResult = wcscmp( pszStack[k], szTitle );
			if ( nResult == 0 ){
				break;
			}
		}
		if ( k < nDepth ){
			//	���[�v�r����break;���Ă����B�����܂łɓ������o�������݂��Ă����B
			//	�̂ŁA�������x���ɍ��킹��AppendData.
			nDepth = k;
		}
		else if( nMaxStack> k ){
			//	���܂܂łɓ������o�������݂��Ȃ������B
			//	�̂ŁApszStack�ɃR�s�[����AppendData.
			wcscpy(pszStack[nDepth], szTitle);
		}else{
			// 2002.11.03 Moca �ő�l�𒴂���ƃo�b�t�@�I�[�o�[�������邩��K������
			// nDepth = nMaxStack;
			bAppend = FALSE;
		}
		
		if( FALSE != bAppend ){
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , pszText, 0, nDepth );
			nDepth++;
		}
		delete [] pszText;

	}
	return;
}



/*! COBOL �A�E�g���C����� */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				k;
	wchar_t			szDivision[1024];
	wchar_t			szLabel[1024];
	const wchar_t*	pszKeyWord;
	int				nKeyWordLen;
	BOOL			bDivision;

	szDivision[0] = L'\0';
	szLabel[0] =  L'\0';


	CLogicInt	nLineCount;
	for( nLineCount = CLogicInt(0); nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* �R�����g�s�� */
		if( 7 <= nLineLen && pLine[6] == L'*' ){
			continue;
		}
		/* ���x���s�� */
		if( 8 <= nLineLen && pLine[7] != L' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == WCODE::CR
				 || pLine[i] == WCODE::LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == L' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != L' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = L'\0';
//			MYTRACE_A( "szLabel=[%ls]\n", szLabel );



			pszKeyWord = L"division";
			nKeyWordLen = wcslen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)wcslen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == auto_memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = L'\0';
					wcscpy( szDivision, szLabel );
					bDivision = TRUE;
					break;
				}
			}
			if( bDivision ){
				continue;
			}
			/*
			  �J�[�\���ʒu�ϊ�
			  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
			  ��
			  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
			*/

			CLayoutPoint ptPos;
			wchar_t	szWork[1024];
			m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			auto_sprintf( szWork, L"%ls::%ls", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1) , szWork, 0 );
		}
	}
	return;
}


/*! �A�Z���u�� �A�E�g���C�����

	@author MIK
	@date 2004.04.12 ��蒼��
*/
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	CLogicInt nTotalLine;

	nTotalLine = m_cDocLineMgr.GetLineCount();

	for( CLogicInt nLineCount = CLogicInt(0); nLineCount < nTotalLine; nLineCount++ ){
		const WCHAR* pLine;
		CLogicInt nLineLen;
		WCHAR* pTmpLine;
		int length;
		int offset;
#define MAX_ASM_TOKEN 2
		WCHAR* token[MAX_ASM_TOKEN];
		int j;
		WCHAR* p;

		//1�s�擾����B
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( pLine == NULL ) break;

		//��Ɨp�ɃR�s�[���쐬����B�o�C�i�����������炻�̌��͒m��Ȃ��B
		pTmpLine = wcsdup( pLine );
		if( pTmpLine == NULL ) break;
		if( wcslen( pTmpLine ) >= (unsigned int)nLineLen ){	//�o�C�i�����܂�ł�����Z���Ȃ�̂�...
			pTmpLine[ nLineLen ] = L'\0';	//�w�蒷�Ő؂�l��
		}

		//�s�R�����g�폜
		p = wcsstr( pTmpLine, L";" );
		if( p ) *p = L'\0';

		length = wcslen( pTmpLine );
		offset = 0;

		//�g�[�N���ɕ���
		for( j = 0; j < MAX_ASM_TOKEN; j++ ) token[ j ] = NULL;
		for( j = 0; j < MAX_ASM_TOKEN; j++ ){
			token[ j ] = my_strtok<WCHAR>( pTmpLine, length, &offset, L" \t\r\n" );
			if( token[ j ] == NULL ) break;
			//�g�[�N���Ɋ܂܂��ׂ������łȂ����H
			if( wcsstr( token[ j ], L"\"") != NULL
			 || wcsstr( token[ j ], L"\\") != NULL
			 || wcsstr( token[ j ], L"'" ) != NULL ){
				token[ j ] = NULL;
				break;
			}
		}

		if( token[ 0 ] != NULL ){	//�g�[�N����1�ȏ゠��
			int nFuncId = -1;
			WCHAR* entry_token = NULL;

			length = wcslen( token[ 0 ] );
			if( length >= 2
			 && token[ 0 ][ length - 1 ] == L':' ){	//���x��
				token[ 0 ][ length - 1 ] = L'\0';
				nFuncId = 51;
				entry_token = token[ 0 ];
			}
			else if( token[ 1 ] != NULL ){	//�g�[�N����2�ȏ゠��
				if( wcsicmp( token[ 1 ], L"proc" ) == 0 ){	//�֐�
					nFuncId = 50;
					entry_token = token[ 0 ];
				}else
				if( wcsicmp( token[ 1 ], L"endp" ) == 0 ){	//�֐��I��
					nFuncId = 52;
					entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("macro") ) == 0 ){	//�}�N��
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				//}else
				//if( my_stricmp( token[ 1 ], _T("struc") ) == 0 ){	//�\����
				//	nFuncId = -1;
				//	entry_token = token[ 0 ];
				}
			}

			if( nFuncId >= 0 ){
				/*
				  �J�[�\���ʒu�ϊ�
				  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
				  ��
				  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
				*/
				CLayoutPoint ptPos;
				m_cLayoutMgr.LogicToLayout(
					CLogicPoint(0, nLineCount),
					&ptPos
				);
				pcFuncInfoArr->AppendData( nLineCount + CLogicInt(1), ptPos.GetY2() + CLayoutInt(1), entry_token, nFuncId );
			}
		}

		free( pTmpLine );
	}

	return;
}



/*! �K�w�t���e�L�X�g �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2003.05.25 genta �������@�ꕔ�C��
	@date 2003.06.21 Moca �K�w��2�i�ȏ�[���Ȃ�ꍇ���l��
*/
void CEditDoc::MakeTopicList_wztxt(CFuncInfoArr* pcFuncInfoArr)
{
	int levelPrev = 0;

	for(CLogicInt nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		const wchar_t*	pLine;
		CLogicInt		nLineLen;

		pLine = m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		//	May 25, 2003 genta ���菇���ύX
		if( *pLine == L'.' )
		{
			const wchar_t* pPos;	//	May 25, 2003 genta
			int			nLength;
			wchar_t		szTitle[1024];

			//	�s���I�h�̐����K�w�̐[���𐔂���
			for( pPos = pLine + 1 ; *pPos == L'.' ; ++pPos )
				;

			CLayoutPoint ptPos;
			m_cLayoutMgr.LogicToLayout(
				CLogicPoint(0, nLineCount),
				&ptPos
			);
			
			int level = pPos - pLine;

			// 2003.06.27 Moca �K�w��2�i�ʏ�[���Ȃ�Ƃ��́A����̗v�f��ǉ�
			if( levelPrev < level && level != levelPrev + 1  ){
				int dummyLevel;
				// (����)��}��
				//	�������CTAG�ꗗ�ɂ͏o�͂���Ȃ��悤��
				for( dummyLevel = levelPrev + 1; dummyLevel < level; dummyLevel++ ){
					pcFuncInfoArr->AppendData(
						nLineCount+CLogicInt(1),
						ptPos.GetY2()+CLayoutInt(1),
						L"(����)",
						FUNCINFO_NOCLIPTEXT,
						dummyLevel - 1
					);
				}
			}
			levelPrev = level;

			nLength = auto_sprintf(szTitle,L"%d - ", level );
			
			wchar_t *pDest = szTitle + nLength; // �������ݐ�
			wchar_t *pDestEnd = szTitle + _countof(szTitle) - 2;
			
			while( pDest < pDestEnd )
			{
				if( *pPos ==L'\r' || *pPos ==L'\n' || *pPos == L'\0')
				{
					break;
				}
				else {
					*pDest++ = *pPos++;
				}
			}
			*pDest = L'\0';
			pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle, 0, level - 1);
		}
	}
}

/*! HTML �A�E�g���C�����

	@author zenryaku
	@date 2003.05.20 zenryaku �V�K�쐬
	@date 2004.04.19 zenryaku ��v�f�𔻒�
	@date 2004.04.20 Moca �R�����g�����ƁA�s���ȏI���^�O�𖳎����鏈����ǉ�
*/
void CEditDoc::MakeTopicList_html(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;
	BOOL			bEndTag;
	BOOL			bCommentTag = FALSE;

	/*	�l�X�g�̐[���́AnMaxStack���x���܂ŁA�ЂƂ̃w�b�_�́A�Œ�32�����܂ŋ��
		�i32�����܂œ����������瓯�����̂Ƃ��Ĉ����܂��j
	*/
	const int nMaxStack = 32;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t pszStack[nMaxStack][32];
	wchar_t szTitle[32];			//	�ꎞ�̈�
	CLogicInt			nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine)
		{
			break;
		}
		for(i=0;i<nLineLen-1;i++)
		{
			// 2004.04.20 Moca �R�����g����������
			if( bCommentTag )
			{
				if( i < nLineLen - 3 && 0 == wmemcmp( L"-->", pLine + i , 3 ) )
				{
					bCommentTag = FALSE;
					i += 2;
				}
				continue;
			}
			// 2004.04.20 Moca To Here
			if(pLine[i]!=L'<' || nDepth>=nMaxStack)
			{
				continue;
			}
			bEndTag	=	FALSE;
			if(pLine[++i]==L'/')
			{
				i++;
				bEndTag	=	TRUE;
			}
			for(j=0;i+j<nLineLen && j<_countof(szTitle)-1;j++)
			{
				if((pLine[i+j]<L'a' || pLine[i+j]>L'z') &&
					(pLine[i+j]<L'A' || pLine[i+j]>L'Z') &&
					!(j!=0 && pLine[i+j]>=L'0' && pLine[i+j]<=L'9'))
				{
					break;
				}
				szTitle[j]	=	pLine[i+j];
			}
			if(j==0)
			{
				// 2004.04.20 Moca From Here �R�����g����������
				if( i < nLineLen - 3 && 0 == wmemcmp( L"!--", pLine + i, 3 ) )
				{
					bCommentTag = TRUE;
					i += 3;
				}
				// 2004.04.20 Moca To Here
				continue;
			}
			szTitle[j]	=	'\0';
			if(bEndTag)
			{
				int nDepthOrg = nDepth; // 2004.04.20 Moca �ǉ�
				// �I���^�O
				while(nDepth>0)
				{
					nDepth--;
					if(!wcsicmp(pszStack[nDepth],szTitle))
					{
						break;
					}
				}
				// 2004.04.20 Moca �c���[���ƈ�v���Ȃ��Ƃ��́A���̏I���^�O�͖���
				if( nDepth == 0 )
				{
					if(wcsicmp(pszStack[nDepth],szTitle))
					{
						nDepth = nDepthOrg;
					}
				}
			}
			else
			{
				if(wcsicmp(szTitle,L"br") && wcsicmp(szTitle,L"area") &&
					wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
				{
					CLayoutPoint ptPos;

					m_cLayoutMgr.LogicToLayout(
						CLogicPoint(i, nLineCount),
						&ptPos
					);

					if(wcsicmp(szTitle,L"hr") && wcsicmp(szTitle,L"meta") && wcsicmp(szTitle,L"link") &&
						wcsicmp(szTitle,L"input") && wcsicmp(szTitle,L"img") && wcsicmp(szTitle,L"area") &&
						wcsicmp(szTitle,L"base") && wcsicmp(szTitle,L"frame") && wcsicmp(szTitle,L"param"))
					{
						// �I���^�O�Ȃ��������S�Ẵ^�O�炵�����̂𔻒�
						wcscpy(pszStack[nDepth],szTitle);
						k	=	j;
						if(j<_countof(szTitle)-3)
						{
							for(;i+j<nLineLen;j++)
							{
								if(pLine[i+j]==L'/' && pLine[i+j+1]==L'>')
								{
									bEndTag	=	TRUE;
									break;
								}
								else if(pLine[i+j]==L'>')
								{
									break;
								}
							}
							if(!bEndTag)
							{
								szTitle[k++]	=	L' ';
								for(j-=k-1;i+j+k<nLineLen && k<_countof(szTitle)-1;k++)
								{
									if(pLine[i+j+k]==L'<' || pLine[i+j+k]==L'\r' || pLine[i+j+k]==L'\n')
									{
										break;
									}
									szTitle[k]	=	pLine[i+j+k];
								}
							j += k-1;
							}
						}
						szTitle[k]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,(bEndTag ? nDepth : nDepth++));
					}
					else
					{
						for(;i+j<nLineLen && j<_countof(szTitle)-1;j++)
						{
							if(pLine[i+j]=='>')
							{
								break;
							}
							szTitle[j]	=	pLine[i+j];
						}
						szTitle[j]	=	L'\0';
						pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1),szTitle,0,nDepth);
					}
				}
			}
			i	+=	j;
		}
	}
}

/*! TeX �A�E�g���C�����

	@author naoh
	@date 2003.07.21 naoh �V�K�쐬
	@date 2005.01.03 naoh �u�}�v�Ȃǂ�"}"���܂ޕ����ɑ΂���C���Aprosper��slide�ɑΉ�
*/
void CEditDoc::MakeTopicList_tex(CFuncInfoArr* pcFuncInfoArr)
{
	const wchar_t*	pLine;
	CLogicInt		nLineLen;
	int				i;
	int				j;
	int				k;

	const int nMaxStack = 8;	//	�l�X�g�̍Ő[
	int nDepth = 0;				//	���܂̃A�C�e���̐[����\�����l�B
	wchar_t szTag[32], szTitle[256];			//	�ꎞ�̈�
	int thisSection=0, lastSection = 0;	// ���݂̃Z�N�V������ނƈ�O�̃Z�N�V�������
	int stackSection[nMaxStack];		// �e�[���ł̃Z�N�V�����̔ԍ�
	int nStartTitlePos;					// \section{dddd} �� dddd �̕����̎n�܂�ԍ�
	int bNoNumber;						// * �t�̏ꍇ�̓Z�N�V�����ԍ���t���Ȃ�

	// ��s����
	CLogicInt	nLineCount;
	for(nLineCount=CLogicInt(0);nLineCount<m_cDocLineMgr.GetLineCount();nLineCount++)
	{
		pLine	=	m_cDocLineMgr.GetLineStr(nLineCount,&nLineLen);
		if(!pLine) break;
		// �ꕶ������
		for(i=0;i<nLineLen-1;i++)
		{
			if(pLine[i] == L'%') break;	// �R�����g�Ȃ�ȍ~�͂���Ȃ�
			if(nDepth>=nMaxStack)continue;
			if(pLine[i] != L'\\')continue;	// �u\�v���Ȃ��Ȃ玟�̕�����
			++i;
			// ���������u\�v�ȍ~�̕�����`�F�b�N
			for(j=0;i+j<nLineLen && j<_countof(szTag)-1;j++)
			{
				if(pLine[i+j] == L'{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
					bNoNumber = (pLine[i+j-1] == '*');
					nStartTitlePos = j+i+1;
					break;
				}
				szTag[j] = pLine[i+j];
			}
			if(j==0) continue;
			if(bNoNumber){
				szTag[j-1] = L'\0';
			}else{
				szTag[j]   = L'\0';
			}
//			MessageBoxA(NULL, szTitle, L"", MB_OK);

			thisSection = 0;
			if(!wcscmp(szTag,L"subsubsection")) thisSection = 4;
			else if(!wcscmp(szTag,L"subsection")) thisSection = 3;
			else if(!wcscmp(szTag,L"section")) thisSection = 2;
			else if(!wcscmp(szTag,L"chapter")) thisSection = 1;
			else if(!wcscmp(szTag,L"begin")) {		// begin�Ȃ� prosper��slide�̉\�����l��
				// �����{slide}{}�܂œǂ݂Ƃ��Ă���
				if(wcsstr(pLine, L"{slide}")){
					k=0;
					for(j=nStartTitlePos+1;i+j<nLineLen && j<_countof(szTag)-1;j++)
					{
						if(pLine[i+j] == '{' && !(i+j>0 && _IS_SJIS_1((unsigned char)pLine[i+j-1])) ) {	// SJIS1�`�F�b�N
							nStartTitlePos = j+i+1;
							break;
						}
						szTag[k++]	=	pLine[i+j];
					}
					szTag[k] = '\0';
					thisSection = 1;
				}
			}

			if( thisSection > 0)
			{
				// section�̒��g�擾
				for(k=0;nStartTitlePos+k<nLineLen && k<_countof(szTitle)-1;k++)
				{
					if(_IS_SJIS_1((unsigned char)pLine[k+nStartTitlePos])) {
						szTitle[k] = pLine[k+nStartTitlePos];
						k++;	// ���̓`�F�b�N�s�v
					} else if(pLine[k+nStartTitlePos] == '}') {
						break;
					}
					szTitle[k] = pLine[k+nStartTitlePos];
				}
				szTitle[k] = '\0';

				CLayoutPoint ptPos;

				WCHAR tmpstr[256];
				WCHAR secstr[4];

				m_cLayoutMgr.LogicToLayout(
					CLogicPoint(i, nLineCount),
					&ptPos
				);

				int sabunSection = thisSection - lastSection;
				if(lastSection == 0){
					nDepth = 0;
					stackSection[0] = 1;
				}else{
					nDepth += sabunSection;
					if(sabunSection > 0){
						if(nDepth >= nMaxStack) nDepth=nMaxStack-1;
						stackSection[nDepth] = 1;
					}else{
						if(nDepth < 0) nDepth=0;
						++stackSection[nDepth];
					}
				}
				tmpstr[0] = L'\0';
				if(!bNoNumber){
					for(k=0; k<=nDepth; k++){
						auto_sprintf(secstr, L"%d.", stackSection[k]);
						wcscat(tmpstr, secstr);
					}
					wcscat(tmpstr, L" ");
				}
				wcscat(tmpstr, szTitle);
				pcFuncInfoArr->AppendData(nLineCount+CLogicInt(1),ptPos.GetY2()+CLayoutInt(1), tmpstr, 0, nDepth);
				if(!bNoNumber) lastSection = thisSection;
			}
			i	+=	j;
		}
	}
}








/*! �R�}���h�R�[�h�ɂ�鏈���U�蕪��

	@param[in] nCommand MAKELONG( �R�}���h�R�[�h�C���M�����ʎq )

	@date 2006.05.19 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX
	@date 2007.06.20 ryoji �O���[�v���ŏ��񂷂�悤�ɕύX
*/
BOOL CEditDoc::HandleCommand( EFunctionCode nCommand )
{
	int				i;
	int				j;
	int				nGroup;
	int				nRowNum;
	int				nPane;
	HWND			hwndWork;
	EditNode*		pEditNodeArr;
	//	May. 19, 2006 genta ���16bit�ɑ��M���̎��ʎq������悤�ɕύX�����̂�
	//	����16�r�b�g�݂̂����o��
	switch( LOWORD( nCommand )){
	case F_PREVWINDOW:	//�O�̃E�B���h�E
		nPane = m_pcEditWnd->m_cSplitterWnd.GetPrevPane();
		if( -1 != nPane ){
			m_pcEditWnd->SetActivePane( nPane );
		}else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( GetOwnerHwnd() == pEditNodeArr[i].GetHwnd() )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// �O�̃E�B���h�E
					for( j = i - 1; j >= 0; --j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j < 0 )
					{
						for( j = nRowNum - 1; j > i; --j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/*
<<<<<<< .mine
					if( i != j )
					{
						// ���̃E�B���h�E���A�N�e�B�u�ɂ���
						hwndWork = pEditNodeArr[j].GetSplitterHwnd();
						// �A�N�e�B�u�ɂ���
						ActivateFrameWindow( hwndWork );
						// �Ō�̃y�C�����A�N�e�B�u�ɂ���
						::PostMessageAny( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
					}
=======
					*/
					/* �O�̃E�B���h�E���A�N�e�B�u�ɂ��� */
					hwndWork = pEditNodeArr[j].GetHwnd();
					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hwndWork );
					/* �Ō�̃y�C�����A�N�e�B�u�ɂ��� */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
//>>>>>>> .r1121
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//���̃E�B���h�E
		nPane = m_pcEditWnd->m_cSplitterWnd.GetNextPane();
		if( -1 != nPane ){
			m_pcEditWnd->SetActivePane( nPane );
		}
		else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				nGroup = 0;
				for( i = 0; i < nRowNum; ++i )
				{
					if( GetOwnerHwnd() == pEditNodeArr[i].GetHwnd() )
					{
						nGroup = pEditNodeArr[i].m_nGroup;
						break;
					}
				}
				if( i < nRowNum )
				{
					// ���̃E�B���h�E
					for( j = i + 1; j < nRowNum; ++j )
					{
						if( nGroup == pEditNodeArr[j].m_nGroup )
							break;
					}
					if( j >= nRowNum )
					{
						for( j = 0; j < i; ++j )
						{
							if( nGroup == pEditNodeArr[j].m_nGroup )
								break;
						}
					}
					/*
<<<<<<< .mine
					if( i != j )
					{
						// ���̃E�B���h�E���A�N�e�B�u�ɂ���
						hwndWork = pEditNodeArr[j].GetSplitterHwnd();
						// �A�N�e�B�u�ɂ���
						ActivateFrameWindow( hwndWork );
						// �ŏ��̃y�C�����A�N�e�B�u�ɂ���
						::PostMessageAny( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
					}
=======
					*/
					/* ���̃E�B���h�E���A�N�e�B�u�ɂ��� */
					hwndWork = pEditNodeArr[j].GetHwnd();
					/* �A�N�e�B�u�ɂ��� */
					ActivateFrameWindow( hwndWork );
					/* �ŏ��̃y�C�����A�N�e�B�u�ɂ��� */
					::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
//>>>>>>> .r1121
				}
				delete [] pEditNodeArr;
			}
		}
		return TRUE;

	default:
		return m_pcEditWnd->GetActiveView().GetCommander().HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
	}
}



/*! �r���[�ɐݒ�ύX�𔽉f������

	@date 2004.06.09 Moca ���C�A�E�g�č\�z����Progress Bar��\������D

*/
void CEditDoc::OnChangeSetting( void )
{
//	return;
	int			i;
	HWND		hwndProgress = NULL;

	CEditWnd*	pCEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta

	//pCEditWnd->m_CFuncKeyWnd.Timer_ONOFF( FALSE ); // 20060126 aroka

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_cStatusBar.GetProgressHwnd();
		//	Status Bar���\������Ă��Ȃ��Ƃ���m_hwndProgressBar == NULL
	}

	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}

	/* �t�@�C���̔r�����[�h�ύX */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_sFile.m_nFileShareMode ){
		/* �t�@�C���̔r�����b�N���� */
		DoFileUnLock();
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	CShareData::getInstance()->TransformFileName_MakeCache();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, false );

	CLogicPoint* posSaveAry = m_pcEditWnd->SavePhysPosOfAllView();

	/* ���C�A�E�g���̍쐬 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		hwndProgress,
		ref
	); /* ���C�A�E�g���̕ύX */

	/* �r���[�ɐݒ�ύX�𔽉f������ */
	for( i = 0; i < 4; ++i ){
		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
	}
	m_pcEditWnd->RestorePhysPosOfAllView( posSaveAry );
	if( hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
}




/* �ҏW�t�@�C�������i�[ */
void CEditDoc::GetFileInfo( FileInfo* pfi ) const
{
	_tcscpy(pfi->m_szPath, GetFilePath());

	pfi->m_nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* �\����̈�ԏ�̍s(0�J�n) */
	pfi->m_nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* �\����̈�ԍ��̌�(0�J�n) */
	//	pfi->GetCaretLayoutPos().GetX() = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX();	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
	//	pfi->GetCaretLayoutPos().GetY() = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetY();	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	CLogicPoint ptXY;
	m_cLayoutMgr.LayoutToLogic(
		m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos(),	/* �r���[����[����̃J�[�\�����ʒu(�O�J�n) */
		&ptXY
	);
	pfi->m_ptCursor.Set(ptXY); //�J�[�\�� �����ʒu(�s������̃o�C�g��, �܂�Ԃ������s�ʒu)

	pfi->m_bIsModified = IsModified() ? TRUE : FALSE;			/* �ύX�t���O */
	pfi->m_nCharCode = m_nCharCode;				/* �����R�[�h��� */

	pfi->m_bIsGrep = m_bGrepMode;
	wcscpy( pfi->m_szGrepKey, m_szGrepKey );

	//�f�o�b�O���j�^(�A�E�g�v�b�g�E�C���h�E)
	pfi->m_bIsDebug = m_bDebugMode;

	return;

}


// 2004/06/21 novice �^�O�W�����v�@�\�ǉ�
#if 0
/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
void CEditDoc::SetReferer( HWND hwndReferer, int nRefererX, int nRefererLine )
{
	m_hwndReferer	= hwndReferer;	/* �Q�ƌ��E�B���h�E */
	m_nRefererX		= nRefererX;	/* �Q�ƌ�  �s������̃o�C�g�ʒu�� */
	m_nRefererLine	= nRefererLine;	/* �Q�ƌ��s  �܂�Ԃ������̕����s�ʒu */
	return;
}
#endif



/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL CEditDoc::OnFileClose( void )
{
	int			nRet;
	int			nBool;
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( GetSplitterHwnd() );

	//	Mar. 30, 2003 genta �T�u���[�`���ɂ܂Ƃ߂�
	AddToMRU();

	if( m_bGrepRunning ){		/* Grep������ */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );	//@@@ 2003.06.25 MIK
		::MYMESSAGEBOX(
			hwndMainFrame,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
			_T("Grep�̏������ł��B\n")
		);
		return FALSE;
	}


	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( IsModified()
	&& FALSE == m_bDebugMode	/* �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ� */
	){
		if( TRUE == m_bGrepMode ){	/* Grep���[�h�̂Ƃ� */
			/* Grep���[�h�ŕۑ��m�F���邩 */
			if( FALSE == m_pShareData->m_Common.m_sSearch.m_bGrepExitConfirm ){
				return TRUE;
			}
		}
		/* �E�B���h�E���A�N�e�B�u�ɂ��� */
		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hwndMainFrame );
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			::MessageBeep( MB_ICONQUESTION );
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%ts\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H\n\n�ǂݎ���p�ŊJ���Ă���̂ŁA���O��t���ĕۑ�����΂����Ǝv���܂��B\n"),
				IsFilePathAvailable() ? GetFilePath() : _T("�i����j")
			);
			switch( nRet ){
			case IDYES:
//				if( IsFilePathAvailable() ){
//					nBool = HandleCommand( F_FILESAVE );
//				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
//				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}else{
			::MessageBeep( MB_ICONQUESTION );
			nRet = ::MYMESSAGEBOX(
				hwndMainFrame,
				MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
				GSTR_APPNAME,
				_T("%ts\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H"),
				IsFilePathAvailable() ? GetFilePath() : _T("�i����j")
			);
			switch( nRet ){
			case IDYES:
				if( IsFilePathAvailable() ){
					//nBool = HandleCommand( F_FILESAVE );
					nBool = FileSave();	// 2006.12.30 ryoji
				}else{
					//nBool = HandleCommand( F_FILESAVEAS_DIALOG );
					nBool = FileSaveAs_Dialog();	// 2006.12.30 ryoji
				}
				return nBool;
			case IDNO:
				return TRUE;
			case IDCANCEL:
			default:
				return FALSE;
			}
		}
	}else{
		return TRUE;
	}
}


/* �����f�[�^�̃N���A */
void CEditDoc::Init( void )
{
//	int types;

	m_bReadOnly = false;	/* �ǂݎ���p���[�h */
	wcscpy( m_szGrepKey, L"" );
	m_bGrepMode = FALSE;	/* Grep���[�h */
	m_eWatchUpdate = WU_QUERY; // Dec. 4, 2002 genta �X�V�Ď����@

	// 2005.06.24 Moca �o�O�C��
	//	�A�E�g�v�b�g�E�B���h�E�Łu����(����)�v���s���Ă��A�E�g�v�b�g�E�B���h�E�̂܂�
	if( m_bDebugMode ){
		m_pcEditWnd->SetDebugModeOFF();
	}

//	Sep. 10, 2002 genta
//	�A�C�R���ݒ�̓t�@�C�����ݒ�ƈ�̉��̂��߂�������͍폜

	/* �t�@�C���̔r�����b�N���� */
	DoFileUnLock();

	/* �t�@�C���̔r�����䃂�[�h */
	m_nFileShareModeOld = SHAREMODE_NOT_EXCLUSIVE;


	/*�A���h�D�E���h�D�o�b�t�@�̃N���A */
	/* �S�v�f�̃N���A */
	m_cOpeBuf.ClearAll();

	/* �e�L�X�g�f�[�^�̃N���A */
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	/* ���ݕҏW���̃t�@�C���̃p�X */
	//	Sep. 10, 2002 genta
	//	�A�C�R���������ɏ����������
	SetFilePath( _T("") );

	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;


	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( GetFilePath() );
	SetDocumentType( doctype, true );

	/* ���C�A�E�g�Ǘ����̏����� */
	/* ���C�A�E�g���̕ύX */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		TRUE,
		NULL,/*hwndProgress*/
		ref
	);


	/* �ύX�t���O */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* �����R�[�h��� */
	m_nCharCode = CODE_DEFAULT;
	m_bBomExist = FALSE;	//	Jul. 26, 2003 ryoji

	//	May 12, 2000
	m_cNewLineCode.SetType( EOL_CRLF );
	
	//	Oct. 2, 2005 genta �}�����[�h
	SetInsMode( m_pShareData->m_Common.m_sGeneral.m_bIsINSMode != FALSE );

	return;
}

/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
void CEditDoc::InitAllView( void )
{
	int		i;

	m_nCommandExecNum = 0;	/* �R�}���h���s�� */
	/* �擪�փJ�[�\�����ړ� */
	for( i = 0; i < 4; ++i ){
		//	Apr. 1, 2001 genta
		// �ړ������̏���
		m_pcEditWnd->m_pcEditViewArr[i]->m_cHistory->Flush();

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_pcEditWnd->m_pcEditViewArr[i]->GetSelectionInfo().DisableSelectArea( FALSE );

		m_pcEditWnd->m_pcEditViewArr[i]->OnChangeSetting();
		m_pcEditWnd->m_pcEditViewArr[i]->GetCaret().MoveCursor( CLayoutPoint(0, 0), TRUE );
		m_pcEditWnd->m_pcEditViewArr[i]->GetCaret().m_nCaretPosX_Prev = CLayoutInt(0);
	}

	return;
}


/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_sFile.m_bCheckFileTimeStamp	/* �X�V�̊Ď� */
	 // Dec. 4, 2002 genta
	 && m_eWatchUpdate != WU_NONE
	 && m_pShareData->m_Common.m_sFile.m_nFileShareMode == SHAREMODE_NOT_EXCLUSIVE	/* �t�@�C���̔r�����䃂�[�h */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* �A�N�e�B�u? */
	 && hwndActive == GetOwnerHwnd()
	 && IsFilePathAvailable()
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */

	){
		/* �t�@�C���X�^���v���`�F�b�N���� */

		// 2005.10.20 ryoji FindFirstFile���g���悤�ɕύX�i�t�@�C�������b�N����Ă��Ă��^�C���X�^���v�擾�\�j
		FILETIME ftime;
		if( GetLastWriteTimestamp( GetFilePath(), ftime )){
			if( 0 != ::CompareFileTime( &m_FileTime, &ftime ) )	//	Aug. 13, 2003 wmlhq �^�C���X�^���v���Â��ύX����Ă���ꍇ�����o�ΏۂƂ���
			{
				bUpdate = TRUE;
				m_FileTime = ftime;
			}
		}
	}

	//	From Here Dec. 4, 2002 genta
	if( bUpdate ){
		switch( m_eWatchUpdate ){
		case WU_NOTIFY:
			{
				TCHAR szText[40];
				//	���ݎ����̎擾
				SYSTEMTIME st;
				FILETIME lft;
				if( ::FileTimeToLocalFileTime( &m_FileTime, &lft ) &&
					::FileTimeToSystemTime( &lft, &st )){
					// nothing to do
				}
				else {
					//	�t�@�C�������̕ϊ��Ɏ��s�����ꍇ��
					//	���ݎ����ł��܂���
					::GetLocalTime( &st );
				}
				auto_sprintf( szText, _T("���t�@�C���X�V %02d:%02d:%02d"), st.wHour, st.wMinute, st.wSecond );
				m_pcEditWnd->SendStatusMessage( szText );
			}	
			break;
		default:
			{
				m_eWatchUpdate = WU_NONE; // �X�V�Ď��̗}��

				CDlgFileUpdateQuery dlg( GetFilePath(), IsModified() );
				int result = dlg.DoModal( m_hInstance, GetSplitterHwnd(), IDD_FILEUPDATEQUERY, 0 );

				switch( result ){
				case 1:	// �ēǍ�
					/* ����t�@�C���̍ăI�[�v�� */
					ReloadCurrentFile( m_nCharCode, m_bReadOnly );
					m_eWatchUpdate = WU_QUERY;
					break;
				case 2:	// �Ȍ�ʒm���b�Z�[�W�̂�
					m_eWatchUpdate = WU_NOTIFY;
					break;
				case 3:	// �Ȍ�X�V���Ď����Ȃ�
					m_eWatchUpdate = WU_NONE;
					break;
				case 0:	// CLOSE
				default:
					m_eWatchUpdate = WU_QUERY;
					break;
				}
			}
			break;
		}
	}
	//	To Here Dec. 4, 2002 genta
	return;
}





/*! ����t�@�C���̍ăI�[�v�� */
void CEditDoc::ReloadCurrentFile(
	ECodeType	nCharCode,		/*!< [in] �����R�[�h��� */
	bool	bReadOnly		/*!< [in] �ǂݎ���p���[�h */
)
{
	if( -1 == _taccess( GetFilePath(), 0 ) ){
		/* �t�@�C�������݂��Ȃ� */
		//	Jul. 26, 2003 ryoji BOM��W���ݒ��
		m_nCharCode = nCharCode;
		switch( m_nCharCode ){
		case CODE_UNICODE:
		case CODE_UNICODEBE:
			m_bBomExist = TRUE;
			break;
		case CODE_UTF8:
		default:
			m_bBomExist = FALSE;
			break;
		}
		return;
	}


	bool	bOpened;
	CLayoutInt	nViewTopLine;
	CLayoutInt	nViewLeftCol;
	CLayoutPoint ptCaretPosXY;
	nViewTopLine = m_pcEditWnd->GetActiveView().GetTextArea().GetViewTopLine();	/* �\����̈�ԏ�̍s(0�J�n) */
	nViewLeftCol = m_pcEditWnd->GetActiveView().GetTextArea().GetViewLeftCol();	/* �\����̈�ԍ��̌�(0�J�n) */
	ptCaretPosXY = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos();

	TCHAR	szFilePath[MAX_PATH];
	_tcscpy( szFilePath, GetFilePath() );

	// Mar. 30, 2003 genta �u�b�N�}�[�N�ۑ��̂���MRU�֓o�^
	AddToMRU();

	/* �����f�[�^�̃N���A */
	Init();

	/* �S�r���[�̏����� */
	InitAllView();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �t�@�C���ǂݍ��� */
	FileRead(
		szFilePath,
		&bOpened,
		nCharCode,	/* �����R�[�h�������� */
		bReadOnly,	/* �ǂݎ���p�� */
		FALSE		/* �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
	);

	// ���C�A�E�g�s�P�ʂ̃J�[�\���ʒu����
	// �������ł̓I�v�V�����̃J�[�\���ʒu�����i�����s�P�ʁj���w�肳��Ă��Ȃ��ꍇ�ł���������
	// 2007.08.23 ryoji �\���̈敜��
	if( ptCaretPosXY.GetY2() < m_cLayoutMgr.GetLineCount() ){
		m_pcEditWnd->GetActiveView().GetTextArea().SetViewTopLine(nViewTopLine);
		m_pcEditWnd->GetActiveView().GetTextArea().SetViewLeftCol(nViewLeftCol);
	}
	m_pcEditWnd->GetActiveView().GetCaret().MoveCursorProperly( ptCaretPosXY, TRUE );	// 2007.08.23 ryoji MoveCursor()->MoveCursorProperly()
	m_pcEditWnd->GetActiveView().GetCaret().m_nCaretPosX_Prev = m_pcEditWnd->GetActiveView().GetCaret().GetCaretLayoutPos().GetX2();

}

//	From Here Nov. 20, 2000 genta
/*!	IME��Ԃ̐ݒ�
	
	@param mode [in] IME�̃��[�h
	
	@date Nov 20, 2000 genta
*/
void CEditDoc::SetImeMode( int mode )
{
	DWORD	conv, sent;
	HIMC	hIme;

	hIme = ImmGetContext( GetOwnerHwnd() );

	//	�ŉ��ʃr�b�g��IME���g��On/Off����
	if( ( mode & 3 ) == 2 ){
		ImmSetOpenStatus( hIme, FALSE );
	}
	if( ( mode >> 2 ) > 0 ){
		ImmGetConversionStatus( hIme, &conv, &sent );

		switch( mode >> 2 ){
		case 1:	//	FullShape
			conv |= IME_CMODE_FULLSHAPE;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 2:	//	FullShape & Hiragana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
			conv &= ~( IME_CMODE_KATAKANA | IME_CMODE_NOCONVERSION );
			break;
		case 3:	//	FullShape & Katakana
			conv |= IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE | IME_CMODE_KATAKANA;
			conv &= ~IME_CMODE_NOCONVERSION;
			break;
		case 4: //	Non-Conversion
			conv |= IME_CMODE_NOCONVERSION;
			break;
		}
		ImmSetConversionStatus( hIme, conv, sent );
	}
	if( ( mode & 3 ) == 1 ){
		ImmSetOpenStatus( hIme, TRUE );
	}
	ImmReleaseContext( GetOwnerHwnd(), hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$x�̓W�J

	���ꕶ���͈ȉ��̒ʂ�
	@li $  $���g
	@li A  �A�v����
	@li F  �J���Ă���t�@�C���̃t���p�X�B���O���Ȃ����(����)�B
	@li f  �J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
	@li g  �J���Ă���t�@�C���̖��O�i�g���q�����j
	@li /  �J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
	@li N  �J���Ă���t�@�C���̖��O(�ȈՕ\��)
	@li C  ���ݑI�𒆂̃e�L�X�g
	@li x  ���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
	@li y  ���݂̕����s�ʒu(1�J�n)
	@li d  ���݂̓��t(���ʐݒ�̓��t����)
	@li t  ���݂̎���(���ʐݒ�̎�������)
	@li p  ���݂̃y�[�W
	@li P  ���y�[�W
	@li D  �t�@�C���̃^�C���X�^���v(���ʐݒ�̓��t����)
	@li T  �t�@�C���̃^�C���X�^���v(���ʐݒ�̎�������)
	@li V  �G�f�B�^�̃o�[�W����������
	@li h  Grep�����L�[�̐擪32byte
	@li S  �T�N���G�f�B�^�̃t���p�X
	@li I  ini�t�@�C���̃t���p�X
	@li M  ���ݎ��s���Ă���}�N���t�@�C���p�X

	@date 2003.04.03 genta wcsncpy_ex�����ɂ��for���̍팸
	@date 2005.09.15 FILE ���ꕶ��S, M�ǉ�
	@date 2007.09.21 kobake ���ꕶ��A(�A�v����)��ǉ�
*/
void CEditDoc::ExpandParameter(const WChar* pszSource, WChar* pszBuffer, int nBufferLen)
{
	// Apr. 03, 2003 genta �Œ蕶������܂Ƃ߂�
	static const wchar_t	PRINT_PREVIEW_ONLY[]	= L"(����v���r���[�ł̂ݎg�p�ł��܂�)";
	const int				PRINT_PREVIEW_ONLY_LEN	= _countof( PRINT_PREVIEW_ONLY ) - 1;
	static const wchar_t	NO_TITLE[]				= L"(����)";
	const int				NO_TITLE_LEN			= _countof( NO_TITLE ) - 1;
	static const wchar_t	NOT_SAVED[]				= L"(�ۑ�����Ă��܂���)";
	const int				NOT_SAVED_LEN			= _countof( NOT_SAVED ) - 1;

	const wchar_t *p, *r;	//	p�F�ړI�̃o�b�t�@�Br�F��Ɨp�̃|�C���^�B
	wchar_t *q, *q_max;

	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max;){
		if( *p != '$' ){
			*q++ = *p++;
			continue;
		}
		switch( *(++p) ){
		case L'$':	//	 $$ -> $
			*q++ = *p++;
			break;
		case L'A':	//�A�v����
			q = wcs_pushW( q, q_max - q, GSTR_APPNAME_W, wcslen(GSTR_APPNAME_W) );
			++p;
			break;
		case L'F':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�j
			if ( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				r = to_wchar(GetFilePath());
				q = wcs_pushW( q, q_max - q, r, wcslen( r ));
				++p;
			}
			break;
		case L'f':	//	�J���Ă���t�@�C���̖��O�i�t�@�C����+�g���q�̂݁j
			// Oct. 28, 2001 genta
			//	�t�@�C�����݂̂�n���o�[�W����
			//	�|�C���^�𖖔���
			if ( ! IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				// 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				//	����\\�������ɂ����Ă����̌��ɂ�\0������̂ŃA�N�Z�X�ᔽ�ɂ͂Ȃ�Ȃ��B
				q = wcs_pushT( q, q_max - q, GetFileName());
				++p;
			}
			break;
		case L'g':	//	�J���Ă���t�@�C���̖��O�i�g���q�������t�@�C�����̂݁j
			//	From Here Sep. 16, 2002 genta
			if ( ! IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�|�C���^�𖖔���
				const wchar_t *dot_position, *end_of_path;
				r = to_wchar(GetFileName()); // 2002.10.13 Moca �t�@�C����(�p�X�Ȃ�)���擾�B���{��Ή�
				end_of_path = dot_position =
					r + wcslen( r );
				//	��납��.��T��
				for( --dot_position ; dot_position > r && *dot_position != '.'
					; --dot_position )
					;
				//	r�Ɠ����ꏊ�܂ōs���Ă��܂�����.����������
				if( dot_position == r )
					dot_position = end_of_path;

				q = wcs_pushW( q, q_max - q, r, dot_position - r );
				++p;
			}
			break;
			//	To Here Sep. 16, 2002 genta
		case L'/':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
			// Oct. 28, 2001 genta
			if ( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			} 
			else {
				//	�p�X�̋�؂�Ƃ���'/'���g���o�[�W����
				for( r = to_wchar(GetFilePath()); *r != L'\0' && q < q_max; ++r, ++q ){
					if( *r == L'\\' )
						*q = L'/';
					else
						*q = *r;
				}
				++p;
			}
			break;
		//	From Here 2003/06/21 Moca
		case L'N':
			if( !IsFilePathAvailable() ){
				q = wcs_pushW( q, q_max - q, NO_TITLE, NO_TITLE_LEN );
				++p;
			}
			else {
				TCHAR szText[1024];
				CShareData::getInstance()->GetTransformFileNameFast( GetFilePath(), szText, 1023 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2003/06/21 Moca
		//	From Here Jan. 15, 2002 hor
		case L'C':	//	���ݑI�𒆂̃e�L�X�g
			{
				CNativeW cmemCurText;
				m_pcEditWnd->GetActiveView().GetCurrentTextForSearch( cmemCurText );

				q = wcs_pushW( q, q_max - q, cmemCurText.GetStringPtr(), cmemCurText.GetStringLength());
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		//	From Here 2002/12/04 Moca
		case L'x':	//	���݂̕������ʒu(�擪����̃o�C�g��1�J�n)
			{
				wchar_t szText[11];
				_itow( m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos().x + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		case L'y':	//	���݂̕����s�ʒu(1�J�n)
			{
				wchar_t szText[11];
				_itow( m_pcEditWnd->GetActiveView().GetCaret().GetCaretLogicPos().y + 1, szText, 10 );
				q = wcs_pushW( q, q_max - q, szText);
				++p;
			}
			break;
		//	To Here 2002/12/04 Moca
		case L'd':	//	���ʐݒ�̓��t����
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L't':	//	���ʐݒ�̎�������
			{
				TCHAR szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			break;
		case L'p':	//	���݂̃y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					q = wcs_pushW( q, q_max - q, szText, wcslen(szText));
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'P':	//	���y�[�W
			{
				CEditWnd*	pcEditWnd = m_pcEditWnd;	//	Sep. 10, 2002 genta
				if (pcEditWnd->m_pPrintPreview){
					wchar_t szText[1024];
					_itow(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					q = wcs_pushW( q, q_max - q, szText);
					++p;
				}
				else {
					q = wcs_pushW( q, q_max - q, PRINT_PREVIEW_ONLY, PRINT_PREVIEW_ONLY_LEN );
					++p;
				}
			}
			break;
		case L'D':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetDateFormat( systimeL, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'T':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				TCHAR szText[1024];
				CShareData::getInstance()->MyGetTimeFormat( systimeL, szText, _countof( szText ) - 1 );
				q = wcs_pushT( q, q_max - q, szText);
				++p;
			}
			else {
				q = wcs_pushW( q, q_max - q, NOT_SAVED, NOT_SAVED_LEN );
				++p;
			}
			break;
		case L'V':	// Apr. 4, 2003 genta
			// Version number
			{
				wchar_t buf[28]; // 6(�����܂�WORD�̍ő咷) * 4 + 4(�Œ蕔��)
				//	2004.05.13 Moca �o�[�W�����ԍ��́A�v���Z�X���ƂɎ擾����
				DWORD dwVersionMS, dwVersionLS;
				GetAppVersionInfo( NULL, VS_VERSION_INFO,
					&dwVersionMS, &dwVersionLS );
				int len = auto_sprintf( buf, L"%d.%d.%d.%d",
					HIWORD( dwVersionMS ),
					LOWORD( dwVersionMS ),
					HIWORD( dwVersionLS ),
					LOWORD( dwVersionLS )
				);
				q = wcs_pushW( q, q_max - q, buf, len );
				++p;
			}
			break;
		case L'h':	//	Apr. 4, 2003 genta
			//	Grep Key������ MAX 32����
			//	���g��SetParentCaption()���ڐA
			{
				CNativeW	cmemDes;
				// m_szGrepKey �� cmemDes
				LimitStringLengthW( m_szGrepKey, wcslen( m_szGrepKey ),
					(q_max - q > 32 ? 32 : q_max - q - 3), cmemDes );
				if( (int)wcslen( m_szGrepKey ) > cmemDes.GetStringLength() ){
					cmemDes.AppendString(L"...");
				}
				q = wcs_pushW( q, q_max - q, cmemDes.GetStringPtr(), cmemDes.GetStringLength());
				++p;
			}
			break;
		case L'S':	//	Sep. 15, 2005 FILE
			//	�T�N���G�f�B�^�̃t���p�X
			{
				SFilePath	szPath;

				::GetModuleFileName( ::GetModuleHandle( NULL ), szPath, _countof2(szPath) );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'I':	//	May. 19, 2007 ryoji
			//	ini�t�@�C���̃t���p�X
			{
				TCHAR	szPath[_MAX_PATH + 1];
				CShareData::getInstance()->GetIniFileName( szPath );
				q = wcs_pushT( q, q_max - q, szPath );
				++p;
			}
			break;
		case 'M':	//	Sep. 15, 2005 FILE
			//	���ݎ��s���Ă���}�N���t�@�C���p�X�̎擾
			{
				// ���s���}�N���̃C���f�b�N�X�ԍ� (INVALID_MACRO_IDX:���� / STAND_KEYMACRO:�W���}�N��)
				switch( m_pcSMacroMgr->GetCurrentIdx() ){
				case INVALID_MACRO_IDX:
					break;
				case STAND_KEYMACRO:
					{
						TCHAR* pszMacroFilePath = CShareData::getInstance()->GetShareData()->m_szKeyMacroFileName;
						q = wcs_pushT( q, q_max - q, pszMacroFilePath );
					}
					break;
				default:
					{
						TCHAR szMacroFilePath[_MAX_PATH * 2];
						int n = CShareData::getInstance()->GetMacroFilename( m_pcSMacroMgr->GetCurrentIdx(), szMacroFilePath, _countof(szMacroFilePath) );
						if ( 0 < n ){
							q = wcs_pushT( q, q_max - q, szMacroFilePath );
						}
					}
					break;
				}
				++p;
			}
			break;
		//	Mar. 31, 2003 genta
		//	��������
		//	${cond:string1$:string2$:string3$}
		//	
		case L'{':	// ��������
			{
				int cond;
				cond = ExParam_Evaluate( p + 1 );
				while( *p != '?' && *p != '\0' )
					++p;
				if( *p == '\0' )
					break;
				p = ExParam_SkipCond( p + 1, cond );
			}
			break;
		case L':':	// ��������̒���
			//	��������̖����܂�SKIP
			p = ExParam_SkipCond( p + 1, -1 );
			break;
		case L'}':	// ��������̖���
			//	���ɂ��邱�Ƃ͂Ȃ�
			++p;
			break;
		default:
			*q++ = '$';
			*q++ = *p++;
			break;
		}
	}
	*q = '\0';
}

/*! @brief �����̓ǂݔ�΂�

	��������̍\�� ${cond:A0$:A1$:A2$:..$} �ɂ����āC
	�w�肵���ԍ��ɑΉ�����ʒu�̐擪�ւ̃|�C���^��Ԃ��D
	�w��ԍ��ɑΉ����镨���������$}�̎��̃|�C���^��Ԃ��D

	${���o�ꂵ���ꍇ�ɂ̓l�X�g�ƍl����$}�܂œǂݔ�΂��D

	@param pszSource [in] �X�L�������J�n���镶����̐擪�Dcond:�̎��̃A�h���X��n���D
	@param part [in] �ړ�����ԍ����ǂݔ�΂�$:�̐��D-1��^����ƍŌ�܂œǂݔ�΂��D

	@return �ړ���̃|�C���^�D�Y���̈�̐擪�����邢��$}�̒���D

	@author genta
	@date 2003.03.31 genta �쐬
*/
const wchar_t* CEditDoc::ExParam_SkipCond(const wchar_t* pszSource, int part)
{
	if( part == 0 )
		return pszSource;
	
	int nest = 0;	// ����q�̃��x��
	bool next = true;	// �p���t���O
	const wchar_t *p;
	for( p = pszSource; next && *p != L'\0'; ++p ) {
		if( *p == L'$' && p[1] != L'\0' ){ // $�������Ȃ疳��
			switch( *(++p)){
			case L'{':	// ����q�̊J�n
				++nest;
				break;
			case L'}':
				if( nest == 0 ){
					//	�I���|�C���g�ɒB����
					next = false; 
				}
				else {
					//	�l�X�g���x����������
					--nest;
				}
				break;
			case L':':
				if( nest == 0 && --part == 0){ // ����q�łȂ��ꍇ�̂�
					//	�ړI�̃|�C���g
					next = false;
				}
				break;
			}
		}
	}
	return p;
}

/*!	@brief �����̕]��

	@param pCond [in] ������ʐ擪�D'?'�܂ł������ƌ��Ȃ��ĕ]������
	@return �]���̒l

	@note
	�|�C���^�̓ǂݔ�΂���Ƃ͍s��Ȃ��̂ŁC'?'�܂ł̓ǂݔ�΂���
	�Ăяo�����ŕʓr�s���K�v������D

	@author genta
	@date 2003.03.31 genta �쐬

*/
int CEditDoc::ExParam_Evaluate( const wchar_t* pCond )
{
	switch( *pCond ){
	case L'R': // �ǂ݂Ƃ��p
		if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
			return 0;
		}
		else if( SHAREMODE_NOT_EXCLUSIVE != m_nFileShareModeOld && /* �t�@�C���̔r�����䃂�[�h */
			NULL == m_hLockedFile		/* ���b�N���Ă��Ȃ� */
		){
			return 1;
		}
		else{
			return 2;
		}
	case L'w': // Grep���[�h/Output Mode
		if( m_bGrepMode ){
			return 0;
		}else if( m_bDebugMode ){
			return 1;
		}else {
			return 2;
		}
	case L'M': // �L�[�{�[�h�}�N���̋L�^��
		if( TRUE == m_pShareData->m_bRecordingKeyMacro &&
		m_pShareData->m_hwndRecordingKeyMacro == GetOwnerHwnd() ){ /* �E�B���h�E */
			return 0;
		}else {
			return 1;
		}
	case L'U': // �X�V
		if( IsModified()){
			return 0;
		}
		else {
			return 1;
		}
	case L'I': // �A�C�R��������Ă��邩
		if( ::IsIconic( GetOwnerHwnd() )){
			return 0;
		} else {
 			return 1;
 		}
	default:
		return 0;
	}
	return 0;
}



HWND CEditDoc::GetSplitterHwnd() const
{
	return m_pcEditWnd->m_cSplitterWnd.GetHwnd();
}

HWND CEditDoc::GetOwnerHwnd() const
{
	return m_pcEditWnd->GetHwnd();
}
