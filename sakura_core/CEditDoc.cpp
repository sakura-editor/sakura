//	$Id$
/*!	@file
	@brief �����֘A���̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, mik, jepro
	Copyright (C) 2002, YAZAKI, hor, genta, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditDoc.h"
#include "debug.h"
//#include "keycode.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include <DLGS.H>
#include "CShareData.h"
#include "CEditWnd.h"
#include "sakura_rc.h"
#include "etc_uty.h"
#include "global.h"
#include "CFuncInfoArr.h" /// 2002/2/3 aroka
#include "CSMacroMgr.h"///
#include "CMarkMgr.h"///
#include "CDocLine.h" /// 2002/2/3 aroka
#include "CPrintPreview.h"

#define IDT_ROLLMOUSE	1

//	May 12, 2000 genta ���������@�ύX
CEditDoc::CEditDoc() :
	m_cNewLineCode( EOL_CRLF ),		//	New Line Type
	m_cSaveLineCode( EOL_NONE ),		//	�ۑ�����Line Type
	m_bGrepRunning( FALSE ),		/* Grep������ */
//@@@ 2002.01.14 YAZAKI ����v���r���[��CPrintPreview�ɓƗ����������Ƃɂ��ύX
//	m_bPrintPreviewMode( FALSE ),	/* ����v���r���[���[�h�� */
	m_nCommandExecNum( 0 ),			/* �R�}���h���s�� */
	m_hwndReferer( NULL ),			/* �Q�ƌ��E�B���h�E */
	m_nRefererX( 0 ),				/* �Q�ƌ� �s������̃o�C�g�ʒu�� */
	m_nRefererLine( 0 ),			/* �Q�ƌ��s �܂�Ԃ������̕����s�ʒu */
	m_bReadOnly( FALSE ),			/* �ǂݎ���p���[�h */
	m_bDebugMode( FALSE ),			/* �f�o�b�O���j�^���[�h */
	m_bGrepMode( FALSE ),			/* Grep���[�h�� */
	m_nCharCode( 0 ),				/* �����R�[�h��� */
	m_nActivePaneIndex( 0 ),
//@@@ 2002.01.14 YAZAKI �s�g�p�̂���
//	m_pcOpeBlk( NULL ),				/* ����u���b�N */
	m_bDoing_UndoRedo( FALSE ),		/* �A���h�D�E���h�D�̎��s���� */
	m_nFileShareModeOld( 0 ),		/* �t�@�C���̔r�����䃂�[�h */
	m_hLockedFile( NULL ),			/* ���b�N���Ă���t�@�C���̃n���h�� */
	m_pszAppName( "EditorClient" ),
	m_hInstance( NULL ),
	m_hWnd( NULL ),
	m_nSettingTypeLocked( false ),	//	�ݒ�l�ύX�\�t���O
	m_bIsModified( false )	/* �ύX�t���O */ // Jan. 22, 2002 genta �^�ύX
{
//	m_pcDlgTest = new CDlgTest;

	m_szFilePath[0] = '\0';			/* ���ݕҏW���̃t�@�C���̃p�X */
	strcpy( m_szGrepKey, "" );
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
//	m_cShareData.Init();

	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* OPENFILENAME�̏����� */
	memset( &m_ofn, 0, sizeof( OPENFILENAME ) );
	m_ofn.lStructSize = sizeof( OPENFILENAME );
	m_ofn.nFilterIndex = 3;
	GetCurrentDirectory( _MAX_PATH, m_szInitialDir );	/* �u�J���v�ł̏����f�B���N�g�� */
	strcpy( m_szDefaultWildCard, "*.*" );				/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */
	/* CHOOSEFONT�̏����� */
	memset( &m_cf, 0, sizeof( CHOOSEFONT ) );
	m_cf.lStructSize = sizeof( m_cf );
	m_cf.hwndOwner = m_hWnd;
	m_cf.hDC = NULL;
	m_cf.lpLogFont = &(m_pShareData->m_Common.m_lf);
	m_cf.Flags = CF_FIXEDPITCHONLY | CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#ifdef _DEBUG
//	m_cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
//#endif


	/* ���C�A�E�g�Ǘ����̏����� */
	m_cLayoutMgr.Create( &m_cDocLineMgr );
	/* ���C�A�E�g���̕ύX */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* �p�����[�h���b�v������ */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* �s�R�����g�f���~�^ */
		ref.m_szLineComment2,		/* �s�R�����g�f���~�^2 */
		ref.m_szLineComment3,		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
		ref.m_szBlockCommentFrom,	/* �u���b�N�R�����g�f���~�^(From) */
		ref.m_szBlockCommentTo,		/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* �u���b�N�R�����g�f���~�^2(From) */
		ref.m_szBlockCommentTo2,	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
		ref.m_nStringType,			/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		TRUE,
		NULL,/*hwndProgress*/
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
	);
//	MYTRACE( "CEditDoc::CEditDoc()�����\n" );

	//	Aug, 21, 2000 genta
	//	�����ۑ��̐ݒ�
	ReloadAutoSaveParam();

	//	Sep, 29, 2001 genta
	//	�}�N��
	m_pcSMacroMgr = new CSMacroMgr;
	//strcpy(m_pszCaption, "sakura");	//@@@	YAZAKI
	
	//	m_FileTime�̏�����
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;
	return;
}


CEditDoc::~CEditDoc()
{
//	delete (CDialog*)m_pcDlgTest;
//	m_pcDlgTest = NULL;

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
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
/////////////////////////////////////////////////////////////////////////////
BOOL CEditDoc::Create(
	HINSTANCE hInstance,
	HWND hwndParent,
	CImageListMgr* pcIcons
 )
{
	HWND		hWndArr[4];
	CEditWnd*	pCEditWnd;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;

	/* �����t���[���쐬 */
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	m_cSplitterWnd.Create( m_hInstance, m_hwndParent, pCEditWnd );

	/* �r���[ */
	m_cEditViewArr[0].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 0, /*FALSE,*/ TRUE  );
	m_cEditViewArr[1].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 1, /*TRUE ,*/ FALSE );
	m_cEditViewArr[2].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 2, /*TRUE ,*/ FALSE );
	m_cEditViewArr[3].Create( m_hInstance, m_cSplitterWnd.m_hWnd, this, 3, /*TRUE ,*/ FALSE );

	m_cEditViewArr[0].OnKillFocus();
	m_cEditViewArr[1].OnKillFocus();
	m_cEditViewArr[2].OnKillFocus();
	m_cEditViewArr[3].OnKillFocus();

	m_cEditViewArr[0].OnSetFocus();

	/* �q�E�B���h�E�̐ݒ� */
	hWndArr[0] = m_cEditViewArr[0].m_hWnd;
	hWndArr[1] = m_cEditViewArr[1].m_hWnd;
	hWndArr[2] = m_cEditViewArr[2].m_hWnd;
	hWndArr[3] = m_cEditViewArr[3].m_hWnd;
	m_cSplitterWnd.SetChildWndArr( hWndArr );
	m_hWnd = m_cSplitterWnd.m_hWnd;


	//	Oct. 2, 2001 genta
	m_cFuncLookup.Init( m_hInstance, m_pcSMacroMgr, &m_pShareData->m_Common );

	/* �ݒ�v���p�e�B�V�[�g�̏������P */
//@@	m_cProp1.Create( m_hInstance, m_hWnd );
	//	Sep. 29, 2001 genta �}�N���N���X��n���悤��
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
	m_cPropCommon.Create( m_hInstance, m_hWnd, pcIcons, m_pcSMacroMgr, &(pCEditWnd->m_CMenuDrawer) );
	m_cPropTypes.Create( m_hInstance, m_hWnd );

	/* ���͕⊮�E�B���h�E�쐬 */
	m_cHokanMgr.DoModeless( m_hInstance, m_cEditViewArr[0].m_hWnd, (LPARAM)&(m_cEditViewArr[0]) );

	return TRUE;
}





/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT CEditDoc::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	switch( uMsg ){
	case WM_ENTERMENULOOP:
	case WM_EXITMENULOOP:
		m_cEditViewArr[0].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[1].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[2].DispatchEvent( hwnd, uMsg, wParam, lParam );
		m_cEditViewArr[3].DispatchEvent( hwnd, uMsg, wParam, lParam );
		return 0L;
	default:
		return m_cEditViewArr[m_nActivePaneIndex].DispatchEvent( hwnd, uMsg, wParam, lParam );
	}
}



void CEditDoc::OnMove( int x, int y, int nWidth, int nHeight )
{
//	m_cSplitterWnd.OnMove( x, y, nWidth, nHeight );
	::MoveWindow( m_cSplitterWnd.m_hWnd, x, y, nWidth, nHeight, TRUE );

	return;
}




/*! �e�L�X�g���I������Ă��邩 */
BOOL CEditDoc::IsTextSelected( void )
{
	return m_cEditViewArr[m_nActivePaneIndex].IsTextSelected();
}




BOOL CEditDoc::SelectFont( LOGFONT* plf )
{
	m_cf.hwndOwner = m_hWnd;
	m_cf.lpLogFont = plf;
	if( TRUE != ChooseFont( &m_cf ) ){
#ifdef _DEBUG
		DWORD nErr;
		nErr = CommDlgExtendedError();
		switch( nErr ){
		case CDERR_FINDRESFAILURE:	MYTRACE( "CDERR_FINDRESFAILURE \n" );	break;
		case CDERR_INITIALIZATION:	MYTRACE( "CDERR_INITIALIZATION \n" );	break;
		case CDERR_LOCKRESFAILURE:	MYTRACE( "CDERR_LOCKRESFAILURE \n" );	break;
		case CDERR_LOADRESFAILURE:	MYTRACE( "CDERR_LOADRESFAILURE \n" );	break;
		case CDERR_LOADSTRFAILURE:	MYTRACE( "CDERR_LOADSTRFAILURE \n" );	break;
		case CDERR_MEMALLOCFAILURE:	MYTRACE( "CDERR_MEMALLOCFAILURE\n" );	break;
		case CDERR_MEMLOCKFAILURE:	MYTRACE( "CDERR_MEMLOCKFAILURE \n" );	break;
		case CDERR_NOHINSTANCE:		MYTRACE( "CDERR_NOHINSTANCE \n" );		break;
		case CDERR_NOHOOK:			MYTRACE( "CDERR_NOHOOK \n" );			break;
		case CDERR_NOTEMPLATE:		MYTRACE( "CDERR_NOTEMPLATE \n" );		break;
		case CDERR_STRUCTSIZE:		MYTRACE( "CDERR_STRUCTSIZE \n" );		break;
		case CFERR_MAXLESSTHANMIN:	MYTRACE( "CFERR_MAXLESSTHANMIN \n" );	break;
		case CFERR_NOFONTS:			MYTRACE( "CFERR_NOFONTS \n" );			break;
		}
#endif
		return FALSE;
	}else{
//		MYTRACE( "LOGFONT.lfPitchAndFamily = " );
//		if( plf->lfPitchAndFamily & DEFAULT_PITCH ){
//			MYTRACE( "DEFAULT_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FIXED_PITCH ){
//			MYTRACE( "FIXED_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & VARIABLE_PITCH ){
//			MYTRACE( "VARIABLE_PITCH " );
//		}
//		if( plf->lfPitchAndFamily & FF_DECORATIVE  ){
//			MYTRACE( "FF_DECORATIVE " );
//		}
//		if( plf->lfPitchAndFamily & FF_DONTCARE ){
//			MYTRACE( "FF_DONTCARE " );
//		}
//		if( plf->lfPitchAndFamily & FF_MODERN ){
//			MYTRACE( "FF_MODERN " );
//		}
//		if( plf->lfPitchAndFamily & FF_ROMAN ){
//			MYTRACE( "FF_ROMAN " );
//		}
//		if( plf->lfPitchAndFamily & FF_SCRIPT ){
//			MYTRACE( "FF_SCRIPT " );
//		}
//		if( plf->lfPitchAndFamily & FF_SWISS ){
//			MYTRACE( "FF_SWISS " );
//		}
//		MYTRACE( "\n" );

//		MYTRACE( "/* LOGFONT�̏����� */\n" );
//		MYTRACE( "memset( &m_pShareData->m_Common.m_lf, 0, sizeof(LOGFONT) );\n" );
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfHeight			= %d;\n", m_pShareData->m_Common.m_lf.lfHeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWidth			= %d;\n", m_pShareData->m_Common.m_lf.lfWidth			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfEscapement		= %d;\n", m_pShareData->m_Common.m_lf.lfEscapement		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOrientation		= %d;\n", m_pShareData->m_Common.m_lf.lfOrientation		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfWeight			= %d;\n", m_pShareData->m_Common.m_lf.lfWeight			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfItalic			= %d;\n", m_pShareData->m_Common.m_lf.lfItalic			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfUnderline		= %d;\n", m_pShareData->m_Common.m_lf.lfUnderline		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfStrikeOut		= %d;\n", m_pShareData->m_Common.m_lf.lfStrikeOut		);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfCharSet			= %d;\n", m_pShareData->m_Common.m_lf.lfCharSet			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfOutPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfOutPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfClipPrecision	= %d;\n", m_pShareData->m_Common.m_lf.lfClipPrecision	);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfQuality			= %d;\n", m_pShareData->m_Common.m_lf.lfQuality			);
//		MYTRACE( "m_pShareData->m_Common.m_lf.lfPitchAndFamily	= %d;\n", m_pShareData->m_Common.m_lf.lfPitchAndFamily	);
//		MYTRACE( "strcpy( m_pShareData->m_Common.m_lf.lfFaceName, \"%s\" );\n", m_pShareData->m_Common.m_lf.lfFaceName	);

	}

	return TRUE;
}




/*! �t�@�C�����J�� */
BOOL CEditDoc::FileRead(
	char*	pszPath,	//!< [in/out]
	BOOL*	pbOpened,
	int		nCharCode,			/*!< [in] �����R�[�h�������� */
	BOOL	bReadOnly,			/*!< [in] �ǂݎ���p�� */
	BOOL	bConfirmCodeChange	/*!< [in] �����R�[�h�ύX���̊m�F�����邩�ǂ��� */
)
{
	int				i;
	HWND			hWndOwner;
	BOOL			bRet;
	FileInfo		fi;
	FileInfo*		pfi;
	HWND			hwndProgress;
	CWaitCursor		cWaitCursor( m_hWnd );
	BOOL			bIsExistInMRU;
	int				nRet;
	BOOL			bFileIsExist;
	int				doctype;

	m_bReadOnly = bReadOnly;	/* �ǂݎ���p���[�h */

//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU			cMRU;

	/* �t�@�C���̑��݃`�F�b�N */
	bFileIsExist = FALSE;
	if( -1 == _access( pszPath, 0 ) ){
	}else{
		HANDLE			hFind;
		WIN32_FIND_DATA	w32fd;
		hFind = ::FindFirstFile( pszPath, &w32fd );
		::FindClose( hFind );
//? 2000.01.18 �V�X�e�������̃t�@�C�����J���Ȃ����
//?		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ){
//?		}else{
			bFileIsExist = TRUE;
//?		}
		/* �t�H���_���w�肳�ꂽ�ꍇ */
		if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
			/* �w��t�H���_�Łu�J���_�C�A���O�v��\�� */
			{
				char*		pszPathNew = new char[_MAX_PATH];
//				int			nCharCode;
//				BOOL		bReadOnly;

				strcpy( pszPathNew, "" );

				/* �u�t�@�C�����J���v�_�C�A���O */
				nCharCode = CODE_AUTODETECT;	/* �����R�[�h�������� */
				bReadOnly = FALSE;
//				::ShowWindow( m_hWnd, SW_SHOW );
				if( !OpenFileDialog( m_hWnd, pszPath, pszPathNew, &nCharCode, &bReadOnly ) ){
					delete [] pszPathNew;
					return FALSE;
				}
				strcpy( pszPath, pszPathNew );
				delete [] pszPathNew;
				if( -1 == _access( pszPath, 0 ) ){
					bFileIsExist = FALSE;
				}else{
					bFileIsExist = TRUE;
				}
			}
		}

	}




	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	*pbOpened = FALSE;
	bRet = TRUE;
	if( NULL == pszPath ){
		MYMESSAGEBOX(
			m_hWnd,
			MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
			"�o�O���႟�������I�I�I",
			"CEditDoc::FileRead()\n\nNULL == pszPath\n�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������ˁB"
		);
		return FALSE;
	}

	/* �w��t�@�C�����J����Ă��邩���ׂ� */
	if( CShareData::getInstance()->IsPathOpened( pszPath, &hWndOwner ) ){
		::SendMessage( hWndOwner, MYWM_GETFILEINFO, 0, 0 );
//		pfi = (FileInfo*)m_pShareData->m_szWork;
		pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

		/* �A�N�e�B�u�ɂ��� */
		ActivateFrameWindow( hWndOwner );

		*pbOpened = TRUE;
		/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
		cMRU.Add( pfi );

		bRet = FALSE;
		goto end_of_func;
	}
	for( i = 0; i < 4; ++i ){
		if( m_cEditViewArr[i].IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			m_cEditViewArr[i].DisableSelectArea( TRUE );
		}
	}

	strcpy( m_szFilePath, pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */


	/* �w�肳�ꂽ�����R�[�h��ʂɕύX���� */
	//	Oct. 25, 2000 genta
	//	�����R�[�h�Ƃ��Ĉُ�Ȓl���ݒ肳�ꂽ�ꍇ�̑Ή�
	//	-1�ȏ�CODE_MAX�����̂ݎ󂯕t����
	//	Oct. 26, 2000 genta
	//	CODE_AUTODETECT�͂��͈̔͂���O��Ă��邩��ʂɃ`�F�b�N
	if( ( -1 <= nCharCode && nCharCode < CODE_CODEMAX ) || nCharCode == CODE_AUTODETECT )
		m_nCharCode = nCharCode;

	/* MRU���X�g�ɑ��݂��邩���ׂ�  ���݂���Ȃ�΃t�@�C������Ԃ� */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	if ( cMRU.GetFileInfo( pszPath, &fi ) ){
		bIsExistInMRU = TRUE;

//		m_cDlgJump.m_bPLSQL = fi.m_bPLSQL;			/* �s�W�����v�� PL/SQL���[�h�� */
//		m_cDlgJump.m_nPLSQL_E1 = fi.m_nPLSQL_E1;	/* �s�W�����v�� PL/SQL���[�h�̂Ƃ��̊�_ */

		if( -1 == m_nCharCode ){
			/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
			m_nCharCode = fi.m_nCharCode;
		}
		/* �t�@�C�������݂��Ȃ� */
		if( FALSE == bFileIsExist &&
			CODE_AUTODETECT == m_nCharCode	/* �����R�[�h�������� */
		){
			m_nCharCode = 0;
		}
		if( CODE_AUTODETECT == m_nCharCode ){	/* �����R�[�h�������� */
			/*
			|| �t�@�C���̓��{��R�[�h�Z�b�g����
			||
			|| �y�߂�l�z
			||	SJIS	0
			||	JIS		1
			||	EUC		2
			||	Unicode	3
			||	�G���[	-1
			*/
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( -1 == m_nCharCode ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					"%s\n�����R�[�h�̔��ʏ����ŃG���[���������܂����B",
					pszPath
				);
				strcpy( m_szFilePath, "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
		if( m_nCharCode != fi.m_nCharCode ){
			if( bConfirmCodeChange ){
				char*	pszCodeName = NULL;
				char*	pszCodeNameNew = NULL;
				switch( fi.m_nCharCode ){
				case CODE_SJIS:		/* SJIS */		pszCodeName = "SJIS";break;	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
				case CODE_JIS:		/* JIS */		pszCodeName = "JIS";break;
				case CODE_EUC:		/* EUC */		pszCodeName = "EUC";break;
				case CODE_UNICODE:	/* Unicode */	pszCodeName = "Unicode";break;
				case CODE_UTF8:		/* UTF-8 */		pszCodeName = "UTF-8";break;
				case CODE_UTF7:		/* UTF-7 */		pszCodeName = "UTF-7";break;
				}
				switch( m_nCharCode ){
				case CODE_SJIS:		/* SJIS */		pszCodeNameNew = "SJIS";break;	//Sept. 1, 2000 jepro '�V�t�g'��'S'�ɕύX
				case CODE_JIS:		/* JIS */		pszCodeNameNew = "JIS";break;
				case CODE_EUC:		/* EUC */		pszCodeNameNew = "EUC";break;
				case CODE_UNICODE:	/* Unicode */	pszCodeNameNew = "Unicode";break;
				case CODE_UTF8:		/* UTF-8 */		pszCodeNameNew = "UTF-8";break;
				case CODE_UTF7:		/* UTF-7 */		pszCodeNameNew = "UTF-7";break;
				}
				if( pszCodeName != NULL ){
					::MessageBeep( MB_ICONQUESTION );
					nRet = MYMESSAGEBOX(
						m_hWnd,
						MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST,
						"�����R�[�h���",
						"%s\n\n���̃t�@�C���́A�O��͕ʂ̕����R�[�h %s �ŊJ����Ă��܂��B\n�O��Ɠ��������R�[�h���g���܂����H\n\n�E[�͂�(Y)]  ��%s\n�E[������(N)]��%s\n�E[�L�����Z��]���J���܂���",
						m_szFilePath, pszCodeName, pszCodeName, pszCodeNameNew
					);
					if( IDYES == nRet ){
						/* �O��Ɏw�肳�ꂽ�����R�[�h��ʂɕύX���� */
						m_nCharCode = fi.m_nCharCode;
					}else
					if( IDCANCEL == nRet ){
						m_nCharCode = 0;
						strcpy( m_szFilePath, "" );
						bRet = FALSE;
						goto end_of_func;
					}
				}else{
					MYMESSAGEBOX(
						m_hWnd,
						MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST,
						"�o�O���႟�������I�I�I",
						"�y�Ώ��z�G���[�̏o���󋵂���҂ɘA�����Ă��������B"
					);
					strcpy( m_szFilePath, "" );
					bRet = FALSE;
					goto end_of_func;
				}
			}
		}
	}else{
		bIsExistInMRU = FALSE;
		/* �t�@�C�������݂��Ȃ� */
		if( FALSE == bFileIsExist &&
			CODE_AUTODETECT == m_nCharCode		/* �����R�[�h�������� */
		){
			m_nCharCode = 0;
		}
		if( CODE_AUTODETECT == m_nCharCode ){	/* �����R�[�h�������� */
			/*
			|| �t�@�C���̓��{��R�[�h�Z�b�g����
			||
			|| �y�߂�l�z
			||	SJIS	0
			||	JIS		1
			||	EUC		2
			||	Unicode	3
			||	�G���[	-1
			*/
			m_nCharCode = CMemory::CheckKanjiCodeOfFile( pszPath );
			if( -1 == m_nCharCode ){
				::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
					"%s\n�����R�[�h�̔��ʏ����ŃG���[���������܂����B",
					pszPath
				);
				strcpy( m_szFilePath, "" );
				bRet = FALSE;
				goto end_of_func;
			}
		}
	}
	if( -1 == m_nCharCode ){
		m_nCharCode = 0;
	}

//		if( (_access( pszPath, 0 )) == -1 ){
//			::MYMESSAGEBOX(
//				m_hwndParent,
//				MB_OK | MB_ICONSTOP | MB_TOPMOST,
//				GSTR_APPNAME,
//				"\'%s\'\n�t�@�C���͑��݂��܂���B �V�K�ɍ쐬���܂��B",
//				pszPath
//			);
//
//			strcpy( m_szFilePath, pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */
//			m_nCharCode = CODE_SJIS;
//
//			return TRUE;
//		}

	//	Nov. 12, 2000 genta �����O�t�@�C�����̎擾��O���Ɉړ�
	char szWork[MAX_PATH];
	/* �����O�t�@�C�������擾���� */
	if( TRUE == ::GetLongFileName( pszPath, szWork ) ){
		strcpy( m_szFilePath, szWork );
	}

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* �t�@�C�������݂��Ȃ� */
	if( FALSE == bFileIsExist ){
//		::MessageBeep( MB_ICONINFORMATION );

		::MYMESSAGEBOX(
			m_hwndParent,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
//			"\'%s\'\n�t�@�C���͑��݂��܂���B �t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��Ƀt�@�C�����쐬����܂��B",
			"%s\n�Ƃ����t�@�C���͑��݂��܂���B\n\n�t�@�C����ۑ������Ƃ��ɁA�f�B�X�N��ɂ��̃t�@�C�����쐬����܂��B",	//Mar. 24, 2001 jepro �኱�C��
			pszPath
		);

//		::MessageBeep( MB_ICONHAND );
//		::MYMESSAGEBOX( hwndParent, MB_OK | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
//			"'%s'\n�t�@�C�������݂��܂���B",  pszPath
//
//		);
	}else{
		/* �t�@�C����ǂ� */
		if( NULL != hwndProgress ){
			::ShowWindow( hwndProgress, SW_SHOW );
		}
		if( FALSE == m_cDocLineMgr.ReadFile( m_szFilePath, m_hWnd, hwndProgress,
			m_nCharCode, &m_FileTime, m_pShareData->m_Common.GetAutoMIMEdecode() ) ){
			strcpy( m_szFilePath, "" );
			bRet = FALSE;
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
			ref.m_nMaxLineSize,
			ref.m_bWordWrap,			/* �p�����[�h���b�v������ */
			ref.m_nTabSpace,
			ref.m_szLineComment,		/* �s�R�����g�f���~�^ */
			ref.m_szLineComment2,		/* �s�R�����g�f���~�^2 */
			ref.m_szLineComment3,		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
			ref.m_szBlockCommentFrom,	/* �u���b�N�R�����g�f���~�^(From) */
			ref.m_szBlockCommentTo,		/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
			ref.m_szBlockCommentFrom2,	/* �u���b�N�R�����g�f���~�^2(From) */
			ref.m_szBlockCommentTo2,	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
			ref.m_nStringType,			/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
			TRUE,
			hwndProgress,
			ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
			ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
		);
	}

	/* �S�r���[�̏������F�t�@�C���I�[�v��/�N���[�Y�����ɁA�r���[������������ */
	InitAllView();

	//	Nov. 20, 2000 genta
	//	IME��Ԃ̐ݒ�
	SetImeMode( GetDocumentAttribute().m_nImeState );

	if( bIsExistInMRU && m_pShareData->m_Common.GetRestoreCurPosition() ){
//#ifdef _DEBUG
//	if( FALSE == m_bDebugMode ){
//		m_cShareData.TraceOut( "bIsExistInMRU==TRUE [%s] fi.m_nX=%d, fi.m_nY=%d\n", fi.m_szPath, fi.m_nX, fi.m_nY );
//	}
//#endif
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nCaretPosX;
		int		nCaretPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			fi.m_nX,
			fi.m_nY,
			&nCaretPosX,
			&nCaretPosY
		);
		if( nCaretPosY >= m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
//			m_cEditViewArr[m_nActivePaneIndex].Command_GOFILEEND(FALSE);
			m_cEditViewArr[m_nActivePaneIndex].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine = fi.m_nViewTopLine; // 2001/10/20 novice
			m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol = fi.m_nViewLeftCol; // 2001/10/20 novice
			m_cEditViewArr[m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE );
			m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX_Prev =
				m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
		}
	}
	// 2002.01.16 hor �u�b�N�}�[�N����
	if( bIsExistInMRU ){
		if( m_pShareData->m_Common.GetRestoreBookmarks() ){
			m_cDocLineMgr.SetBookMarks(fi.m_szMarkLines);
		}
	}else{
		strcpy(fi.m_szMarkLines,"");
	}
	SetFileInfo( &fi );

	//	May 12, 2000 genta
	//	���s�R�[�h�̐ݒ�
	//	May 12, 2000 genta
	{
		SetNewLineCode( EOL_CRLF );
		CDocLine*	pFirstlineinfo = m_cDocLineMgr.GetLineInfo( 0 );
		if( pFirstlineinfo != NULL ){
			enumEOLType t = (enumEOLType)pFirstlineinfo->m_cEol;
			if( t != EOL_NONE && t != EOL_UNKNOWN )
				SetNewLineCode( t );
		}
	}

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );

end_of_func:;
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}
	if( TRUE == bRet && 0 < lstrlen( m_szFilePath ) ){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	return bRet;
}


/*!	@brief �t�@�C���̕ۑ�
	
	@param pszPath [in] �ۑ��t�@�C����
	@param cEolType [in] ���s�R�[�h���
	
	pszPath��NULL�ł����Ă͂Ȃ�Ȃ��B
	
	@date Feb. 9, 2001 genta ���s�R�[�h�p�����ǉ�
*/
BOOL CEditDoc::FileWrite( const char* pszPath, enumEOLType cEolType )
{
	BOOL		bRet;
	FileInfo	fi;
	HWND		hwndProgress;
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU		cMRU;
	//	Feb. 9, 2001 genta
	CEOL	cEol( cEolType );

	if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
		::MessageBeep( MB_ICONHAND );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONSTOP | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n\n�͓ǂݎ���p���[�h�ŊJ���Ă��܂��B �㏑���ۑ��͂ł��܂���B\n\n���O��t���ĕۑ�������΂����Ǝv���܂��B",
			lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j"
		);
		return FALSE;
	}


	bRet = TRUE;

	CEditWnd* pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_SHOW );
	}


	/* �t�@�C���̔r�����b�N���� */
	DoFileUnLock();

	if( m_pShareData->m_Common.m_bBackUp ){	/* �o�b�N�A�b�v�̍쐬 */
		MakeBackUp();
	}

	CWaitCursor cWaitCursor( m_hWnd );
	if( FALSE == m_cDocLineMgr.WriteFile( pszPath, m_hWnd, hwndProgress, m_nCharCode, &m_FileTime, cEol ) ){
		bRet = FALSE;
		goto end_of_func;
	}
	
	/* �����O�t�@�C�������擾����B�i�㏑���ۑ��̂Ƃ��̂݁j */
	char szWork[MAX_PATH];
	if( TRUE == ::GetLongFileName( m_szFilePath, szWork ) ){
		strcpy( m_szFilePath, szWork );
	}

	int	v;
	for( v = 0; v < 4; ++v ){
		if( m_nActivePaneIndex != v ){
			m_cEditViewArr[v].RedrawAll();
		}
	}
	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();

	strcpy( m_szFilePath, pszPath ); /* ���ݕҏW���̃t�@�C���̃p�X */

	SetModified(false,false);	//	Jan. 22, 2002 genta �֐��� �X�V�t���O�̃N���A

	SetFileInfo( &fi );

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );

	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
	m_cOpeBuf.SetNoModified();

	m_bReadOnly = FALSE;	/* �ǂݎ���p���[�h */

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();
end_of_func:;

	if( 0 < lstrlen( m_szFilePath ) &&
		FALSE == m_bReadOnly && /* �ǂݎ���p���[�h �ł͂Ȃ� */
		TRUE == bRet
	){
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	if( NULL != hwndProgress ){
		::ShowWindow( hwndProgress, SW_HIDE );
	}

	return bRet;
}



/* �u�t�@�C�����J���v�_�C�A���O */
BOOL CEditDoc::OpenFileDialog(
	HWND		hwndParent,
	const char*	pszOpenFolder,	//NULL�ȊO���w�肷��Ə����t�H���_���w��ł���
	char*		pszPath,		//�J���t�@�C���̃p�X���󂯎��A�h���X
	int*		pnCharCode,		//�w�肳�ꂽ�����R�[�h��ʂ��󂯎��A�h���X
	BOOL*		pbReadOnly		//�ǂݎ���p��
)
{
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndParent );
	ActivateFrameWindow( hwndParent );

//	int		i;
//	int		j;
	char**	ppszMRU;
	char**	ppszOPENFOLDER;

	/* MRU���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);


	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
//@@@ 2001.12.26 YAZAKI OPENFOLDER���X�g�́ACMRUFolder�ɂ��ׂĈ˗�����
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	if( 0 == lstrlen( m_szFilePath ) ){
		if( NULL == pszOpenFolder ){
			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				ppszMRU[0],
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
		}else{
			char*	pszFolderNew = new char[MAX_PATH];
			int		nDummy;
			int		nCharChars;
			strcpy( pszFolderNew, pszOpenFolder );
			nDummy = lstrlen( pszFolderNew );
			/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
			nCharChars = &pszFolderNew[nDummy] - CMemory::MemCharPrev( pszFolderNew, nDummy, &pszFolderNew[nDummy] );
			if( 1 == nCharChars && pszFolderNew[nDummy - 1] == '\\' ){
			}else{
				strcat( pszFolderNew, "\\" );
			}


			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				pszFolderNew/*pszOpenFolder*/,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
			delete [] pszFolderNew;
		}
	}else{
		if( NULL == pszOpenFolder ){
			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				m_szFilePath,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
		}else{
			char*	pszFolderNew = new char[MAX_PATH];
			int		nDummy;
			int		nCharChars;
			strcpy( pszFolderNew, pszOpenFolder );
			nDummy = lstrlen( pszFolderNew );
			/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
			nCharChars = &pszFolderNew[nDummy] - CMemory::MemCharPrev( pszFolderNew, nDummy, &pszFolderNew[nDummy] );
			if( 1 == nCharChars && pszFolderNew[nDummy - 1] == '\\' ){
			}else{
				strcat( pszFolderNew, "\\" );
			}

			m_cDlgOpenFile.Create(
				m_hInstance,
				/*NULL*//*m_hWnd*/hwndParent,
				m_szDefaultWildCard,
				pszFolderNew/*pszOpenFolder*/,
				(const char **)ppszMRU,
				(const char **)ppszOPENFOLDER
			);
			delete [] pszFolderNew;
		}
	}
	if( m_cDlgOpenFile.DoModalOpenDlg( pszPath, pnCharCode, pbReadOnly ) ){
		delete [] ppszMRU;
		delete [] ppszOPENFOLDER;
		return TRUE;
	}else{
		delete [] ppszMRU;
		delete [] ppszOPENFOLDER;
		return FALSE;
	}
}



//pszOpenFolder pszOpenFolder


/* �u�t�@�C������t���ĕۑ��v�_�C�A���O

	�t�@�C���������ĕۑ��_�C�A���O��\�����āA
	�@pszPath�F�ۑ��t�@�C����
	�@pnCharCode�F�ۑ������R�[�h�Z�b�g
	�@pcEol�F�ۑ����s�R�[�h
	���擾����B
*/
//	Feb. 9, 2001 genta	���s�R�[�h�����������ǉ�
BOOL CEditDoc::SaveFileDialog( char* pszPath, int* pnCharCode, CEOL* pcEol )
{
	char**	ppszMRU;		//	�ŋ߂̃t�@�C��
	char**	ppszOPENFOLDER;	//	�ŋ߂̃t�H���_
	BOOL	bret;

	/* MRU���X�g�̃t�@�C���̃��X�g */
	CMRU cMRU;
	ppszMRU = NULL;
	ppszMRU = new char*[ cMRU.Length() + 1 ];
	cMRU.GetPathList(ppszMRU);

	/* OPENFOLDER���X�g�̃t�@�C���̃��X�g */
	CMRUFolder cMRUFolder;
	ppszOPENFOLDER = NULL;
	ppszOPENFOLDER = new char*[ cMRUFolder.Length() + 1 ];
	cMRUFolder.GetPathList(ppszOPENFOLDER);

	/* �t�@�C���ۑ��_�C�A���O�̏����� */
	/* �t�@�C�����̖����t�@�C����������AppszMRU[0]���f�t�H���g�t�@�C�����Ƃ��āHppszOPENFOLDER����Ȃ��H */
	if( 0 == lstrlen( m_szFilePath ) ){
		m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, m_szDefaultWildCard, ppszMRU[0], (const char **)ppszMRU, (const char **)ppszOPENFOLDER );
	}else{
		m_cDlgOpenFile.Create( m_hInstance, /*NULL*/m_hWnd, m_szDefaultWildCard, m_szFilePath, (const char **)ppszMRU, (const char **)ppszOPENFOLDER );
	}

	/* �_�C�A���O��\�� */
	bret = m_cDlgOpenFile.DoModalSaveDlg( pszPath, pnCharCode, pcEol );

	delete [] ppszMRU;
	delete [] ppszOPENFOLDER;
	return bret;
}





/*! ���ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheet( int nPageNum/*, int nActiveItem*/ )
{
	int		i;
//	BOOL	bModify;
	m_cPropCommon.m_Common = m_pShareData->m_Common;
	m_cPropCommon.m_nKeyNameArrNum = m_pShareData->m_nKeyNameArrNum;
	for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
		m_cPropCommon.m_pKeyNameArr[i] = m_pShareData->m_pKeyNameArr[i];
	}
	m_cPropCommon.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;
	for( i = 0; i < MAX_TYPES; ++i ){
		m_cPropCommon.m_Types[i] = m_pShareData->m_Types[i];
	}
	/* �}�N���֌W
	@@@ 2002.01.03 YAZAKI ���ʐݒ�w�}�N���x���^�u��؂�ւ��邾���Őݒ肪�ۑ�����Ȃ��悤�ɁB
	*/
	for( i = 0; i < MAX_CUSTMACRO; ++i ){
		m_cPropCommon.m_MacroTable[i] = m_pShareData->m_MacroTable[i];
	}
	strcpy(m_cPropCommon.m_szMACROFOLDER, m_pShareData->m_szMACROFOLDER);

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropCommon.DoPropertySheet( nPageNum/*, nActiveItem*/ ) ){
//		/* �ύX���ꂽ���H */
//		if( 0 != memcmp( m_pShareData->m_pKeyNameArr, m_cPropCommon.m_pKeyNameArr, sizeof( m_pShareData->m_pKeyNameArr ) ) ){
//			m_pShareData->m_bKeyBindModify = TRUE;	/* �ύX�t���O �L�[���蓖�� */
			for( i = 0; i < sizeof( m_pShareData->m_pKeyNameArr ) / sizeof( m_pShareData->m_pKeyNameArr[0] ); ++i ){
//				if( 0 != memcmp( &m_cPropCommon.m_pKeyNameArr[i], &m_pShareData->m_pKeyNameArr[i], sizeof( m_cPropCommon.m_pKeyNameArr[i] ) ) ){
					m_pShareData->m_pKeyNameArr[i] = m_cPropCommon.m_pKeyNameArr[i];
//					m_pShareData->m_bKeyBindModifyArr[i] = TRUE;	/* �ύX�t���O �L�[���蓖��(�L�[����) */
//				}
			}
//		}
//		/* �ύX�󋵂𒲍� */
//		bModify = m_pShareData->m_CKeyWordSetMgr.IsModify(
//			m_cPropCommon.m_CKeyWordSetMgr,
//			&m_pShareData->m_bKeyWordSetModifyArr[0]
//		);
//		if( bModify ){
//			m_pShareData->m_bKeyWordSetModify = TRUE;
			m_pShareData->m_CKeyWordSetMgr = m_cPropCommon.m_CKeyWordSetMgr;
//		}

//		/* �ύX�t���O(���ʐݒ�̑S��) �̃Z�b�g */
//		if( 0 != memcmp( &m_pShareData->m_Common, &m_cPropCommon.m_Common, sizeof( Common ) ) ){
//			/* �ύX�t���O(���ʐݒ�̑S��) �̃Z�b�g */
//			m_pShareData->m_nCommonModify = TRUE;
			m_pShareData->m_Common = m_cPropCommon.m_Common;
//		}else{
//		}

//		/* ���ʐݒ�ƃL�[���[�h�ݒ肪���ύX�̏ꍇ�́A�Ȃɂ����Ȃ� */
//		if( FALSE == m_pShareData->m_nCommonModify
//		 && FALSE == m_pShareData->m_bKeyWordSetModify
//		){
//			return FALSE;
//		}

		for( i = 0; i < MAX_TYPES; ++i ){
//			/* �ύX���ꂽ���H */
//			if( 0 != memcmp( &m_pShareData->m_Types[i], &m_cPropCommon.m_Types[i], sizeof( Types ) ) ){
				/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
				m_pShareData->m_Types[i] = m_cPropCommon.m_Types[i];
//				/* �ύX�t���O(�^�C�v�ʐݒ�) �̃Z�b�g */
//				m_pShareData->m_nTypesModifyArr[i] = TRUE;
//			}
		}

		/* �}�N���֌W */
		for( i = 0; i < MAX_CUSTMACRO; ++i ){
			m_pShareData->m_MacroTable[i] = m_cPropCommon.m_MacroTable[i];
		}
		strcpy(m_pShareData->m_szMACROFOLDER, m_cPropCommon.m_szMACROFOLDER);

		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, m_hwndParent );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */

		return TRUE;
	}else{
		return FALSE;
	}
}



/*! �^�C�v�ʐݒ� �v���p�e�B�V�[�g */
BOOL CEditDoc::OpenPropertySheetTypes( int nPageNum, int nSettingType )
{
	m_cPropTypes.m_Types = m_pShareData->m_Types[nSettingType];
	m_cPropTypes.m_CKeyWordSetMgr = m_pShareData->m_CKeyWordSetMgr;

	/* �v���p�e�B�V�[�g�̍쐬 */
	if( m_cPropTypes.DoPropertySheet( nPageNum ) ){
//		/* �ύX���ꂽ���H */
//		if( 0 == memcmp( &m_pShareData->m_Types[nSettingType], &m_cPropTypes.m_Types, sizeof( Types ) ) ){
//			/* ���ύX */
//			return FALSE;
//		}
//		/* �ύX�t���O(�^�C�v�ʐݒ�) �̃Z�b�g */
//		m_pShareData->m_nTypesModifyArr[nSettingType] = TRUE;
		/* �ύX���ꂽ�ݒ�l�̃R�s�[ */
		m_pShareData->m_Types[nSettingType] = m_cPropTypes.m_Types;

//		/* �܂�Ԃ��������ύX���ꂽ */
//		if( m_cPropTypes.m_nMaxLineSize_org != m_cPropTypes.m_Types.m_nMaxLineSize){
//			/*�A���h�D�E���h�D�o�b�t�@�̃N���A */
//			/* �S�v�f�̃N���A */
//			m_cOpeBuf.ClearAll();
//		}
		/* �A�N�Z�����[�^�e�[�u���̍č쐬 */
		::SendMessage( m_pShareData->m_hwndTray, MYWM_CHANGESETTING,  (WPARAM)0, (LPARAM)0 );

		/* �ݒ�ύX�𔽉f������ */
		CShareData::getInstance()->SendMessageToAllEditors( MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, m_hwndParent );	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */

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
	UINT uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( ::IsClipboardFormatAvailable( CF_OEMTEXT )
	 || ::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return TRUE;
	}
	return FALSE;
}





/*! �e�E�B���h�E�̃^�C�g�����X�V

	@param bKillFocus [in] true: Active�̕\�� / false: Inactive�̕\��
*/
void CEditDoc::SetParentCaption( BOOL bKillFocus )
{
	if( NULL == m_hWnd ){
		return;
	}
	if( !m_cEditViewArr[m_nActivePaneIndex].m_bDrawSWITCH ){
		return;
	}


	HWND	hwnd;
	char	pszCap[1024];	//	Nov. 6, 2000 genta �I�[�o�[�w�b�h�y���̂���Heap��Stack�ɕύX

//	/* �A�C�R��������Ă��Ȃ����̓t���p�X */
//	/* �A�C�R��������Ă��鎞�̓t�@�C�����̂� */
//	if( ::IsIconic( m_hWnd ) ){
//		bKillFocus = TRUE;
//	}else{
//		bKillFocus = FALSE;
//	}


	const char*	pszAppName = GSTR_APPNAME;
	char*		pszMode;
	char*		pszKeyMacroRecking;


	hwnd = m_hwndParent;

	if( m_bReadOnly ){	/* �ǂݎ���p���[�h */
		pszMode = "�i�ǂݎ���p�j";
	}else
	if( 0 != m_nFileShareModeOld && /* �t�@�C���̔r�����䃂�[�h */
		NULL == m_hLockedFile		/* ���b�N���Ă��Ȃ� */
	){
		pszMode = "�i�㏑���֎~�j";
	}else{
		pszMode = "";
	}

	if( TRUE == m_pShareData->m_bRecordingKeyMacro &&	/* �L�[�{�[�h�}�N���̋L�^�� */
		m_pShareData->m_hwndRecordingKeyMacro == hwnd	/* �L�[�{�[�h�}�N�����L�^���̃E�B���h�E */
	){
		pszKeyMacroRecking = "  �y�L�[�}�N���̋L�^���z";
	}else{
		pszKeyMacroRecking = "";
	}



	if( m_bGrepMode ){
		/* �f�[�^���w��o�C�g���ȓ��ɐ؂�l�߂� */
		CMemory		cmemDes;
		int			nDesLen;
		const char*	pszDes;
		LimitStringLengthB( m_szGrepKey, lstrlen( m_szGrepKey ), 64, cmemDes );
		pszDes = cmemDes.GetPtr( NULL );
		nDesLen = lstrlen( pszDes );
//		wsprintf( pszCap, "�yGrep�z\"%s%s\" - %s",
		wsprintf( pszCap, "%s%s - %s",
			pszDes, ( (int)lstrlen( m_szGrepKey ) > nDesLen ) ? "�E�E�E":"",
			pszAppName
		);
//#ifdef _DEBUG
	}else
	if( m_bDebugMode ){
		wsprintf( pszCap, "�A�E�g�v�b�g - %s%s",
			pszAppName,
			m_bReadOnly ? "�i�㏑���֎~�j" : ""	/* �ǂݎ���p���[�h */
		 );
//#endif
	}else{

		if( 0 < lstrlen( m_szFilePath ) && (::IsIconic( hwnd ) || bKillFocus ) ){
			char szFname[_MAX_FNAME];
			char szExt[_MAX_EXT];
			_splitpath( m_szFilePath, NULL, NULL, szFname, szExt );
			//Oct. 11, 2000 jepro note�F �A�N�e�B�u�łȂ����̃^�C�g���\��
			wsprintf(
				pszCap,
				"%s%s%s - %s %d.%d.%d.%d %s%s",	//Jul. 06, 2001 jepro UR �͂����t���Ȃ��Ȃ����̂�Y��Ă���
				szFname, szExt,
				IsModified() ? "�i�X�V�j" : "",	/* �ύX�t���O */
				pszAppName,
				HIWORD( m_pShareData->m_dwProductVersionMS ),
				LOWORD( m_pShareData->m_dwProductVersionMS ),
				HIWORD( m_pShareData->m_dwProductVersionLS ),
				LOWORD( m_pShareData->m_dwProductVersionLS ),
				pszMode,	/* ���[�h */
				pszKeyMacroRecking
			);
		}else{

			//Oct. 11, 2000 jepro note�F �A�N�e�B�u�Ȏ��̃^�C�g���\��
			wsprintf(
				pszCap,
				"%s%s - %s %d.%d.%d.%d %s%s",		//Jul. 06, 2001 jepro UR �͂����t���Ȃ��Ȃ����̂�Y��Ă���
				lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j",
				IsModified() ? "�i�X�V�j" : "",	/* �ύX�t���O */
				pszAppName,
				HIWORD( m_pShareData->m_dwProductVersionMS ),
				LOWORD( m_pShareData->m_dwProductVersionMS ),
				HIWORD( m_pShareData->m_dwProductVersionLS ),
				LOWORD( m_pShareData->m_dwProductVersionLS ),
				pszMode,	/* ���[�h */
				pszKeyMacroRecking
			);
		}
	}
	// delete [] pszCap;
	//if (strcmp( m_pszCaption, pszCap ) != 0){
		::SetWindowText( hwnd, pszCap );
	//	strcpy( m_pszCaption, pszCap );
	//}
	return;
}




/*! �o�b�N�A�b�v�̍쐬
	@author genta
	@date 2001.06.12 asa-o
		�t�@�C���̎��������Ƀo�b�N�A�b�v�t�@�C�������쐬����@�\
	@date 2001.12.11 MIK �o�b�N�A�b�v�t�@�C�����S�~���ɓ����@�\
	
*/
BOOL CEditDoc::MakeBackUp( void )
{
	time_t	ltime;
	struct	tm *today, *gmt, xmas = { 0, 0, 12, 25, 11, 93 };
	char	szTime[64];
	char	szForm[64];
	char	szPath[_MAX_PATH];
	char	szDrive[_MAX_DIR];
	char	szDir[_MAX_DIR];
	char	szFname[_MAX_FNAME];
	char	szExt[_MAX_EXT];
//	int		nLen;
	int		nRet;
	char*	pBase;

	/* �t�@�C�������t���Ă��邩 */
	if( 0 >= lstrlen( m_szFilePath ) ){
		return FALSE;
	}

	/* �o�b�N�A�b�v�\�[�X�̑��݃`�F�b�N */
	if( (_access( m_szFilePath, 0 )) == -1 ){
		return FALSE;
	}

	/* �p�X�̕��� */
	_splitpath( m_szFilePath, szDrive, szDir, szFname, szExt );

	if( m_pShareData->m_Common.m_bBackUpFolder ){	/* �w��t�H���_�Ƀo�b�N�A�b�v���쐬���� */
		strcpy( szPath, m_pShareData->m_Common.m_szBackUpFolder );
		/* �t�H���_�̍Ōオ���p����'\\'�łȂ��ꍇ�́A�t������ */
		AddLastYenFromDirectoryPath( szPath );
	}
	else{
		wsprintf( szPath, "%s%s", szDrive, szDir );
	}
	pBase = szPath + strlen( szPath );

	/* �o�b�N�A�b�v�t�@�C�����̃^�C�v 1=(.bak) 2=*_���t.* */
	switch( m_pShareData->m_Common.GetBackupType() ){
	case 1:
		wsprintf( pBase, "%s%s", szFname, ".bak" );
		break;
	case 2:	//	���t�C����
		_tzset();
		_strdate( szTime );
		time( &ltime );				/* �V�X�e�������𓾂܂� */
		gmt = gmtime( &ltime );		/* �����W�����ɕϊ����� */
		today = localtime( &ltime );/* ���n���Ԃɕϊ����� */

		strcpy( szForm, "" );
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
			strcat( szForm, "%Y" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
			strcat( szForm, "%m" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
			strcat( szForm, "%d" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
			strcat( szForm, "%H" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
			strcat( szForm, "%M" );
		}
		if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
			strcat( szForm, "%S" );
		}
		/* YYYYMMDD�����b �`���ɕϊ� */
		strftime( szTime, sizeof( szTime ) - 1, szForm, today );
		wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
		break;
//	2001/06/12 Start by asa-o: �t�@�C���ɕt������t��O��̕ۑ���(�X�V����)�ɂ���
	case 4:	//	���t�C����
		{
			HANDLE		hFile;
			FILETIME	LastWriteTime,
						LocalTime;
			SYSTEMTIME	SystemTime;

			hFile = ::CreateFile(m_szFilePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			::GetFileTime(hFile,NULL,NULL,&LastWriteTime);			// �t�@�C���̃^�C�v�X�^���v���擾(�X�V�����̂�)
			CloseHandle(hFile);
			::FileTimeToLocalFileTime(&LastWriteTime,&LocalTime);	// ���n�����ɕϊ�
			::FileTimeToSystemTime(&LocalTime,&SystemTime);			// �V�X�e���^�C���ɕϊ�

			strcpy( szTime, "" );
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_YEAR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̔N */
				wsprintf(szTime,"%d",SystemTime.wYear);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MONTH) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̌� */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wMonth);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_DAY) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̓� */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wDay);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_HOUR) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̎� */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wHour);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_MIN) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕� */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wMinute);
			}
			if( m_pShareData->m_Common.GetBackupOpt(BKUP_SEC) ){	/* �o�b�N�A�b�v�t�@�C�����F���t�̕b */
				wsprintf(szTime,"%s%02d",szTime,SystemTime.wSecond);
			}
			wsprintf( pBase, "%s_%s%s", szFname, szTime, szExt );
		}
		break;
// 2001/06/12 End

	case 3: //	?xx : xx = 00~99, ?�͔C�ӂ̕���
		//	Aug. 15, 2000 genta
		//	�����ł͍쐬����o�b�N�A�b�v�t�@�C�����̂ݐ�������D
		//	�t�@�C������Rotation�͊m�F�_�C�A���O�̌�ōs���D
		szExt[0] = '.';
		szExt[1] = m_pShareData->m_Common.GetBackupExtChar();
		szExt[2] = '0';
		szExt[3] = '0';
		szExt[4] = '\0';

		wsprintf( pBase, "%s%s", szFname, szExt );
		break;
	}

	if( m_pShareData->m_Common.m_bBackUpDialog ){	/* �o�b�N�A�b�v�̍쐬�O�Ɋm�F */
		::MessageBeep( MB_ICONQUESTION );
//From Here Feb. 27, 2001 JEPROtest �L�����Z�����ł���悤�ɂ��A���b�Z�[�W��ǉ�����
//		if( IDYES != MYMESSAGEBOX(
//			m_hWnd,
//			MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,
//			"�o�b�N�A�b�v�쐬�̊m�F",
//			"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H\n\n%s\n    ��\n%s\n\n",
//			lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j",
//			szPath
//		) ){
//			return FALSE;
//		}
		if( m_pShareData->m_Common.m_bBackUpDustBox ){	//@@@ 2001.12.11 add start MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%s\n    ��\n%s\n\n�쐬�����o�b�N�A�b�v�t�@�C�������ݔ��ɕ��荞�݂܂��B\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j",
				szPath
			);
		}else{	//@@@ 2001.12.11 add end MIK
			nRet = ::MYMESSAGEBOX(
				m_hWnd,
				MB_YESNO/*CANCEL*/ | MB_ICONQUESTION | MB_TOPMOST,
				"�o�b�N�A�b�v�쐬�̊m�F",
				"�ύX�����O�ɁA�o�b�N�A�b�v�t�@�C�����쐬���܂��B\n��낵���ł����H  [������(N)] ��I�Ԃƍ쐬�����ɏ㏑���i�܂��͖��O��t���āj�ۑ��ɂȂ�܂��B\n\n%s\n    ��\n%s\n\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j",
				szPath
			);	//Jul. 06, 2001 jepro [���O��t���ĕۑ�] �̏ꍇ������̂Ń��b�Z�[�W���C��
		}	//@@@ 2001.12.11 add MIK
		if( IDNO == nRet ){
			return FALSE;
		}else if( IDCANCEL == nRet ){
			return FALSE;// FALSE �ł̓_���ł�������Ԃ��΂����̂��킩��܂���B�B�B
		}
//To Here Feb. 27, 2001
	}

	//	From Here Aug. 16, 2000 genta
	if( m_pShareData->m_Common.GetBackupType() == 3 ){
		//	���ɑ��݂���Backup�����炷����
		int				i;

		//	�t�@�C�������p
		HANDLE			hFind;
		WIN32_FIND_DATA	fData;

		pBase = pBase + strlen( pBase ) - 2;	//	2: �g���q�̍Ō��2���̈Ӗ�
		//::MessageBox( NULL, pBase, "���������ꏊ", MB_OK );

		//------------------------------------------------------------------
		//	1. �Y���f�B���N�g������backup�t�@�C����1���T��
		for( i = 0; i <= 99; i++ ){	//	�ő�l�Ɋւ�炸�C99�i2���̍ő�l�j�܂ŒT��
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );

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
		int boundary = m_pShareData->m_Common.GetBackupCount();
		boundary = boundary > 0 ? boundary - 1 : 0;	//	�ŏ��l��0
		//::MessageBox( NULL, pBase, "���������ꏊ", MB_OK );

		for( ; i >= boundary; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );
			if( ::DeleteFile( szPath ) == 0 ){
				::MessageBox( m_hWnd, szPath, "�폜���s", MB_OK );
				return FALSE;
				//	���s�����ꍇ
				//	��ōl����
			}
		}

		//	���̈ʒu��i�͑��݂���o�b�N�A�b�v�t�@�C���̍ő�ԍ���\���Ă���D

		//	3. ��������0�Ԃ܂ł̓R�s�[���Ȃ���ړ�
		char szNewPath[MAX_PATH];
		char *pNewNrBase;

		strcpy( szNewPath, szPath );
		pNewNrBase = szNewPath + strlen( szNewPath ) - 2;

		for( ; i >= 0; --i ){
			//	�t�@�C�������Z�b�g
			wsprintf( pBase, "%02d", i );
			wsprintf( pNewNrBase, "%02d", i + 1 );

			//	�t�@�C���̈ړ�
			if( ::MoveFile( szPath, szNewPath ) == 0 ){
				//	���s�����ꍇ
				//	��ōl����
				::MessageBox( m_hWnd, szPath, "�ړ����s", MB_OK );
				return FALSE;
			}
		}
	}
	//	To Here Aug. 16, 2000 genta

	//::MessageBox( NULL, szPath, "���O�̃o�b�N�A�b�v�t�@�C��", MB_OK );
	/* �o�b�N�A�b�v�̍쐬 */
	if( ::CopyFile( m_szFilePath, szPath, FALSE ) ){
		/* ����I�� */
		//@@@ 2001.12.11 start MIK
		if( m_pShareData->m_Common.m_bBackUpDustBox ){
			char	szDustPath[_MAX_PATH+1];
			strcpy(szDustPath, szPath);
			szDustPath[strlen(szDustPath) + 1] = '\0';
			SHFILEOPSTRUCT	fos;
			fos.hwnd   = m_hWnd;
			fos.wFunc  = FO_DELETE;
			fos.pFrom  = szDustPath;
			fos.pTo    = NULL;
			fos.fFlags = FOF_ALLOWUNDO | FOF_SIMPLEPROGRESS | FOF_NOCONFIRMATION;	//�_�C�A���O�Ȃ�
			//fos.fFlags = FOF_ALLOWUNDO | FOF_FILESONLY;
			//fos.fFlags = FOF_ALLOWUNDO;	//�_�C�A���O���\�������B
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
	}
	return TRUE;
}




/* �t�@�C���̔r�����b�N */
void CEditDoc::DoFileLock( void )
{
	char*	pszMode;
	int		nAccessMode;
	BOOL	bCheckOnly;

	/* ���b�N���Ă��� */
	if( NULL != m_hLockedFile ){
		/* ���b�N���� */
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
	}

	/* �t�@�C�������݂��Ȃ� */
	if( -1 == _access( m_szFilePath, 0 ) ){
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = 0;
		return;
	}else{
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = m_pShareData->m_Common.m_nFileShareMode;
	}


	/* �t�@�C�����J���Ă��Ȃ� */
	if( 0 == lstrlen( m_szFilePath ) ){
		return;
	}
	/* �ǂݎ���p���[�h */
	if( TRUE == m_bReadOnly ){
		return;
	}


	nAccessMode = 0;
	if( m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_DENY_WRITE ||
		m_pShareData->m_Common.m_nFileShareMode == OF_SHARE_EXCLUSIVE ){
		bCheckOnly = FALSE;
	}else{
		/* �r�����䂵�Ȃ����ǃ��b�N����Ă��邩�̃`�F�b�N�͍s���̂�return���Ȃ� */
//		return;
		bCheckOnly = TRUE;
	}
	/* �����݋֎~���ǂ������ׂ� */
	if( -1 == _access( m_szFilePath, 2 ) ){	/* �A�N�Z�X���F�������݋��� */
#if 0
		// Apr. 28, 2000 genta: Request from Koda

		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"����\n%s\n�͓ǎ��p�ɐݒ肳��Ă��܂��B �܂��́A�������݂̃A�N�Z�X��������܂���B",
			lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j"
		);
#endif
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}


	m_hLockedFile = ::_lopen( m_szFilePath, OF_READWRITE );
	_lclose( m_hLockedFile );
	if( HFILE_ERROR == m_hLockedFile ){
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n�͌��ݑ��̃v���Z�X�ɂ���ď����݂��֎~����Ă��܂��B",
			lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j"
		);
		m_hLockedFile = NULL;
		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();
		return;
	}
	m_hLockedFile = ::_lopen( m_szFilePath, nAccessMode | m_pShareData->m_Common.m_nFileShareMode );
	if( HFILE_ERROR == m_hLockedFile ){
		switch( m_pShareData->m_Common.m_nFileShareMode ){
		case OF_SHARE_EXCLUSIVE:	/* �ǂݏ��� */
			pszMode = "�ǂݏ����֎~���[�h";
			break;
		case OF_SHARE_DENY_WRITE:	/* ���� */
			pszMode = "�������݋֎~���[�h";
			break;
		}
		::MessageBeep( MB_ICONEXCLAMATION );
		MYMESSAGEBOX(
			m_hWnd,
			MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"%s\n��%s�Ń��b�N�ł��܂���ł����B\n���݂��̃t�@�C���ɑ΂���r������͖����ƂȂ�܂��B",
			lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j",
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
		::_lclose( m_hLockedFile );
		m_hLockedFile = NULL;
		/* �t�@�C���̔r�����䃂�[�h */
		m_nFileShareModeOld = 0;
	}
	return;
}

//	Mar. 15, 2000 genta
//	From Here
/*!
	�X�y�[�X�̔���
*/
inline bool C_IsSpace( char c ){
	return ('\t' == c ||
			 ' ' == c ||
			  CR == c ||
			  LF == c
	);
}

/*!
	�֐��ɗp���邱�Ƃ��ł��镶�����ǂ����̔���
*/
inline bool C_IsWordChar( char c ){
	return ( '_' == c ||
			 ':' == c ||
			 '~' == c ||
			('a' <= c && c <= 'z' )||
			('A' <= c && c <= 'Z' )||
			('0' <= c && c <= '9' )
		);
}
//	To Here

//	From Here Apr. 1, 2001 genta
/*!
	����Ȋ֐��� "operator" ���ǂ����𔻒肷��B

	������"operator"���ꎩ�g���A���邢��::�̌���operator�Ƒ�����
	�I����Ă���Ƃ���operator�Ɣ���B

	���Z�q�̕]��������ۏ؂��邽��2��if���ɕ����Ă���

	@param szStr ����Ώۂ̕�����
	@param nLen ������̒����B
	�{���I�ɂ͕s�v�ł��邪�A�������̂��߂Ɋ��ɂ���l�𗘗p����B
*/
bool C_IsOperator( char* szStr, int nLen	)
{
	if( nLen >= 8 && szStr[ nLen - 1 ] == 'r' ){
		if( nLen > 8 ?
				strcmp( szStr + nLen - 9, ":operator" ) == 0 :	// �����o�[�֐��ɂ���`
				strcmp( szStr, "operator" ) == 0	// friend�֐��ɂ���`
		 ){
		 	return true;
		}
	}
	return false;
}
//	To Here Apr. 1, 2001 genta


/*!
	@brief C/C++�֐����X�g�쐬

	@par MODE�ꗗ
	- 0	�ʏ�
	- 20	Single quotation������ǂݍ��ݒ�
	- 21	Double quotation������ǂݍ��ݒ�
	- 8	�R�����g�ǂݍ��ݒ�
	- 1	�P��ǂݍ��ݒ�
	- 2	�L����ǂݍ��ݒ�
	- 999	���߂���P�ꖳ����

	@par FuncId�̒l�̈Ӗ�
	10�̈ʂŖړI�ʂɎg�������Ă���DC/C++�p��10�ʂ�0
	- 1: �錾
	- 2: �ʏ�̊֐� (�ǉ������񖳂�)

	@param pcFuncInfoArr [out] �֐��ꗗ��Ԃ����߂̃N���X�B
	�����Ɋ֐��̃��X�g��o�^����B
*/
void CEditDoc::MakeFuncList_C( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;			//	nNestLevel	{}�̃��x��
	int			nNestLevel2;		//	nNestLevel2	()�ɑ΂���ʒu
	int			nCharChars;			//	���o�C�g������ǂݔ�΂����߂̂���
	char		szWordPrev[256];	//	1�O��word
	char		szWord[256];		//	���݉�ǒ���word������Ƃ���
	int			nWordIdx = 0;
	int			nMaxWordLeng = 100;	//	���e�����word�̍ő咷��
	int			nMode;				//	���݂�state
	char		szFuncName[256];	//	�֐���
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	//	Mar. 4, 2001 genta
	bool		bLineTop;			//	�s�����ǂ����𔻕ʂ��邽�߂̃t���O
									//	�v���v���Z�b�T�w�ߗp
	//	Mar. 4, 2001 genta
	bool		bCppInitSkip;		//	C++�̃����o�[�ϐ��A�e�N���X�̏������q��SKIP
	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nNestLevel2 = 0;
	nFuncNum = 0;
	bCppInitSkip = false;
//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		//	Mar. 4, 2001 genta
		bLineTop = true;
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
				bLineTop = false;
				continue;
			}
			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �R�����g�ǂݍ��ݒ� */
			if( 8 == nMode ){
				if( i < nLineLen - 1 && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( C_IsWordChar( pLine[i] ) ){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					//	From Here Mar. 31, 2001 genta
					//	operator�L�[���[�h(���Z�qoverload)�̑Ή�
					//	�������Aoperator�L�[���[�h�̌��ɃX�y�[�X�������Ă���Ƃ��܂������Ȃ��B
					if( C_IsOperator( szWord, nWordIdx + 1 ) ){
						//	operator���I
						/*  overload���鉉�Z�q�ꗗ
							& && &=
							| || |=
							+ ++ +=
							- -- -= -> ->*
							* *=
							/ /=
							% %=
							^ ^=
							! !=
							= ==
							< <= << <<=
							> >= >> >>=
							()
							[]
							~
							,
						*/
						int oplen = 0;	// ���Z�q�{�̕��̕�����
						switch( pLine[i] ){
						case '&': // no break
						case '|': // no break
						case '+':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '-':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '-' ||
									pLine[ i + 1 ] == '=' )
									oplen = 2;
								else if( pLine[ i + 1 ] == '>' ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '*' )
											oplen = 3;
									}
								}
							}
							break;
						case '*': // no break
						case '/': // no break
						case '%': // no break
						case '^': // no break
						case '!': // no break
						case '=':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '<': // no break
						case '>':
							oplen = 1;
							if( i + 1 < nLineLen ){
								if( pLine[ i + 1 ] == pLine[ i ] ){
									oplen = 2;
									if( i + 2 < nLineLen ){
										if( pLine[ i + 2 ] == '=' )
											oplen = 3;
									}
								}
								else if( pLine[ i + 1 ] == '=' )
									oplen = 2;
							}
							break;
						case '(':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* ���ʑΉ��΍� ( */ ')' )
									oplen = 2;
							break;
						case '[':
							if( i + 1 < nLineLen )
								if( pLine[ i + 1 ] == /* ���ʑΉ��΍� [ */ ']' )
									oplen = 2;
							break;
						case '~': // no break
						case ',':
							oplen = 2;
							break;
						}

						//	oplen �̒��������L�[���[�h�ɒǉ�
						for( ; oplen > 0 ; oplen--, i++ ){
							++nWordIdx;
							szWord[nWordIdx] = pLine[i];
						}
						szWord[nWordIdx + 1] = '\0';
							// �L����̏������s���O�͋L�����i�͋L����̐擪���w���Ă����B
							// ���̎��_��i�͋L�����1����w���Ă���

							// operator�̌��ɕs���ȕ���������ꍇ�̓���
							// ( �Ŏn�܂�ꍇ��operator�Ƃ����֐��ƔF�������
							// ����ȊO�̋L�����Ə]���ʂ�L����global�̂����Ɍ����B

							// ���Z�q�������Ă���ꍇ�̓���
							// ��������()�̏ꍇ�͂��ꂪ���Z�q�ƌ��Ȃ���邽�߁A���̍s�͊֐���`�ƔF������Ȃ�
							// ����ȊO�̏ꍇ��operator�Ƃ����֐��ƔF�������
					}
					//	To Here Mar. 31, 2001 genta
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( C_IsWordChar( pLine[i] ) ||
					C_IsSpace( pLine[i] ) ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '"' == pLine[i] ||
					 '/' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}else{

					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( C_IsSpace( pLine[i] ) ){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( C_IsSpace( pLine[i] ) )
					continue;
				//	Mar 4, 2001 genta
				//	�v���v���Z�b�T�w�߂͖�������
				if( bLineTop && '#' == pLine[i] )
					break;
				bLineTop = false;
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '/' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else
				if( '{' == pLine[i] ){
					if( 2 == nNestLevel2 ){
						//	������)�̌��{ ���Ȃ킿�֐��̎n�܂�
						if( 0 != strcmp( "sizeof", szFuncName ) ){
							nFuncId = 2;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						}
					}
					nNestLevel2 = 0;
					bCppInitSkip = false;	//	Mar. 4, 2001 genta
					++nNestLevel;
					nMode = 0;
					continue;
				}else
				if( '}' == pLine[i] ){
					nNestLevel2 = 0;
					nNestLevel--;
					nMode = 0;
					continue;
				}else
				if( '(' == pLine[i] ){
					if( nNestLevel == 0 && !bCppInitSkip ){
						strcpy( szFuncName, szWordPrev );
						nFuncLine = nLineCount + 1;
						nNestLevel2 = 1;
					}
					nMode = 0;
					continue;
				}else
				if( ')' == pLine[i] ){
					if( 1 == nNestLevel2 ){
						nNestLevel2 = 2;
					}
					nMode = 0;
					continue;
				}else
				if( ';' == pLine[i] ){
					if( 2 == nNestLevel2 ){
						//	������')'�̌��';' ���Ȃ킿�֐��錾
						if( 0 != strcmp( "sizeof", szFuncName ) ){
							nFuncId = 1;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId );
//							pcFuncInfoArr->AppendData( nFuncLine, szFuncName, nFuncId );
						}
					}
					nNestLevel2 = 0;
					nMode = 0;
					continue;
				}else{
					if( C_IsWordChar( pLine[i] ) ){
						if( 2 == nNestLevel2 ){
							//	�����ʂ��������ǂƂ肠�����o�^�����Ⴄ
							if( 0 != strcmp( "sizeof", szFuncName ) ){
								nFuncId = 2;
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
							}
							nNestLevel2 = 0;
							//	Mar 4, 2001 genta	�������q�������Ƃ��͂���ȍ~�̓o�^�𐧌�����
							if( pLine[i] == ':' )
								bCppInitSkip = true;
						}

						//	//	Mar. 15, 2000 genta
						//	From Here
						//	���O��word�̍Ōオ::���C���邢�͒����word�̐擪��::�Ȃ�
						//	�N���X����q�ƍl���ė��҂�ڑ�����D

						{
							int pos = strlen( szWordPrev ) - 2;
							if( //	�O�̕�����̖����`�F�b�N
								( pos > 0 &&	szWordPrev[pos] == ':' &&
								szWordPrev[pos + 1] == ':' ) ||
								//	���̕�����̐擪�`�F�b�N
								( i < nLineLen - 1 && pLine[i] == ':' &&
									pLine[i+1] == ':' )
							){
								//	�O�̕�����ɑ�����
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	From Here Apr. 1, 2001 genta
							//	operator new/delete ���Z�q�̑Ή�
							else if( C_IsOperator( szWordPrev, pos + 2 ) ){
								//	�X�y�[�X�����āA�O�̕�����ɑ�����
								szWordPrev[pos + 2] = ' ';
								szWordPrev[pos + 3] = '\0';
								strcpy( szWord, szWordPrev );
								nWordIdx = strlen( szWord );
							}
							//	To Here Apr. 1, 2001 genta
							else{
								nWordIdx = 0;
							}
						}
						//	strcpy( szWordPrev, szWord );	�s�v�H
						//	To Here

						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						//	//	Mar. 15, 2000 genta
						//	From Here
						//	�����`�F�b�N�͕K�{
						if( nWordIdx < nMaxWordLeng ){
							nMode = 1;
						}
						else{
							nMode = 999;
						}
						//	To Here
					}else{
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 2;
					}
				}
			}
		}
	}
	return;
}




/*! PL/SQL�֐����X�g�쐬 */
void CEditDoc::MakeFuncList_PLSQL( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
	char		szFuncName[80];
	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	int			nFuncOrProc = 0;
	int			nParseCnt = 0;

	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nFuncNum = 0;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
//			if( 1 < nCharChars ){
//				i += (nCharChars - 1);
//				continue;
//			}
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					if( i + 1 < nLineLen && '\'' == pLine[i + 1] ){
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
				if( i + 1 < nLineLen && '*' == pLine[i] &&  '/' == pLine[i + 1] ){
					++i;
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( (1 == nCharChars && (
					'_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )
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
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}else{
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "FUNCTION" ) ){
						nFuncOrProc = 1;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;

					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PROCEDURE" ) ){
						nFuncOrProc = 2;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 0 == nParseCnt && 0 == _stricmp( szWord, "PACKAGE" ) ){
						nFuncOrProc = 3;
						nParseCnt = 1;
						nFuncLine = nLineCount + 1;
					}else
					if( 1 == nParseCnt && 3 == nFuncOrProc && 0 == _stricmp( szWord, "BODY" ) ){
						nFuncOrProc = 4;
						nParseCnt = 1;
					}else
					if( 1 == nParseCnt ){
						if( 1 == nFuncOrProc ||
							2 == nFuncOrProc ||
							3 == nFuncOrProc ||
							4 == nFuncOrProc ){
							++nParseCnt;
							strcpy( szFuncName, szWord );
						}else
						if( 3 == nFuncOrProc ){

						}
					}else
					if( 2 == nParseCnt ){
						if( 0 == _stricmp( szWord, "IS" ) ){
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
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nFuncLine - 1,
								&nPosX,
								&nPosY
							);
							pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1, szFuncName, nFuncId );
							nParseCnt = 0;
						}
						if( 0 == _stricmp( szWord, "AS" ) ){
							if( 3 == nFuncOrProc ){
								nFuncId = 31;	/* �p�b�P�[�W�d�l�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}else
							if( 4 == nFuncOrProc ){
								nFuncId = 41;	/* �p�b�P�[�W�{�� */
								++nFuncNum;
								/*
								  �J�[�\���ʒu�ϊ�
								  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
								  ��
								  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
								*/
								int		nPosX;
								int		nPosY;
								m_cLayoutMgr.CaretPos_Phys2Log(
									0,
									nFuncLine - 1,
									&nPosX,
									&nPosY
								);
								pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
								nParseCnt = 0;
							}
						}
					}
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
					nMode = 0;
					i--;
					continue;
				}
			}else
			/* �L����ǂݍ��ݒ� */
			if( 2 == nMode ){
				if( '_' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i] ||
					 '{' == pLine[i] ||
					 '}' == pLine[i] ||
					 '(' == pLine[i] ||
					 ')' == pLine[i] ||
					 ';' == pLine[i] ||
					'\'' == pLine[i] ||
					 '/' == pLine[i] ||
					 '-' == pLine[i]
				){
					strcpy( szWordPrev, szWord );
					nWordIdx = 0;
					szWord[0] = '\0';
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
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);
					}
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					continue;
				}else
				if( i < nLineLen - 1 && '-' == pLine[i] &&  '-' == pLine[i + 1] ){
					break;
				}else
				if( i < nLineLen - 1 && '/' == pLine[i] &&  '*' == pLine[i + 1] ){
					++i;
					nMode = 8;
					continue;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( ';' == pLine[i] ){
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
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nFuncLine - 1,
							&nPosX,
							&nPosY
						);
						pcFuncInfoArr->AppendData( nFuncLine, nPosY + 1 , szFuncName, nFuncId );
						nParseCnt = 0;
					}
					nMode = 0;
					continue;
				}else{
					if( (1 == nCharChars && (
						'_' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )
						) )
					 || 2 == nCharChars
					){
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;

//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';
						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
						nWordIdx += (nCharChars);

						nMode = 1;
					}else{
						strcpy( szWordPrev, szWord );
						nWordIdx = 0;
//						szWord[nWordIdx] = pLine[i];
//						szWord[nWordIdx + 1] = '\0';

						memcpy( &szWord[nWordIdx], &pLine[i], nCharChars );
						szWord[nWordIdx + nCharChars] = '\0';
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





/* �e�L�X�g�E�g�s�b�N���X�g�쐬 */
void CEditDoc::MakeTopicList_txt( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
	int						j;
	int						nCharChars;
	int						nCharChars2;
	char*					pszStarts;
	int						nStartsLen;
	char*					pszText;

	pszStarts = m_pShareData->m_Common.m_szMidashiKigou; 	/* ���o���L�� */
	nStartsLen = lstrlen( pszStarts );

//	for( nLineCount = 0; nLineCount <  m_cLayoutMgr.GetLineCount(); ++nLineCount ){
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
//		pLine = (const unsigned char *)m_cLayoutMgr.GetLineStr( nLineCount, &nLineLen );
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		for( i = 0; i < nLineLen; ++i ){
			if( pLine[i] == ' ' ||
				pLine[i] == '\t'){
				continue;
			}else
			if( i + 1 < nLineLen && pLine[i] == 0x81 && pLine[i + 1] == 0x40 ){
				++i;
				continue;
			}
			break;
		}
		if( i >= nLineLen ){
			continue;
		}
		nCharChars = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
		for( j = 0; j < nStartsLen; j+=nCharChars2 ){
			nCharChars2 = CMemory::MemCharNext( pszStarts, nStartsLen, &pszStarts[j] ) - &pszStarts[j];
			if( nCharChars == nCharChars2 ){
				if( 0 == memcmp( &pLine[i], &pszStarts[j], nCharChars ) ){
					break;
				}
			}
		}
		if( j >= nStartsLen ){
			continue;
		}
		if( pLine[i] == '(' ){
			if( ( pLine[i + 1] >= '0' && pLine[i + 1] <= '9' ) ||
				( pLine[i + 1] >= 'A' && pLine[i + 1] <= 'Z' ) ||
				( pLine[i + 1] >= 'a' && pLine[i + 1] <= 'z' )
			){
			}else{
				continue;
			}
		}
		pszText = new char[nLineLen + 1];
		memcpy( pszText, (const char *)&pLine[i], nLineLen );
		pszText[nLineLen] = '\0';
		for( i = 0; i < (int)lstrlen(pszText); ++i ){
			if( pszText[i] == CR ||
				pszText[i] == LF ){
				pszText[i] = '\0';
			}
		}
//		MYTRACE( "pszText=[%s]\n", pszText );
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			0,
			nLineCount,
			&nPosX,
			&nPosY
		);
		pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)pszText, 0 );
//		pcFuncInfoArr->AppendData( nLineCount + 1, (char *)pszText, 0 );
		delete [] pszText;
	}
	return;
}




/*! COBOL �A�E�g���C����� */
void CEditDoc::MakeTopicList_cobol( CFuncInfoArr* pcFuncInfoArr )
{
	const unsigned char*	pLine;
	int						nLineLen;
	int						nLineCount;
	int						i;
//	int						j;
//	int						nCharChars;
//	int						nCharChars2;
//	int						nStartsLen;
//	char*					pszText;
	int						k;
//	int						m;
	char					szDivision[1024];
	char					szLabel[1024];
	const char*				pszKeyWord;
	int						nKeyWordLen;
	BOOL					bDivision;

	szDivision[0] = '\0';
	szLabel[0] =  '\0';


	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = (const unsigned char *)m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		if( NULL == pLine ){
			break;
		}
		/* �R�����g�s�� */
		if( 7 <= nLineLen && pLine[6] == '*' ){
			continue;
		}
		/* ���x���s�� */
		if( 8 <= nLineLen && pLine[7] != ' ' ){
			k = 0;
			for( i = 7; i < nLineLen; ){
				if( pLine[i] == '.'
				 || pLine[i] == CR
				 || pLine[i] == LF
				){
					break;
				}
				szLabel[k] = pLine[i];
				++k;
				++i;
				if( pLine[i - 1] == ' ' ){
					for( ; i < nLineLen; ++i ){
						if( pLine[i] != ' ' ){
							break;
						}
					}
				}
			}
			szLabel[k] = '\0';
//			MYTRACE( "szLabel=[%s]\n", szLabel );



			pszKeyWord = "division";
			nKeyWordLen = lstrlen( pszKeyWord );
			bDivision = FALSE;
			for( i = 0; i <= (int)lstrlen( szLabel ) - nKeyWordLen; ++i ){
				if( 0 == memicmp( &szLabel[i], pszKeyWord, nKeyWordLen ) ){
					szLabel[i + nKeyWordLen] = '\0';
					strcpy( szDivision, szLabel );
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

			int		nPosX;
			int		nPosY;
			char	szWork[1024];
			m_cLayoutMgr.CaretPos_Phys2Log(
				0,
				nLineCount,
				&nPosX,
				&nPosY
			);
			wsprintf( szWork, "%s::%s", szDivision, szLabel );
			pcFuncInfoArr->AppendData( nLineCount + 1, nPosY + 1 , (char *)szWork, 0 );
		}
	}
	return;
}




/*! �A�Z���u�� �A�E�g���C����� */
void CEditDoc::MakeTopicList_asm( CFuncInfoArr* pcFuncInfoArr )
{
	const char*	pLine;
	int			nLineLen;
	int			nLineCount;
	int			i;
	int			nNestLevel;
//	int			nNestLevel2;
	int			nCharChars;
	char		szWordPrev[100];
	char		szWord[100];
	int			nWordIdx = 0;
	int			nMaxWordLeng = 70;
	int			nMode;
//	char		szFuncName[80];
//	int			nFuncLine;
	int			nFuncId;
	int			nFuncNum;
	char		szProcName[1024];
	BOOL		bProcReading;

	nNestLevel = 0;
	szWordPrev[0] = '\0';
	szWord[nWordIdx] = '\0';
	nMode = 0;
	nFuncNum = 0;
	szProcName[0] = '\0';
	bProcReading = FALSE;
	for( nLineCount = 0; nLineCount <  m_cDocLineMgr.GetLineCount(); ++nLineCount ){
		pLine = m_cDocLineMgr.GetLineStr( nLineCount, &nLineLen );
		for( i = 0; i < nLineLen; ++i ){
			/* 1�o�C�g������������������ */
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 1 < nCharChars ){
				i += (nCharChars - 1);
				continue;
			}

			/* �G�X�P�[�v�V�[�P���X�͏�Ɏ�菜�� */
			if( '\\' == pLine[i] ){
				++i;
			}else
			/* �V���O���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 20 == nMode ){
				if( '\'' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �_�u���N�H�[�e�[�V����������ǂݍ��ݒ� */
			if( 21 == nMode ){
				if( '"' == pLine[i] ){
					nMode = 0;
					continue;
				}else{
				}
			}else
			/* �P��ǂݍ��ݒ� */
			if( 1 == nMode ){
				if( '_' == pLine[i] ||
					':' == pLine[i] ||
					'~' == pLine[i] ||
					('a' <= pLine[i] &&	pLine[i] <= 'z' )||
					('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
					('0' <= pLine[i] &&	pLine[i] <= '9' )||
					'.' == pLine[i]
				){
					++nWordIdx;
					if( nWordIdx >= nMaxWordLeng ){
						nMode = 999;
						continue;
					}else{
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
					}
				}else{
					if( 0 == stricmp( "proc", szWord ) ){
						strcpy( szProcName, szWordPrev );
						bProcReading = TRUE;
						nFuncId = 50;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
//						char szWork[256];
						pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szProcName, nFuncId );
					}else
					if( 0 == stricmp( "endp", szWord ) ){
						nFuncId = 52;
						++nFuncNum;
						/*
						  �J�[�\���ʒu�ϊ�
						  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
						  ��
						  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						*/
						int		nPosX;
						int		nPosY;
						m_cLayoutMgr.CaretPos_Phys2Log(
							0,
							nLineCount/*nFuncLine - 1*/,
							&nPosX,
							&nPosY
						);
//						char szWork[256];
						pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szWordPrev, nFuncId );

						strcpy( szProcName, "" );
						bProcReading = FALSE;
					}else{
						/* ���x�� */
						if( 0 <= i - 1 && ':' == pLine[i - 1] ){
							nFuncId = 51;
							++nFuncNum;
							/*
							  �J�[�\���ʒu�ϊ�
							  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
							  ��
							  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
							*/
							int		nPosX;
							int		nPosY;
							m_cLayoutMgr.CaretPos_Phys2Log(
								0,
								nLineCount/*nFuncLine - 1*/,
								&nPosX,
								&nPosY
							);
	//						char szWork[256];
							pcFuncInfoArr->AppendData( nLineCount + 1/*nFuncLine*/, nPosY + 1, szWord, nFuncId );

						}

					}
					strcpy( szWordPrev , szWord );

					nMode = 0;
					i--;
					continue;
				}
			}else
			/* ���߂���P�ꖳ���� */
			if( 999 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					nMode = 0;
					continue;
				}
			}else
			/* �m�[�}�����[�h */
			if( 0 == nMode ){
				/* �󔒂�^�u�L�������΂� */
				if( '\t' == pLine[i] ||
					 ' ' == pLine[i] ||
					  CR == pLine[i] ||
					  LF == pLine[i]
				){
					continue;
				}else
				/* ���C���R�����g�̎��ʎq������ꍇ�A���̍s�̏����� */
				if( i < nLineLen - 1 && ';' == pLine[i] ){
					break;
				}else
				if( '\'' == pLine[i] ){
					nMode = 20;
					continue;
				}else
				if( '"' == pLine[i] ){
					nMode = 21;
					continue;
				}else{
					if( '_' == pLine[i] ||
						':' == pLine[i] ||
						'~' == pLine[i] ||
						('a' <= pLine[i] &&	pLine[i] <= 'z' )||
						('A' <= pLine[i] &&	pLine[i] <= 'Z' )||
						('0' <= pLine[i] &&	pLine[i] <= '9' )||
						'.' == pLine[i]
					){
						nWordIdx = 0;
						szWord[nWordIdx] = pLine[i];
						szWord[nWordIdx + 1] = '\0';
						nMode = 1;
					}else{
						nMode = 0;
					}
				}
			}
		}
	}
	return;
}




/* �A�N�e�B�u�ȃy�C����ݒ� */
void  CEditDoc::SetActivePane( int nIndex )
{
	m_cEditViewArr[m_nActivePaneIndex].OnKillFocus();

	m_nActivePaneIndex = nIndex;	/* �A�N�e�B�u�ȃr���[ */

	m_cEditViewArr[m_nActivePaneIndex].OnSetFocus();

	m_cEditViewArr[m_nActivePaneIndex].RedrawAll();	/* �t�H�[�J�X�ړ����̍ĕ`�� */

	m_cSplitterWnd.SetActivePane( nIndex );

	if( NULL != m_cDlgFind.m_hWnd ){		/* �u�����v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgFind.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cDlgReplace.m_hWnd ){	/* �u�u���v�_�C�A���O */
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cDlgReplace.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}
	if( NULL != m_cHokanMgr.m_hWnd ){	/* �u���͕⊮�v�_�C�A���O */
		m_cHokanMgr.Hide();
		/* ���[�h���X���F�����ΏۂƂȂ�r���[�̕ύX */
		m_cHokanMgr.ChangeView( (LPARAM)&m_cEditViewArr[m_nActivePaneIndex] );
	}

//	2001/06/20 Start by asa-o:	�A�N�e�B�u�łȂ��y�C���̃J�[�\���A���_�[�o�[���\��
	m_cEditViewArr[m_nActivePaneIndex].CaretUnderLineON(TRUE);
	m_cEditViewArr[m_nActivePaneIndex^1].CaretUnderLineOFF(TRUE);
	m_cEditViewArr[m_nActivePaneIndex^2].CaretUnderLineOFF(TRUE);
	m_cEditViewArr[(m_nActivePaneIndex^2)^1].CaretUnderLineOFF(TRUE);
//	2001/06/20 End

	return;
}



/* �A�N�e�B�u�ȃy�C�����擾 */
int CEditDoc::GetActivePane( void )
{
	return m_nActivePaneIndex;
}






/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
BOOL CEditDoc::HandleCommand( int nCommand )
{
	int				i;
	int				j;
	int				nRowNum;
	int				nPane;
	HWND			hwndWork;
	EditNode*		pEditNodeArr;
	switch( nCommand ){
	case F_PREVWINDOW:	//�O�̃E�B���h�E
		nPane = m_cSplitterWnd.GetPrevPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				for( i = 0; i < nRowNum; ++i ){
					j = 0;
					if( m_hwndParent == pEditNodeArr[i].m_hWnd ){
						j = i;
						break;
					}
				}
				if( j == 0 ){
					j = nRowNum - 1;
				}else{
					j--;
				}
				/* ���̃E�B���h�E���A�N�e�B�u�ɂ��� */
				hwndWork = pEditNodeArr[j].m_hWnd;
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( hwndWork );
//				if( ::IsIconic( hwndWork ) ){
//					::ShowWindow( hwndWork, SW_RESTORE );
//				}else{
//					::ShowWindow( hwndWork, SW_SHOW );
//				}
//				::SetForegroundWindow( hwndWork );
//				::SetActiveWindow( hwndWork );
				/* �Ō�̃y�C�����A�N�e�B�u�ɂ��� */
				::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 1 );
				delete [] pEditNodeArr;
			}
		}
		return TRUE;
	case F_NEXTWINDOW:	//���̃E�B���h�E
		nPane = m_cSplitterWnd.GetNextPane();
		if( -1 != nPane ){
			SetActivePane( nPane );
		}else{
			/* ���݊J���Ă���ҏW���̃��X�g�𓾂� */
			nRowNum = CShareData::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
			if(  nRowNum > 0 ){
				/* �����̃E�B���h�E�𒲂ׂ� */
				for( i = 0; i < nRowNum; ++i ){
					j = 0;
					if( m_hwndParent == pEditNodeArr[i].m_hWnd ){
						j = i;
						break;
					}
				}
				if( j == nRowNum - 1 ){
					j = 0;
				}else{
					++j;
				}
				/* ���̃E�B���h�E���A�N�e�B�u�ɂ��� */
				hwndWork = pEditNodeArr[j].m_hWnd;
				/* �A�N�e�B�u�ɂ��� */
				ActivateFrameWindow( hwndWork );
//				if( ::IsIconic( hwndWork ) ){
//					::ShowWindow( hwndWork, SW_RESTORE );
//				}else{
//					::ShowWindow( hwndWork, SW_SHOW );
//				}
//				::SetForegroundWindow( hwndWork );
//				::SetActiveWindow( hwndWork );

				/* �ŏ��̃y�C�����A�N�e�B�u�ɂ��� */
				::PostMessage( hwndWork, MYWM_SETACTIVEPANE, (WPARAM)-1, 0 );
				delete [] pEditNodeArr;
			}
		}
		return TRUE;

	default:
		return m_cEditViewArr[m_nActivePaneIndex].HandleCommand( nCommand, TRUE, 0, 0, 0, 0 );
	}
}




/* �r���[�ɐݒ�ύX�𔽉f������ */
void CEditDoc::OnChangeSetting( void )
{
//	return;
	int			i;
	HWND		hwndProgress;

	CEditWnd*	pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );

	pCEditWnd->m_CFuncKeyWnd.m_nCurrentKeyState = -1;

	if( NULL != pCEditWnd ){
		hwndProgress = pCEditWnd->m_hwndProgressBar;
	}else{
		hwndProgress = NULL;
	}


	/* �t�@�C���̔r�����[�h�ύX */
	if( m_nFileShareModeOld != m_pShareData->m_Common.m_nFileShareMode ){
		/* �t�@�C���̔r�����b�N���� */
		DoFileUnLock();
		/* �t�@�C���̔r�����b�N */
		DoFileLock();
	}
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, false );

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	int		nX[4];
	int		nY[4];
	for( i = 0; i < 4; ++i ){
		m_cLayoutMgr.CaretPos_Log2Phys(
			m_cEditViewArr[i].m_nCaretPosX,
			m_cEditViewArr[i].m_nCaretPosY,
			&nX[i],
			&nY[i]
		);
	}

	/* ���C�A�E�g���̍쐬 */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* �p�����[�h���b�v������ */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* �s�R�����g�f���~�^ */
		ref.m_szLineComment2,		/* �s�R�����g�f���~�^2 */
		ref.m_szLineComment3,		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
		ref.m_szBlockCommentFrom,	/* �u���b�N�R�����g�f���~�^(From) */
		ref.m_szBlockCommentTo,		/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* �u���b�N�R�����g�f���~�^2(From) */
		ref.m_szBlockCommentTo2,	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
		ref.m_nStringType,			/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		TRUE,
		hwndProgress,
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
	); /* ���C�A�E�g���̕ύX */

	/* �r���[�ɐݒ�ύX�𔽉f������ */
	for( i = 0; i < 4; ++i ){
		m_cEditViewArr[i].OnChangeSetting();
		/*
		  �J�[�\���ʒu�ϊ�
		  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
		  ��
		  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
		*/
		int		nPosX;
		int		nPosY;
		m_cLayoutMgr.CaretPos_Phys2Log(
			nX[i],
			nY[i],
			&nPosX,
			&nPosY
		);
		if( nPosY >= m_cLayoutMgr.GetLineCount() ){
			/*�t�@�C���̍Ō�Ɉړ� */
//			m_cEditViewArr[i].Command_GOFILEEND(FALSE);
			m_cEditViewArr[i].HandleCommand( F_GOFILEEND, 0, 0, 0, 0, 0 );
		}else{
			m_cEditViewArr[i].MoveCursor( nPosX, nPosY, TRUE );
		}
	}
}




/* �ҏW�t�@�C�������i�[ */
void CEditDoc::SetFileInfo( FileInfo* pfi )
{
	int		nX;
	int		nY;

	strcpy( pfi->m_szPath, m_szFilePath );
	pfi->m_nViewTopLine = m_cEditViewArr[m_nActivePaneIndex].m_nViewTopLine;	/* �\����̈�ԏ�̍s(0�J�n) */
	pfi->m_nViewLeftCol = m_cEditViewArr[m_nActivePaneIndex].m_nViewLeftCol;	/* �\����̈�ԍ��̌�(0�J�n) */
	//	pfi->m_nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
	//	pfi->m_nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_cLayoutMgr.CaretPos_Log2Phys(
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX,	/* �r���[���[����̃J�[�\�����ʒu(�O�J�n) */
		m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY,	/* �r���[��[����̃J�[�\���s�ʒu(�O�J�n) */
		&nX,
		&nY
	);
	pfi->m_nX = nX;		/* �J�[�\�� �����ʒu(�s������̃o�C�g��) */
	pfi->m_nY = nY;		/* �J�[�\�� �����ʒu(�܂�Ԃ������s�ʒu) */


	pfi->m_bIsModified = IsModified() ? TRUE : FALSE;			/* �ύX�t���O */
	pfi->m_nCharCode = m_nCharCode;				/* �����R�[�h��� */
//	pfi->m_bPLSQL = m_cDlgJump.m_bPLSQL,		/* �s�W�����v�� PL/SQL���[�h�� */
//	pfi->m_nPLSQL_E1 = m_cDlgJump.m_nPLSQL_E1;	/* �s�W�����v�� PL/SQL���[�h�̂Ƃ��̊�_ */

	pfi->m_bIsGrep = m_bGrepMode;
	strcpy( pfi->m_szGrepKey, m_szGrepKey );
	return;

}


/* �^�O�W�����v���ȂǎQ�ƌ��̏���ێ����� */
void CEditDoc::SetReferer( HWND hwndReferer, int nRefererX, int nRefererLine )
{
	m_hwndReferer	= hwndReferer;	/* �Q�ƌ��E�B���h�E */
	m_nRefererX		= nRefererX;	/* �Q�ƌ�  �s������̃o�C�g�ʒu�� */
	m_nRefererLine	= nRefererLine;	/* �Q�ƌ��s  �܂�Ԃ������̕����s�ʒu */
	return;
}



/*! �t�@�C�������Ƃ���MRU�o�^ & �ۑ��m�F �� �ۑ����s

	@retval TRUE: �I�����ėǂ� / FALSE: �I�����Ȃ�
*/
BOOL CEditDoc::OnFileClose( void )
{
	int			nRet;
	int			nBool;
	FileInfo	fi;
	HWND		hwndMainFrame;
	hwndMainFrame = ::GetParent( m_hWnd );
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	CMRU		cMRU;

	/* MRU���X�g�̓o�^ */
	SetFileInfo( &fi );

	// 2002.01.16 hor �u�b�N�}�[�N�L�^
	strcpy( fi.m_szMarkLines, m_cDocLineMgr.GetBookMarks() );

	/* MRU���X�g�ւ̓o�^ */
//@@@ 2001.12.26 YAZAKI MRU���X�g�́ACMRU�Ɉ˗�����
	cMRU.Add( &fi );

	if( m_bGrepRunning ){		/* Grep������ */
		::MYMESSAGEBOX(
			hwndMainFrame,
			MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
			GSTR_APPNAME,
			"Grep�̏������ł��B\n"
		);
		return FALSE;
	}


	/* �e�L�X�g���ύX����Ă���ꍇ */
	if( IsModified()
	&& FALSE == m_bDebugMode	/* �f�o�b�O���j�^���[�h�̂Ƃ��͕ۑ��m�F���Ȃ� */
//	&& FALSE == m_bReadOnly		/* �ǂݎ���p���[�h */
	){
		if( TRUE == m_bGrepMode ){	/* Grep���[�h�̂Ƃ� */
			/* Grep���[�h�ŕۑ��m�F���邩 */
			if( FALSE == m_pShareData->m_Common.m_bGrepExitConfirm ){
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
				"%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H\n\n�ǂݎ���p�ŊJ���Ă���̂ŁA���O��t���ĕۑ�����΂����Ǝv���܂��B\n",
				lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j"
			);
			switch( nRet ){
			case IDYES:
//				if( 0 < lstrlen( m_szFilePath ) ){
//					nBool = HandleCommand( F_FILESAVE );
//				}else{
					nBool = HandleCommand( F_FILESAVEAS_DIALOG );
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
				"%s\n�͕ύX����Ă��܂��B ����O�ɕۑ����܂����H",
				lstrlen( m_szFilePath ) ? m_szFilePath : "�i����j"
			);
			switch( nRet ){
			case IDYES:
				if( 0 < lstrlen( m_szFilePath ) ){
					nBool = HandleCommand( F_FILESAVE );
				}else{
					nBool = HandleCommand( F_FILESAVEAS_DIALOG );
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

	m_bReadOnly = FALSE;	/* �ǂݎ���p���[�h */
	strcpy( m_szGrepKey, "" );
	m_bGrepMode = FALSE;	/* Grep���[�h */

//@@@ 2001.12.26 YAZAKI �傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
#ifdef _DEBUG
	hIconBig = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
	hIconSmall = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ), IMAGE_ICON, 16, 16, 0);
#else
	hIconBig = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
	hIconSmall = (HICON)LoadImage( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ), IMAGE_ICON, 16, 16, 0);
#endif
	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall );
	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)hIconBig );

//	HICON	hIcon;
//#ifdef _DEBUG
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_DEBUG ) );
//#else
//	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_STD ) );
//#endif
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)NULL );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_SMALL, (LPARAM)hIcon );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)NULL );
//	::SendMessage( m_hwndParent, WM_SETICON, ICON_BIG, (LPARAM)hIcon );

	/* �t�@�C���̔r�����b�N���� */
	DoFileUnLock();

	/* �t�@�C���̔r�����䃂�[�h */
	m_nFileShareModeOld = 0;


	/*�A���h�D�E���h�D�o�b�t�@�̃N���A */
	/* �S�v�f�̃N���A */
	m_cOpeBuf.ClearAll();

	/* �e�L�X�g�f�[�^�̃N���A */
	m_cDocLineMgr.Empty();
	m_cDocLineMgr.Init();

	/* ���ݕҏW���̃t�@�C���̃p�X */
	m_szFilePath[0] = '\0';

	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */
	m_FileTime.dwLowDateTime = 0;
	m_FileTime.dwHighDateTime = 0;


	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
	int doctype = CShareData::getInstance()->GetDocumentType( m_szFilePath );
	SetDocumentType( doctype, true );

	/* ���C�A�E�g�Ǘ����̏����� */
	//	m_cLayoutMgr.Create( &m_cDocLineMgr, GetDocumentAttribute().m_nMaxLineSize, GetDocumentAttribute().m_nTabSpace ) ;
	/* ���C�A�E�g���̕ύX */
	Types& ref = GetDocumentAttribute();
	m_cLayoutMgr.SetLayoutInfo(
		ref.m_nMaxLineSize,
		ref.m_bWordWrap,			/* �p�����[�h���b�v������ */
		ref.m_nTabSpace,
		ref.m_szLineComment,		/* �s�R�����g�f���~�^ */
		ref.m_szLineComment2,		/* �s�R�����g�f���~�^2 */
		ref.m_szLineComment3,		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
		ref.m_szBlockCommentFrom,	/* �u���b�N�R�����g�f���~�^(From) */
		ref.m_szBlockCommentTo,		/* �u���b�N�R�����g�f���~�^(To) */
//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
		ref.m_szBlockCommentFrom2,	/* �u���b�N�R�����g�f���~�^2(From) */
		ref.m_szBlockCommentTo2,	/* �u���b�N�R�����g�f���~�^2(To) */
//#endif
		ref.m_nStringType,			/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		TRUE,
		NULL,/*hwndProgress*/
		ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp,	/* �V���O���N�H�[�e�[�V�����������\������ */
		ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
	);


	/* �ύX�t���O */
	SetModified(false,false);	//	Jan. 22, 2002 genta

	/* �����R�[�h��� */
	m_nCharCode = 0;

	//	May 12, 2000
	m_cNewLineCode.SetType( EOL_CRLF );

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
		m_cEditViewArr[i].m_cHistory->Flush();

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		m_cEditViewArr[i].DisableSelectArea( FALSE );

		m_cEditViewArr[i].OnChangeSetting();
		m_cEditViewArr[i].MoveCursor( 0, 0, TRUE );
	}

	return;
}


/* �t�@�C���̃^�C���X�^���v�̃`�F�b�N���� */
void CEditDoc::CheckFileTimeStamp( void )
{
	HWND		hwndActive;
	BOOL		bUpdate;
	bUpdate = FALSE;
	if( m_pShareData->m_Common.m_bCheckFileTimeStamp	/* �X�V�̊Ď� */
	 && m_pShareData->m_Common.m_nFileShareMode == 0	/* �t�@�C���̔r�����䃂�[�h */
	 && NULL != ( hwndActive = ::GetActiveWindow() )	/* �A�N�e�B�u? */
	 && hwndActive == m_hwndParent
	 && 0 < lstrlen( m_szFilePath )
	 && ( m_FileTime.dwLowDateTime != 0 || m_FileTime.dwHighDateTime != 0 ) 	/* ���ݕҏW���̃t�@�C���̃^�C���X�^���v */

	){
		while( 1 ){
			/* �t�@�C���X�^���v���`�F�b�N���� */
//			MYTRACE( "�t�@�C���X�^���v���`�F�b�N����\n" );

			FILETIME	FileTimeNow;
			HFILE		hFile;
			BOOL		bWork;
			LONG		lWork;

			hFile = _lopen( m_szFilePath, OF_READ );
			if( HFILE_ERROR == hFile ){
				break;
			}
			bWork = ::GetFileTime( (HANDLE)hFile, NULL, NULL, &FileTimeNow );
			_lclose( hFile );
			if( 0 == bWork ){
				break;
			}
			lWork = ::CompareFileTime( &m_FileTime, &FileTimeNow );
			if( -1 == lWork ){
				bUpdate = TRUE;
//				MYTRACE( "���X�V����Ă��܂�����������������������\n" );
				m_FileTime = FileTimeNow;
			}
			break;
		}
	}

	if( !bUpdate ){
		return;
	}
	if( IDYES != MYMESSAGEBOX( m_hwndParent, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME,
		"%s\n\n���̃t�@�C���͊O���̃G�f�B�^���ŕύX����Ă��܂��B%s",
		m_szFilePath,
		(IsModified())?"\n�ă��[�h���s���ƕύX�������܂�����낵���ł���?":"�ă��[�h���܂���?"
	) ){
		return;
	}

	int		nCharCode;				/* �����R�[�h��� */
	BOOL	bReadOnly;				/* �ǂݎ���p���[�h */
	nCharCode = m_nCharCode;		/* �����R�[�h��� */
	bReadOnly = m_bReadOnly;		/* �ǂݎ���p���[�h */
	/* ����t�@�C���̍ăI�[�v�� */
	ReloadCurrentFile(
		nCharCode,		/* �����R�[�h��� */
		bReadOnly		/* �ǂݎ���p���[�h */
	);
	return;
}





/*! ����t�@�C���̍ăI�[�v�� */
void CEditDoc::ReloadCurrentFile(
	BOOL	nCharCode,		/*!< [in] �����R�[�h��� */
	BOOL	bReadOnly		/*!< [in] �ǂݎ���p���[�h */
)
{
	if( -1 == _access( m_szFilePath, 0 ) ){
		/* �t�@�C�������݂��Ȃ� */
		m_nCharCode = nCharCode;
		return;
	}


	BOOL	bOpened;
	char	szFilePath[MAX_PATH];
	int		nCaretPosX;
	int		nCaretPosY;
	nCaretPosX = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosX;
	nCaretPosY = m_cEditViewArr[m_nActivePaneIndex].m_nCaretPosY;

	strcpy( szFilePath, m_szFilePath );

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

	m_cEditViewArr[m_nActivePaneIndex].MoveCursor( nCaretPosX, nCaretPosY, TRUE );
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

	hIme = ImmGetContext( m_hwndParent );

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
	ImmReleaseContext( m_hwndParent, hIme );
}
//	To Here Nov. 20, 2000 genta


/*!	$x�̓W�J

*/
void CEditDoc::ExpandParameter(const char* pszSource, char* pszBuffer, int nBufferLen)
{
	const char *p, *r;	//	p�F�ړI�̃o�b�t�@�Br�F��Ɨp�̃|�C���^�B
	char *q, *q_max;
	for( p = pszSource, q = pszBuffer, q_max = pszBuffer + nBufferLen; *p != '\0' && q < q_max; ++p, ++q){
		if( *p != '$' ){
			*q = *p;
			continue;
		}
		switch( p[1] ){
		case '$':	//	 $$ -> $
			*q = *p;
			++p;
			break;
		case 'F':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�j
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(����)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				for( r = m_szFilePath; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q; // genta
				++p;
			}
			break;
		case 'f':	//	�J���Ă���t�@�C���̖��O�i�t�@�C�����̂݁j
			// Oct. 28, 2001 genta
			//	�t�@�C�����݂̂�n���o�[�W����
			//	�|�C���^�𖖔���
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(����)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				r = m_szFilePath + strlen( m_szFilePath );
				
				//	��납���؂��T��
				for( --r; r >= m_szFilePath && *r != '\\' ; --r )
					;
				//	\\�����������ꍇ�͂P�ڂ̏����ɂ���Đ擪�̂P�O�Ƀ|�C���^������B
				//	����\\�������ɂ����Ă����̌��ɂ�\0������̂ŃA�N�Z�X�ᔽ�ɂ͂Ȃ�Ȃ��B
				for( ++r ; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case '/':	//	�J���Ă���t�@�C���̖��O�i�t���p�X�B�p�X�̋�؂肪/�j
			// Oct. 28, 2001 genta
			if (m_szFilePath[0] == '\0'){
				memcpy(q, "(����)", 6);
				q += 6 - 1;
				++p;
			} 
			else {
				//	�p�X�̋�؂�Ƃ���'/'���g���o�[�W����
				for( r = m_szFilePath; *r != '\0' && q < q_max; ++r, ++q ){
					if( *r == '\\' )
						*q = '/';
					else
						*q = *r;
				}
				--q;
				++p;
			}
			break;
		//	From Here Jan. 15, 2002 hor
		case 'C':	//	���ݑI�𒆂̃e�L�X�g
			{
				CMemory cmemCurText;
				m_cEditViewArr[m_nActivePaneIndex].GetCurrentTextForSearch( cmemCurText );
				for( r = cmemCurText.GetPtr( NULL ); *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
		//	To Here Jan. 15, 2002 hor
			break;
		case 'd':	//	���ʐݒ�̓��t����
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetDateFormat( systime, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 't':	//	���ʐݒ�̎�������
			{
				char szText[1024];
				SYSTEMTIME systime;
				::GetLocalTime( &systime );
				CShareData::getInstance()->MyGetTimeFormat( systime, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 'p':	//	���݂̃y�[�W
			{
				CEditWnd* pcEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetCurPageNum() + 1, szText, 10);
					for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
				else {
					for ( r = "(����v���r���[�ł̂ݎg�p�ł��܂�)"; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
			}
			break;
		case 'P':	//	���y�[�W
			{
				CEditWnd* pcEditWnd = ( CEditWnd* )::GetWindowLong( m_hwndParent, GWL_USERDATA );
				if (pcEditWnd->m_pPrintPreview){
					char szText[1024];
					itoa(pcEditWnd->m_pPrintPreview->GetAllPageNum(), szText, 10);
					for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
				else {
					for ( r = "(����v���r���[�ł̂ݎg�p�ł��܂�)"; *r != '\0' && q < q_max; ++r, ++q )
						*q = *r;
					--q;
					++p;
				}
			}
			break;
		case 'D':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetDateFormat( systimeL, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			else {
				for ( r = "(�ۑ�����Ă��܂���)"; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		case 'T':	//	�^�C���X�^���v
			if (m_FileTime.dwLowDateTime){
				FILETIME	FileTime;
				SYSTEMTIME	systimeL;
				::FileTimeToLocalFileTime( &m_FileTime, &FileTime );
				::FileTimeToSystemTime( &FileTime, &systimeL );
				char szText[1024];
				CShareData::getInstance()->MyGetTimeFormat( systimeL, szText, sizeof( szText ) - 1 );
				for ( r = szText; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			else {
				for ( r = "(�ۑ�����Ă��܂���)"; *r != '\0' && q < q_max; ++r, ++q )
					*q = *r;
				--q;
				++p;
			}
			break;
		default:
			*q = *p;
			break;
		}
	}
	*q = '\0';
}
/*[EOF]*/
