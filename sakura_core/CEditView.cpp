//	$Id$
/*!	@file
	�����E�B���h�E�̊Ǘ�
	
	@author Norio Nakatani
	@date	1998/03/13 �쐬
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

//#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "debug.h"
#include "keycode.h"
#include "funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
//#include "_global_fio.h"
#include "etc_uty.h"
#include "CJre.h"
#include "global.h"
//#include "CDataObject.h"
#include "CAutoSave.h"


#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif




CEditView*	g_m_pcEditView;
LRESULT CALLBACK EditViewWndProc( HWND, UINT, WPARAM, LPARAM );
VOID CALLBACK EditViewTimerProc( HWND, UINT, UINT, DWORD );

#define IDT_ROLLMOUSE	1

/* ���\�[�X�w�b�_�[ */
#define	 BFT_BITMAP		0x4d42	  /* 'BM' */

/* ���\�[�X��DIB���ǂ����𔻒f����}�N�� */
#define	 ISDIB(bft)		((bft) == BFT_BITMAP)

/* �w�肳�ꂽ�l���ł��߂��o�C�g���E�ɐ��񂳂���}�N�� */
#define	 WIDTHBYTES(i)	((i+31)/32*4)

//DWORD DoGrepProc(
//	DWORD	dwGrepParam
//);

BOOL IsDataAvailable( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
//	STGMEDIUM	stgMedium;
	FORMATETC	fe;
	BOOL		bRes;
//�����`
//	fe.cfFormat = cfFormat;
//	fe.ptd = NULL;
//	fe.dwAspect = DVASPECT_CONTENT;
//	fe.lindex = -1;
//	fe.tymed = (TYMED)-1;
//	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );


/*
	TYMED_HGLOBAL	= 1,
	TYMED_FILE		= 2,
	TYMED_ISTREAM	= 4,
	TYMED_ISTORAGE	= 8,
	TYMED_GDI		= 16,
	TYMED_MFPICT	= 32,
	TYMED_ENHMF		= 64,
	TYMED_NULL		= 0
*/
//	MYTRACE( "=====================\n" );

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_FILE;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ISTREAM;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ISTORAGE;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_GDI;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_MFPICT;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_ENHMF;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}


	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_NULL;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );
	if( bRes ){
		return FALSE;
	}

	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)TYMED_HGLOBAL;
	bRes = SUCCEEDED( pDataObject->QueryGetData( &fe ) );
//	MYTRACE( "bRes= %d\n", bRes );

//	MYTRACE( "=====================\n" );
	return bRes;
}
HGLOBAL GetGlobalData( LPDATAOBJECT pDataObject, CLIPFORMAT cfFormat )
{
	FORMATETC fe;
	fe.cfFormat = cfFormat;
	fe.ptd = NULL;
	fe.dwAspect = DVASPECT_CONTENT;
	fe.lindex = -1;
	fe.tymed = (TYMED)-1;

	HGLOBAL hDest = NULL;
	STGMEDIUM stgMedium;
	if( SUCCEEDED( pDataObject->GetData( &fe, &stgMedium ) ) ){
		if( stgMedium.pUnkForRelease == NULL ){
			if( stgMedium.tymed == TYMED_HGLOBAL )
				hDest = stgMedium.hGlobal;
		}else{
			if( stgMedium.tymed == TYMED_HGLOBAL ){
				DWORD nSize = ::GlobalSize( stgMedium.hGlobal );
				hDest = ::GlobalAlloc( GMEM_SHARE|GMEM_MOVEABLE, nSize );
				if( hDest != NULL ){
					// copy the bits
					LPVOID lpSource = ::GlobalLock( stgMedium.hGlobal );
					LPVOID lpDest = ::GlobalLock( hDest );
					memcpy( lpDest, lpSource, nSize );
					::GlobalUnlock( hDest );
					::GlobalUnlock( stgMedium.hGlobal );
				}
			}
			::ReleaseStgMedium( &stgMedium );
		}
	}
	return hDest;
}


/*
|| �E�B���h�E�v���V�[�W��
||
*/
LRESULT CALLBACK EditViewWndProc(
	HWND		hwnd,	// handle of window
	UINT		uMsg,	// message identifier
	WPARAM		wParam,	// first message parameter
	LPARAM		lParam 	// second message parameter
)
{
	CEditView*	pCEdit;
	switch( uMsg ){
	case WM_CREATE:
		pCEdit = ( CEditView* )g_m_pcEditView;
		return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
	default:
		pCEdit = ( CEditView* )::GetWindowLong( hwnd, 0 );
		if( NULL != pCEdit ){
			//	May 16, 2000 genta
			//	From Here
			if( uMsg == WM_COMMAND ){
				::SendMessage( ::GetParent( pCEdit->m_hwndParent ), WM_COMMAND, wParam,  lParam );
			}
			else{
				return pCEdit->DispatchEvent( hwnd, uMsg, wParam, lParam );
			}
			//	To Here
		}
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}


/*
||  �^�C�}�[���b�Z�[�W�̃R�[���o�b�N�֐�
||
||	���݂́A�}�E�X�ɂ��̈�I�����̃X�N���[�������̂��߂Ƀ^�C�}�[���g�p���Ă��܂��B
*/
VOID CALLBACK EditViewTimerProc(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
)
{
	CEditView*	pCEditView;
	pCEditView = ( CEditView* )::GetWindowLong( hwnd, 0 );
	if( NULL != pCEditView ){
		pCEditView->OnTimer( hwnd, uMsg, idEvent, dwTime );
	}
	return;
}




CEditView::CEditView() : m_cHistory( new CAutoMarkMgr )
{
	TEXTMETRIC	tm;
	LOGFONT		lf;
	HDC			hdc;
	HFONT		hFontOld;
	int			i;

	m_CurSrch_CJre.Init();

	m_bDrawSWITCH = TRUE;
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	m_bDragMode = FALSE;					/* �I���e�L�X�g�̃h���b�O���� */
	m_bCurSrchKeyMark = FALSE;				/* ���������� */
	strcpy( m_szCurSrchKey, "" );			/**/
	m_bCurSrchRegularExp = 0;				/* �����^�u��  1==���K�\�� */
	m_bCurSrchLoHiCase = 0;					/* �����^�u��  1==�p�啶���������̋�� */
	m_bCurSrchWordOnly = 0;					/* �����^�u��  1==�P��̂݌��� */
//	m_CurSrch_CJre.Init();

	m_bExecutingKeyMacro = FALSE;			/* �L�[�{�[�h�}�N���̎��s�� */
	m_bPrevCommand = 0;
	m_nMyIndex = 0;


	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	/* LOGFONT�̏����� */
	memset( &lf, 0, sizeof( LOGFONT ) );
	lf.lfHeight			= -12;
	lf.lfWidth			= 0;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0x0;
	lf.lfUnderline		= 0x0;
	lf.lfStrikeOut		= 0x0;
	lf.lfCharSet		= 0x80;
	lf.lfOutPrecision	= 0x3;
	lf.lfClipPrecision	= 0x2;
	lf.lfQuality		= 0x1;
	lf.lfPitchAndFamily	= 0x31;
	strcpy( lf.lfFaceName, "�l�r �S�V�b�N" );
	m_hFontCaretPosInfo = ::CreateFontIndirect( &lf );

	hdc = ::GetDC( ::GetDesktopWindow() );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
	::GetTextMetrics( hdc, &tm );
	m_nCaretPosInfoCharWidth = tm.tmAveCharWidth;
	m_nCaretPosInfoCharHeight = tm.tmHeight;
	for( i = 0; i < ( sizeof( m_pnCaretPosInfoDx ) / sizeof( m_pnCaretPosInfoDx[0] ) ); ++i ){
		m_pnCaretPosInfoDx[i] = ( m_nCaretPosInfoCharWidth );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( ::GetDesktopWindow(), hdc );

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_cShareData.Init();
	m_pShareData = m_cShareData.GetShareData( NULL, NULL );
	m_bCommandRunning = FALSE;	/* �R�}���h�̎��s�� */
	m_pcOpeBlk = NULL;			/* ����u���b�N */
	m_bDoing_UndoRedo = FALSE;	/* �A���h�D�E���h�D�̎��s���� */
	m_pcsbwVSplitBox = NULL;	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = NULL;	/* ���������{�b�N�X */
	m_pszAppName = "EditorClient";
	m_hInstance = NULL;
	m_hWnd = NULL;
	m_hwndVScrollBar = NULL;
	m_nVScrollRate = 1;			/* �����X�N���[���o�[�̏k�� */
	m_hwndHScrollBar = NULL;
	m_hwndSizeBox = NULL;
	m_nCaretPosX = 0;			/* �r���[���[����̃J�[�\�����ʒu(�O�I���W��) */
	m_nCaretPosX_Prev = 0;		/* �r���[���[����̃J�[�\�������O�̈ʒu(�O�I���W��) */
	m_nCaretPosY = 0;			/* �r���[��[����̃J�[�\���s�ʒu(�O�I���W��) */

	m_nCaretPosX_PHY = 0;		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g��(�O�J�n) */
	m_nCaretPosY_PHY = 0;		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ�(�O�J�n) */


	m_nCaretWidth = 0;			/* �L�����b�g�̕� */
	m_nCaretHeight = 0;			/* �L�����b�g�̍��� */
	m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N */
	m_bBeginSelect = FALSE;		/* �͈͑I�� */
	m_bBeginBoxSelect = FALSE;	/* ��`�͈͑I�� */
	m_bBeginLineSelect = FALSE;	/* �s�P�ʑI�� */
	m_bBeginWordSelect = FALSE;	/* �P��P�ʑI�� */
//	m_nSelectLineBgn = 0;		/* �͈͑I���J�n�s(���_) */
//	m_nSelectColmBgn = 0;		/* �͈͑I���J�n��(���_) */
//	m_nSelectLineFrom = 0;		/* �͈͑I���J�n�s */
//	m_nSelectColmFrom = 0;		/* �͈͑I���J�n�� */
//	m_nSelectLineTo = 0;		/* �͈͑I���I���s */
//	m_nSelectColmTo = 0;		/* �͈͑I���I���� */

//	m_nSelectLineBgn = -1;		/* �͈͑I���J�n�s(���_) */
//	m_nSelectColmBgn = -1;		/* �͈͑I���J�n��(���_) */
	m_nSelectLineBgnFrom = -1;	/* �͈͑I���J�n�s(���_) */
	m_nSelectColmBgnFrom = -1;	/* �͈͑I���J�n��(���_) */
	m_nSelectLineBgnTo = -1;	/* �͈͑I���J�n�s(���_) */
	m_nSelectColmBgnTo = -1;	/* �͈͑I���J�n��(���_) */

	m_nSelectLineFrom = -1;		/* �͈͑I���J�n�s */
	m_nSelectColmFrom = -1;		/* �͈͑I���J�n�� */
	m_nSelectLineTo = -1;		/* �͈͑I���I���s */
	m_nSelectColmTo = -1;		/* �͈͑I���I���� */

	m_nSelectLineFromOld = 0;	/* �͈͑I���J�n�s */
	m_nSelectColmFromOld = 0;	/* �͈͑I���J�n�� */
	m_nSelectLineToOld = 0;		/* �͈͑I���I���s */
	m_nSelectColmToOld = 0;		/* �͈͑I���I���� */
	m_nViewAlignLeft = 0;		/* �\����̍��[���W */
	m_nViewAlignLeftCols = 0;	/* �s�ԍ���̌��� */
	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;		/* �\����̏�[���W */
	/* ���[���[�\�� */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
//	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ���[���[���� */
//	}
	m_nViewCx = 0;				/* �\����̕� */
	m_nViewCy = 0;				/* �\����̍��� */
	m_nViewColNum = 0;			/* �\����̌��� */
	m_nViewRowNum = 0;			/* �\����̍s�� */
	m_nViewTopLine = 0;			/* �\����̈�ԏ�̍s */
	m_nViewLeftCol = 0;			/* �\����̈�ԍ��̌� */
	m_hdcCompatDC = NULL;		/* �ĕ`��p�R���p�`�u���c�b */
	m_hbmpCompatBMP = NULL;		/* �ĕ`��p�������a�l�o */
	m_hbmpCompatBMPOld = NULL;	/* �ĕ`��p�������a�l�o(OLD) */
	m_nCharWidth = 10;			/* ���p�����̕� */
	m_nCharHeight = 18;			/* �����̍��� */
	/* �t�H���g�쐬 */
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* �����t�H���g�쐬 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );





	lf = m_pShareData->m_Common.m_lf;
	lf.lfCharSet = SHIFTJIS_CHARSET;
	lf.lfOutPrecision = 1;
	strcpy( lf.lfFaceName, "�l�r �S�V�b�N" );
	m_hFont_ZEN = CreateFontIndirect( &lf );
	m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	m_bInMenuLoop = FALSE;				/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
//	MYTRACE( "CEditView::CEditView()�����\n" );
	m_bHokan = FALSE;

	m_hFontOld = NULL;

	//	Aug. 31, 2000 genta
	m_cHistory->SetMax( 30 );

	return;
}


CEditView::~CEditView()
{
	DeleteObject( m_hFont_HAN );
	DeleteObject( m_hFont_HAN_FAT );
	DeleteObject( m_hFont_HAN_UL );
	DeleteObject( m_hFont_HAN_FAT_UL );
	DeleteObject( m_hFont_ZEN );

	if( m_hWnd != NULL ){
		DestroyWindow( m_hWnd );
	}

	/* �ĕ`��p�������a�l�o */
	if( m_hbmpCompatBMP != NULL ){
		/* �ĕ`��p�������a�l�o(OLD) */
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
	}
	/* �ĕ`��p�R���p�`�u���c�b */
	if( m_hdcCompatDC != NULL ){
		::DeleteDC( m_hdcCompatDC );
	}

	/* �L�����b�g�̍s���ʒu�\���p�t�H���g */
	::DeleteObject( m_hFontCaretPosInfo );

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;
	return;
}



BOOL CEditView::Create(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	CEditDoc*	pcEditDoc,
	int			nMyIndex,
	BOOL		bShow
)
{
	WNDCLASS	wc;
	SCROLLINFO	si;
	HDC			hdc;
	m_hInstance = hInstance;
	m_hwndParent = hwndParent;
	m_pcEditDoc = pcEditDoc;
	m_nMyIndex = nMyIndex;

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 	/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;								/* �\����̏�[���W */
	/* ���[���[�\�� */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ���[���[���� */
	}


	/* �E�B���h�E�N���X�̓o�^ */
	//	Apr. 27, 2000 genta
	//	�T�C�Y�ύX���̂������}���邽��CS_HREDRAW | CS_VREDRAW ���O����
	wc.style			= CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
	wc.lpfnWndProc		= (WNDPROC)EditViewWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof( LONG );
	wc.hInstance		= m_hInstance;
	wc.hIcon			= LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor			= NULL/*LoadCursor( NULL, IDC_IBEAM )*/;
	wc.hbrBackground	= (HBRUSH)NULL/*(COLOR_WINDOW + 1)*/;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= m_pszAppName;
	if( 0 == ::RegisterClass( &wc ) ){
	}
	/* �G�f�B�^�E�B���h�E�̍쐬 */
	g_m_pcEditView = this;
	m_hWnd = ::CreateWindowEx(
		0
		/*| WS_EX_CLIENTEDGE */
		| WS_EX_STATICEDGE
		,	// extended window style

		m_pszAppName,			// pointer to registered class name
		m_pszAppName,			// pointer to window name
		0
		| WS_VISIBLE
		| WS_CHILD
		| WS_CLIPCHILDREN
		/*| WS_BORDER*/
		, // window style

		CW_USEDEFAULT,			// horizontal position of window
		0,						// vertical position of window
		CW_USEDEFAULT,			// window width
		0,						// window height
		hwndParent,				// handle to parent or owner window
		NULL,					// handle to menu or child-window identifier
		m_hInstance,			// handle to application instance
		(LPVOID)this			// pointer to window-creation data
	);
	if( NULL == m_hWnd ){
		return FALSE;
	}

//	CDropTarget::Register_DropTarget( m_hWnd );
	m_pcDropTarget->Register_DropTarget( m_hWnd );
//	::OleInitialize(NULL);
//	if( FAILED( ::RegisterDragDrop( m_hWnd, this ) ) ){
//		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
//			"::RegisterDragDrop()\n���s"
//		);
//
//	}

	/* ����Tip�\���E�B���h�E�쐬 */
	m_cTipWnd.Create( m_hInstance, m_hWnd/*m_pShareData->m_hwndTray*/ );

	/* �ĕ`��p�R���p�`�u���c�b */
	hdc = ::GetDC( m_hWnd );
	m_hdcCompatDC = ::CreateCompatibleDC( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* ���������{�b�N�X */
	m_pcsbwVSplitBox = new CSplitBoxWnd;
	m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
	/* ���������{�b�N�X */
	m_pcsbwHSplitBox = new CSplitBoxWnd;
	m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );


	/* �X�N���[���o�[�̍쐬 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		"SCROLLBAR",						/* scroll bar control class */
		(LPSTR) NULL,						/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		m_hWnd,								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		m_hInstance,						/* instance owning this window */
		(LPVOID) NULL						/* pointer not needed */
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL;
	si.nMin  = 0;
	si.nMax  = 29;
	si.nPage = 10;
	si.nPos  = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* �X�N���[���o�[�̍쐬 */
	m_hwndHScrollBar = NULL;
	if( m_pShareData->m_Common.m_bScrollBarHorz ){	/* �����X�N���[���o�[���g�� */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd,								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;
		si.nMin  = 0;
		si.nMax  = 29;
		si.nPage = 10;
		si.nPos  = 0;
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
		::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );
	}


	/* �T�C�Y�{�b�N�X */
	if( m_pShareData->m_Common.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			"SCROLLBAR",						/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			"STATIC",							/* scroll bar control class */
			(LPSTR) NULL,						/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			m_hWnd, 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			m_hInstance,						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}

	SetFont();
	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	if( bShow ){
		ShowWindow( m_hWnd, SW_SHOW );
	}

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

//	/* ���͕⊮�E�B���h�E�쐬 */
//	m_cHokanMgr.DoModeless( m_hInstance , m_hWnd, (LPARAM)this );

	/* �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾 */
	int nKeyBoardSpeed;
	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 2;
	/* �^�C�}�[�N�� */
	if( 0 == ::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc ) ){
		::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
			"CEditView::Create()\n�^�C�}�[���N���ł��܂���B\n�V�X�e�����\�[�X���s�����Ă���̂�������܂���B"
		);
	}
	return TRUE;
}








/*
|| ���b�Z�[�W�f�B�X�p�b�`��
*/
LRESULT CEditView::DispatchEvent(
	HWND	hwnd,	// handle of window
	UINT	uMsg,	// message identifier
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{


	HDC			hdc;
	PAINTSTRUCT	ps;
//	int			nPos;
	int			nPosX;
	int			nPosY;
//	WORD		fwKeys;
//	short		zDelta;
//	short		xPos;
//	short		yPos;
//	int			i;
//	int			nScrollCode;
	switch ( uMsg ){
//-	case WM_DESTROYCLIPBOARD:
//-		MYTRACE( "CEditView WM_DESTROYCLIPBOARD\n" );
//-		return 0;
	case WM_MOUSEWHEEL:
		return OnMOUSEWHEEL( wParam, lParam );



	case WM_CREATE:
		::SetWindowLong( hwnd, 0, (LONG)this );

//		/* �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾 */
//		int	nKeyBoardSpeed;
//		SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//		/* �^�C�}�[�N�� */
//		::SetTimer( hwnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );

		return 0L;

	case WM_SIZE:
//		MYTRACE( "	WM_SIZE\n" );
		OnSize( LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;



	case WM_SETFOCUS:
//		MYTRACE( "	WM_SETFOCUS m_nMyIndex=%d\n", m_nMyIndex );
		OnSetFocus();

		/* �e�E�B���h�E�̃^�C�g�����X�V */
		SetParentCaption();

		return 0L;
	case WM_KILLFOCUS:
		OnKillFocus();

//		/* �e�E�B���h�E�̃^�C�g�����X�V */
//		SetParentCaption( TRUE );

		return 0L;
	case WM_CHAR:
//		MYTRACE( "WM_CHAR\n" );
		HandleCommand( F_CHAR, TRUE, wParam, 0, 0, 0 );
		return 0L;

	case WM_IME_COMPOSITION:
		if( m_pShareData->m_Common.m_bIsINSMode /* �}�����[�h���H */
		 &&	lParam & GCS_RESULTSTR
		){
			HIMC hIMC;
			DWORD dwSize;
			HGLOBAL hstr;
			LPSTR lpstr;
			hIMC = ImmGetContext( hwnd );

			if( !hIMC ){
				return 0;
//				MyError( ERROR_NULLCONTEXT );
			}

			// Get the size of the result string.
			dwSize = ImmGetCompositionString(hIMC, GCS_RESULTSTR, NULL, 0);

			// increase buffer size for NULL terminator,
			//   maybe it is in Unicode
			dwSize += sizeof( WCHAR );

			hstr = GlobalAlloc( GHND, dwSize );
			if( hstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALALLOC );
			}

			lpstr = (LPSTR)GlobalLock( hstr );
			if( lpstr == NULL ){
				return 0;
//				 MyError( ERROR_GLOBALLOCK );
			}

			// Get the result strings that is generated by IME into lpstr.
			ImmGetCompositionString(hIMC, GCS_RESULTSTR, lpstr, dwSize);

//			MYTRACE( "lpstr=[%s]", lpstr );
			/* �e�L�X�g��\��t�� */
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpstr, TRUE, 0, 0 );

			ImmReleaseContext( hwnd, hIMC );

			// add this string into text buffer of application

			GlobalUnlock( hstr );
			GlobalFree( hstr );
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
//			return 0;
		}else{
			return DefWindowProc( hwnd, uMsg, wParam, lParam );
		}
	case WM_IME_CHAR:
		if( FALSE == m_pShareData->m_Common.m_bIsINSMode ){ /* �㏑�����[�h���H */
			HandleCommand( F_IME_CHAR, TRUE, (WORD)( (((WORD)wParam&0x00ff)<<8) | (((WORD)wParam&0xff00)>>8) ), 0, 0, 0 );
		}
		return 0L;

//		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
//			return 1L;
//		}
//		/* �R�}���h�̎��s�� */
//		m_bCommandRunning = TRUE;
//
//		if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
////			while( NULL != m_pcOpeBlk ){}
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = new COpeBlk;
//
//		Command_IME_CHAR( (WORD)( (((WORD)wParam&0x00ff)<<8) | (((WORD)wParam&0xff00)>>8) ) ); /* �Q�o�C�g�������� */
//
//		/* �A���h�D�o�b�t�@�̏��� */
//		if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
//			/* ����̒ǉ� */
//			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
//		}else{
//			delete m_pcOpeBlk;
//		}
//		m_pcOpeBlk = NULL;
//		m_bCommandRunning = FALSE;
//        return 0L;

	case WM_KEYUP:
		/* �L�[���s�[�g��� */
		m_bPrevCommand = 0;
        return 0L;


//	case WM_MBUTTONDBLCLK:
	case WM_LBUTTONDBLCLK:


//		MYTRACE( " WM_LBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDBLCLK( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

//	case WM_MBUTTONDOWN:
	case WM_LBUTTONDOWN:
//	case WM_RBUTTONDOWN:
		::SetFocus( ::GetParent( m_hwndParent ) );

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* �A�N�e�B�u�ȃy�C����ݒ� */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
		}
//		MYTRACE( " WM_LBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
//	case WM_MBUTTONUP:
	case WM_LBUTTONUP:

//		MYTRACE( " WM_LBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnLBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;
	case WM_MOUSEMOVE:
		OnMOUSEMOVE( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_RBUTTONDBLCLK:
		::SetFocus( ::GetParent( m_hwndParent ) );
//		MYTRACE( " WM_RBUTTONDBLCLK wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		return 0L;
//	case WM_RBUTTONDOWN:
//		MYTRACE( " WM_RBUTTONDOWN wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
//		OnRBUTTONDOWN( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
//		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
//			/* �A�N�e�B�u�ȃy�C����ݒ� */
//			m_pcEditDoc->SetActivePane( m_nMyIndex );
//		}
//		return 0L;
	case WM_RBUTTONUP:
//		MYTRACE( " WM_RBUTTONUP wParam=%08xh, x=%d y=%d\n", wParam, LOWORD( lParam ), HIWORD( lParam ) );
		OnRBUTTONUP( wParam, (short)LOWORD( lParam ), (short)HIWORD( lParam ) );
		return 0L;

	case WM_VSCROLL:
		::SetFocus( ::GetParent( m_hwndParent ) );

//		MYTRACE( "	WM_VSCROLL nPos=%d\n", GetScrollPos( m_hwndVScrollBar, SB_CTL ) );
		OnVScroll(
			(int) LOWORD( wParam ),
			((int) HIWORD( wParam )) * m_nVScrollRate,
			(HWND) lParam
		);

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* �A�N�e�B�u�ȃy�C����ݒ� */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
		}
		return 0L;

	case WM_HSCROLL:
		::SetFocus( ::GetParent( m_hwndParent ) );
//		MYTRACE( "	WM_HSCROLL nPos=%d\n", GetScrollPos( m_hwndHScrollBar, SB_CTL ) );
		OnHScroll(
			(int) LOWORD( wParam ),
			((int) HIWORD( wParam )),
			(HWND) lParam
		);

		if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
			/* �A�N�e�B�u�ȃy�C����ݒ� */
			m_pcEditDoc->SetActivePane( m_nMyIndex );
		}
		return 0L;

	case WM_ENTERMENULOOP:
		m_bInMenuLoop = TRUE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */

		/* ����Tip���N������Ă��� */
		if( 0 == m_dwTipTimer ){
			/* ����Tip������ */
			m_cTipWnd.Hide();
			m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
		}
		if( m_bHokan ){
			m_pcEditDoc->m_cHokanMgr.Hide();
			m_bHokan = FALSE;
		}
		return 0L;

	case WM_EXITMENULOOP:
		m_bInMenuLoop = FALSE;	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��܂� */
		return 0L;


    case WM_PAINT:
		hdc = BeginPaint( hwnd, &ps );
		OnPaint( hdc, &ps, FALSE );
        EndPaint(hwnd, &ps);
        return 0L;

	case WM_CLOSE:
//		MYTRACE( "	WM_CLOSE\n" );
		DestroyWindow( hwnd );
		return 0L;
	case WM_DESTROY:
//		CDropTarget::Revoke_DropTarget();
		m_pcDropTarget->Revoke_DropTarget();
//		::RevokeDragDrop( m_hWnd );
//		::OleUninitialize();

		/* �^�C�}�[�I�� */
		::KillTimer( m_hWnd, IDT_ROLLMOUSE );


//		MYTRACE( "	WM_DESTROY\n" );
		/*
		||�q�E�B���h�E�̔j��
		*/
		if( NULL != m_hwndHScrollBar ){
			DestroyWindow( m_hwndVScrollBar );
			m_hwndVScrollBar = NULL;
		}
		if( NULL != m_hwndHScrollBar ){
			DestroyWindow( m_hwndHScrollBar );
			m_hwndHScrollBar = NULL;
		}
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		if( NULL != m_pcsbwVSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}

		if( NULL != m_pcsbwHSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}


		m_hWnd = NULL;
		return 0L;

	case MYWM_DOSPLIT:
		nPosX = (int)wParam;
		nPosY = (int)lParam;
//		MYTRACE( "MYWM_DOSPLIT nPosX=%d nPosY=%d\n", nPosX, nPosY );
		::SendMessage( m_hwndParent, MYWM_DOSPLIT, wParam, lParam );
		return 0L;

	case MYWM_SETACTIVEPANE:
		m_pcEditDoc->SetActivePane( m_nMyIndex );
		::PostMessage( m_hwndParent, MYWM_SETACTIVEPANE, (WPARAM)m_nMyIndex, 0 );
		return 0L;


	default:
		return DefWindowProc( hwnd, uMsg, wParam, lParam );
	}
}




void CEditView::OnMove( int x, int y, int nWidth, int nHeight )
{
	MoveWindow( m_hWnd, x, y, nWidth, nHeight, TRUE );
	return;
}




/* �E�B���h�E�T�C�Y�̕ύX���� */
void CEditView::OnSize( int cx, int cy )
{
	if( NULL == m_hWnd ){
		return;
	}
	if( cx == 0 && cy == 0 ){
		return;
	}
//#ifdef _DEBUG
//	/* �f�o�b�O���j�^�ɏo�� */
//	m_cShareData.TraceOut( "%s(%d): CEditView::OnSize( int cx, int cy ); cx=%d cy=%d\n", __FILE__, __LINE__, cx, cy );
//#endif

	int	nCxHScroll;
	int	nCyHScroll;
	int	nCxVScroll;
	int	nCyVScroll;
	int	nVSplitHeight;	/* ���������{�b�N�X�̍��� */
	int	nHSplitWidth;	/* ���������{�b�N�X�̕� */


	nVSplitHeight = 0;	/* ���������{�b�N�X�̍��� */
	nHSplitWidth = 0;	/* ���������{�b�N�X�̕� */

	HDC	hdc;
	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );


	hdc = ::GetDC( m_hWnd );

	/* ���������{�b�N�X */
	if( NULL != m_pcsbwVSplitBox ){
		nVSplitHeight = 7;
		::MoveWindow( m_pcsbwVSplitBox->m_hWnd, cx - nCxVScroll , 0, nCxVScroll, nVSplitHeight, TRUE );
	}
	/* ���������{�b�N�X */
	if( NULL != m_pcsbwHSplitBox ){
		nHSplitWidth = 7;
		::MoveWindow( m_pcsbwHSplitBox->m_hWnd,0, cy - nCyHScroll, nHSplitWidth, nCyHScroll, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll , 0 + nVSplitHeight, nCxVScroll, cy - nCyVScroll - nVSplitHeight, TRUE );
	}
	/* �����X�N���[���o�[ */
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0 + nHSplitWidth, cy - nCyHScroll, cx - nCxVScroll - nHSplitWidth, nCyHScroll, TRUE );
	}

	/* �T�C�Y�{�b�N�X */
	if( NULL != m_hwndSizeBox ){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}else{
	}


	m_nViewCx = cx - nCxVScroll - m_nViewAlignLeft;														/* �\����̕� */
	m_nViewCy = cy - ((NULL != m_hwndHScrollBar)?nCyHScroll:0) - m_nViewAlignTop;						/* �\����̍��� */
	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* �\����̌��� */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* �\����̍s�� */

//1999.12.1 �d�l�ύX
//	/* �E�B���h�E���T�C�Y���ɃL�����b�g�ʒu�փX�N���[�� */
//	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �ĕ`��p�������a�l�o */
	if( m_hbmpCompatBMP != NULL ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );	/* �ĕ`��p�������a�l�o(OLD) */
		::DeleteObject( m_hbmpCompatBMP );
	}
	m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, cx, cy );
	m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );
	::ReleaseDC( m_hWnd, hdc );

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();


//	/* ���݂̃E�B���h�E���Ő܂�Ԃ�	*/
//	Command_WRAPWINDIWWIDTH();






	/* �E�B���h�E�T�C�Y�ɐ܂�Ԃ���������Ǐ]�����郂�[�h */
//
//	if( 10 > m_nViewColNum - 1 ){
//		::MessageBeep( MB_ICONHAND );
//		return;
//	}
//	m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize = m_nViewColNum - 1;
//
//	m_pcEditDoc->OnChangeSetting();	/* �r���[�ɐݒ�ύX�𔽉f������ */
//
//	/* �ݒ�ύX�𔽉f������ */
//	m_cShareData.SendMessageToAllEditors(
//		MYWM_CHANGESETTING, (WPARAM)0, (LPARAM)0, ::GetParent( m_hwndParent )
//	);	/* �S�ҏW�E�B���h�E�փ��b�Z�[�W���|�X�g���� */
//
//	m_nViewLeftCol = 0;		/* �\����̈�ԍ��̌�(0�J�n) */
//
//	/* �t�H�[�J�X�ړ����̍ĕ`�� */
//	RedrawAll();
//	return;

	return;
}


/* �L�����b�g�̕\���E�X�V */
void CEditView::ShowEditCaret( void )
{
	const char*		pLine;
	int				nLineLen;
	int				nCaretWidth;
	int				nCaretHeight;
	int				nIdxFrom;
	int				nCharChars;
	HDC				hdc;
	const CLayout*	pcLayout;
//	HPEN			hPen, hPenOld;


//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "ShowEditCaret() m_nMyIndex=%d m_nCaretWidth=%d\n", m_nMyIndex, m_nCaretWidth );
//}

/*
	   �Ȃ񂩃t���[���E�B���h�E���A�N�e�B�u�łȂ��Ƃ��ɓ����I�ɃJ�[�\���ړ������
	   �J�[�\�����Ȃ��̂ɁA�J�[�\��������Ƃ������ƂɂȂ��Ă��܂�
	   �̂ŃA�N�e�B�u�ɂ��Ă��J�[�\�����o�Ă��Ȃ��Ƃ�������
	   �t���[���E�B���h�E���A�N�e�B�u�łȂ��Ƃ��́A�J�[�\�����Ȃ����Ƃɂ���
*/
	if( ::GetActiveWindow() != ::GetParent( m_hwndParent ) ){
		m_nCaretWidth = 0;
//		MYTRACE( "�A�N�e�B�u�łȂ��̂ɃJ�[�\���������������������������B\n" );
		return;
	}

	/* �A�N�e�B�u�ȃy�C�����擾 */
	if( m_nMyIndex != m_pcEditDoc->GetActivePane() ){
		m_nCaretWidth = 0;
//if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//	MYTRACE( "m_nMyIndex[%s] != m_pcEditDoc->GetActivePane()\n", m_nMyIndex, m_pcEditDoc->GetActivePane() );
//}
		return;
	}
	/* �L�����b�g�̕��A���������� */
	if( 0 == m_pShareData->m_Common.GetCaretType() ){	/* �J�[�\���̃^�C�v 0=win 1=dos */
		nCaretHeight = m_nCharHeight;					/* �L�����b�g�̍��� */
		if( m_pShareData->m_Common.m_bIsINSMode ){
			nCaretWidth = 2;
		}else{
			nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
				if( nIdxFrom >= nLineLen ||
					pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
					pLine[nIdxFrom] == TAB ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				}else{
					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxFrom] ) - &pLine[nIdxFrom];
					if( 0 < nCharChars ){
						nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
					}
				}
			}
		}
	}else
	if( 1 == m_pShareData->m_Common.GetCaretType() ){	/* �J�[�\���̃^�C�v 0=win 1=dos */
		if( m_pShareData->m_Common.m_bIsINSMode ){
			nCaretHeight = m_nCharHeight / 2;			/* �L�����b�g�̍��� */
		}else{
			nCaretHeight = m_nCharHeight;				/* �L�����b�g�̍��� */
		}
		nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ||
				pLine[nIdxFrom] == CR || pLine[nIdxFrom] == LF ||
				pLine[nIdxFrom] == TAB ){
				nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nIdxFrom] ) - &pLine[nIdxFrom];
				if( 0 < nCharChars ){
					nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * nCharChars;
				}
			}
		}

	}
//	int		nDummy;
//	if( m_nMyIndex == 0 ){
//		nDummy = 0;
//	}else
//	if( m_nMyIndex == 1 ){
//		nDummy = 1;
//	}else
//	if( m_nMyIndex == 2 ){
//		nDummy = 2;
//	}else
//	if( m_nMyIndex == 3 ){
//		nDummy = 3;
//	}

	hdc = ::GetDC( m_hWnd );
	if( m_nCaretWidth == 0 ){	/* �L�����b�g���Ȃ������ꍇ */

//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//			/* �J�[�\���s�A���_�[���C���̕`�� */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//		}

		/* �L�����b�g�̍쐬 */
		::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
		/* �L�����b�g�̈ʒu�𒲐� */
		::SetCaretPos(
			m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
			m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
		);
		/* �L�����b�g�̕\�� */
		::ShowCaret( m_hWnd );
	}else{
		if( m_nCaretWidth != nCaretWidth || m_nCaretHeight != nCaretHeight ){	/* �L�����b�g�͂��邪�A�傫�����ς�����ꍇ */


			/* ���݂̃L�����b�g���폜 */
			::DestroyCaret();

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && -1 != m_nOldUnderLineY ){
//				/* �J�[�\���s�A���_�[���C���̏��� */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//				m_nOldUnderLineY = -1;
//			}

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//				/* �J�[�\���s�A���_�[���C���̕`�� */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//			}

			/* �L�����b�g�̍쐬 */
			::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
			/* �L�����b�g�̈ʒu�𒲐� */
			::SetCaretPos(
				m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
				m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
			);
			/* �L�����b�g�̕\�� */
			::ShowCaret( m_hWnd );
		}else{
			/* �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ */
			/* �L�����b�g���B�� */
			::HideCaret( m_hWnd );

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && -1 != m_nOldUnderLineY ){
//				/* �J�[�\���s�A���_�[���C���̏��� */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//				m_nOldUnderLineY = -1;
//			}

//			if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelecting() ){
//				/* �J�[�\���s�A���_�[���C���̕`�� */
//				hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//				hPenOld = (HPEN)::SelectObject( hdc, hPen );
//				m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//				::MoveToEx(
//					hdc,
//					m_nViewAlignLeft,
//					m_nOldUnderLineY,
//					NULL
//				);
//				::LineTo(
//					hdc,
//					m_nViewCx + m_nViewAlignLeft,
//					m_nOldUnderLineY
//				);
//				::SelectObject( hdc, hPenOld );
//				::DeleteObject( hPen );
//			}

			/* �L�����b�g�̈ʒu�𒲐� */
			::SetCaretPos(
				m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),
				m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight
			);
			/* �L�����b�g�̕\�� */
			::ShowCaret( m_hWnd );
		}
	}
	m_nCaretWidth = nCaretWidth;
	m_nCaretHeight = nCaretHeight;	/* �L�����b�g�̍��� */
	SetIMECompFormPos();

	/* ���[���[�`�� */
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );


	return;
}





/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
void CEditView::OnSetFocus( void )
{
//NG	/* 1999.11.15 */
//NG	::SetFocus( m_hwndParent );
//NG	::SetFocus( m_hWnd );


	ShowEditCaret();
	SetIMECompFormPos();
	SetIMECompFormFont();

	return;
}





/* ���̓t�H�[�J�X���������Ƃ��̏��� */
void CEditView::OnKillFocus( void )
{
//	if( m_nMyIndex == 0 && m_nCaretPosX == 0 && m_nCaretPosY == 0 ){
//		MYTRACE( "OnKillFocus()\n" );
//	}
	HDC	hdc;


	::DestroyCaret();
	m_nCaretWidth = 0;

	/* ���[���[�`�� */
	hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}


	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

//	if( m_bHokan ){
//		m_pcEditDoc->m_cHokanMgr.Hide();
//		m_bHokan = FALSE;
//	}

	return;
}





/* �����X�N���[���o�[���b�Z�[�W���� */
void CEditView::OnVScroll( int nScrollCode, int nPos, HWND hwndScrollBar )
{
//	int		i;
	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine + 1 );
//		}
		ScrollAtV( m_nViewTopLine + m_pShareData->m_Common.m_nRepeatedScrollLineNum );

		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( m_nViewTopLine - 1 );
//		}
		ScrollAtV( m_nViewTopLine - m_pShareData->m_Common.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		ScrollAtV( m_nViewTopLine + m_nViewRowNum );
		break;
	case SB_PAGEUP:
		ScrollAtV( m_nViewTopLine - m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		ScrollAtV( nPos );
		break;
	case SB_THUMBTRACK:
		ScrollAtV( nPos );
		break;
	default:
		break;
	}
	return;
}




/* �����X�N���[���o�[���b�Z�[�W���� */
void CEditView::OnHScroll( int nScrollCode, int nPos, HWND hwndScrollBar )
{
//	int		i;
	switch( nScrollCode ){
	case SB_LINELEFT:
//		for( i = 0; i < 2; ++i ){
			ScrollAtH( m_nViewLeftCol - 4 );
//		}
		break;
	case SB_LINERIGHT:
//		for( i = 0; i < 2; ++i ){
			ScrollAtH( m_nViewLeftCol + 4 );
//		}
		break;
	case SB_PAGELEFT:
		ScrollAtH( m_nViewLeftCol - m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		ScrollAtH( m_nViewLeftCol + m_nViewColNum );
		break;
	case SB_THUMBPOSITION:
		ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	case SB_THUMBTRACK:
		ScrollAtH( nPos );
//		MYTRACE( "nPos=%d\n", nPos );
		break;
	}
	return;
}



//void CEditView::Draw3dRect( HDC hdc, int x, int y, int cx, int cy,
//	COLORREF clrTopLeft, COLORREF clrBottomRight )
//{
//	return;
//}
//
//void CEditView::FillSolidRect( HDC hdc, int x, int y, int cx, int cy, COLORREF clr )
//{
//	RECT	rc;
//	::SetBkColor( hdc, clr );
//	::SetRect( &rc, x, y, x + cx, y + cy );
//	::ExtTextOut( hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL );
//	return;
//}

#if 0//////////////////////////////////////////////
/************************************************************************
 *
 * 	�֐�:	OpenDIB( LPSTR szFile )
 *
 * 	�ړI:	DIB�t�@�C�����J���ă�����DIB���쐬���܂��B�܂��ABITMAPINFO�A
 *	�p���b�g�f�[�^�A�r�b�g���܂ރ������n���h�����쐬���܂��B
 *
 *	�߂�l:	DIB�����ʂ���n���h��
 *
 ************************************************************************/
HANDLE CEditView::OpenDIB ( LPCSTR szFile )
{
	unsigned			fh;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	DWORD				dwLen = 0;
	DWORD				dwBits;
	HANDLE				hdib;
	HANDLE				h;
	OFSTRUCT			of;

	/* �t�@�C�����J����DIB����ǂݎ�� */
	fh = OpenFile( szFile, &of, OF_READ );
	if( fh == -1 )
		return NULL;

	hdib = ReadDibBitmapInfo( fh );
	if( !hdib )
		return NULL;
	DibInfo( hdib, &bi );

	/* DIB�̕ێ��ɕK�v�ȃ������ʂ��v�Z */
	dwBits = bi.biSizeImage;
	dwLen  = bi.biSize + (DWORD)PaletteSize( &bi ) + dwBits;

	/* DIB��ێ�����r�b�g�}�b�v���o�b�t�@�T�C�Y�𑝂₷ */
	h = GlobalReAlloc( hdib, dwLen, GHND );
	if( !h ){
		GlobalFree( hdib );
		hdib = NULL;
	}
	else
		hdib = h;
	/* �r�b�g��ǂݎ�� */
	if( hdib ){
		lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
		lread( fh, (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize(lpbi), dwBits );
		GlobalUnlock( hdib );
	}
	_lclose( fh );
	return hdib;
}

/************************************************************************
 *
 *	�֐�:	ReadDibBitmapInfo( int fh )
 *
 *	�ړI:	DIB�`���̃t�@�C����ǂݎ��A����BITMAPINFO��
 *	�O���[�o���n���h����Ԃ��܂��B
 *	���̊֐��͈ȑO��(BITMAPCOREHEADER)�`���ƐV����(BITMAPINFOHEADER)�`����
 * 	�����������ł��܂����A�Ԃ��̂͂˂ɐV����BITMAPINFO�ł��B
 *
 *	 �߂�l: �t�@�C������DIB��BITMAPINFO�����ʂ���n���h��
 *
 ************************************************************************/
HANDLE CEditView::ReadDibBitmapInfo ( int fh )
{
	DWORD				off;
	HANDLE				hbi = NULL;
	int					size;
	int					i;
	WORD				nNumColors;

	RGBQUAD FAR			*pRgb;
	BITMAPINFOHEADER	bi;
	BITMAPCOREHEADER	bc;
	LPBITMAPINFOHEADER	lpbi;
	BITMAPFILEHEADER	bf;
	DWORD				dwWidth = 0;
	DWORD				dwHeight = 0;
	WORD				wPlanes, wBitCount;

	if( fh == -1 )
		return NULL;

	/* �t�@�C���|�C���^�����Z�b�g���A�t�@�C���w�b�_�[��ǂݎ�� */
	off = _llseek( fh, 0L, SEEK_CUR );
	if( sizeof( bf ) != _lread( fh, (LPSTR)&bf, sizeof( bf ) ) )
		return FALSE;

	/* RC�w�b�_�[�����邩���ׂ� */
	if( !ISDIB( bf.bfType ) ){
		bf.bfOffBits = 0L;
		_llseek( fh, off, SEEK_SET );
	}
	if( sizeof( bi ) != _lread( fh, (LPSTR)&bi, sizeof( bi ) ) )
		return FALSE;

	nNumColors = DibNumColors( &bi );

	/*
	 * ���u���b�N�̓��e(BITMAPINFO�܂���BITMAPCORE)���`�F�b�N���A
	 * ����ɏ]���ď����擾����BBITMAPCOREHEADER�Ȃ�΁A����
	 * BITMAPINFOHEADER�`���̃u���b�N�ɓ]������B
	 */
	switch ( size = (int)bi.biSize ){
	  case sizeof( BITMAPINFOHEADER ):
		break;
	  case  sizeof( BITMAPCOREHEADER ):

		bc = *(BITMAPCOREHEADER*)&bi;

		dwWidth		= (DWORD)bc.bcWidth;
		dwHeight	= (DWORD)bc.bcHeight;
		wPlanes		= bc.bcPlanes;
		wBitCount	= bc.bcBitCount;

		bi.biSize		= sizeof( BITMAPINFOHEADER );
		bi.biWidth		= dwWidth;
		bi.biHeight		= dwHeight;
		bi.biPlanes		= wPlanes;
		bi.biBitCount	= wBitCount;

		bi.biCompression	= BI_RGB;
		bi.biSizeImage	 	= 0;
		bi.biXPelsPerMeter	= 0;
		bi.biYPelsPerMeter	= 0;
		bi.biClrUsed		= nNumColors;
		bi.biClrImportant       = nNumColors;
		_llseek( fh, (LONG)sizeof( BITMAPCOREHEADER ) - sizeof( BITMAPINFOHEADER ), SEEK_CUR );
		break;
	  default:
		/* DIB�ł͂Ȃ� */
		return NULL;
	}
	/* 0�Ȃ�΃f�t�H���g�l��ݒ� */
	if( bi.biSizeImage == 0 ){
		bi.biSizeImage = WIDTHBYTES( (DWORD)bi.biWidth * bi.biBitCount ) * bi.biHeight;
	}
	if( bi.biClrUsed == 0 )
		bi.biClrUsed = DibNumColors(&bi);
	/* BITMAPINFO�\���̂ƃJ���[�e�[�u�������蓖�Ă� */
	hbi = GlobalAlloc( GHND, (LONG)bi.biSize + nNumColors * sizeof( RGBQUAD ) );
	if( !hbi )
		return NULL;
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hbi );
	*lpbi = bi;
	/* �J���[�e�[�u�����w���|�C���^���擾 */
	pRgb = (RGBQUAD FAR *)( (LPSTR)lpbi + bi.biSize );
	if (nNumColors){
		if( size == sizeof( BITMAPCOREHEADER ) ){
			/*
			 * �Â��J���[�e�[�u��(3�o�C�g��RGBTRIPLE)��V�����J���[�e�[�u��(4�o�C�g��RGBQUAD)�ɕϊ�
			 */
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBTRIPLE ) );
			for( i = nNumColors - 1; i >= 0; i-- ){
				RGBQUAD rgb;
				rgb.rgbRed		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
				rgb.rgbBlue		= ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
				rgb.rgbGreen	= ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
				rgb.rgbReserved = (BYTE)0;
				pRgb[i]			= rgb;
			}
		}
		else
			_lread( fh, (LPSTR)pRgb, nNumColors * sizeof( RGBQUAD ) );
	}
	if( bf.bfOffBits != 0L )
		_llseek( fh, off + bf.bfOffBits, SEEK_SET );
	GlobalUnlock( hbi );
	return hbi;
}


/************************************************************************
 *
 *	�֐�:	DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
 *
 *	�ړI:	CF_DIB�`���̃������u���b�N�Ɋ֘A�t�����Ă���DIB�����擾���܂��B
 *
 *  �߂�l:	TRUE	- ����ɏI�������ꍇ
 *			FALSE	- ����ȊO�̏ꍇ
 *
 ************************************************************************/
BOOL CEditView::DibInfo( HANDLE hbi, LPBITMAPINFOHEADER lpbi )
{
	if( hbi ){
		*lpbi = *(LPBITMAPINFOHEADER)GlobalLock( hbi );

		/* �f�t�H���g�̃����o�ݒ� */
		if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
			if( lpbi->biSizeImage == 0L )
				lpbi->biSizeImage =
					WIDTHBYTES( lpbi->biWidth*lpbi->biBitCount ) * lpbi->biHeight;

			if( lpbi->biClrUsed == 0L )
				lpbi->biClrUsed = DibNumColors( lpbi );
		}
		GlobalUnlock( hbi );
		return TRUE;
	}
	return FALSE;
}

/********************************************************************************
 *
 *	�֐�:	PaletteSize( VOID FAR * pv )
 *
 *	�ړI:	�p���b�g�̃o�C�g�����v�Z���܂��B���u���b�N��BITMAPCOREHEADER�^
 *			�Ȃ�΁A�F����3�{���p���b�g�T�C�Y�ɂȂ�܂��B����ȊO�̏ꍇ�́A
 *			�F����4�{���p���b�g�T�C�Y�ɂȂ�܂��B
 *
 *	�߂�l:	�p���b�g�̃o�C�g��
 *
 *******************************************************************************/
WORD CEditView::PaletteSize ( VOID FAR * pv )
{
	LPBITMAPINFOHEADER lpbi;
	WORD		NumColors;
	lpbi		= (LPBITMAPINFOHEADER)pv;
	NumColors	= DibNumColors(lpbi);

	if( lpbi->biSize == sizeof( BITMAPCOREHEADER ) )
		return NumColors * sizeof( RGBTRIPLE );
	else
		return NumColors * sizeof( RGBQUAD );
}





/********************************************************************************
 *
 *	�֐�:	DibNumColors( VOID FAR * pv )
 *
 *	�ړI:	���u���b�N��BitCount�����o���Q�Ƃ��āADIB�̐F���𔻒f���܂��B
 *
 *	�߂�l:	DIB�̐F��
 *
 *******************************************************************************/
WORD CEditView::DibNumColors ( VOID FAR * pv)
{
	int					bits;
	LPBITMAPINFOHEADER lpbi;
	LPBITMAPCOREHEADER lpbc;
	lpbi = ( (LPBITMAPINFOHEADER)pv );
	lpbc = ( (LPBITMAPCOREHEADER)pv );
	/*
	 *	BITMAPINFO�`���w�b�_�[�̏ꍇ�A�p���b�g�̃T�C�Y��BITMAPCORE�`����
	 *	�w�b�_�[��biClrUsed�������Ă���B�p���b�g�̃T�C�Y�́A�s�N�Z��������
	 *	�̃r�b�g���ɂ��قȂ�B
	 */
	if( lpbi->biSize != sizeof( BITMAPCOREHEADER ) ){
		if( lpbi->biClrUsed != 0 )
			return (WORD)lpbi->biClrUsed;
		bits = lpbi->biBitCount;
	}
	else
		bits = lpbc->bcBitCount;

	switch ( bits ){
		case 1:	return 2;
		case 4:	return 16;
		case 8:	return 256;
		/* 24�r�b�gDIB�ɂ̓J���[�e�[�u���͂Ȃ� */
		default:	return 0;
	}
}

 /********** 64K�o�C�g�ȏ�̓ǂݏ������s���v���C�x�[�g���[�`�� *********/
/************************************************************************
 *
 *	�֐�:	lread( int fh, VOID FAR *pv, DWORD ul )
 *
 *	�ړI:	�f�[�^�����ׂēǂݎ��܂�32K�o�C�g���f�[�^��ǂݎ��܂��B
 *
 *	�߂�l:	0			- ����ɓǂݎ��Ȃ������ꍇ
 *	�ǂݎ�����o�C�g��	- ����ȊO�̏ꍇ
 *
 ************************************************************************/
DWORD CEditView::lread( int fh, void* pv, DWORD ul )
{
	DWORD	ulT = ul;
	BYTE	*hp = (BYTE *)pv;

#define	 MAXREAD_BYTES	32768	/* �ǂݎ�菈�����̓ǂݎ��\�ȃo�C�g�� */

	while( ul > (DWORD)MAXREAD_BYTES ){
		if( _lread( fh, (LPSTR)hp, (WORD)MAXREAD_BYTES ) != MAXREAD_BYTES )
			return 0;
		ul -= MAXREAD_BYTES;
		hp += MAXREAD_BYTES;
	}
	if( _lread(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul )
		return 0;
	return ulT;
}


/************************************************************************
 * �֐�:  PrintBitmap(int , int , int , int , LPCSTR );
 *
 *	�ړI:	�w�肵��DIB�t�@�C����ǂݍ���Ŏw�蕶�����͈͂ɐL�k������܂��B
 *			����ʒu�͌��݂̃t�H���g�T�C�Y�ɉe������܂��B
 *
 *	�߂�l:	�Ȃ�
 *
 ************************************************************************/
void CEditView::PrintBitmap( HDC hdc, int x1, int y1, const char* szFile )
{
	HANDLE				hdib;
	BITMAPINFOHEADER	bi;
	LPBITMAPINFOHEADER	lpbi;
	LPSTR				pBuf;

	/* DIB�t�@�C�����J���ă�����DIB���쐬 */
	hdib = OpenDIB ( szFile );
	if( hdib == NULL ){
//		MYTRACE( " OpenDIB()�̎��s�Ɏ��s  \n" );
		return;
	}
	DibInfo( hdib, &bi );

	/* DIB���v�����^DC�ɐL�k���ē]�� */
	lpbi = (LPBITMAPINFOHEADER)GlobalLock( hdib );
	if( !lpbi ){
		GlobalFree( (HGLOBAL)hdib );
		return ;
	}
	pBuf = (LPSTR)lpbi + (WORD)lpbi->biSize + PaletteSize( lpbi );
	StretchDIBits ( hdc,
		x1,
		y1,
		bi.biWidth,
		bi.biHeight,
		0,
		0,
		bi.biWidth,
		bi.biHeight,
		pBuf,
		(LPBITMAPINFO)lpbi,
		DIB_RGB_COLORS,
		SRCCOPY
	);
	GlobalUnlock( hdib );
	GlobalFree( hdib );
	return;
}

#endif //////////////////////////////////////////////






/*
�v���v���Z�b�T �f�B���N�e�B�u

#define	#endif	#ifdef	#line
#elif	#error	#ifndef	#pragma
#else	#if	#include	#undef



C�L�[���[�h

auto	double	int	struct
break	else	long	switch
case	enum	register	typedef
char	extern	return	union
const	float	short	unsigned
continue	for	signed	void
default		goto	sizeof	volatile
do	if	static	while



C++�L�[���[�h

asm	auto	bad_cast	bad_typeid
break	case	catch	char
class	const	const_cast	continue
default	delete	do	double
dynamic_cast	else	enum	except
extern	finally	float	for
friend	goto	if	inline
int	long	namespace	new
operator	private	protected	public
register	reinterpret_cast	return	short
signed	sizeof	static	static_cast
struct	switch	template	this
throw	try	type_info	typedef
typeid	union	unsigned	using
virtual	void	volatile	while
xalloc


C++���Z�q

::	�X�R�[�v����	�Ȃ�
::	�O���[�o��	�Ȃ�
[ ]	�z��Y��	������E
( )	�֐��Ăяo��	������E
( )	�ϊ�	�Ȃ�
.	�����o�I�� (�I�u�W�F�N�g)	������E
->	�����o�I�� (�|�C���^)	������E
++	��u�C���N�������g	�Ȃ�
--	��u�f�N�������g	�Ȃ�
new	�I�u�W�F�N�g���蓖��	�Ȃ�
delete	�I�u�W�F�N�g���	�Ȃ�
delete[ ]	�I�u�W�F�N�g���	�Ȃ�
++	�O�u�C���N�������g	�Ȃ�
--	�O�u�f�N�������g	�Ȃ�
*	�Q��	�Ȃ�
&	�A�h���X	�Ȃ�
+	�P���v���X	�Ȃ�
-	�Z�p�ے� (�P��)	�Ȃ�
!	�_�� NOT	�Ȃ�
~	�r�b�g���Ƃ̕␔	�Ȃ�
sizeof	�I�u�W�F�N�g�̃T�C�Y	�Ȃ�
sizeof ( )	�^�̃T�C�Y	�Ȃ�
typeid( )	�^��	�Ȃ�
(type)	�^�L���X�g (�ϊ�)	�E���獶
const_cast	�^�L���X�g (�ϊ�)	�Ȃ�
dynamic_cast	�^�L���X�g (�ϊ�)	�Ȃ�
reinterpret_cast	�^�L���X�g (�ϊ�)	�Ȃ�
static_cast	�^�L���X�g (�ϊ�)	�Ȃ�
.*	�N���X �����o�ւ̓K�p�|�C���^ (�I�u�W�F�N�g)	������E
->*	�|�C���^������A�N���X �����o�ւ̋t�Q�ƃ|�C���^	������E
*	��Z	������E
/	���Z	������E
%	��] (���W���[��)	������E
+	���Z	������E
-	���Z	������E
<<	���V�t�g	������E
>>	�E�V�t�g	������E
<	���Ȃ�	������E
>	��Ȃ�	������E
<=	�ȉ�	������E
>=	�ȏ�	������E
==	����	������E
!=	�s����	������E
&	�r�b�g���Ƃ� AND	������E
^	�r�b�g���Ƃ̔r���I OR	������E
|	�r�b�g���Ƃ� OR	������E
&&	�_�� AND	������E
||	�_�� OR	������E
e1?e2:e3	����	�E���獶
=	���	�E���獶
*=	��Z���	�E���獶
/=	���Z���	�E���獶
%=	��]���	�E���獶
+=	���Z���	�E���獶
-=	���Z���	�E���獶
<<=	���V�t�g���	�E���獶
>>=	�E�V�t�g���	�E���獶
&=	�r�b�g���Ƃ� AND ���	�E���獶
|=	�r�b�g���Ƃ� OR ���	�E���獶
^=	�r�b�g���Ƃ̔r���I OR ���	�E���獶
, 	�J���}	������E


*/


/* 2�_��Ίp�Ƃ����`�����߂� */
void CEditView::TwoPointToRect(
		RECT*	prcRect,
	int		nLineFrom,
	int		nColmFrom,
	int		nLineTo,
	int		nColmTo
)
{
	if( nLineFrom < nLineTo ){
		prcRect->top	= nLineFrom;
		prcRect->bottom	= nLineTo;
	}else{
		prcRect->top	= nLineTo;
		prcRect->bottom	= nLineFrom;
	}
	if( nColmFrom < nColmTo ){
		prcRect->left	= nColmFrom;
		prcRect->right	= nColmTo;
	}else{
		prcRect->left	= nColmTo;
		prcRect->right	= nColmFrom;
	}
	return;

}

#if 0//////////////////////////////////////////
/* �f�o�b�O�p ���[�W������`�̃_���v */
void CEditView::TraceRgn( HRGN hrgn )
{
//	unsigned int	i;
	char*			pBuf;
	int				nRgnDataSize;
	RGNDATA*		pRgnData;
	RECT*			pRect;
	nRgnDataSize = ::GetRegionData( hrgn, 0, NULL );
	pBuf = new char[nRgnDataSize];
	pRgnData = (RGNDATA*)pBuf;
	nRgnDataSize = ::GetRegionData( hrgn, nRgnDataSize, pRgnData );
	pRect = (RECT*)&pRgnData->Buffer[0];
//	if( 0 < pRgnData->rdh.nCount ){
//		m_cShareData.TraceOut( "---------\n" );;
//		for( i = 0; i < pRgnData->rdh.nCount; ++i ){
//			m_cShareData.TraceOut( "\t(%d, %d, %d, %d \n", pRect[i].left, pRect[i].right, pRect[i].top, pRect[i].bottom );;
//		}
//	}
	delete [] pBuf;
	return;
}
#endif //#if 0




/* �I��̈�̕`�� */
void CEditView::DrawSelectArea( void )
{
	/* �J�[�\���s�A���_�[���C����OFF */
	CaretUnderLineOFF( TRUE );
	if( !m_bDrawSWITCH ){
		return;
	}

	int			nFromLine;
	int			nFromCol;
	int			nToLine;
	int			nToCol;
	HDC			hdc;
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	int			nROP_Old;
	int			nLineNum;
	RECT		rcOld;
	RECT		rcNew;
	HRGN		hrgnOld = NULL;
	HRGN		hrgnNew = NULL;
	HRGN		hrgnDraw = NULL;

//	MYTRACE( "DrawSelectArea()  m_bBeginBoxSelect=%s\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( m_bBeginBoxSelect ){		/* ��`�͈͑I�� */
		hdc = ::GetDC( m_hWnd );
		hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcOld,
			m_nSelectLineFromOld,	/* �͈͑I���J�n�s */
			m_nSelectColmFromOld,	/* �͈͑I���J�n�� */
			m_nSelectLineToOld,		/* �͈͑I���I���s */
			m_nSelectColmToOld		/* �͈͑I���I���� */
		);
		if( rcOld.left	< m_nViewLeftCol ){
			rcOld.left = m_nViewLeftCol;
		}
		if( rcOld.right	< m_nViewLeftCol ){
			rcOld.right = m_nViewLeftCol;
		}
		if( rcOld.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcOld.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcOld.top < m_nViewTopLine ){
			rcOld.top = m_nViewTopLine;
		}
		if( rcOld.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcOld.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcOld.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcOld.left  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcOld.right		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcOld.right * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcOld.top		= ( rcOld.top - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		rcOld.bottom	= ( rcOld.bottom + 1 - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		hrgnOld = ::CreateRectRgnIndirect( &rcOld );

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcNew,
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo			/* �͈͑I���I���� */
		);
		if( rcNew.left	< m_nViewLeftCol ){
			rcNew.left = m_nViewLeftCol;
		}
		if( rcNew.right	< m_nViewLeftCol ){
			rcNew.right = m_nViewLeftCol;
		}
		if( rcNew.right > m_nViewLeftCol + m_nViewColNum + 1 ){
			rcNew.right = m_nViewLeftCol + m_nViewColNum + 1;
		}
		if( rcNew.top < m_nViewTopLine ){
			rcNew.top = m_nViewTopLine;
		}
		if( rcNew.bottom > m_nViewTopLine + m_nViewRowNum ){
			rcNew.bottom = m_nViewTopLine + m_nViewRowNum;
		}
		rcNew.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcNew.left  * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcNew.right		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + rcNew.right * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		rcNew.top		= ( rcNew.top - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		rcNew.bottom	= ( rcNew.bottom + 1 - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
		hrgnNew = ::CreateRectRgnIndirect( &rcNew );

		/* ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W��������� */
		hrgnDraw = ::CreateRectRgnIndirect( &rcNew );

		if( rcNew.left <= rcNew.right ){
			/* ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂� */
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){
				::PaintRgn( hdc, hrgnDraw );
			}
		}else{
			hrgnDraw = hrgnOld;
			::PaintRgn( hdc, hrgnDraw );

		}

		//////////////////////////////////////////
		/* �f�o�b�O�p ���[�W������`�̃_���v */
//@@		TraceRgn( hrgnDraw );


		if( NULL != hrgnDraw ){
			::DeleteObject( hrgnDraw );
		}
		if( NULL != hrgnNew ){
			::DeleteObject( hrgnNew );
		}
		if( NULL != hrgnOld ){
			::DeleteObject( hrgnOld );
		}
		::SetROP2( hdc, nROP_Old );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( m_hWnd, hdc );
	}else{
		hdc = ::GetDC( m_hWnd );
		hBrush = ::CreateSolidBrush( SELECTEDAREA_RGB );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		nROP_Old = ::SetROP2( hdc, SELECTEDAREA_ROP2 );

		/* ���ݕ`�悳��Ă���͈͂Ǝn�_������ */
		if( m_nSelectLineFrom == m_nSelectLineFromOld &&
			m_nSelectColmFrom  == m_nSelectColmFromOld ){
			/* �͈͂�����Ɋg�傳�ꂽ */
			if( m_nSelectLineTo > m_nSelectLineToOld ||
			    (m_nSelectLineTo == m_nSelectLineToOld &&
				m_nSelectColmTo > m_nSelectColmToOld ) ){
				nFromLine	= m_nSelectLineToOld;
				nFromCol	= m_nSelectColmToOld;
				nToLine		= m_nSelectLineTo;
				nToCol		= m_nSelectColmTo;
			}else{
				nFromLine	= m_nSelectLineTo;
				nFromCol	= m_nSelectColmTo;
				nToLine		= m_nSelectLineToOld;
				nToCol		= m_nSelectColmToOld;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else
		if( m_nSelectLineTo == m_nSelectLineToOld &&
			m_nSelectColmTo  == m_nSelectColmToOld ){
			/* �͈͂��O���Ɋg�傳�ꂽ */
			if( m_nSelectLineFrom < m_nSelectLineFromOld ||
			    (m_nSelectLineFrom == m_nSelectLineFromOld &&
				m_nSelectColmFrom < m_nSelectColmFromOld ) ){
				nFromLine	= m_nSelectLineFrom;
				nFromCol	= m_nSelectColmFrom;
				nToLine		= m_nSelectLineFromOld;
				nToCol		= m_nSelectColmFromOld;
			}else{
				nFromLine	= m_nSelectLineFromOld;
				nFromCol	= m_nSelectColmFromOld;
				nToLine		= m_nSelectLineFrom;
				nToCol		= m_nSelectColmFrom;
			}
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}else{
			nFromLine		= m_nSelectLineFromOld;
			nFromCol		= m_nSelectColmFromOld;
			nToLine			= m_nSelectLineToOld;
			nToCol			= m_nSelectColmToOld;
			for( nLineNum	= nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
			nFromLine	= m_nSelectLineFrom;
			nFromCol	= m_nSelectColmFrom;
			nToLine		= m_nSelectLineTo;
			nToCol		= m_nSelectColmTo;
			for( nLineNum = nFromLine; nLineNum <= nToLine; ++nLineNum ){
				if( nLineNum >= m_nViewTopLine && nLineNum <= m_nViewTopLine + m_nViewRowNum + 1 ){
					DrawSelectAreaLine( hdc, nLineNum, nFromLine, nFromCol, nToLine, nToCol );
				}
			}
		}
		::SetROP2( hdc, nROP_Old );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hBrush );
		::ReleaseDC( m_hWnd, hdc );
	}
	return;
}




/* �w��s�̑I��̈�̕`�� */
void CEditView::DrawSelectAreaLine(
		HDC hdc, int nLineNum, int nFromLine, int nFromCol, int nToLine, int nToCol
)
{
//	MYTRACE( "CEditView::DrawSelectAreaLine()\n" );
	HRGN			hrgnDraw;
	const char*		pLine;
	int				nLineLen;
	int				i;
	int				nCharChars;
	int				nPosX;
	RECT			rcClip;
	int				nSelectFrom;
	int				nSelectTo;
	const CLayout*	pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		nLineLen = 0;
	}
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
//		if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//		if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
		if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
			++nPosX;
			i = nLineLen;
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];

			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( nFromLine == nToLine ){
			nSelectFrom = nFromCol;
			nSelectTo   = nToCol;
	}else{
		if( nLineNum == nFromLine ){
			nSelectFrom = nFromCol;
			nSelectTo   = nPosX;
		}else
		if( nLineNum == nToLine ){
			nSelectFrom = 0;
			nSelectTo   = nToCol;
		}else{
			nSelectFrom = 0;
			nSelectTo   = nPosX;
		}
	}
	if( nSelectFrom < m_nViewLeftCol ){
		nSelectFrom = m_nViewLeftCol;
	}
	if( nSelectTo < m_nViewLeftCol ){
		nSelectTo = m_nViewLeftCol;
	}
	rcClip.left		= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nSelectFrom * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	rcClip.right	= (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nSelectTo   * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	rcClip.top		= ( nLineNum - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
	rcClip.bottom	= rcClip.top + m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace;
	if( rcClip.right - rcClip.left > 3000 ){
		rcClip.right = rcClip.left + 3000;
	}
//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1 );

	hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
	::PaintRgn( hdc, hrgnDraw );
	::DeleteObject( hrgnDraw );


//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1);
//	::FillRect( hdc, &rcClip, hBrushTextCol );

//	//	/* �f�o�b�O���j�^�ɏo�� */
//	m_cShareData.TraceOut( "DrawSelectAreaLine() rcClip.left=%d, rcClip.top=%d, rcClip.right=%d, rcClip.bottom=%d\n", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

	return;
}





/* �e�L�X�g���I������Ă��邩 */
BOOL CEditView::IsTextSelected( void )
{
	if( m_nSelectLineFrom	== -1 ||
		m_nSelectLineTo		== -1 ||
		m_nSelectColmFrom	== -1 ||
		m_nSelectColmTo		== -1
	){
//	if( m_nSelectLineFrom == m_nSelectLineTo &&
//		m_nSelectColmFrom  == m_nSelectColmTo ){
		return FALSE;
	}
	return TRUE;
}


/* �e�L�X�g�̑I�𒆂� */
BOOL CEditView::IsTextSelecting( void )
{
	if( m_bBeginSelect ||
		IsTextSelected()
	){
//		MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d TRUE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
		return TRUE;
	}
//	MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d FALSE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
	return FALSE;
}


/* �t�H���g�̕ύX */
void CEditView::SetFont( void )
{
	HDC			hdc;
	HFONT		hFontOld;
	TEXTMETRIC	tm;
	int			i;
	SIZE		sz;

	hdc = ::GetDC( m_hWnd );
	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
//	hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN_FAT );
	::GetTextMetrics( hdc, &tm );


// 1999.12.9
//	m_nCharWidth = tm.tmAveCharWidth - 1;
//	m_nCharHeight = tm.tmHeight + tm.tmExternalLeading;
	/* �����̑傫���𒲂ׂ� */
// 2000.2.8
//	::GetTextExtentPoint32( hdc, "X", 1, &sz );
//	m_nCharHeight = sz.cy;
//	m_nCharWidth = sz.cx;
	::GetTextExtentPoint32( hdc, "��", 2, &sz );
	m_nCharHeight = sz.cy;
	m_nCharWidth = sz.cx / 2;


// �s�̍�����2�̔{���ɂ���
//	if( ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) % 2 ){
//		++m_nCharHeight;
//	}

	m_nViewColNum = m_nViewCx / ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );	/* �\����̌��� */
	m_nViewRowNum = m_nViewCy / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );	/* �\����̍s�� */
	/* �s�ԍ��\���ɕK�v�ȕ���ݒ� */
	DetectWidthOfLineNumberArea( FALSE );
	/* ������`��p�������z�� */
	for( i = 0; i < ( sizeof(m_pnDx) / sizeof(m_pnDx[0]) ); ++i ){
		m_pnDx[i] = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	}
	::SelectObject( hdc, hFontOld );
	::ReleaseDC( m_hWnd, hdc );
	::InvalidateRect( m_hWnd, NULL, TRUE );
	if( m_nCaretWidth == 0 ){	/* �L�����b�g���Ȃ������ꍇ */
	}else{
		OnKillFocus();
		OnSetFocus();
	}
	return;
}



/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
int CEditView::DetectWidthOfLineNumberArea_calculate( void )
{
	int			i;
	int			nAllLines;
	int			nWork;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		nAllLines = m_pcEditDoc->m_cDocLineMgr.GetLineCount();
	}else{
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	}
	if( 0 < nAllLines ){
		nWork = 100;
		for( i = 3; i < 12; ++i ){
			if( ( nWork - 1 ) / nAllLines >= 1 ){
				break;
			}
			nWork *= 10;
		}
	}else{
		i = 1;
	}
	return i;

}


/*
�s�ԍ��\���ɕK�v�ȕ���ݒ蕝���ύX���ꂽ�ꍇ��TRUE��Ԃ�
*/
BOOL CEditView::DetectWidthOfLineNumberArea( BOOL bRedraw )
{
	int				i;
	PAINTSTRUCT		ps;
	HDC				hdc;
//	int				nAllLines;
//	int				nWork;
	int				m_nViewAlignLeftNew;
	int				nCxVScroll;
	RECT			rc;

	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_GYOU].m_bDisp ){
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		i = DetectWidthOfLineNumberArea_calculate();
		m_nViewAlignLeftNew = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) * (i + 1);	/* �\����̍��[���W */
		m_nViewAlignLeftCols = i + 1;
	}else{
		m_nViewAlignLeftNew = 8;
		m_nViewAlignLeftCols = 0;
	}
	if( m_nViewAlignLeftNew != m_nViewAlignLeft ){
		m_nViewAlignLeft = m_nViewAlignLeftNew;
		::GetClientRect( m_hWnd, &rc );
		nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
		m_nViewCx = (rc.right - rc.left) - nCxVScroll - m_nViewAlignLeft;	/* �\����̕� */


		if( bRedraw ){
			/* �ĕ`�� */
			hdc = ::GetDC( m_hWnd );
			ps.rcPaint.left = 0;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = 0;
			ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
			OnKillFocus();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			OnSetFocus();
			::ReleaseDC( m_hWnd, hdc );
		}
		return TRUE;
	}else{
		return FALSE;
	}
}





/* �X�N���[���o�[�̏�Ԃ��X�V���� */
void CEditView::AdjustScrollBars( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}


	int			nAllLines;
	int			nVScrollRate;
	SCROLLINFO	si;
//	int			nNowPos;

	if( NULL != m_hwndVScrollBar ){
		/* �����X�N���[���o�[ */
		/* nAllLines / nVScrollRate < 65535 �ƂȂ鐮��nVScrollRate�����߂� */
		nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
		nAllLines+=2;
		nVScrollRate = 1;
		while( nAllLines / nVScrollRate > 65535 ){
			++nVScrollRate;
		}
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;
		si.nMin  = 0;
		si.nMax  = nAllLines / nVScrollRate - 1;	/* �S�s�� */
		si.nPage = m_nViewRowNum / nVScrollRate;	/* �\����̍s�� */
		si.nPos  = m_nViewTopLine / nVScrollRate;	/* �\����̈�ԏ�̍s(0�J�n) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* �����X�N���[���o�[�̏k�� */
	}
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;

//@@		::GetScrollInfo( m_hwndHScrollBar, SB_CTL, &si );
//@@		if( si.nMax == m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - 1
//@@		 && si.nPage == (UINT)m_nViewColNum
//@@		 && si.nPos  == m_nViewLeftCol
//@@	   /*&& si.nTrackPos == 1*/ ){
//@@		}else{
			/* �����X�N���[���o�[ */
			si.cbSize = sizeof( si );
			si.fMask = SIF_ALL;
			si.nMin  = 0;
			si.nMax  = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - 1;	/* �܂�Ԃ������� */
			si.nPage = m_nViewColNum;		/* �\����̌��� */
			si.nPos  = m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */
			si.nTrackPos = 1;
			::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
//@@		}
	}

	return;
}





/*
||
|| �s���w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ��i���^���j
||
*/
int CEditView::MoveCursor( int nWk_CaretPosX, int nWk_CaretPosY, BOOL bDraw, int nCaretMarginRate )
{
	/* �X�N���[������ */
	int		nScrollRowNum = 0;
	int		nScrollColNum = 0;
	RECT	rcScrol;
	RECT	rcClip;
	RECT	rcClip2;
//	int		nIndextY = 8;
	int		nCaretMarginY;
	HDC		hdc;
//	HPEN	hPen, hPenOld;
	int		nScrollMarginRight;
	int		nScrollMarginLeft;

	if( 0 >= m_nViewColNum ){
		return 0;
	}
	hdc = ::GetDC( m_hWnd );

	/* �J�[�\���s�A���_�[���C����OFF */
	CaretUnderLineOFF( bDraw );

	nCaretMarginY = m_nViewRowNum / nCaretMarginRate;
	if( m_bBeginSelect ){	/* �͈͑I�� */
		nCaretMarginY = 0;
	}else{
		if( 1 > nCaretMarginY ){
			nCaretMarginY = 1;
		}
	}
	if( nWk_CaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		nWk_CaretPosY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1;
		if( nWk_CaretPosY < 0 ){
			nWk_CaretPosY = 0;
		}
	}

	/* �����X�N���[���ʁi�������j�̎Z�o */
	nScrollColNum = 0;
	nScrollMarginRight = 4;
	nScrollMarginLeft = 4;
	if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize > m_nViewColNum &&
		nWk_CaretPosX > m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ){
		nScrollColNum =
			( m_nViewLeftCol + m_nViewColNum - nScrollMarginRight ) - nWk_CaretPosX;
	}else
	if( 0 < m_nViewLeftCol &&
		nWk_CaretPosX < m_nViewLeftCol + nScrollMarginLeft
	){
		nScrollColNum = m_nViewLeftCol + nScrollMarginLeft - nWk_CaretPosX;
		if( 0 > m_nViewLeftCol - nScrollColNum ){
			nScrollColNum = m_nViewLeftCol;
		}

	}

	m_nViewLeftCol -= nScrollColNum;
//	if( 0 > m_nViewLeftCol ){
//		m_nViewLeftCol = 0;
//	}

//#ifdef _DEBUG
//	if( m_nMyIndex == 0 ){
//		MYTRACE( "����m_nViewLeftCol=%d\n", m_nViewLeftCol );
//	}
//#endif

	/* �����X�N���[���ʁi�s���j�̎Z�o */
	if( nWk_CaretPosY < m_nViewTopLine + ( nCaretMarginY ) ){
		if( nWk_CaretPosY < ( nCaretMarginY ) ){
			nScrollRowNum = m_nViewTopLine;
		}else{
			nScrollRowNum = m_nViewTopLine + ( nCaretMarginY ) - nWk_CaretPosY;
		}
	}else
	if( nWk_CaretPosY >= m_nViewTopLine + m_nViewRowNum - ( nCaretMarginY + 2 ) ){
		if( nWk_CaretPosY > m_pcEditDoc->m_cLayoutMgr.GetLineCount() - ( nCaretMarginY + 2 ) ){
			if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 > m_nViewRowNum ){
				nScrollRowNum = m_nViewTopLine - ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 - m_nViewRowNum );
			}else{
				nScrollRowNum = 0;
			}
		}else{
			nScrollRowNum =
				m_nViewTopLine + m_nViewRowNum - ( nCaretMarginY ) - ( nWk_CaretPosY + 2 );
		}
	}
	if( bDraw ){
		/* �X�N���[�� */
		if( abs( nScrollColNum ) >= m_nViewColNum ||
			abs( nScrollRowNum ) >= m_nViewRowNum ){
			m_nViewTopLine -= nScrollRowNum;
			if( bDraw ){
				::InvalidateRect( m_hWnd, NULL, TRUE );
				/* �X�N���[���o�[�̏�Ԃ��X�V���� */
				AdjustScrollBars();
			}
		}else
		if( nScrollRowNum != 0 || nScrollColNum != 0 ){
			rcScrol.left = 0;
			rcScrol.right = m_nViewCx + m_nViewAlignLeft;
			rcScrol.top = m_nViewAlignTop;
			rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
			if( nScrollRowNum > 0 ){
				rcScrol.bottom =
					m_nViewCy + m_nViewAlignTop -
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top = m_nViewAlignTop;
				rcClip.bottom =
					m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			}else
			if( nScrollRowNum < 0 ){
				rcScrol.top =
					m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				m_nViewTopLine -= nScrollRowNum;
				rcClip.left = 0;
				rcClip.right = m_nViewCx + m_nViewAlignLeft;
				rcClip.top =
					m_nViewCy + m_nViewAlignTop +
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
				rcClip.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( nScrollColNum > 0 ){
				rcScrol.left = m_nViewAlignLeft;
				rcScrol.right =
					m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left = m_nViewAlignLeft;
				rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}else
			if( nScrollColNum < 0 ){
				rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.left =
					m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				rcClip2.right = m_nViewCx + m_nViewAlignLeft;
				rcClip2.top = m_nViewAlignTop;
				rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
			}
			if( m_bDrawSWITCH ){
//				::ScrollWindow(
//					m_hWnd,		/* �X�N���[������E�B���h�E�̃n���h�� */
//					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* �����X�N���[���� */
//					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* �����X�N���[���� */
//					&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
//					NULL		/* �N���b�s���O�����`�̍\���̂̃A�h���X */
//				);
				::ScrollWindowEx(
					m_hWnd,
					nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),	/* �����X�N���[���� */
					nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),	/* �����X�N���[���� */
					&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
					NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
				);

				if( nScrollRowNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip, TRUE );
					if( nScrollColNum != 0 ){
						rcClip.left = 0;
						rcClip.right = m_nViewAlignLeft;
						rcClip.top = 0;
						rcClip.bottom = m_nViewCy + m_nViewAlignTop;
						::InvalidateRect( m_hWnd, &rcClip, TRUE );
					}
				}
				if( nScrollColNum != 0 ){
					::InvalidateRect( m_hWnd, &rcClip2, TRUE );
	 			}
				::UpdateWindow( m_hWnd );
			}
			/* �X�N���[���o�[�̏�Ԃ��X�V���� */
			AdjustScrollBars();
		}
	}

	/* �L�����b�g�ړ� */
	m_nCaretPosX = nWk_CaretPosX;
	m_nCaretPosY = nWk_CaretPosY;

	/* �J�[�\���ʒu�ϊ�
	||  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	||  �������ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		m_nCaretPosX,
		m_nCaretPosY,
		&m_nCaretPosX_PHY,	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g��(�O�J�n) */
		&m_nCaretPosY_PHY	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ�(�O�J�n) */
	);

	/* �J�[�\���s�A���_�[���C����ON */
	CaretUnderLineON( bDraw );
	if( bDraw ){
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();

		/* �L�����b�g�̍s���ʒu��\������ */
		DrawCaretPosInfo();
	}
	::ReleaseDC( m_hWnd, hdc );

//	/*
//	|| �w�肳�ꂽ�����s�̃��C�A�E�g�f�[�^(CLayout)�ւ̃|�C���^��Ԃ�
//	*/
//	CLayout*	pCLayout;
//	pCLayout = (CLayout*)m_pcEditDoc->m_cLayoutMgr.GetLineData( m_nCaretPosY );
//	if( NULL != pCLayout ){
//		pCLayout->DUMP();
//
//	}

//	/*
//	  �J�[�\���ʒu�ϊ�
//	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//	  �� �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//	*/
//	int		nX;
//	int		nY;
//	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
//		m_nCaretPosX,
//		m_nCaretPosY,
//		&nX,
//		&nY
//	);
//	MYTRACE( "��nX=%d,nY=%d ", nX, nY );
//	/*
//	  �J�[�\���ʒu�ϊ�
//	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//	  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//	*/
//	int		nPosX2;
//	int		nPosY2;
//	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(
//		nX,
//		nY,
//		&nPosX2,
//		&nPosY2
//	);
//	MYTRACE( "��nPosX2=%d,nPosY2=%d ", nPosX2, nPosY2 );
//	if( nPosX2 != m_nCaretPosX || nPosY2 != m_nCaretPosY ){
//		MYTRACE( "�������G���[" );
//	}
//	MYTRACE( "\n" );

	return nScrollRowNum;


}




/* IME�ҏW�G���A�̈ʒu��ύX */
void CEditView::SetIMECompFormPos( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	RECT			rc;
	POINT			po;
	COMPOSITIONFORM	CompForm;
	HIMC			hIMC = ::ImmGetContext( m_hWnd );
	POINT			point;
	HWND			hwndFrame;
	hwndFrame = ::GetParent( m_hwndParent );

	::GetCaretPos( &point );
	CompForm.dwStyle = CFS_POINT;
	CompForm.ptCurrentPos.x = (long) point.x + 1;
	CompForm.ptCurrentPos.y = (long) point.y + 1 + m_nCaretHeight - m_nCharHeight;

	::GetWindowRect( m_hWnd, &rc );
	po.x = 0;
	po.y = 0;
	::ClientToScreen( hwndFrame, &po );
	CompForm.ptCurrentPos.x += ( rc.left - po.x );
	CompForm.ptCurrentPos.y += ( rc.top  - po.y );

	if ( hIMC ){
		::ImmSetCompositionWindow( hIMC, &CompForm );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
	return;
}





/* IME�ҏW�G���A�̕\���t�H���g��ύX */
void CEditView::SetIMECompFormFont( void )
{
	//
	// If current composition form mode is near caret operation,
	// application should inform IME UI the caret position has been
	// changed. IME UI will make decision whether it has to adjust
	// composition window position.
	//
	//
	HIMC	hIMC = ::ImmGetContext( m_hWnd );
	if ( hIMC ){
		::ImmSetCompositionFont( hIMC, &(m_pShareData->m_Common.m_lf) );
	}
	::ImmReleaseContext( m_hWnd , hIMC );
	return;
}





/* �}�E�X���ɂ����W�w��ɂ��J�[�\���ړ�
|| �K�v�ɉ����ďc/���X�N���[��������
|| �����X�N���[���������ꍇ�͂��̍s����Ԃ�(���^��)
*/
int CEditView::MoveCursorToPoint( int xPos, int yPos )
{
	const char*		pLine;
	int				nLineLen;
	int				i;
	int				nCharChars;
	int				nNewX;
	int				nNewY;
	int				nPosX;
	int				nScrollRowNum = 0;
	const CLayout*	pcLayout;
	nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	if( 0 > nNewY ){
		nNewY = 0;
	}
	/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
	if( nNewY >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
		nNewY = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - 1;
		if( 0 > nNewY ){
			nNewY = 0;
		}
		nLineLen = 0;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nNewY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			nNewX = nLineLen;
		}else
		/* ���s�ŏI����Ă��邩 */
		if( EOL_NONE != pcLayout->m_cEol.GetLen() ){
//		if( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' ){
			nNewX = 0;
			++nNewY;
		}else{
			nNewX = LineIndexToColmn( pLine, nLineLen, nLineLen );
		}
		nScrollRowNum = MoveCursor( nNewX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}else
	/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
	if( nNewY < 0 ){
		nNewX = 0;
		nNewY = 0;
		nScrollRowNum = MoveCursor( nNewX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}else{
		/* �ړ���̍s�̃f�[�^���擾 */
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nNewY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			nLineLen = 0;
		}
		nPosX = 0;
		nCharChars = 0;
		for( i = 0; i < nLineLen; ){
			nPosX += nCharChars;
//			if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//			if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
			if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
				i = nLineLen;
				break;
			}
			if( pLine[i] == TAB ){
				nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
				if( nPosX + nCharChars > nNewX ){
					break;
				}
				++i;
			}else{
				nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
				if( 0 == nCharChars ){
					nCharChars = 1;
				}
				if( nPosX + nCharChars > nNewX ){
					break;
				}
				i+= nCharChars;
			}
		}
		if( i >= nLineLen ){
			/* �t���[�J�[�\�����[�h�� */
			if( m_pShareData->m_Common.m_bIsFreeCursorMode
			  || ( m_bBeginSelect && m_bBeginBoxSelect )	/* �}�E�X�͈͑I�� && ��`�͈͑I�� */
			  || m_bDragMode /* OLE DropTarget */
			){
				if( nNewY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
					pLine[ nLineLen - 1 ] != '\n' && pLine[ nLineLen - 1 ] != '\r'
				){
					nPosX = LineIndexToColmn( pLine, nLineLen, nLineLen );
				}else{
					nPosX = nNewX;
					if( nPosX < 0 ){
						nPosX = 0;
					}else
					if( nPosX > m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){	/* �܂�Ԃ������� */
						nPosX = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
					}
				}
			}
		}
		nScrollRowNum = MoveCursor( nPosX, nNewY, TRUE, 1000 );
		m_nCaretPosX_Prev = m_nCaretPosX;
	}
	return nScrollRowNum;
}
//_CARETMARGINRATE_CARETMARGINRATE_CARETMARGINRATE



/* �}�E�X���{�^������ */
void CEditView::OnLBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{

	if( m_bHokan ){
		m_pcEditDoc->m_cHokanMgr.Hide();
		m_bHokan = FALSE;
	}

//	DWORD	nKeyBoardSpeed;
	int			nCaretPosY_Old;
	int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
	int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
	int			nUrlLen;	// URL�̒���(�o�C�g��)
	CMemory		cmemCurText;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWork;

	if( m_pcEditDoc->m_cLayoutMgr.GetLineCount() == 0 ){
		return;
	}
	if( m_nCaretWidth == 0 ){ //�t�H�[�J�X���Ȃ��Ƃ�
		return;
	}
	nCaretPosY_Old = m_nCaretPosY;

	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
//	MYTRACE( "OnLBUTTONDOWN() nNewX=%d nNewY=%d\n", nNewX, nNewY );

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		if( m_pShareData->m_Common.m_bUseOLE_DropSource ){		/* OLE�ɂ��h���b�O���ɂ��邩 */
			/* �s�I���G���A���h���b�O���� */
			if( xPos < m_nViewAlignLeft - ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) ){
				goto normal_action;
			}
			/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
			if( 0 == IsCurrentPositionSelected(
				nNewX,		// �J�[�\���ʒuX
				nNewY		// �J�[�\���ʒuY
				)
			){
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
					DWORD dwEffects;
					m_bDragSource = TRUE;
					CDataObject data( cmemCurText.GetPtr( NULL ) );
					dwEffects = data.DragDrop( TRUE, DROPEFFECT_COPY | DROPEFFECT_MOVE );
					m_bDragSource = FALSE;
//					MYTRACE( "dwEffects=%d\n", dwEffects );
					if( 0 == dwEffects ){
						if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̑I��͈͂��I����Ԃɖ߂� */
							DisableSelectArea( TRUE );
						}
					}
				}
				return;
			}
		}
	}

normal_action:;

	/* ALT�L�[��������Ă����� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( TRUE );
		}
		if( yPos >= m_nViewAlignTop  && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
				MoveCursorToPoint( xPos, yPos );
			}else
			if( xPos < m_nViewAlignLeft ){
				MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
			}else{
				return;
			}
		}
		m_nMouseRollPosXOld = xPos;		/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;		/* �}�E�X�͈͑I��O��ʒu(Y���W) */
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = TRUE;			/* �͈͑I�� */
		m_bBeginBoxSelect = TRUE;		/* ��`�͈͑I�𒆂łȂ� */
		m_bBeginLineSelect = FALSE;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = FALSE;		/* �P��P�ʑI�� */

//		if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
//			/* ALT�L�[��������Ă����� */
//			if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
//				m_bBeginBoxSelect = TRUE;	/* ��`�͈͑I�� */
//			}
//		}
		::SetCapture( m_hWnd );
		::HideCaret( m_hWnd );
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
		if( xPos < m_nViewAlignLeft ){
			/* �J�[�\�����ړ� */
			Command_DOWN( TRUE, FALSE );
		}
	}else{
		/* �J�[�\���ړ� */
		if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
			if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
			}else
			if( xPos < m_nViewAlignLeft ){
			}else{
				return;
			}
		}else{
			return;
		}

		/* �}�E�X�̃L���v�`���Ȃ� */
		m_nMouseRollPosXOld = xPos;		/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;		/* �}�E�X�͈͑I��O��ʒu(Y���W) */
		/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
		m_bBeginSelect = TRUE;			/* �͈͑I�� */
//		m_bBeginBoxSelect = FALSE;		/* ��`�͈͑I�𒆂łȂ� */
		m_bBeginLineSelect = FALSE;		/* �s�P�ʑI�� */
		m_bBeginWordSelect = FALSE;		/* �P��P�ʑI�� */
		::SetCapture( m_hWnd );
		::HideCaret( m_hWnd );


		/* �I���J�n���� */
		/* SHIFT�L�[��������Ă����� */
		if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
			if( IsTextSelected() ){			/* �e�L�X�g���I������Ă��邩 */
				if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( TRUE );
					/* ���݂̃J�[�\���ʒu����I�����J�n���� */
					BeginSelectArea( );
				}else{
				}
			}else{
				/* ���݂̃J�[�\���ʒu����I�����J�n���� */
				BeginSelectArea( );
			}

			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
		}else{
			if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( TRUE );
			}
			/* �J�[�\���ړ� */
			if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
				if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
					MoveCursorToPoint( xPos, yPos );
				}else
				if( xPos < m_nViewAlignLeft ){
					MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
				}
			}
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea( );
		}


		/******* ���̎��_�ŕK�� TRUE == IsTextSelected() �̏�ԂɂȂ� ****:*/
		if( !IsTextSelected() ){
			::MYMESSAGEBOX( m_hWnd, MB_OK | MB_ICONEXCLAMATION, GSTR_APPNAME,
				"�o�O���Ă�"
			);
			return;
		}

		int	nWorkRel;
		nWorkRel = IsCurrentPositionSelected(
			m_nCaretPosX,	// �J�[�\���ʒuX
			m_nCaretPosY	// �J�[�\���ʒuY
		);
//		MYTRACE( "������nWorkRel = %d\n", nWorkRel );


		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );


		/* CTRL�L�[��������Ă����� */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
			m_bBeginWordSelect = TRUE;		/* �P��P�ʑI�� */
			if( !IsTextSelected() ){
				/* ���݈ʒu�̒P��I�� */
				Command_SELECTWORD();
				m_nSelectLineBgnFrom = m_nSelectLineFrom;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnFrom = m_nSelectColmFrom;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnTo = m_nSelectLineTo;		/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnTo = m_nSelectColmTo;		/* �͈͑I���J�n��(���_) */
			}else{

				/* �I��̈�`�� */
				DrawSelectArea();


				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pLine, nLineLen, m_nSelectColmFrom );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineFrom, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
						nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
						nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );


						nWork = IsCurrentPositionSelected(
							nColmFrom,	// �J�[�\���ʒuX
							nLineFrom	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineFrom = nLineFrom;
							m_nSelectColmFrom = nColmFrom;
							if( 1 == nWorkRel ){
								m_nSelectLineBgnFrom = nLineFrom;	/* �͈͑I���J�n�s(���_) */
								m_nSelectColmBgnFrom = nColmFrom;	/* �͈͑I���J�n��(���_) */
								m_nSelectLineBgnTo = nLineTo;		/* �͈͑I���J�n�s(���_) */
								m_nSelectColmBgnTo = nColmTo;		/* �͈͑I���J�n��(���_) */
							}
						}
	//					if( 1 == IsCurrentPositionSelected(
	//						nColmTo,	// �J�[�\���ʒuX
	//						nLineTo		// �J�[�\���ʒuY
	//					) ){
	//						m_nSelectLineFrom = nLineTo;
	//						m_nSelectColmFrom = nColmTo;
	//					}
					}
				}
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineTo, &nLineLen );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pLine, nLineLen, m_nSelectColmTo );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineTo, nIdx,
						&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
						nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
						nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );

						nWork = IsCurrentPositionSelected(
							nColmFrom,	// �J�[�\���ʒuX
							nLineFrom	// �J�[�\���ʒuY
						);
						if( -1 == nWork || 0 == nWork ){
							m_nSelectLineTo = nLineFrom;
							m_nSelectColmTo = nColmFrom;
						}
						if( 1 == IsCurrentPositionSelected(
							nColmTo,	// �J�[�\���ʒuX
							nLineTo		// �J�[�\���ʒuY
						) ){
							m_nSelectLineTo = nLineTo;
							m_nSelectColmTo = nColmTo;
						}
						if( -1 == nWorkRel || 0 == nWorkRel ){
							m_nSelectLineBgnFrom = nLineFrom;	/* �͈͑I���J�n�s(���_) */
							m_nSelectColmBgnFrom = nColmFrom;	/* �͈͑I���J�n��(���_) */
							m_nSelectLineBgnTo = nLineTo;		/* �͈͑I���J�n�s(���_) */
							m_nSelectColmBgnTo = nColmTo;		/* �͈͑I���J�n��(���_) */
						}
					}
				}

				if( 0 < nWorkRel ){

				}
				/* �I��̈�`�� */
				DrawSelectArea();
			}
		}
		if( xPos < m_nViewAlignLeft ){
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
//			BeginSelectArea( );
			m_bBeginLineSelect = TRUE;

			/* �J�[�\�����ړ� */
			Command_DOWN( TRUE, FALSE );
			m_nSelectLineBgnTo = m_nSelectLineTo;	/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnTo = m_nSelectColmTo;	/* �͈͑I���J�n��(���_) */
		}else{
//			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
//			BeginSelectArea( );
//			m_bBeginLineSelect = FALSE;

			/* URL���N���b�N���ꂽ��I�����邩 */
			if( TRUE == m_pShareData->m_Common.m_bSelectClickedURL ){

				/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
				if( IsCurrentPositionURL(
					m_nCaretPosX,	// �J�[�\���ʒuX
					m_nCaretPosY,	// �J�[�\���ʒuY
					&nUrlLine,		// URL�̍s(���s�P��)
					&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
					&nUrlLen,		// URL�̒���(�o�C�g��)
					NULL			// URL�󂯎���
				) ){

					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( TRUE );

					/* �I��͈͂̕ύX */
//					m_nSelectLineBgn = nUrlLine;				/* �͈͑I���J�n�s(���_) */
//					m_nSelectColmBgn = nUrlIdxBgn;				/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnFrom = nUrlLine;			/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = nUrlIdxBgn;			/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnTo = nUrlLine;				/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = nUrlIdxBgn + nUrlLen;	/* �͈͑I���J�n��(���_) */

					m_nSelectLineFrom =	nUrlLine;
					m_nSelectColmFrom = nUrlIdxBgn;
					m_nSelectLineTo = nUrlLine;
					m_nSelectColmTo = nUrlIdxBgn + nUrlLen;

					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
					*/
					int	nX, nY;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nSelectColmBgnFrom, m_nSelectLineBgnFrom, &nX, &nY );
					m_nSelectLineBgnFrom = nY;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = nX;		/* �͈͑I���J�n��(���_) */
					m_nSelectLineFrom =	nY;
					m_nSelectColmFrom  = nX;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nSelectColmTo, m_nSelectLineTo, &nX, &nY );
					m_nSelectLineTo = nY;
					m_nSelectColmTo = nX;
					m_nSelectLineBgnTo = nY;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = nX;		/* �͈͑I���J�n��(���_) */
					/* �I��̈�`�� */
					DrawSelectArea();
				}
			}
		}
	}
//	/* �L�[�{�[�h�̌��݂̃��s�[�g�Ԋu���擾 */
//	SystemParametersInfo( SPI_GETKEYBOARDSPEED, 0, &nKeyBoardSpeed, 0 );
//	nKeyBoardSpeed *= 3;
//	/* �^�C�}�[�N�� */
//	::SetTimer( m_hWnd, IDT_ROLLMOUSE, nKeyBoardSpeed, (TIMERPROC)EditViewTimerProc );
	return;
}

/* �w��J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
/* �߂�l��TRUE�̏ꍇ�A*ppszURL�͌Ăяo������delete���邱�� */
BOOL CEditView::IsCurrentPositionURL(
		int		nCaretPosX,		// �J�[�\���ʒuX
		int		nCaretPosY,		// �J�[�\���ʒuY
		int*	pnUrlLine,		// URL�̍s(���s�P��)
		int*	pnUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		int*	pnUrlLen,		// URL�̒���(�o�C�g��)
		char**	ppszURL			// URL�󂯎���(�֐�����new����)
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::IsCurrentPositionURL" );
#endif
	const char*	pLine;
//	const char*	pLineWork;
	int			nLineLen;
	int			nX;
	int			nY;
//	char*		pURL;
	int			i;
//	BOOL		bFindURL;
//	int			nCharChars;
	int			nUrlLen;

	/*
	  �J�[�\���ʒu�ϊ�
	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
	  ��
	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
	*/
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
		nCaretPosX,
		nCaretPosY,
		(int*)&nX,
		(int*)&nY
	);
	*pnUrlLine = nY;
	pLine = m_pcEditDoc->m_cDocLineMgr.GetLineStr( nY, &nLineLen );

	i = nX - 200;
	if( i < 0 ){
		i = 0;
	}
	for( ; i <= nX && i < nLineLen && i < nX + 200; ){
	/* �J�[�\���ʒu����O����250�o�C�g�܂ł͈͓̔��ōs���Ɍ������ăT�[�` */
		/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
		if( FALSE == IsURL( &pLine[i], nLineLen - i, &nUrlLen ) ){
			++i;
		}else{
			if( i <= nX && nX < i + nUrlLen ){
				/* URL��Ԃ��ꍇ */
				if( NULL != ppszURL ){
					*ppszURL = new char[nUrlLen + 1];
					memcpy( *ppszURL, &pLine[i], nUrlLen );
					(*ppszURL)[nUrlLen] = '\0';
					/* *ppszURL�͌Ăяo������delete���邱�� */
				}
				*pnUrlLen = nUrlLen;
				*pnUrlLine = nY;
				*pnUrlIdxBgn = i;
				return TRUE;
			}else{
				i += nUrlLen;
			}
		}
	}
	return FALSE;
}




/* �}�E�X�E�{�^������ */
void CEditView::OnRBUTTONDOWN( WPARAM fwKeys, int xPos , int yPos )
{
	/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
	int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
	/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
	if( 0 == IsCurrentPositionSelected(
		nNewX,		// �J�[�\���ʒuX
		nNewY		// �J�[�\���ʒuY
		)
	){
		return;
	}
	OnLBUTTONDOWN( fwKeys, xPos , yPos );
	return;

	int			nIdx;
	int			nFuncID;
	nIdx = 0;
	/* Ctrl,ALT,�L�[��������Ă����� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_pKeyNameArr[1]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_pKeyNameArr[1].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}
//	/* �E�N���b�N���j���[ */
//	Command_MENU_RBUTTON();
	return;
}

/* �}�E�X�E�{�^���J�� */
void CEditView::OnRBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
		OnLBUTTONUP( fwKeys, xPos, yPos );
	}


	int		nIdx;
	int		nFuncID;
	nIdx = 0;
	/* Ctrl,ALT,�L�[��������Ă����� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_pKeyNameArr[1]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_pKeyNameArr[1].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}
//	/* �E�N���b�N���j���[ */
//	Command_MENU_RBUTTON();
	return;
}



VOID CEditView::OnTimer(
	HWND hwnd,		// handle of window for timer messages
	UINT uMsg,		// WM_TIMER message
	UINT idEvent,	// timer identifier
	DWORD dwTime 	// current system time
   )
{
	POINT		po;
	RECT		rc;
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	char*		pszWork;
	int			nWorkLength;
	int			i;

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
		if( m_bDragSource ){
			return;
		}
	}
	if( !m_bBeginSelect ){	/* �͈͑I�� */
		//	2001/06/14 asa-o �Q�Ƃ���f�[�^�̕ύX
//		if( m_pShareData->m_Common.m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
		if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
			if( m_nCaretWidth > 0 ){ //�t�H�[�J�X������Ƃ�
				/* �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H */
				GetCursorPos( &po );
				GetWindowRect( m_hWnd, &rc );
				if( !PtInRect( &rc, po ) ){
					return;
				}
				/*  */
				if( m_bInMenuLoop == FALSE	&&	/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
					0 != m_dwTipTimer		&&	/* ����Tip��\�����Ă��Ȃ� */
					300 < ::GetTickCount() - m_dwTipTimer	/* ��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
				){
					/* �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�) */
					if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE ) ){
						pszWork = cmemCurText.GetPtr( NULL );
						nWorkLength	= lstrlen( pszWork );
						for( i = 0; i < nWorkLength; ++i ){
							if( pszWork[i] == '\0' ||
								pszWork[i] == CR ||
								pszWork[i] == LF ){
								break;
							}
						}
						char*	pszBuf = new char[i + 1];
						memcpy( pszBuf, pszWork, i );
						pszBuf[i] = '\0';
						cmemCurText.SetData( pszBuf, i );
						delete [] pszBuf;

						/* ���Ɍ����ς݂� */
						if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
							/* �Y������L�[���Ȃ����� */
							if( !m_cTipWnd.m_KeyWasHit ){
								goto end_of_search;
							}
						}else{
							m_cTipWnd.m_cKey = cmemCurText;
							/* �������s */
							//	2001/06/14 asa-o �Q�Ƃ���f�[�^�̕ύX
//							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
								/* �Y������L�[������ */
								m_cTipWnd.m_KeyWasHit = TRUE;
								pszWork = pcmemRefText->GetPtr( NULL );
//								m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
								m_cTipWnd.m_cInfo.SetDataSz( pszWork );
								delete pcmemRefText;
							}else{
								/* �Y������L�[���Ȃ����� */
								m_cTipWnd.m_KeyWasHit = FALSE;
								goto end_of_search;
							}
						}
						m_dwTipTimer = 0;	/* ����Tip��\�����Ă��� */
						m_poTipCurPos = po;	/* ���݂̃}�E�X�J�[�\���ʒu */

						/* ����Tip��\�� */
						m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
					}
					end_of_search:;
				}
			}
		}
	}else{
		::GetCursorPos( &po );
		::GetWindowRect(m_hWnd, &rc );
		if( !PtInRect( &rc, po ) ){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

		// 1999.12.18 �N���C�A���g�̈���ł̓^�C�}�[�����h���b�O+���[�����Ȃ�
		return;

//		rc.top += m_nViewAlignTop;
		RECT rc2;
		rc2 = rc;
		rc2.bottom = rc.top + m_nViewAlignTop + ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		if( PtInRect( &rc2, po )
		 && 0 < m_nViewTopLine
		){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}
		rc2 = rc;
		rc2.top = rc.bottom - ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		if( PtInRect( &rc2, po )
			&& m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nViewTopLine + m_nViewRowNum
		){
			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
			return;
		}

//		rc.top += 48;
//		rc.bottom -= 48;
//		if( !PtInRect( &rc, po ) ){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//		}
	}
	return;
}





/* �}�E�X�ړ��̃��b�Z�[�W���� */
void CEditView::OnMOUSEMOVE( WPARAM fwKeys, int xPos , int yPos )
{
//#ifdef _DEBUG
//	gm_ProfileOutput = 1;
//	CRunningTimer cRunningTimer( (const char*)"CEditView::OnMOUSEMOVE" );
//#endif
	int			nScrollRowNum;
	POINT		po;
	const char*	pLine;
	int			nLineLen;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	int			nIdx;
	int			nWorkF;
	int			nWorkT;

	int			nSelectLineBgnFrom_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColmBgnFrom_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColmBgnTo_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineFrom_Old;
	int			nSelectColmFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColmTo_Old;
	int			nSelectLineFrom;
	int			nSelectColmFrom;
	int			nSelectLineTo;
	int			nSelectColmTo;

	nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;		/* �͈͑I���J�n�s(���_) */
	nSelectColmBgnFrom_Old	= m_nSelectColmBgnFrom;		/* �͈͑I���J�n��(���_) */
	nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;		/* �͈͑I���J�n�s(���_) */
	nSelectColmBgnTo_Old	= m_nSelectColmBgnTo;		/* �͈͑I���J�n��(���_) */
	nSelectLineFrom_Old		= m_nSelectLineFrom;
	nSelectColmFrom_Old		= m_nSelectColmFrom;
	nSelectLineTo_Old		= m_nSelectLineTo;
	nSelectColmTo_Old		= m_nSelectColmTo;

	if( !m_bBeginSelect ){	/* �͈͑I�� */
		::GetCursorPos( &po );
		//	2001/06/18 asa-o: �⊮�E�B���h�E���\������Ă��Ȃ�
		if(!m_bHokan){
			/* ����Tip���N������Ă��� */
			if( 0 == m_dwTipTimer ){
				if( (m_poTipCurPos.x != po.x || m_poTipCurPos.y != po.y ) ){
					/* ����Tip������ */
					m_cTipWnd.Hide();
					m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
				}
			}else{
				m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
			}
		}
		/* ���݂̃}�E�X�J�[�\���ʒu�����C�A�E�g�ʒu */
		int nNewX = m_nViewLeftCol + (xPos - m_nViewAlignLeft) / ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		int nNewY = m_nViewTopLine + (yPos - m_nViewAlignTop) / ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
		int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
		int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		int			nUrlLen;	// URL�̒���(�o�C�g��)


		/* �I���e�L�X�g�̃h���b�O���� */
		if( m_bDragMode ){
			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
				/* ���W�w��ɂ��J�[�\���ړ� */
				nScrollRowNum = MoveCursorToPoint( xPos , yPos );
			}
		}else{
			/* �s�I���G���A? */
			if( xPos < m_nViewAlignLeft ){
				/* ���J�[�\�� */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else

			if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
			 && TRUE == m_pShareData->m_Common.m_bUseOLE_DropSource /* OLE�ɂ��h���b�O���ɂ��邩 */
			 && 0 == IsCurrentPositionSelected(						/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
				nNewX,	// �J�[�\���ʒuX
				nNewY	// �J�[�\���ʒuY
				)
			){
				/* ���J�[�\�� */
				::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
			}else
			/* �J�[�\���ʒu��URL���L��ꍇ */
			if( IsCurrentPositionURL(
				nNewX,			// �J�[�\���ʒuX
				nNewY,			// �J�[�\���ʒuY
				&nUrlLine,		// URL�̍s(���s�P��)
				&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				&nUrlLen,		// URL�̒���(�o�C�g��)
				NULL/*&pszURL*/	// URL�󂯎���
			) ){
				/* ��J�[�\�� */
				::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
			}else{
				/* �A�C�r�[�� */
				::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
			}
		}
		return;
	}
	::SetCursor( ::LoadCursor( NULL, IDC_IBEAM ) );
	if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
		/* ���W�w��ɂ��J�[�\���ړ� */
		nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
		ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	}else{
		/* ���W�w��ɂ��J�[�\���ړ� */
		if( xPos < m_nViewAlignLeft && m_bBeginLineSelect ){
			nScrollRowNum = MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) , yPos + ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) );
		}else{
			nScrollRowNum = MoveCursorToPoint( xPos , yPos );
		}
		m_nMouseRollPosXOld = xPos;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
		m_nMouseRollPosYOld = yPos;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */

		/* CTRL�L�[��������Ă����� */
//		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		if( !m_bBeginWordSelect ){
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
		}else{
//			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
//			ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
			/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�) */
			ChangeSelectAreaByCurrentCursorTEST(
				(int)m_nCaretPosX,
				(int)m_nCaretPosY,
				(int&)nSelectLineFrom,
				(int&)nSelectColmFrom,
				(int&)nSelectLineTo,
				(int&)nSelectColmTo
			);
			/* �I��͈͂ɕύX�Ȃ� */
			if( nSelectLineFrom_Old == nSelectLineFrom
			 && nSelectColmFrom_Old == nSelectColmFrom
			 && nSelectLineTo_Old == nSelectLineTo
			 && nSelectColmTo_Old == nSelectColmTo
			){
				ChangeSelectAreaByCurrentCursor(
					(int)m_nCaretPosX,
					(int)m_nCaretPosY
				);
				return;
			}
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen ) ) ){
				nIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
				/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_nCaretPosY, nIdx,
					&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
				){
					/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
					nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
					nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );

					nWorkF = IsCurrentPositionSelectedTEST(
						nColmFrom,	// �J�[�\���ʒuX
						nLineFrom,	// �J�[�\���ʒuY
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
					nWorkT = IsCurrentPositionSelectedTEST(
						nColmTo,	// �J�[�\���ʒuX
						nLineTo,	// �J�[�\���ʒuY
						(int)nSelectLineFrom,
						(int)nSelectColmFrom,
						(int)nSelectLineTo,
						(int)nSelectColmTo
					);
//					MYTRACE( "nWorkF=%d nWorkT=%d\n", nWorkF, nWorkT );
					if( -1 == nWorkF/* || 0 == nWorkF*/ ){
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}else
					if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineFrom_Old == nSelectLineFrom
					 && nSelectColmFrom_Old == nSelectColmFrom
					){
						/* �n�_�����ύX���O���ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmTo, nLineTo );
					}else
					if( nSelectLineTo_Old == nSelectLineTo
					 && nSelectColmTo_Old == nSelectColmTo
					){
						/* �I�_�����ύX������ɏk�����ꂽ */
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}
				}else{
					/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
					ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
				}
			}else{
				/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
				ChangeSelectAreaByCurrentCursor( m_nCaretPosX, m_nCaretPosY );
			}
		}
	}
	return;
}
//m_dwTipTimerm_dwTipTimerm_dwTipTimer




/* �}�E�X�z�C�[���̃��b�Z�[�W���� */
LRESULT CEditView::OnMOUSEWHEEL( WPARAM wParam, LPARAM lParam )
{
	WORD	fwKeys;
	short	zDelta;
	short	xPos;
	short	yPos;
	int		i;
	int		nScrollCode;
	int		nRollLineNum;

	fwKeys = LOWORD(wParam);			// key flags
	zDelta = (short) HIWORD(wParam);	// wheel rotation
	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
	yPos = (short) HIWORD(lParam);		// vertical position of pointer
//	MYTRACE( "CEditView::DispatchEvent() WM_MOUSEWHEEL fwKeys=%xh zDelta=%d xPos=%d yPos=%d \n", fwKeys, zDelta, xPos, yPos );
	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}




	/* �}�E�X�z�C�[���ɂ��X�N���[���s�������W�X�g������擾 */
	nRollLineNum = 6;
	/* ���W�X�g���̑��݃`�F�b�N */
	HKEY hkReg;
	DWORD dwType;
	DWORD dwDataLen;	// size of value data
	char szValStr[1024];
	if( ERROR_SUCCESS == ::RegOpenKeyEx( HKEY_CURRENT_USER, "Control Panel\\desktop", 0, KEY_READ, &hkReg ) ){
		dwType = REG_SZ;
		dwDataLen = sizeof( szValStr ) - 1;
		if( ERROR_SUCCESS == ::RegQueryValueEx( hkReg, "WheelScrollLines", NULL, &dwType, (unsigned char *)szValStr, &dwDataLen ) ){
//			MYTRACE( "szValStr=[%s]\n", szValStr );
			nRollLineNum = ::atoi( szValStr );
		}
		::RegCloseKey( hkReg );
	}
	if( -1 == nRollLineNum ){/* �u1��ʕ��X�N���[������v */
		nRollLineNum = m_nViewRowNum;	// �\����̍s��
	}else{
		if( nRollLineNum < 1 ){
			nRollLineNum = 1;
		}
		if( nRollLineNum > 10 ){
			nRollLineNum = 10;
		}
	}
	for( i = 0; i < nRollLineNum; ++i ){
//		::PostMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );
//		::SendMessage( m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );
		if( nScrollCode == SB_LINEUP ){
			ScrollAtV( m_nViewTopLine - 1 );
		}else{
			ScrollAtV( m_nViewTopLine + 1 );
		}
	}
	return 0;
}





/* ���݂̃J�[�\���ʒu����I�����J�n���� */
void CEditView::BeginSelectArea( void )
{
//	m_nSelectLineBgn = m_nCaretPosY;	/* �͈͑I���J�n�s(���_) */
//	m_nSelectColmBgn = m_nCaretPosX;	/* �͈͑I���J�n��(���_) */

	m_nSelectLineBgnFrom = m_nCaretPosY;/* �͈͑I���J�n�s(���_) */
	m_nSelectColmBgnFrom = m_nCaretPosX;/* �͈͑I���J�n��(���_) */
	m_nSelectLineBgnTo = m_nCaretPosY;	/* �͈͑I���J�n�s(���_) */
	m_nSelectColmBgnTo = m_nCaretPosX;	/* �͈͑I���J�n��(���_) */

	m_nSelectLineFrom = m_nCaretPosY;	/* �͈͑I���J�n�s */
	m_nSelectColmFrom = m_nCaretPosX;	/* �͈͑I���J�n�� */
	m_nSelectLineTo = m_nCaretPosY;		/* �͈͑I���I���s */
	m_nSelectColmTo = m_nCaretPosX;		/* �͈͑I���I���� */
	return;
}





/* ���݂̑I��͈͂��I����Ԃɖ߂� */
void CEditView::DisableSelectArea( BOOL bDraw )
{
	m_nSelectLineFromOld = m_nSelectLineFrom;	/* �͈͑I���J�n�s */
	m_nSelectColmFromOld = m_nSelectColmFrom;	/* �͈͑I���J�n�� */
	m_nSelectLineToOld = m_nSelectLineTo;		/* �͈͑I���I���s */
	m_nSelectColmToOld = m_nSelectColmTo;		/* �͈͑I���I���� */
//	m_nSelectLineFrom = 0;
//	m_nSelectColmFrom = 0;
//	m_nSelectLineTo = 0;
//	m_nSelectColmTo = 0;

	m_nSelectLineFrom	= -1;
	m_nSelectColmFrom	= -1;
	m_nSelectLineTo		= -1;
	m_nSelectColmTo		= -1;

	if( bDraw ){
		DrawSelectArea();
	}
	m_bSelectingLock     = FALSE;	/* �I����Ԃ̃��b�N */
	m_nSelectLineFromOld = 0;		/* �͈͑I���J�n�s */
	m_nSelectColmFromOld = 0; 		/* �͈͑I���J�n�� */
	m_nSelectLineToOld = 0;			/* �͈͑I���I���s */
	m_nSelectColmToOld = 0;			/* �͈͑I���I���� */
	m_bBeginBoxSelect = FALSE;		/* ��`�͈͑I�� */
	m_bBeginLineSelect = FALSE;		/* �s�P�ʑI�� */
	m_bBeginWordSelect = FALSE;		/* �P��P�ʑI�� */

	//	From Here Dec. 6, 2000 genta
#if 0
	//	�t���[�J�[�\���łȂ��ꍇ�ɂ͍s�����E�ɂ���
	//	�L�����b�g��K�؂Ȉʒu�܂ňړ�����D
	if( ! m_pShareData->m_Common.m_bIsFreeCursorMode ){
		int len, pos;
		const char *line;


		line = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &len );
		//	�f�[�^���Ȃ���EOF�݂̂̍s i.e. Column�͏��0
		if( line == NULL && m_nCaretPosX > 0){
			MoveCursor( 0, m_nCaretPosY, bDraw );
		}
		else {
			pos = LineIndexToColmn( line, m_nCaretPosY, len );	//	�s���̌��ʒu���v�Z
			if( m_nCaretPosX > pos ){
				MoveCursor( pos, m_nCaretPosY, bDraw );
			}
		}
//		char buf[30];
//		wsprintf( buf, "X[%d] Y[%d], len[%d], pos[%d]", m_nCaretPosX, m_nCaretPosY, len, pos );
//		::MessageBox( NULL, buf, "CEditView::DisableSelectArea", MB_OK );
	}
#endif
	//	To Here Dec. 6, 2000 genta

//	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
//		/* �J�[�\���s�A���_�[���C���̕`�� */
//		HDC		hdc;
//		HPEN	hPen;
//		HPEN	hPenOld;
//		hdc = ::GetDC( m_hWnd );
//		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
//		hPenOld = (HPEN)::SelectObject( hdc, hPen );
//		m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
//		::MoveToEx(
//			hdc,
//			m_nViewAlignLeft,
//			m_nOldUnderLineY,
//			NULL
//		);
//		::LineTo(
//			hdc,
//			m_nViewCx + m_nViewAlignLeft,
//			m_nOldUnderLineY
//		);
//		::SelectObject( hdc, hPenOld );
//		::DeleteObject( hPen );
//		::ReleaseDC( m_hWnd, hdc );
//	}


	/* �J�[�\���s�A���_�[���C����ON */
	CaretUnderLineON( bDraw );
	return;
}





/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
void CEditView::ChangeSelectAreaByCurrentCursor( int nCaretPosX, int nCaretPosY )
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;

	m_nSelectLineFromOld = m_nSelectLineFrom;	/* �͈͑I���J�n�s */
	m_nSelectColmFromOld = m_nSelectColmFrom; 	/* �͈͑I���J�n�� */
	m_nSelectLineToOld = m_nSelectLineTo;		/* �͈͑I���I���s */
	m_nSelectColmToOld = m_nSelectColmTo;		/* �͈͑I���I���� */

	if( m_nSelectLineBgnFrom == m_nSelectLineBgnTo /* �͈͑I���J�n�s(���_) */
	 && m_nSelectColmBgnFrom == m_nSelectColmBgnTo ){
		if( nCaretPosY == m_nSelectLineBgnFrom
		 && nCaretPosX == m_nSelectColmBgnFrom ){
			/* �I������ */
			m_nSelectLineFrom = -1;
			m_nSelectColmFrom  = -1;
			m_nSelectLineTo = -1;
			m_nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_nSelectLineBgnFrom
		 || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX < m_nSelectColmBgnFrom ) ){
			m_nSelectLineFrom = nCaretPosY;
			m_nSelectColmFrom = nCaretPosX;
			m_nSelectLineTo = m_nSelectLineBgnFrom;
			m_nSelectColmTo = m_nSelectColmBgnFrom;
		}else{
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = nCaretPosY;
			m_nSelectColmTo = nCaretPosX;
		}
	}else{
		/* �펞�I��͈͈͓͂̔� */
		if( ( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) )
		 && ( nCaretPosY < m_nSelectLineBgnTo || ( nCaretPosY == m_nSelectLineBgnTo && nCaretPosX < m_nSelectColmBgnTo ) )
		){
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = m_nSelectLineBgnTo;
			m_nSelectColmTo = m_nSelectColmBgnTo;
		}else
		if( !( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) ) ){
			/* �펞�I��͈͂̑O���� */
			m_nSelectLineFrom = nCaretPosY;
			m_nSelectColmFrom  = nCaretPosX;
			m_nSelectLineTo = m_nSelectLineBgnTo;
			m_nSelectColmTo = m_nSelectColmBgnTo;
		}else{
			/* �펞�I��͈͂̌����� */
			m_nSelectLineFrom = m_nSelectLineBgnFrom;
			m_nSelectColmFrom = m_nSelectColmBgnFrom;
			m_nSelectLineTo = nCaretPosY;
			m_nSelectColmTo = nCaretPosX;
		}
	}
	/* �I��̈�̕`�� */
	DrawSelectArea();
	return;
}

/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX(�e�X�g�̂�) */
void CEditView::ChangeSelectAreaByCurrentCursorTEST(
	int		nCaretPosX,
	int		nCaretPosY,
	int&	nSelectLineFrom,
	int&	nSelectColmFrom,
	int&	nSelectLineTo,
	int&	nSelectColmTo
)
{
//	MYTRACE( "ChangeSelectAreaByCurrentCursor( %d, %d )\n", nCaretPosX, nCaretPosY );
//	int			nLineFrom;
//	int			nColmFrom;
//	int			nLineTo;
//	int			nColmTo;
//	const char*	pLine;
//	int			nLineLen;
//	int			nIdx;


	if( m_nSelectLineBgnFrom == m_nSelectLineBgnTo /* �͈͑I���J�n�s(���_) */
	 && m_nSelectColmBgnFrom == m_nSelectColmBgnTo ){
		if( nCaretPosY == m_nSelectLineBgnFrom
		 && nCaretPosX == m_nSelectColmBgnFrom ){
			/* �I������ */
			nSelectLineFrom = -1;
			nSelectColmFrom  = -1;
			nSelectLineTo = -1;
			nSelectColmTo = -1;
		}else
		if( nCaretPosY < m_nSelectLineBgnFrom
		 || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX < m_nSelectColmBgnFrom ) ){
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnFrom;
			nSelectColmTo = m_nSelectColmBgnFrom;
		}else{
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}else{
		/* �펞�I��͈͈͓͂̔� */
		if( ( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) )
		 && ( nCaretPosY < m_nSelectLineBgnTo || ( nCaretPosY == m_nSelectLineBgnTo && nCaretPosX < m_nSelectColmBgnTo ) )
		){
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = m_nSelectLineBgnTo;
			nSelectColmTo = m_nSelectColmBgnTo;
		}else
		if( !( nCaretPosY > m_nSelectLineBgnFrom || ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX >= m_nSelectColmBgnFrom ) ) ){
			/* �펞�I��͈͂̑O���� */
			nSelectLineFrom = nCaretPosY;
			nSelectColmFrom  = nCaretPosX;
			nSelectLineTo = m_nSelectLineBgnTo;
			nSelectColmTo = m_nSelectColmBgnTo;
		}else{
			/* �펞�I��͈͂̌����� */
			nSelectLineFrom = m_nSelectLineBgnFrom;
			nSelectColmFrom = m_nSelectColmBgnFrom;
			nSelectLineTo = nCaretPosY;
			nSelectColmTo = nCaretPosX;
		}
	}
	return;

}


/* �}�E�X���{�^���J���̃��b�Z�[�W���� */
void CEditView::OnLBUTTONUP( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONUP()\n" );
	CMemory		cmemBuf, cmemClip;

	/* �͈͑I���I�� & �}�E�X�L���v�`���[����� */
	if( m_bBeginSelect ){	/* �͈͑I�� */
		/* �}�E�X �L���v�`������� */
		::ReleaseCapture();
		::ShowCaret( m_hWnd );

//		/* �^�C�}�[�I�� */
//		::KillTimer( m_hWnd, IDT_ROLLMOUSE );
		m_bBeginSelect = FALSE;

//		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			if( m_nSelectLineFrom == m_nSelectLineTo &&
				m_nSelectColmFrom == m_nSelectColmTo
			){
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( TRUE );
			}
//		}
	}
	return;
}





/* �}�E�X���{�^���_�u���N���b�N */
void CEditView::OnLBUTTONDBLCLK( WPARAM fwKeys, int xPos , int yPos )
{
//	MYTRACE( "OnLBUTTONDBLCLK()\n" );

	int			nIdx;
	int			nFuncID;
	int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
	int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
	int			nUrlLen;	// URL�̒���(�o�C�g��)
	char*		pszURL;
	const char*	pszMailTo = "mailto:";

	/* �J�[�\���ʒu��URL���L��ꍇ�̂��͈̔͂𒲂ׂ� */
	if( IsCurrentPositionURL(
		m_nCaretPosX,	// �J�[�\���ʒuX
		m_nCaretPosY,	// �J�[�\���ʒuY
		&nUrlLine,		// URL�̍s(���s�P��)
		&nUrlIdxBgn,	// URL�̈ʒu(�s������̃o�C�g�ʒu)
		&nUrlLen,		// URL�̒���(�o�C�g��)
		&pszURL			// URL�󂯎���
	) ){
		char*		pszWork = NULL;
		char*		pszOPEN;

		/* URL���J�� */
	 	/* ���݈ʒu�����[���A�h���X�Ȃ�΁ANULL�ȊO�ƁA���̒�����Ԃ� */
		if( TRUE == IsMailAddress( pszURL, lstrlen( pszURL ), NULL ) ){
			pszWork = new char[ lstrlen( pszURL ) + lstrlen( pszMailTo ) + 1];
			strcpy( pszWork, pszMailTo );
			strcat( pszWork, pszURL );
			pszOPEN = pszWork;
		}else{
			pszOPEN = pszURL;
		}
		::ShellExecute( NULL, "open", pszOPEN, NULL, NULL, SW_SHOW );
		delete [] pszURL;
		if( NULL != pszWork ){
			delete [] pszWork;
		}
		return;
	}

	/* GREP�o�̓��[�h ���� �}�E�X���{�^���_�u���N���b�N�Ń^�O�W�����v �̏ꍇ */
	if( m_pcEditDoc->m_bGrepMode && m_pShareData->m_Common.m_bGTJW_LDBLCLK ){
		/* �^�O�W�����v�@�\ */
		Command_TAGJUMP();
		return;
	}


	nIdx = 0;
	/* Ctrl,ALT,�L�[��������Ă����� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
		nIdx |= _SHIFT;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
		nIdx |= _CTRL;
	}
	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
		nIdx |= _ALT;
	}
	/* �}�E�X���N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_pKeyNameArr[0]�ɓ����Ă��� */
	nFuncID = m_pShareData->m_pKeyNameArr[0].m_nFuncCodeArr[nIdx];
	if( nFuncID != 0 ){
		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
		::SendMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
	}

	/* �h���b�O�I���J�n */
	m_nMouseRollPosXOld = xPos;			/* �}�E�X�͈͑I��O��ʒu(X���W) */
	m_nMouseRollPosYOld = yPos;			/* �}�E�X�͈͑I��O��ʒu(Y���W) */
	/* �͈͑I���J�n & �}�E�X�L���v�`���[ */
	m_bBeginSelect = TRUE;				/* �͈͑I�� */
	m_bBeginBoxSelect = FALSE;			/* ��`�͈͑I�𒆂łȂ� */
	m_bBeginLineSelect = FALSE;			/* �s�P�ʑI�� */
	m_bBeginWordSelect = TRUE;			/* �P��P�ʑI�� */

	if( m_pShareData->m_Common.m_bFontIs_FIXED_PITCH ){	/* ���݂̃t�H���g�͌Œ蕝�t�H���g�ł��� */
		/* ALT�L�[��������Ă����� */
		if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
			m_bBeginBoxSelect = TRUE;	/* ��`�͈͑I�� */
		}
	}
	::SetCapture( m_hWnd );
	::HideCaret( m_hWnd );
	if( IsTextSelected() ){
		/* �펞�I��͈͈͂̔� */
		m_nSelectLineBgnTo = m_nSelectLineTo;
		m_nSelectColmBgnTo = m_nSelectColmTo;
	}else{
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
	}

	return;
}





/* �J�[�\���㉺�ړ����� */
int CEditView::Cursor_UPDOWN( int nMoveLines, int bSelect )
{
	const char*		pLine;
	int				nLineLen;
	int				nPosX;
	int				nPosY = m_nCaretPosY;
	int				i;
	int				nCharChars;
	int				nLineCols;
	int				nScrollLines;
	const CLayout*	pcLayout;
	nScrollLines = 0;
	if( nMoveLines > 0 ){
		/* �J�[�\�����e�L�X�g�ŉ��[�s�ɂ��邩 */
		if( m_nCaretPosY + nMoveLines >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ){
			nMoveLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount() - m_nCaretPosY  - 1;
		}
		if( nMoveLines <= 0 ){
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = LineIndexToColmn( pLine, nLineLen, nLineLen );
				/* ���s�ŏI����Ă��邩 */
				if( ( EOL_NONE != pcLayout->m_cEol.GetLen() )
//				if( ( pLine[ nLineLen - 1 ] == '\n' || pLine[ nLineLen - 1 ] == '\r' )
				 || nLineCols >= m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize
				){
					if( bSelect ){
						if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̃J�[�\���ʒu����I�����J�n���� */
							BeginSelectArea();
						}
					}else{
						if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̑I��͈͂��I����Ԃɖ߂� */
							DisableSelectArea( TRUE );
						}
					}
					nPosX = 0;
					++nPosY;
					if( bSelect ){
						/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nPosX, nPosY );
					}
					nScrollLines = MoveCursor( nPosX, nPosY, TRUE );
				}
			}
			return nScrollLines;
		}
	}else{
		/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
		if( m_nCaretPosY + nMoveLines < 0 ){
			nMoveLines = - m_nCaretPosY;
		}
		if( nMoveLines >= 0 ){
			return nScrollLines;
		}
	}
	if( bSelect ){
		if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
			BeginSelectArea();
		}
	}else{
		if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
			/* ���݂̑I��͈͂��I����Ԃɖ߂� */
			DisableSelectArea( TRUE );
		}
	}
	/* ���̍s�̃f�[�^���擾 */
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY + nMoveLines, &nLineLen, &pcLayout );
	if( NULL == pLine ){
		nLineLen = 0;
	}
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
//		if( i == nLineLen - 1 && ( pLine[i] == '\n' || pLine[i] == '\r' ) ){
//		if( i >= nLineLen - pcLayout->m_cEol.GetLen() ){
		if( i >= nLineLen - (pcLayout->m_cEol.GetLen()?1:0 ) ){
			i = nLineLen;
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > m_nCaretPosX_Prev ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > m_nCaretPosX_Prev ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( i >= nLineLen ){
		/* �t���[�J�[�\�����[�h�� */
		if( m_pShareData->m_Common.m_bIsFreeCursorMode
		 || IsTextSelected() && m_bBeginBoxSelect	/* ��`�͈͑I�� */
		){
			if( m_nCaretPosY + nMoveLines + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount()  ){
				if( NULL != pLine ){
					if( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ){
						nPosX = m_nCaretPosX_Prev;
					}
				}
			}else{
				nPosX = m_nCaretPosX_Prev;
			}
		}
	}
	nScrollLines = MoveCursor( nPosX, m_nCaretPosY + nMoveLines, TRUE );
	if( bSelect ){
//		if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp && !IsTextSelected() && -1 != m_nOldUnderLineY ){
//			HDC		hdc;
//			HPEN	hPen, hPenOld;
//			hdc = ::GetDC( m_hWnd );
//			/* �J�[�\���s�A���_�[���C���̏��� */
//			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
//			hPenOld = (HPEN)::SelectObject( hdc, hPen );
//			::MoveToEx(
//				hdc,
//				m_nViewAlignLeft,
//				m_nOldUnderLineY,
//				NULL
//			);
//			::LineTo(
//				hdc,
//				m_nViewCx + m_nViewAlignLeft,
//				m_nOldUnderLineY
//			);
//			::SelectObject( hdc, hPenOld );
//			::DeleteObject( hPen );
//			m_nOldUnderLineY = -1;
//			::ReleaseDC( m_hWnd, hdc );
//		}
		/* ���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
//		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY + nMoveLines );
		ChangeSelectAreaByCurrentCursor( nPosX, m_nCaretPosY );
	}
	return nScrollLines;
}





/* �w���[�s�ʒu�փX�N���[�� */
void CEditView::ScrollAtV( int nPos )
{
	int			nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
//	RECT		rcClip2;
//	PAINTSTRUCT ps;
//	HDC			hdc;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 ) - m_nViewRowNum;
		if( nPos < 0 ){
			nPos = 0;
		}
	}
	if( m_nViewTopLine == nPos ){
		return;
	}
	/* �����X�N���[���ʁi�s���j�̎Z�o */
	nScrollRowNum = m_nViewTopLine - nPos;

	/* �X�N���[�� */
	if( abs( nScrollRowNum ) >= m_nViewRowNum ){
		m_nViewTopLine = nPos;
//		if( bDraw ){
			::InvalidateRect( m_hWnd, NULL, TRUE );
//			/* �X�N���[���o�[�̏�Ԃ��X�V���� */
//			AdjustScrollBars();
//		}
	}else{
//	}else
//	if( nScrollRowNum != 0 || nScrollColNum != 0 ){
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				m_nViewCy + m_nViewAlignTop -
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top = m_nViewAlignTop;
			rcClip.bottom =
				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
		}else
		if( nScrollRowNum < 0 ){
			rcScrol.top =
				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			m_nViewTopLine = nPos;
			rcClip.left = 0;
			rcClip.right = m_nViewCx + m_nViewAlignLeft;
			rcClip.top =
				m_nViewCy + m_nViewAlignTop +
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
		}
//		if( nScrollColNum > 0 ){
//			rcScrol.left = m_nViewAlignLeft;
//			rcScrol.right =
//				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.left = m_nViewAlignLeft;
//			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.top = m_nViewAlignTop;
//			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
//		}else
//		if( nScrollColNum < 0 ){
//			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.left =
//				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
//			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip2.top = m_nViewAlignTop;
//			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
//		}
		if( m_bDrawSWITCH ){
//			::ScrollWindow(
//				m_hWnd,	/* �X�N���[������E�B���h�E�̃n���h�� */
//				0/*nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )*/,	/* �����X�N���[���� */
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* �����X�N���[���� */
//				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
//				NULL		/* �N���b�s���O�����`�̍\���̂̃A�h���X */
//			);
			::ScrollWindowEx(
				m_hWnd,
				0,	/* �����X�N���[���� */
				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ),		/* �����X�N���[���� */
				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( m_hWnd, &rcClip, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}

//	/* �ĕ`�� */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();

	return;
}




/* �w�荶�[���ʒu�փX�N���[�� */
void CEditView::ScrollAtH( int nPos )
{
	int			nScrollColNum;
	RECT		rcScrol;
//	RECT		rcClip;
	RECT		rcClip2;
//	PAINTSTRUCT ps;
//	HDC			hdc;
	if( nPos < 0 ){
		nPos = 0;
	}else
	if( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewRowNum < nPos ){
		nPos = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewRowNum;
	}
	if( m_nViewLeftCol == nPos ){
		return;
	}
	/* �����X�N���[���ʁi�������j�̎Z�o */
	nScrollColNum = m_nViewLeftCol - nPos;

//	m_nViewLeftCol = nPos;
//#ifdef _DEBUG
//		if( m_nMyIndex == 2 ){
//			MYTRACE( "%s(%d): m_nMyIndex == 2 m_nViewLeftCol = %d\n", __FILE__, __LINE__, m_nViewLeftCol );
//		}
//#endif
	/* �X�N���[�� */
	if( abs( nScrollColNum ) >= m_nViewColNum /*|| abs( nScrollRowNum ) >= m_nViewRowNum*/ ){
//		m_nViewTopLine -= nScrollRowNum;
		m_nViewLeftCol = nPos;
//		if( bDraw ){
			::InvalidateRect( m_hWnd, NULL, TRUE );
//			/* �X�N���[���o�[�̏�Ԃ��X�V���� */
//			AdjustScrollBars();
//		}
	}else{
//	}else
//	if( nScrollRowNum != 0 || nScrollColNum != 0 ){
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
//		if( nScrollRowNum > 0 ){
//			rcScrol.bottom =
//				m_nViewCy + m_nViewAlignTop -
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			m_nViewTopLine -= nScrollRowNum;
//			rcClip.left = 0;
//			rcClip.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip.top = m_nViewAlignTop;
//			rcClip.bottom =
//				m_nViewAlignTop + nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		}else
//		if( nScrollRowNum < 0 ){
//			rcScrol.top =
//				m_nViewAlignTop - nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			m_nViewTopLine -= nScrollRowNum;
//			rcClip.left = 0;
//			rcClip.right = m_nViewCx + m_nViewAlignLeft;
//			rcClip.top =
//				m_nViewCy + m_nViewAlignTop +
//				nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//			rcClip.bottom = m_nViewCy + m_nViewAlignTop;
//		}
		if( nScrollColNum > 0 ){
			rcScrol.left = m_nViewAlignLeft;
			rcScrol.right =
				m_nViewCx + m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left = m_nViewAlignLeft;
			rcClip2.right = m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}else
		if( nScrollColNum < 0 ){
			rcScrol.left = m_nViewAlignLeft - nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.left =
				m_nViewCx + m_nViewAlignLeft + nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			rcClip2.right = m_nViewCx + m_nViewAlignLeft;
			rcClip2.top = m_nViewAlignTop;
			rcClip2.bottom = m_nViewCy + m_nViewAlignTop;
		}
		m_nViewLeftCol = nPos;
		if( m_bDrawSWITCH ){
//			::ScrollWindow(
//				m_hWnd,	/* �X�N���[������E�B���h�E�̃n���h�� */
//				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* �����X�N���[���� */
//				0/*nScrollRowNum * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight )*/,	/* �����X�N���[���� */
//				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
//				NULL		/* �N���b�s���O�����`�̍\���̂̃A�h���X */
//			);
			::ScrollWindowEx(
				m_hWnd,
				nScrollColNum * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ),		/* �����X�N���[���� */
				0,	/* �����X�N���[���� */
				&rcScrol,	/* �X�N���[�������`�̍\���̂̃A�h���X */
				NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
			);
			::InvalidateRect( m_hWnd, &rcClip2, TRUE );
			::UpdateWindow( m_hWnd );
		}
	}
//	/* �ĕ`�� */
//	hdc = ::GetDC( m_hWnd );
//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
//	OnKillFocus();
//	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
//	::ReleaseDC( m_hWnd, hdc );
	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();



	return;
}

/* �I��͈͂̃f�[�^���擾 */
/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
BOOL CEditView::GetSelectedData(
		CMemory&	cmemBuf,
		BOOL		bLineOnly,
		const char*	pszQuote,			/* �擪�ɕt������p�� */
		BOOL		bWithLineNumber,	/* �s�ԍ���t�^���� */
//	Jul. 25, 2000 genta
		enumEOLType	neweol				//	�R�s�[��̉��s�R�[�h EOL_NONE�̓R�[�h�ۑ�
)
{
	const char*		pLine;
	int				nLineLen;
	int				nLineNum;
	int				nIdxFrom;
	int				nIdxTo;
	RECT			rcSel;
	int				nRowNum;
	int				nLineNumCols;
	char*			pszLineNum;
	char*			pszSpaces = "                    ";
	const CLayout*	pcLayout;
	CEOL			appendEol( neweol );
	bool			addnl = false;

	/* �͈͑I��������Ă��Ȃ� */
	if( !IsTextSelected() ){
		return FALSE;
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		nLineNumCols = DetectWidthOfLineNumberArea_calculate();
		nLineNumCols += 1;
		pszLineNum = new char[nLineNumCols + 1];
	}


	if( m_bBeginBoxSelect ){	/* ��`�͈͑I�� */
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo			/* �͈͑I���I���� */
		);
//		cmemBuf.SetData( "", 0 );
		cmemBuf.SetDataSz( "" );
		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pLine, nLineLen, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pLine, nLineLen, rcSel.right );
			}
			if( nIdxTo - nIdxFrom > 0 ){
				if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
				}else{
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}
			++nRowNum;
//			if( nRowNum > 0 ){
				cmemBuf.AppendSz( CRLF );
				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
					break;
				}
//			}
		}
	}else{
		cmemBuf.SetDataSz( "" );
		for( nLineNum = m_nSelectLineFrom; nLineNum <= m_nSelectLineTo; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == m_nSelectLineFrom ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pLine, nLineLen, m_nSelectColmFrom );
			}else{
				nIdxFrom = 0;
			}
			if( nLineNum == m_nSelectLineTo ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nSelectColmTo );
//				if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
//					nIdxTo = nLineLen + pcLayout->m_cEol.GetLen() - 1;
//				}
			}else{
				nIdxTo = nLineLen;// + (pcLayout->m_cEol.GetLen()?pcLayout->m_cEol.GetLen()-1:0) ;
			}
			if( nIdxTo - nIdxFrom == 0 ){
				continue;
			}

#if 0
			//	Jul. 25, 2000	genta
			//	���s�����ύX�̂��ߍ폜
			/* ���s����������񂩂��̂��E�E�E�H */
			if( EOL_NONE != pcLayout->m_cEol && nIdxTo >= nLineLen ){
				nIdxTo = nLineLen - 1;
			}
#endif

			if( NULL != pszQuote && 0 < lstrlen( pszQuote ) ){	/* �擪�ɕt������p�� */
//				cmemBuf.Append( pszQuote, lstrlen( pszQuote ) );
				cmemBuf.AppendSz( pszQuote );
			}
			if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
				wsprintf( pszLineNum, " %d:" , nLineNum + 1 );
				cmemBuf.Append( pszSpaces, nLineNumCols - lstrlen( pszLineNum ) );
//				cmemBuf.Append( pszLineNum, lstrlen( pszLineNum ) );
				cmemBuf.AppendSz( pszLineNum );
			}


			if( EOL_NONE != pcLayout->m_cEol ){
//			if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
//				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
//				cmemBuf.AppendSz( CRLF );

				if( nIdxTo >= nLineLen ){
					cmemBuf.Append( &pLine[nIdxFrom], nLineLen - 1 - nIdxFrom );
					//	Jul. 25, 2000 genta
					cmemBuf.AppendSz( ( neweol == EOL_UNKNOWN ) ?
						(pcLayout->m_cEol).GetValue() :	//	�R�[�h�ۑ�
						appendEol.GetValue() );			//	�V�K���s�R�[�h
				}
				else {
					cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				}
			}else{
				cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
				if( nIdxTo - nIdxFrom >= nLineLen ){
					if( m_pShareData->m_Common.m_bAddCRLFWhenCopy ||  /* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
						NULL != pszQuote || /* �擪�ɕt������p�� */
						bWithLineNumber 	/* �s�ԍ���t�^���� */
					){
//						cmemBuf.Append( CRLF, lstrlen( CRLF ) );
						//	Jul. 25, 2000 genta
						cmemBuf.AppendSz(( neweol == EOL_UNKNOWN ) ?
							CRLF :						//	�R�[�h�ۑ�
							appendEol.GetValue() );		//	�V�K���s�R�[�h
					}
				}
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}
	}
	if( bWithLineNumber ){	/* �s�ԍ���t�^���� */
		delete [] pszLineNum;
	}
	return TRUE;
}




/* �I��͈͓��̑S�s���N���b�v�{�[�h�ɃR�s�[���� */
void CEditView::CopySelectedAllLines(
	const char*	pszQuote,		/* �擪�ɕt������p�� */
	BOOL		bWithLineNumber	/* �s�ԍ���t�^���� */
)
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	int			nSelectLineFromOld;	/* �͈͑I���J�n�s */
	int			nSelectColFromOld; 	/* �͈͑I���J�n�� */
	int			nSelectLineToOld;	/* �͈͑I���I���s */
	int			nSelectColToOld;	/* �͈͑I���I���� */
	RECT		rcSel;
	CMemory		cmemBuf;
//	HGLOBAL		hgClip;
//	char*		pszClip;
//	const char*	pLine;
//	int			nLineLen;
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* �͈͑I���J�n�s */
			m_nSelectColmFrom,					/* �͈͑I���J�n�� */
			m_nSelectLineTo,					/* �͈͑I���I���s */
			m_nSelectColmTo						/* �͈͑I���I���� */
		);
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
//		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
//		MoveCursor( rcSel.left, rcSel.top, FALSE );
		m_nSelectLineFrom = rcSel.top;			/* �͈͑I���J�n�s */
		m_nSelectColmFrom = 0;					/* �͈͑I���J�n�� */
		m_nSelectLineTo = rcSel.bottom + 1;		/* �͈͑I���I���s */
		m_nSelectColmTo = 0;					/* �͈͑I���I���� */
		m_bBeginBoxSelect = FALSE;
	}else{
		nSelectLineFromOld = m_nSelectLineFrom;	/* �͈͑I���J�n�s */
		nSelectColFromOld = 0;					/* �͈͑I���J�n�� */
		nSelectLineToOld = m_nSelectLineTo;		/* �͈͑I���I���s */
		if( m_nSelectColmTo > 0 ){
			++nSelectLineToOld;					/* �͈͑I���I���s */
		}
		nSelectColToOld = 0;					/* �͈͑I���I���� */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
		m_nSelectLineFrom = nSelectLineFromOld;	/* �͈͑I���J�n�s */
		m_nSelectColmFrom = nSelectColFromOld; 	/* �͈͑I���J�n�� */
		m_nSelectLineTo = nSelectLineToOld;		/* �͈͑I���I���s */
		m_nSelectColmTo = nSelectColToOld;		/* �͈͑I���I���� */
	}
	/* �ĕ`�� */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left = 0;
	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top = m_nViewAlignTop;
	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	OnKillFocus();
	OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
	/* �I��͈͂��N���b�v�{�[�h�ɃR�s�[ */
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ�͏I������ */
	if( FALSE == GetSelectedData(
		cmemBuf,
		FALSE,
		pszQuote, /* ���p�� */
		bWithLineNumber /* �s�ԍ���t�^���� */
	) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	MySetClipboardData( cmemBuf.GetPtr( NULL ), cmemBuf.GetLength(), FALSE );


//	/* Windows�N���b�v�{�[�h�ɃR�s�[ */
//	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, cmemBuf.GetLength() + 1 );
//	pszClip = (char*)::GlobalLock( hgClip );
//	memcpy( pszClip, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() + 1 );
//	::GlobalUnlock( hgClip );
//	::OpenClipboard( m_hWnd );
//	::EmptyClipboard();
//	::SetClipboardData( CF_OEMTEXT, hgClip );
//	::CloseClipboard();
	return;
}






/* �I���G���A�̃e�L�X�g���w����@�ŕϊ� */
void CEditView::ConvSelectedArea( int nFuncCode )
{
	CMemory		cmemBuf;
	int			nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
	int			nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
	COpe*		pcOpe = NULL;
//	BOOL		bBoxSelected;
//	HDC			hdc;
//	PAINTSTRUCT	ps;
	RECT		rcSel;

	int			nPosX;
	int			nPosY;
	int			nIdxFrom;
	int			nIdxTo;
	int			nLineNum;
	int			nDelPos;
	int			nDelLen;
	int			nDelPosNext;
	int			nDelLenNext;
	const char*	pLine;
	int			nLineLen;
	const char*	pLine2;
	int			nLineLen2;
	int			i;
	CMemory*	pcMemDeleted;
	CWaitCursor cWaitCursor( m_hWnd );

	int			nSelectLineFromOld;				/* �͈͑I���J�n�s */
	int			nSelectColFromOld; 				/* �͈͑I���J�n�� */
	int			nSelectLineToOld;				/* �͈͑I���I���s */
	int			nSelectColToOld;				/* �͈͑I���I���� */
	BOOL		bBeginBoxSelectOld;

	/* �e�L�X�g���I������Ă��邩 */
	if( !IsTextSelected() ){
		return;
	}

	nSelectLineFromOld	= m_nSelectLineFrom;	/* �͈͑I���J�n�s */
	nSelectColFromOld	= m_nSelectColmFrom;	/* �͈͑I���J�n�� */
	nSelectLineToOld	= m_nSelectLineTo;		/* �͈͑I���I���s */
	nSelectColToOld		= m_nSelectColmTo;		/* �͈͑I���I���� */
	bBeginBoxSelectOld	= m_bBeginBoxSelect;


	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){

		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,					/* �͈͑I���J�n�s */
			m_nSelectColmFrom,					/* �͈͑I���J�n�� */
			m_nSelectLineTo,					/* �͈͑I���I���s */
			m_nSelectColmTo						/* �͈͑I���I���� */
		);

		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );

		nIdxFrom = 0;
		nIdxTo = 0;
		for( nLineNum = rcSel.bottom; nLineNum >= rcSel.top - 1; nLineNum-- ){
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pLine, nLineLen, rcSel.left );
				nIdxTo		= LineColmnToIndex( pLine, nLineLen, rcSel.right );

				for( i = nIdxFrom; i <= nIdxTo; ++i ){
					if( pLine[i] == CR || pLine[i] == LF ){
						nIdxTo = i;
						break;
					}
				}
			}else{
				nIdxFrom	= 0;
				nIdxTo		= 0;
			}
			nDelPos = nDelPosNext;
			nDelLen	= nDelLenNext;
			if( nLineNum < rcSel.bottom && 0 < nDelLen ){
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2 );
				nPosX = LineIndexToColmn( pLine2, nLineLen2, nDelPos );
				nPosY =  nLineNum + 1;
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
			    }else{
			    	pcOpe = NULL;
			    }

				pcMemDeleted = new CMemory;
				/* �w��ʒu�̎w�蒷�f�[�^�폜 */
				DeleteData2(
					nPosX/*rcSel.left*/,
					nPosY/*nLineNum + 1*/,
					nDelLen,
					pcMemDeleted,
					pcOpe,		/* �ҏW����v�f COpe */
					FALSE,
					FALSE
				);
				cmemBuf.SetData( pcMemDeleted );
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_After,
						&pcOpe->m_nCaretPosY_PHY_After
					);
					/* ����̒ǉ� */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}else{
					delete pcMemDeleted;
					pcMemDeleted = NULL;
				}
				/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
				ConvMemory( &cmemBuf, nFuncCode );
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe = new COpe;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(
						nPosX,
						nPosY,
						&pcOpe->m_nCaretPosX_PHY_Before,
						&pcOpe->m_nCaretPosY_PHY_Before
					);
				}
				/* ���݈ʒu�Ƀf�[�^��}�� */
				InsertData_CEditView(
					nPosX,
					nPosY,
					cmemBuf.GetPtr( NULL ),
					cmemBuf.GetLength(),
					&nNewLine,
					&nNewPos,
					pcOpe,
					TRUE/*FALSE*/
				);
				/* �J�[�\�����ړ� */
				MoveCursor( nNewPos, nNewLine, FALSE );
				m_nCaretPosX_Prev = m_nCaretPosX;
				if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
					pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
					pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
					/* ����̒ǉ� */
					m_pcOpeBlk->AppendOpe( pcOpe );
				}
			}
		}
		/* �}���f�[�^�̐擪�ʒu�փJ�[�\�����ړ� */
		MoveCursor( rcSel.left, rcSel.top, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nOpe = OPE_MOVECARET;									/* ������ */
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;				/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;				/* ����O�̃L�����b�g�ʒu�x */
			pcOpe->m_nCaretPosX_PHY_After = pcOpe->m_nCaretPosX_PHY_Before;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = pcOpe->m_nCaretPosY_PHY_Before;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
	}else{
		/* �I��͈͂̃f�[�^���擾 */
		/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
		GetSelectedData( cmemBuf, FALSE, NULL, FALSE );

		/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
		ConvMemory( &cmemBuf, nFuncCode );

//		/* �I���G���A���폜 */
//		DeleteData( FALSE );

		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo,		/* �͈͑I���I���� */
			NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			cmemBuf.m_pData,		/* �}������f�[�^ */
			cmemBuf.m_nDataLen,		/* �}������f�[�^�̒��� */
			TRUE/*bRedraw*/
		);
		return;


#if 0///////////////////////////////
		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe = new COpe;
			pcOpe->m_nCaretPosX_PHY_Before = m_nCaretPosX_PHY;	/* ����O�̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_Before = m_nCaretPosY_PHY;	/* ����O�̃L�����b�g�ʒu�x */
		}
		/* ���݈ʒu�Ƀf�[�^��}�� */
		InsertData_CEditView(
			m_nCaretPosX,
			m_nCaretPosY,
			cmemBuf.GetPtr( NULL ),
			cmemBuf.GetLength(),
			&nNewLine,
			&nNewPos,
			pcOpe,
			TRUE/*FALSE*/
		);

		/* �J�[�\�����ړ� */
		MoveCursor( nNewPos, nNewLine, TRUE );
		m_nCaretPosX_Prev = m_nCaretPosX;

		if( !m_bDoing_UndoRedo ){	/* �A���h�D�E���h�D�̎��s���� */
			pcOpe->m_nCaretPosX_PHY_After = m_nCaretPosX_PHY;	/* �����̃L�����b�g�ʒu�w */
			pcOpe->m_nCaretPosY_PHY_After = m_nCaretPosY_PHY;	/* �����̃L�����b�g�ʒu�x */
			/* ����̒ǉ� */
			m_pcOpeBlk->AppendOpe( pcOpe );
		}
#endif ///////////////////////////////

	}
#if 0///////////////////////////////
	m_nSelectLineFrom	= nSelectLineFromOld;	/* �͈͑I���J�n�s */
	m_nSelectColmFrom	= nSelectColFromOld;	/* �͈͑I���J�n�� */
	m_nSelectLineTo		= nSelectLineToOld;		/* �͈͑I���I���s */
	m_nSelectColmTo		= nSelectColToOld;		/* �͈͑I���I���� */
	m_bBeginBoxSelect	= bBeginBoxSelectOld;

	m_pcEditDoc->m_bIsModified = TRUE;	/* �ύX�t���O */
	SetParentCaption();					/* �e�E�B���h�E�̃^�C�g�����X�V */

	/* �ĕ`�� */
	//	::UpdateWindow();
	hdc = ::GetDC( m_hWnd );
	ps.rcPaint.left		= 0;
	ps.rcPaint.right	= m_nViewAlignLeft + m_nViewCx;
	ps.rcPaint.top		= m_nViewAlignTop;
	ps.rcPaint.bottom	= m_nViewAlignTop + m_nViewCy;
	OnKillFocus();
	OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
#endif ///////////////////////////////


	return;
}


/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
void CEditView::ConvMemory( CMemory* pCMemory, int nFuncCode )
{
	switch( nFuncCode ){
	case F_TOLOWER: pCMemory->ToLower(); break;						/* �p�啶�����p������ */
	case F_TOUPPER: pCMemory->ToUpper(); break;						/* �p���������p�啶�� */
	case F_TOHANKAKU: pCMemory->ToHankaku(); break;					/* �S�p�����p */
	case F_TOZENKAKUKATA: pCMemory->ToZenkaku( 0, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA: pCMemory->ToZenkaku( 0, 1 );	/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 1 );	/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_CODECNV_EMAIL:		pCMemory->JIStoSJIS(); break;		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	case F_CODECNV_EUC2SJIS:	pCMemory->EUCToSJIS(); break;		/* EUC��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODE2SJIS:pCMemory->UnicodeToSJIS(); break;	/* Unicode��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:	pCMemory->SJIStoJIS();break;		/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC: 	pCMemory->SJISToEUC();break;		/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_UTF82SJIS:	pCMemory->UTF8ToSJIS();break;		/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:	pCMemory->UTF7ToSJIS();break;		/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:	pCMemory->SJISToUTF7();break;		/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:	pCMemory->SJISToUTF8();break;		/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_AUTO2SJIS:	pCMemory->AUTOToSJIS();break;		/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_TABTOSPACE:
		pCMemory->TABToSPACE(
			m_pcEditDoc->GetDocumentAttribute().m_nTabSpace
		);break;	/* TAB���� */
	case F_SPACETOTAB:	//#### Stonee, 2001/05/27
		pCMemory->SPACEToTAB(
			m_pcEditDoc->GetDocumentAttribute().m_nTabSpace
		);
		break;		/* �󔒁�TAB */
	}
	return;

}







/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
int CEditView::LineColmnToIndex( const char* pLine, int nLineLen, int nColmn )
{
//#ifdef _DEBUG
//	CRunningTimer cRunningTimer( (const char*)"CEditView::LineColmnToIndex" );
//#endif
	int		nPosX;
	int		i;
	int		nCharChars;
	nPosX = 0;
//	*pnLineAllColLen = 0;
	for( i = 0; i < nLineLen; ){
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
//	if( i >= nLineLen  ){
//		*pnLineAllColLen = nPosX;
//	}
	return i;
}



/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */
/* �w�肳�ꂽ�����A�s���Z���ꍇ��pnLineAllColLen�ɍs�S�̂̕\��������Ԃ� */
/* ����ȊO�̏ꍇ��pnLineAllColLen�ɂO���Z�b�g���� */
int CEditView::LineColmnToIndex2( const char* pLine, int nLineLen, int nColmn, int* pnLineAllColLen )
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( (const char*)"CEditView::LineColmnToIndex(0)" );
#endif
	int		nPosX;
	int		i;
	int		nCharChars;
//	int		bEOL = FALSE;
	nPosX = 0;
	*pnLineAllColLen = 0;
	for( i = 0; i < nLineLen; ){
//		if( bEOL ){
//			break;
//		}
//		if( pLine[i] == '\r' || pLine[i] == '\n' ){
//			bEOL = TRUE;
//		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			if( nPosX + nCharChars > nColmn ){
				break;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	if( i >= nLineLen  ){
		*pnLineAllColLen = nPosX;
	}
	return i;
}





/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
*/
int CEditView::LineIndexToColmn( const char* pLine, int nLineLen, int nIndex )
{
	int		nPosX;
	int		i;
	int		nCharChars;
	nPosX = 0;
	for( i = 0; i < nLineLen; ){
		if( i >= nIndex ){
			break;
		}
		if( pLine[i] == TAB ){
			nCharChars = m_pcEditDoc->GetDocumentAttribute().m_nTabSpace - ( nPosX % m_pcEditDoc->GetDocumentAttribute().m_nTabSpace );
			++i;
		}else{
			nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
			if( 0 == nCharChars ){
				nCharChars = 1;
			}
			i+= nCharChars;
		}
		nPosX += nCharChars;
	}
	return nPosX;
}





/* �|�b�v�A�b�v���j���[(�E�N���b�N) */
int	CEditView::CreatePopUpMenu_R( void )
{
	int			nId;
//	HMENU		hMenuTop;
	HMENU		hMenu;
	POINT		po;
//	UINT		fuFlags;
//	int			cMenuItems;
//	int			nPos;
	RECT		rc;
	CMemory		cmemCurText;
	CMemory*	pcmemRefText;
	char*		pszWork;
	int			nWorkLength;
	int			i;
	int			nMenuIdx;
	char		szLabel[300];
	char		szLabel2[300];
	UINT		uFlags;
//	BOOL		bBool;


	CEditWnd*	pCEditWnd;
	pCEditWnd = ( CEditWnd* )::GetWindowLong( ::GetParent( m_hwndParent ), GWL_USERDATA );
	pCEditWnd->m_CMenuDrawer.ResetContents();

	/* �E�N���b�N���j���[�̒�`�̓J�X�^�����j���[�z���0�Ԗ� */
	nMenuIdx = 0;
//	if( nMenuIdx < 0 || MAX_CUSTOM_MENU	<= nMenuIdx ){
//		return 0;
//	}
//	if( 0 == m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx] ){
//		return 0;
//	}



	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			::LoadString( m_hInstance, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
			/* �L�[ */
			if( '\0' == m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] ){
				strcpy( szLabel2, szLabel );
			}else{
				wsprintf( szLabel2, "%s (&%c)", szLabel, m_pShareData->m_Common.m_nCustMenuItemKeyArr[nMenuIdx][i] );
			}
			/* �@�\�����p�\�����ׂ� */
			if( TRUE == CEditWnd::IsFuncEnable( m_pcEditDoc, m_pShareData, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ) ){
				uFlags = MF_STRING | MF_ENABLED;
			}else{
				uFlags = MF_STRING | MF_DISABLED | MF_GRAYED;
			}
//			bBool = ::AppendMenu( hMenu, uFlags, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel2 );
			pCEditWnd->m_CMenuDrawer.MyAppendMenu(
				hMenu, /*MF_BYPOSITION | MF_STRING*/uFlags,
				m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] , szLabel2 );

		}
	}


	if( !m_bBeginSelect ){	/* �͈͑I�� */
		//	2001/06/14 asa-o �Q�Ƃ���f�[�^�̕ύX
//		if( m_pShareData->m_Common.m_bUseKeyWordHelp ){ /* �L�[���[�h�w���v���g�p���� */
		if( m_pcEditDoc->GetDocumentAttribute().m_bUseKeyWordHelp ){
			if( m_nCaretWidth > 0 ){					//�t�H�[�J�X������Ƃ�
				/* �E�B���h�E���Ƀ}�E�X�J�[�\�������邩�H */
				GetCursorPos( &po );
				GetWindowRect( m_hWnd, &rc );
				if( PtInRect( &rc, po ) ){
					if( m_bInMenuLoop == FALSE	//&&				/* ���j���[ ���[�_�� ���[�v�ɓ����Ă��Ȃ� */
						//0 != m_dwTipTimer		&&					/* ����Tip��\�����Ă��Ȃ� */
						//1000 < ::GetTickCount() - m_dwTipTimer	/* ��莞�Ԉȏ�A�}�E�X���Œ肳��Ă��� */
					){
						/* �I��͈͂̃f�[�^���擾(�����s�I���̏ꍇ�͐擪�̍s�̂�) */
						if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE ) ){
							pszWork = cmemCurText.GetPtr( NULL );
							nWorkLength	= lstrlen( pszWork );
							for( i = 0; i < nWorkLength; ++i ){
								if( pszWork[i] == '\0' ||
									pszWork[i] == CR ||
									pszWork[i] == LF ){
									break;
								}
							}
							char*	pszBuf = new char[i + 1];
							memcpy( pszBuf, pszWork, i );
							pszBuf[i] = '\0';
							cmemCurText.SetData( pszBuf, i );
							delete [] pszBuf;


							/* ���Ɍ����ς݂� */
							if( CMemory::IsEqual( cmemCurText, m_cTipWnd.m_cKey ) ){
								/* �Y������L�[���Ȃ����� */
								if( !m_cTipWnd.m_KeyWasHit ){
									goto end_of_search;
								}
							}else{
								m_cTipWnd.m_cKey = cmemCurText;
								/* �������s */
								//	2001/06/14 asa-o �Q�Ƃ���f�[�^�̕ύX
	//							if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
								if( m_cDicMgr.Search( cmemCurText.GetPtr( NULL ), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
									/* �Y������L�[������ */
									m_cTipWnd.m_KeyWasHit = TRUE;
									pszWork = pcmemRefText->GetPtr( NULL );
//									m_cTipWnd.m_cInfo.SetData( pszWork, lstrlen( pszWork ) );
									m_cTipWnd.m_cInfo.SetDataSz( pszWork );
									delete pcmemRefText;
								}else{
									/* �Y������L�[���Ȃ����� */
									m_cTipWnd.m_KeyWasHit = FALSE;
									goto end_of_search;
								}
							}
							m_dwTipTimer = 0;	/* ����Tip��\�����Ă��� */
							m_poTipCurPos = po;	/* ���݂̃}�E�X�J�[�\���ʒu */
	//						/* ����Tip��\�� */
	//						m_cTipWnd.Show( po.x, po.y + m_nCharHeight, NULL );
							pszWork = m_cTipWnd.m_cInfo.GetPtr(NULL);
							if( 80 < lstrlen( pszWork ) ){
								char*	pszShort = new char[80 + 1];
								memcpy( pszShort, pszWork, 80 );
								pszShort[80] = '\0';
								::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, pszShort );
								delete [] pszShort;
							}else{
								::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, pszWork );
							}
							::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL );
						}
						end_of_search:;
					}
				}
			}
		}
	}
	po.x = 0;
	po.y = 0;
	::GetCursorPos( &po );
	po.y -= 4;
	nId = ::TrackPopupMenu(
		hMenu,
		TPM_TOPALIGN
		| TPM_LEFTALIGN
		| TPM_RETURNCMD
		| TPM_LEFTBUTTON
		/*| TPM_RIGHTBUTTON*/
		,
		po.x,
		po.y,
		0,
		::GetParent( m_hwndParent )/*m_hWnd*/,
		NULL
	);
//	::DestroyMenu( hMenuTop );
	::DestroyMenu( hMenu );
//	MYTRACE( "nId=%d\n", nId );
	return nId;
}



/* �e�E�B���h�E�̃^�C�g�����X�V */
void CEditView::SetParentCaption( BOOL bKillFocus )
{
	m_pcEditDoc->SetParentCaption( bKillFocus );
	return;
}


/* �L�����b�g�̍s���ʒu��\������ */
void CEditView::DrawCaretPosInfo( void )
{
	if( !m_bDrawSWITCH ){
		return;
	}

	HDC				hdc;
	POINT			poFrame;
	POINT			po;
	RECT			rcFrame;
	HFONT			hFontOld;
	char			szText[64];
	HWND			hwndFrame;
	int				nStrLen;
	RECT			rc;
	unsigned char*	pLine;
	int				nLineLen;
	int				nIdxFrom;
	int				nCharChars;
	CEditWnd*		pCEditWnd;
	const CLayout*	pcLayout;
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Uni ",
		"UTF-8",
		"UTF-7"
	};
	char*			pCodeNameArr2[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
	int	nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );

	hwndFrame = ::GetParent( m_hwndParent );
	pCEditWnd = ( CEditWnd* )::GetWindowLong( hwndFrame, GWL_USERDATA );
	/* �J�[�\���ʒu�̕����R�[�h */
//	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen );
	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );

	//	May 12, 2000 genta
	//	���s�R�[�h�̕\����ǉ�
	//	From Here
	CEOL cNlType = m_pcEditDoc->GetNewLineCode();
	const char *nNlTypeName = cNlType.GetName();
	//	To Here


	/* �X�e�[�^�X�o�[�ɏ�Ԃ������o�� */
	if( NULL == pCEditWnd->m_hwndStatusBar ){
		hdc = ::GetWindowDC( hwndFrame );
		poFrame.x = 0;
		poFrame.y = 0;
		::ClientToScreen( hwndFrame, &poFrame );
		::GetWindowRect( hwndFrame, &rcFrame );
		po.x = rcFrame.right - rcFrame.left;
		po.y = poFrame.y - rcFrame.top;
		hFontOld = (HFONT)::SelectObject( hdc, m_hFontCaretPosInfo );
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ�
		//	From Here
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( (const char *)pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
				/* szText */
//				wsprintf( szText, "%s(%s)       %6d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
				wsprintf( szText, "%s(%s)       %6d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );	//Oct. 31, 2000 JEPRO //Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					nCharChars = CMemory::MemCharNext( (char *)pLine, nLineLen, (char *)&pLine[nIdxFrom] ) - (char *)&pLine[nIdxFrom];
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
				if( 1 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s)   [%02x]%6d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom], m_nCaretPosY + 1, m_nCaretPosX + 1 );
					wsprintf( szText, "%s(%s)   [%02x]%6d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom], m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}else
				if( 2 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s) [%02x%02x]%6d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , m_nCaretPosY + 1, m_nCaretPosX + 1);
					wsprintf( szText, "%s(%s) [%02x%02x]%6d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , m_nCaretPosY + 1, m_nCaretPosX + 1);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}else
				if( 4 == nCharChars ){
					/* szText */
//					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , m_nCaretPosY + 1, m_nCaretPosX + 1);
					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , m_nCaretPosY + 1, m_nCaretPosX + 1);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}else{
					/* szText */
//					wsprintf( szText, "%s(%s)       %6d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
					wsprintf( szText, "%s(%s)       %6d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}
			}
		}else{
			/* szText */
//			wsprintf( szText, "%s(%s)       %6d �s�A%d��          ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );
			wsprintf( szText, "%s(%s)       %6d�F%d            ", pCodeNameArr[m_pcEditDoc->m_nCharCode], nNlTypeName, m_nCaretPosY + 1, m_nCaretPosX + 1 );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
		}
		//	To Here
		/* ������`�� */
//		nStrLen = 35;
		nStrLen = 30;	//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
		rc.left = po.x - nStrLen * m_nCaretPosInfoCharWidth - 5;
		rc.right = rc.left + nStrLen * m_nCaretPosInfoCharWidth;
		rc.top = po.y - m_nCaretPosInfoCharHeight - 2;
		rc.bottom = rc.top + m_nCaretPosInfoCharHeight;
		::SetTextColor( hdc, ::GetSysColor( COLOR_MENUTEXT ) );
		::SetBkColor( hdc, ::GetSysColor( COLOR_MENU ) );
		::ExtTextOut(
			hdc,
			rc.left,
			rc.top,
			/*ETO_CLIPPED | */ ETO_OPAQUE,
			&rc,
			szText,
			nStrLen,
			m_pnCaretPosInfoDx
		);
		::SelectObject( hdc, hFontOld );
		::ReleaseDC( hwndFrame, hdc );
	}else{
		char	szText_1[64];
		char	szText_2[64];
//		char	szText_3[64];
//		char	szText_4[64];
		char	szText_5[64];
//		wsprintf( szText_1, "%8d �s %5d ��", m_nCaretPosY + 1, m_nCaretPosX + 1 );
		wsprintf( szText_1, "%6d �s %5d ��", m_nCaretPosY + 1, m_nCaretPosX + 1 );	//Oct. 30, 2000 JEPRO �疜�s���v���

		nCharChars = 0;
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( (const char *)pLine, nLineLen, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					nCharChars = CMemory::MemCharNext( (char *)pLine, nLineLen, (char *)&pLine[nIdxFrom] ) - (char *)&pLine[nIdxFrom];
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
			}
		}

		if( 1 == nCharChars ){
			wsprintf( szText_2, "%02x  ", pLine[nIdxFrom] );
		}else
		if( 2 == nCharChars ){
			wsprintf( szText_2, "%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1] );
		}else
		if( 4 == nCharChars ){
			wsprintf( szText_2, "%02x%02x%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1], pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] );
		}else{
			wsprintf( szText_2, "    " );
		}

		if( m_pShareData->m_Common.m_bIsINSMode ){
			strcpy( szText_5, "�}��" );
		}else{
			strcpy( szText_5, "�㏑" );
		}
//		::GetClientRect( pCEditWnd->m_hwndStatusBar, &rc );
//		int			nStArr[7];
//		const char*	pszLabel[6] = { "", szText_1, szText_2, pCodeNameArr2[m_pcEditDoc->m_nCharCode], "REC", szText_5 };
//		int			nStArrNum = 6;
//		int			nAllWidth;
//		SIZE		sz;
//		HDC			hdc;
//		int			i;
//		TEXTMETRIC	tm;
//		nAllWidth = rc.right - rc.left;
//		hdc = ::GetDC( pCEditWnd->m_hwndStatusBar );
//		nStArr[nStArrNum - 1] = nAllWidth;
//		if( pCEditWnd->m_nWinSizeType != SIZE_MAXIMIZED ){	/* �T�C�Y�ύX�̃^�C�v */
//			nStArr[nStArrNum - 1] -= 16;
//		}
////		::GetTextMetrics( hdc, &tm );
//		for( i = nStArrNum - 1; i > 0; i-- ){
////			sz.cx = tm.tmMaxCharWidth * lstrlen( pszLabel[i] )  / 2;
//			::GetTextExtentPoint32( hdc, pszLabel[i], lstrlen( pszLabel[i] ), &sz );
//			nStArr[i - 1] = nStArr[i] - ( sz.cx + ::GetSystemMetrics( SM_CXEDGE ) );
//		}
//		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETPARTS, nStArrNum, (LPARAM) (LPINT)nStArr );
//		::ReleaseDC( pCEditWnd->m_hwndStatusBar, hdc );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 1 | 0, (LPARAM) (LPINT)szText_1 );
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ��D���̔ԍ���1�����炷
		//	From Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 2 | 0, (LPARAM) (LPINT)nNlTypeName );
		//	To Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 3 | 0, (LPARAM) (LPINT)szText_2 );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 4 | 0, (LPARAM)(LPINT)pCodeNameArr2[m_pcEditDoc->m_nCharCode] );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 5 | SBT_OWNERDRAW, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 6 | 0, (LPARAM) (LPINT)szText_5 );
	}

	return;
}





/* �ݒ�ύX�𔽉f������ */
void CEditView::OnChangeSetting( void )
{
	RECT		rc;
	LOGFONT		lf;

	m_nTopYohaku = m_pShareData->m_Common.m_nRulerBottomSpace; 		/* ���[���[�ƃe�L�X�g�̌��� */
	m_nViewAlignTop = m_nTopYohaku;									/* �\����̏�[���W */
	/* ���[���[�\�� */
//	if( m_pShareData->m_Common.m_bRulerDisp ){
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ���[���[���� */
	}

	/* �t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN );
	m_hFont_HAN = CreateFontIndirect( &(m_pShareData->m_Common.m_lf) );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_FAT );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT = CreateFontIndirect( &lf );

	/* �����t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	m_hFont_HAN_UL = CreateFontIndirect( &lf );

	/* ���������t�H���g�쐬 */
	::DeleteObject( m_hFont_HAN_FAT_UL );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfUnderline = TRUE;
	lf.lfWeight += 300;
	if( 1000 < lf.lfWeight ){
		lf.lfWeight = 1000;
	}
	m_hFont_HAN_FAT_UL = CreateFontIndirect( &lf );


	::DeleteObject( m_hFont_ZEN );
	lf = m_pShareData->m_Common.m_lf;
	lf.lfCharSet = SHIFTJIS_CHARSET;
	lf.lfOutPrecision = 1;
	strcpy( lf.lfFaceName, "�l�r �S�V�b�N" );
	m_hFont_ZEN = CreateFontIndirect( &lf );

	/* �t�H���g�̕ύX */
	SetFont();

	/* �J�[�\���ړ� */
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �E�B���h�E�T�C�Y�̕ύX���� */
	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );


	/* �ĕ`�� */
	::InvalidateRect( m_hWnd, NULL, TRUE );

	return;
}




/* �t�H�[�J�X�ړ����̍ĕ`�� */
void CEditView::RedrawAll( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	/* �ĕ`�� */
	hdc = ::GetDC( m_hWnd );

	OnKillFocus();

//	ps.rcPaint.left = 0;
//	ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
//	ps.rcPaint.top = 0;
//	ps.rcPaint.bottom = m_nViewAlignTop + m_nViewCy;
	::GetClientRect( m_hWnd, &ps.rcPaint );
//	ps.rcPaint.right -= ps.rcPaint.left;
//	ps.rcPaint.bottom -= ps.rcPaint.top;
//	ps.rcPaint.left = 0;
//	ps.rcPaint.top = 0;

	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();
	/* �J�[�\���ړ� */
	MoveCursor( m_nCaretPosX, m_nCaretPosY, TRUE );

	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	return;
}



/* �����̕\����Ԃ𑼂̃r���[�ɃR�s�[ */
void CEditView::CopyViewStatus( CEditView* pView )
{
	if( pView == NULL ){
		return;
	}
	if( pView == this ){
		return;
	}

	/* ���͏�� */
	pView->m_nCaretPosX 			= m_nCaretPosX;			/* �r���[���[����̃J�[�\�����ʒu�i�O�J�n�j*/
	pView->m_nCaretPosX_Prev		= m_nCaretPosX_Prev;	/* �r���[���[����̃J�[�\�����ʒu�i�O�I���W���j*/
	pView->m_nCaretPosY				= m_nCaretPosY;			/* �r���[��[����̃J�[�\���s�ʒu�i�O�J�n�j*/
	pView->m_nCaretWidth			= m_nCaretWidth;		/* �L�����b�g�̕� */
	pView->m_nCaretHeight			= m_nCaretHeight;		/* �L�����b�g�̍��� */

	/* �L�[��� */
	pView->m_bSelectingLock			= m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	pView->m_bBeginSelect			= m_bBeginSelect;		/* �͈͑I�� */
	pView->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* ��`�͈͑I�� */

//	pView->m_nSelectLineBgn			= m_nSelectLineBgn;		/* �͈͑I���J�n�s(���_) */
//	pView->m_nSelectColmBgn			= m_nSelectColmBgn;		/* �͈͑I���J�n��(���_) */
	pView->m_nSelectLineBgnFrom		= m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
	pView->m_nSelectColmBgnFrom		= m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
	pView->m_nSelectLineBgnTo		= m_nSelectLineBgnTo;	/* �͈͑I���J�n�s(���_) */
	pView->m_nSelectColmBgnTo		= m_nSelectColmBgnTo;	/* �͈͑I���J�n��(���_) */

	pView->m_nSelectLineFrom		= m_nSelectLineFrom;	/* �͈͑I���J�n�s */
	pView->m_nSelectColmFrom		= m_nSelectColmFrom;	/* �͈͑I���J�n�� */
	pView->m_nSelectLineTo			= m_nSelectLineTo;		/* �͈͑I���I���s */
	pView->m_nSelectColmTo			= m_nSelectColmTo;		/* �͈͑I���I���� */
	pView->m_nSelectLineFromOld		= m_nSelectLineFromOld;	/* �͈͑I���J�n�s */
	pView->m_nSelectColmFromOld		= m_nSelectColmFromOld;	/* �͈͑I���J�n�� */
	pView->m_nSelectLineToOld		= m_nSelectLineToOld;	/* �͈͑I���I���s */
	pView->m_nSelectColmToOld		= m_nSelectColmToOld;	/* �͈͑I���I���� */
	pView->m_nMouseRollPosXOld		= m_nMouseRollPosXOld;	/* �}�E�X�͈͑I��O��ʒu(X���W) */
	pView->m_nMouseRollPosYOld		= m_nMouseRollPosYOld;	/* �}�E�X�͈͑I��O��ʒu(Y���W) */

	/* ��ʏ�� */
	pView->m_nViewAlignLeft			= m_nViewAlignLeft;		/* �\����̍��[���W */
	pView->m_nViewAlignLeftCols		= m_nViewAlignLeftCols;	/* �s�ԍ���̌��� */
	pView->m_nViewAlignTop			= m_nViewAlignTop;		/* �\����̏�[���W */
//	pView->m_nViewCx				= m_nViewCx;			/* �\����̕� */
//	pView->m_nViewCy				= m_nViewCy;			/* �\����̍��� */
//	pView->m_nViewColNum			= m_nViewColNum;		/* �\����̌��� */
//	pView->m_nViewRowNum			= m_nViewRowNum;		/* �\����̍s�� */
	pView->m_nViewTopLine			= m_nViewTopLine;		/* �\����̈�ԏ�̍s(0�J�n) */
	pView->m_nViewLeftCol			= m_nViewLeftCol;		/* �\����̈�ԍ��̌�(0�J�n) */

	/* �\�����@ */
	pView->m_nCharWidth				= m_nCharWidth;			/* ���p�����̕� */
	pView->m_nCharHeight			= m_nCharHeight;		/* �����̍��� */

	return;
}


/* �c�E���̕����{�b�N�X�E�T�C�Y�{�b�N�X�̂n�m�^�n�e�e */
void CEditView::SplitBoxOnOff( BOOL bVert, BOOL bHorz, BOOL bSizeBox )
{
	RECT	rc;
	if( bVert ){
		if( NULL != m_pcsbwVSplitBox ){	/* ���������{�b�N�X */
		}else{
			m_pcsbwVSplitBox = new CSplitBoxWnd;
			m_pcsbwVSplitBox->Create( m_hInstance, m_hWnd, TRUE );
		}
	}else{
		if( NULL != m_pcsbwVSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwVSplitBox;
			m_pcsbwVSplitBox = NULL;
		}else{
		}
	}
	if( bHorz ){
		if( NULL != m_pcsbwHSplitBox ){	/* ���������{�b�N�X */
		}else{
			m_pcsbwHSplitBox = new CSplitBoxWnd;
			m_pcsbwHSplitBox->Create( m_hInstance, m_hWnd, FALSE );
		}
	}else{
		if( NULL != m_pcsbwHSplitBox ){	/* ���������{�b�N�X */
			delete m_pcsbwHSplitBox;
			m_pcsbwHSplitBox = NULL;
		}else{
		}
	}

	if( bSizeBox ){
		if( NULL != m_hwndSizeBox ){
			::DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,													/* no extended styles */
			"SCROLLBAR",										/* scroll bar control class */
			(LPSTR) NULL,										/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,													/* horizontal position */
			0,													/* vertical position */
			200,												/* width of the scroll bar */
			CW_USEDEFAULT,										/* default height */
			m_hWnd,												/* handle of main window */
			(HMENU) NULL,										/* no menu for a scroll bar */
			m_hInstance,										/* instance owning this window */
			(LPVOID) NULL										/* pointer not needed */
		);
	}else{
		if( NULL != m_hwndSizeBox ){
			DestroyWindow( m_hwndSizeBox );
			m_hwndSizeBox = NULL;
		}
		m_hwndSizeBox = ::CreateWindowEx(
			0L,														/* no extended styles */
			"STATIC",												/* scroll bar control class */
			(LPSTR) NULL,											/* text for window title bar */
			WS_VISIBLE | WS_CHILD /*| SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,														/* horizontal position */
			0,														/* vertical position */
			200,													/* width of the scroll bar */
			CW_USEDEFAULT,											/* default height */
			m_hWnd,													/* handle of main window */
			(HMENU) NULL,											/* no menu for a scroll bar */
			m_hInstance,											/* instance owning this window */
			(LPVOID) NULL											/* pointer not needed */
		);
	}
	::ShowWindow( m_hwndSizeBox, SW_SHOW );

	::GetClientRect( m_hWnd, &rc );
	OnSize( rc.right, rc.bottom );

	return;
}





/* Grep���s */
DWORD CEditView::DoGrep(
	CMemory*	pcmGrepKey,
	CMemory*	pcmGrepFile,
	CMemory*	pcmGrepFolder,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle
)
{
#ifdef _DEBUG
	CRunningTimer cRunningTimer( "CEditView::DoGrep" );
#endif
	m_pcEditDoc->m_bGrepRunning = TRUE;


	int			nDummy;
	int			nHitCount = 0;
	char		szKey[_MAX_PATH];
	char		szFile[_MAX_PATH];
	char		szPath[_MAX_PATH];
//	int			nNewLine;		/* �}�����ꂽ�����̎��̈ʒu�̍s */
//	int			nNewPos;		/* �}�����ꂽ�����̎��̈ʒu�̃f�[�^�ʒu */
//	COpe*		pcOpe = NULL;
	CDlgCancel	cDlgCancel;
	HWND		hwndCancel;
	char*		pszWork;
	HWND		hwndMainFrame;
	int			nCharChars;
	CJre		cJre;
	CMemory		cmemMessage;
	CMemory		cmemWork;
	int			nWork;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	cmemMessage.AllocBuffer( 64000 );



//	int*				pnKey_CharUsedArr;
//	pnKey_CharUsedArr		= NULL;
//	GrepParam*			pGrepParam;

//	CEditView*			pCEditView;
//	CMemory				cmGrepKey;
//	CMemory				cmGrepFile;
//	CMemory				cmGrepFolder;
//	BOOL				bGrepSubFolder;
//	BOOL				bGrepLoHiCase;
//	BOOL				bGrepRegularExp;
//	BOOL				bKanjiCode_AutoDetect;
//	BOOL				bGrepOutputLine;

//	pGrepParam				= (GrepParam*)dwGrepParam;

//	pCEditView				= (CEditView*)pGrepParam->pCEditView;
//	cmGrepKey				= *pGrepParam->pcmGrepKey;
//	cmGrepFile				= *pGrepParam->pcmGrepFile;
//	cmGrepFolder			= *pGrepParam->pcmGrepFolder;
//	bGrepSubFolder			= pGrepParam->bGrepSubFolder;
//	bGrepLoHiCase			= pGrepParam->bGrepLoHiCase;
//	bGrepRegularExp			= pGrepParam->bGrepRegularExp;
//	bKanjiCode_AutoDetect	= pGrepParam->bKanjiCode_AutoDetect;
//	bGrepOutputLine			= pGrepParam->bGrepOutputLine;

	m_bDoing_UndoRedo		= TRUE;


	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	m_pcOpeBlk = new COpeBlk;

	m_bCurSrchKeyMark = TRUE;								/* ����������̃}�[�N */
	strcpy( m_szCurSrchKey, pcmGrepKey->GetPtr( NULL ) );	/* ���������� */
	m_bCurSrchRegularExp = bGrepRegularExp;					/* �����^�u��  1==���K�\�� */
	m_bCurSrchLoHiCase = bGrepLoHiCase;						/* �����^�u��  1==�p�啶���������̋�� */
	/* ���K�\�� */
	if( m_bCurSrchRegularExp ){
		/* CJre�N���X�̏����� */
		m_CurSrch_CJre.Init();
		/* jre32.dll�̑��݃`�F�b�N */
		if( FALSE == m_CurSrch_CJre.IsExist() ){
			::MessageBox( m_hWnd, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
			return 0;
		}

		/* �����p�^�[���̃R���p�C�� */
		m_CurSrch_CJre.Compile( m_szCurSrchKey );
	}

//�܂� m_bCurSrchWordOnly = m_pShareData->m_Common.m_bWordOnly;	/* �����^�u��  1==�P��̂݌��� */
//	if( m_bCurSrchRegularExp ){
//		/* jre32.dll�̑��݃`�F�b�N */
//		if( FALSE == m_CurSrch_CJre.IsExist() ){
//			m_bCurSrchKeyMark = FALSE;
//		}else{
//			/* �����p�^�[���̃R���p�C�� */
//			if( !m_CurSrch_CJre.Compile( m_szCurSrchKey ) ){
//				m_bCurSrchKeyMark = FALSE;
//			}
//		}
//	}

//	cDlgCancel.Create( m_hInstance, m_hwndParent );
//	hwndCancel = cDlgCancel.Open( MAKEINTRESOURCE(IDD_GREPRUNNING) );
	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );

	pszWork = pcmGrepKey->GetPtr( NULL );
	strcpy( szKey, pszWork );

	strcpy( m_pcEditDoc->m_szGrepKey, szKey );
	m_pcEditDoc->m_bGrepMode = TRUE;

//	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG, (LPARAM)::LoadIcon( m_hInstance, IDI_QUESTION ) );
	HICON	hIcon;
//	hIcon = ::LoadIcon( NULL, IDI_QUESTION );
	hIcon = ::LoadIcon( m_hInstance, MAKEINTRESOURCE( IDI_ICON_GREP ) );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_SMALL,	(LPARAM)NULL );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_SMALL,	(LPARAM)hIcon );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG,	(LPARAM)NULL );
	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG,	(LPARAM)hIcon );

	pszWork = pcmGrepFolder->GetPtr( NULL );
	strcpy( szPath, pszWork );
	nDummy = lstrlen( szPath );
	/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
	nCharChars = &szPath[nDummy] - CMemory::MemCharPrev( szPath, nDummy, &szPath[nDummy] );
	if( 1 == nCharChars && szPath[nDummy - 1] == '\\' ){
	}else{
		strcat( szPath, "\\" );
	}
	strcpy( szFile, pcmGrepFile->GetPtr( &nDummy ) );



	/* �Ō�Ƀe�L�X�g��ǉ� */
	cmemMessage.AppendSz( "��������  " );
	if( 0 < lstrlen( szKey ) ){
		CMemory cmemWork2;
		cmemWork2.SetDataSz( szKey );
		if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace( "\\", "\\\\" );
			cmemWork2.Replace( "\'", "\\\'" );
			cmemWork2.Replace( "\"", "\\\"" );
		}else{
			cmemWork2.Replace( "\'", "\'\'" );
			cmemWork2.Replace( "\"", "\"\"" );
		}
		cmemWork.AppendSz( "\"" );
		cmemWork.Append( &cmemWork2 );
		cmemWork.AppendSz( "\"\r\n" );
	}else{
		cmemWork.AppendSz( "�u�t�@�C�������v\r\n" );
	}
	cmemMessage += cmemWork;



	cmemMessage.AppendSz( "�����Ώ�   " );
	cmemWork.SetDataSz( szFile );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;




	cmemMessage.AppendSz( "\r\n" );
	cmemMessage.AppendSz( "�t�H���_   " );
	cmemWork.SetDataSz( szPath );
	if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	}else{
	}
	cmemMessage += cmemWork;
	cmemMessage.AppendSz( "\r\n" );

	if( bGrepSubFolder ){
		pszWork = "    (�T�u�t�H���_������)\r\n";
	}else{
		pszWork = "    (�T�u�t�H���_���������Ȃ�)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( bGrepLoHiCase ){
		pszWork = "    (�p�啶������������ʂ���)\r\n";
	}else{
		pszWork = "    (�p�啶������������ʂ��Ȃ�)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( bGrepRegularExp ){
		cmemMessage.AppendSz( "    (���K�\��)\r\n" );
	}

	if( bKanjiCode_AutoDetect ){
		cmemMessage.AppendSz( "    (�����R�[�h�Z�b�g�̎�������)\r\n" );
	}

	if( bGrepOutputLine ){
	/* �Y���s */
		pszWork = "    (��v�����s���o��)\r\n";
	}else{
		pszWork = "    (��v�����ӏ��̂ݏo��)\r\n";
	}
	cmemMessage.AppendSz( pszWork );



	pszWork = "\r\n\r\n";
	cmemMessage.AppendSz( "\r\n\r\n" );
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
	}
	if( bGrepRegularExp ){
		/* CJre�N���X�̏����� */
		cJre.Init();
		/* jre32.dll�̑��݃`�F�b�N */
		if( FALSE == cJre.IsExist() ){
			::MessageBox( 0, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
			return 0;
		}
		/* �����p�^�[���̃R���p�C�� */
		if( !cJre.Compile( szKey ) ){
			return 0;
		}
	}else{
		/* ���������̏�� */
		CDocLineMgr::CreateCharCharsArr(
			(const unsigned char *)szKey,
			lstrlen( szKey ),
			&pnKey_CharCharsArr
		);
//		/* ���������̏��(�L�[������̎g�p�����\)�쐬 */
//		CDocLineMgr::CreateCharUsedArr(
//			(const unsigned char *)szKey,
//			lstrlen( szKey ),
//			pnKey_CharCharsArr,
//			&pnKey_CharUsedArr
//		);

	}

	/* �\������ON/OFF */
	m_bDrawSWITCH = FALSE;



	if( -1 == DoGrepTree(
		&cDlgCancel, hwndCancel, szKey,
		pnKey_CharCharsArr,
//		pnKey_CharUsedArr,
		szFile, szPath, bGrepSubFolder, bGrepLoHiCase,
		bGrepRegularExp, bKanjiCode_AutoDetect,
		bGrepOutputLine, nGrepOutputStyle, &cJre, 0, &nHitCount
	) ){
		wsprintf( szPath, "���f���܂����B\r\n", nHitCount );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
	}
	wsprintf( szPath, "%d ����������܂����B\r\n", nHitCount );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
	Command_GOFILEEND( FALSE );

#ifdef _DEBUG
	wsprintf( szPath, "��������: %d�~���b\r\n", cRunningTimer.Read() );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
	Command_GOFILEEND( FALSE );
#endif

	cDlgCancel.CloseDialog( 0 );

	/* �A�N�e�B�u�ɂ��� */
	hwndMainFrame = ::GetParent( m_hwndParent );
	/* �A�N�e�B�u�ɂ��� */
	ActivateFrameWindow( hwndMainFrame );


	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != m_pcOpeBlk ){
		if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
			/* ����̒ǉ� */
			m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
		}else{
			delete m_pcOpeBlk;
		}
		m_pcOpeBlk = NULL;
	}

	//	Apr. 13, 2001 genta
	//	Grep���s��̓t�@�C����ύX�����̏�Ԃɂ���D
	m_pcEditDoc->m_bIsModified = FALSE;
	
	m_pcEditDoc->m_bGrepRunning = FALSE;
	m_bDoing_UndoRedo = FALSE;

	if( NULL != pnKey_CharCharsArr ){
		delete [] pnKey_CharCharsArr;
		pnKey_CharCharsArr = NULL;
	}
//	if( NULL != pnKey_CharUsedArr ){
//		delete [] pnKey_CharUsedArr;
//		pnKey_CharUsedArr = NULL;
//	}

	/* �\������ON/OFF */
	m_bDrawSWITCH = TRUE;

	/* �t�H�[�J�X�ړ����̍ĕ`�� */
	RedrawAll();

	return nHitCount;
}



/* Grep���s */
int CEditView::DoGrepTree(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
//	int*		pnKey_CharUsedArr,
	const char*	pszFile,
	const char*	pszPath,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle,
	CJre*		pCJre,
	int			nNest,
	int*		pnHitCount
)
{
	int				nPos;
	char			szFile[_MAX_PATH];
	char			szPath[_MAX_PATH];
	char			szPath2[_MAX_PATH];
//	char			szTab[64];
	int				nFileLen;
	char*			pszToken;
	HANDLE			hFind;
//	int				i;
	int				nRet;
	CMemory			cmemMessage;
	int				nHitCountOld;
	char*			pszWork;
	int				nWork;
	WIN32_FIND_DATA	w32fd;
	nHitCountOld = -100;
//	MSG msg;
//	BOOL ret;

	::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );

	strcpy( szFile, pszFile );
	nFileLen = lstrlen( szFile );
	if( TRUE == bGrepSubFolder ){
		strcpy( szPath, pszPath );
		strcat( szPath, "*.*" );
		hFind = ::FindFirstFile( szPath, &w32fd );
		if( INVALID_HANDLE_VALUE == hFind ){
		}else{
			do{
				/* �������̃��[�U�[������\�ɂ��� */
				if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
					goto cancel_return;
				}
				/* ���f�{�^�������`�F�b�N */
				if( pcDlgCancel->IsCanceled() ){
					goto cancel_return;
				}
				if( w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY &&
					0 != strcmp( w32fd.cFileName, "." ) &&
					0 != strcmp( w32fd.cFileName, ".." )
				){
//					szTab[0] = '\0';
//					for( i= 0; i < nNest; ++i ){
//						strcat( szTab, "\t" );
//					}
					strcpy( szPath2, pszPath );
					strcat( szPath2, w32fd.cFileName );
					strcat( szPath2, "\\" );
					if( -1 == DoGrepTree(
						pcDlgCancel, hwndCancel,
						pszKey,
						pnKey_CharCharsArr,
//						pnKey_CharUsedArr,
						pszFile, szPath2,
						bGrepSubFolder, bGrepLoHiCase,
						bGrepRegularExp, bKanjiCode_AutoDetect,
						bGrepOutputLine, nGrepOutputStyle, pCJre, nNest + 1, pnHitCount
					) ){
						goto cancel_return;
					}
				}
			}while( TRUE == ::FindNextFile( hFind, &w32fd ) );
			::FindClose( hFind );
		}
	}
	nPos = 0;
	pszToken = my_strtok( szFile, nFileLen, &nPos, " ;," );
	while( NULL != pszToken ){
		strcpy( szPath, pszPath );
		strcat( szPath, pszToken );
		hFind = ::FindFirstFile( szPath, &w32fd );
		if( INVALID_HANDLE_VALUE == hFind ){
			goto last_of_this_loop;
		}
		do{
			/* �������̃��[�U�[������\�ɂ��� */
			if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
				goto cancel_return;
			}
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				goto cancel_return;
			}
			if( (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ){
			}else{
//				::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, szPath2 );
				::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, w32fd.cFileName );

				wsprintf( szPath2, "%s%s", pszPath, w32fd.cFileName );
				/* �t�@�C�����̌��� */
				nRet = DoGrepFile(
					pcDlgCancel, hwndCancel, pszKey,
					pnKey_CharCharsArr,
//					pnKey_CharUsedArr,
					pszFile, szPath2,
					bGrepSubFolder, bGrepLoHiCase,
					bGrepRegularExp, bKanjiCode_AutoDetect,
					bGrepOutputLine, nGrepOutputStyle,
					pCJre, nNest, pnHitCount, szPath2, cmemMessage
				);
//				::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				if( *pnHitCount - nHitCountOld  >= 10 ){
					/* ���ʏo�� */
					pszWork = cmemMessage.GetPtr( &nWork );
					if( 0 < nWork ){
						Command_ADDTAIL( pszWork, nWork );
						Command_GOFILEEND( FALSE );
						/* ���ʊi�[�G���A���N���A */
						cmemMessage.SetDataSz( "" );
					}
					nHitCountOld = *pnHitCount;
				}
				if( -1 == nRet ){
					goto cancel_return;
				}
			}
		}while( TRUE == ::FindNextFile( hFind, &w32fd ) );
		::FindClose( hFind );
last_of_this_loop:;
		pszToken = my_strtok( szFile, nFileLen, &nPos, " ;," );
	}
	/* ���ʏo�� */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* ���ʊi�[�G���A���N���A */
		cmemMessage.SetDataSz( "" );
	}
	return 0;
cancel_return:;
	/* ���ʏo�� */
	pszWork = cmemMessage.GetPtr( &nWork );
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
		Command_GOFILEEND( FALSE );
		/* ���ʊi�[�G���A���N���A */
		cmemMessage.SetDataSz( "" );
	}
	return -1;

}







/* Grep���s */
int CEditView::DoGrepFile(
	CDlgCancel* pcDlgCancel,
	HWND		hwndCancel,
	const char*	pszKey,
	int*		pnKey_CharCharsArr,
//	int*		pnKey_CharUsedArr,
	const char*	pszFile,
	const char*	pszPath,
	BOOL		bGrepSubFolder,
	BOOL		bGrepLoHiCase,
	BOOL		bGrepRegularExp,
	BOOL		bKanjiCode_AutoDetect,
	BOOL		bGrepOutputLine,
	int			nGrepOutputStyle,
	CJre*		pCJre,
	int			nNest,
	int*		pnHitCount,
	const char*	pszFullPath,
	CMemory&	cmemMessage
)
{
	int		nHitCount;
	char	szLine[16000];
	char	szWork[16500];
	char	szWork0[_MAX_PATH + 100];
	HFILE	hFile;
	int		nLine;
//	CJre	cJre;
	char*	pszRes;
	CMemory	cmemBuf;
//	CMemory	cmemLine;
	int		nCharCode;
	char*	pCompareData;
	char*	pszCodeName;
	int		nFileLength;
	int		nReadLength;
	int		nReadSize;
	HGLOBAL	hgRead = NULL;;
	char*	pBuf;
	int		nEOF;
	int		nBgn;
	int		nPos;
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
//	CMemory	cmemMessage;
//	char*	pszWork;
//	int		nWork;
	int		nLineLen;
	const	char*	pLine;
//	int		nLineLen;
//	enumEOLType nEOLType;
	CEOL	cEol;
	int		nEolCodeLen;
	int		k;
//	int		nLineNum;






//	if( bGrepRegularExp ){
//		/* CJre�N���X�̏����� */
//		cJre.Init();
//
//		/* jre32.dll�̑��݃`�F�b�N */
//		if( FALSE == cJre.IsExist() ){
//			::MessageBox( 0, "jre32.dll��������܂���B\n���K�\���𗘗p����ɂ�jre32.dll���K�v�ł��B\n", "���", MB_OK | MB_ICONEXCLAMATION );
//			return -1;
//		}
//		/* �����p�^�[���̃R���p�C�� */
//		if( !cJre.Compile( pszKey ) ){
//			return -1;
//		}
//	}

	nCharCode = 0;
	pszCodeName = "";
	if( bKanjiCode_AutoDetect ){
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
		nCharCode = CMemory::CheckKanjiCodeOfFile( pszFullPath );
		if( -1 == nCharCode ){
			wsprintf( szLine, "�����R�[�h�̔��ʏ����ŃG���[ [%s]\r\n", pszFullPath );
			Command_ADDTAIL( szLine, lstrlen( szLine ) );
			return 0;
		}
		pszCodeName = (char*)gm_pszCodeNameArr_3[nCharCode];
//		switch( nCharCode ){
//		case CODE_SJIS:	/* SJIS */
//			pszCodeName = "[SJIS]";
//			break;
//		case CODE_JIS:	/* JIS */
//			pszCodeName = "[JIS]";
//			break;
//		case CODE_EUC:	/* EUC */
//			pszCodeName = "[EUC]";
//			break;
//		case CODE_UNICODE:	/* Unicode */
//			pszCodeName = "[Unicode]";
//			break;
//		case CODE_UTF8:	/* UTF-8 */
//			pszCodeName = "[UTF-8]";
//			break;
//		case CODE_UTF7:	/* UTF-7 */
//			pszCodeName = "[UTF-7]";
//			break;
//		default:
//			wsprintf( szLine, "�����R�[�h�̔��ʂ��Ȃ񂩃w���Ȍ��ʁB�o�O�B[%s]\r\n", pszFullPath );
//			Command_ADDTAIL( szLine, lstrlen( szLine ) );
//			return 0;
//		}
	}
	nHitCount = 0;
	nLine = 0;
	hFile = _lopen( pszPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		wsprintf( szLine, "file open error [%s]\r\n", pszFullPath );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		return 0;
	}

	/* �t�@�C���T�C�Y�̎擾 */
	nFileLength = _llseek( hFile, 0, FILE_END );
	_llseek( hFile, 0, FILE_BEGIN );

	hgRead = ::GlobalAlloc( GHND, nFileLength + 1 );
	if( NULL == hgRead ){
		wsprintf( szLine, "CEditView::DoGrepFile()\n[%s] �������m�ۂɎ��s���܂����B\n%d�o�C�g \r\n", pszFullPath, nFileLength );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		_lclose( hFile );
		return 0;
	}
	pBuf = (char*)::GlobalLock( hgRead );
	nEOF = TRUE;
	if( nCharCode == 3 ){ /* Unicode */
		nReadSize = _lread( hFile, pBuf, 2);
	}
	nReadLength = 0;
	nBgn = 0;
	nPos = 0;
	nReadSize = _lread( hFile, pBuf, nFileLength/*nReadBufSize*/);
	pBuf[nFileLength] = '\0';

	if( HFILE_ERROR == nReadSize ){
		wsprintf( szLine, "file read error %s\r\n", pszFullPath );
		Command_ADDTAIL( szLine, lstrlen( szLine ) );
		_lclose( hFile );
		return 0;
	}
	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
		return -1;
	}
	/* ���f�{�^�������`�F�b�N */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	switch( nCharCode ){
	case CODE_EUC /* EUC */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* EUC��SJIS�R�[�h�ϊ� */
		cmemBuf.EUCToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_JIS /* JIS */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
		cmemBuf.JIStoSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UNICODE /* Unicode */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* Unicode��SJIS�R�[�h�ϊ� */
		cmemBuf.UnicodeToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF8 /* UTF-8 */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-8��SJIS�R�[�h�ϊ� */
		cmemBuf.UTF8ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_UTF7 /* UTF-7 */:
		cmemBuf.SetData( pBuf, nReadSize );
		/* UTF-7��SJIS�R�[�h�ϊ� */
		cmemBuf.UTF7ToSJIS();
		memcpy( pBuf, cmemBuf.GetPtr( NULL ), cmemBuf.GetLength() );
		nReadSize = cmemBuf.GetLength();
		break;
	case CODE_SJIS /* SJIS */:
	default:
		break;
	}


	int	nKeyKen = lstrlen( pszKey );

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	if( 0 == nKeyKen ){
		if( 1 == nGrepOutputStyle ){
		/* �m�[�}�� */
			wsprintf( szWork0, "%s %s\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ�� */
			wsprintf( szWork0, "��\"%s\" %s\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendSz( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}
	wsprintf( szWork0, "��\"%s\" %s\r\n", pszFullPath, pszCodeName );


	nBgn = 0;
//	nPos = 0;
	nLine = 0;
	while( NULL != ( pLine = GetNextLine( pBuf, nReadSize, &nLineLen, &nBgn, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLine;
//		AddLineStrX( pLine, nLineLen + nEolCodeLen, nEOLType );
		nReadLength += nLineLen + nEolCodeLen;
		nLineLen += nEolCodeLen;
		pCompareData = (char*)pLine;

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
		}

		if( bGrepRegularExp ){
			if( NULL != ( pszRes = (char *)pCJre->GetMatchInfo( pCompareData, nLineLen, 0 ) )
			){
				nColm = pszRes - pCompareData + 1;


//				if( nLineLen > sizeof( szLine ) - 10 ){
//					nLineLen = sizeof( szLine ) - 10;
//				}
//				memcpy( szLine, pCompareData, nLineLen - 1 );
//				szLine[nLineLen - 1] = '\0';

				if( bGrepOutputLine ){
				/* �Y���s */
					k = nLineLen - nEolCodeLen;
					if( k > 1000 ){
						k = 1000;
					}
					pCompareData[k] = '\0';

					if( 1 == nGrepOutputStyle ){
					/* �m�[�}�� */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pCompareData );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						wsprintf( szWork, "�E(%6d,%-5d): %s\r\n", nLine, nColm, pCompareData );
					}
				}else{
				/* �Y������ */
					char* pszHit;
					pszHit = new char[pCJre->m_jreData.nLength + 1];
					memcpy( pszHit, pszRes, pCJre->m_jreData.nLength );
					pszHit[pCJre->m_jreData.nLength] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* �m�[�}�� */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pszHit );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						wsprintf( szWork, "�E(%6d,%-5d): %s\r\n", nLine, nColm, pszHit );
					}
					delete [] pszHit;
				}
				if( 2 == nGrepOutputStyle ){
				/* WZ�� */
					if( !bOutFileName ){
						cmemMessage.AppendSz( szWork0 );
						bOutFileName = TRUE;
					}
				}
				cmemMessage.AppendSz( szWork );
				++nHitCount;
				++(*pnHitCount);
				if( 0 == ( (*pnHitCount) % 16 ) ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				}
			}
		}else{
			/* �����񌟍� */
			if( NULL != ( pszRes = (char *)CDocLineMgr::SearchString(
				(const unsigned char *)pCompareData, nLineLen,
				0,
				(const unsigned char *)pszKey, nKeyKen,
				pnKey_CharCharsArr,
//				pnKey_CharUsedArr,
				bGrepLoHiCase
			) ) ){
				nColm = pszRes - pCompareData + 1;

//				if( nLineLen > sizeof( szLine ) - 10 ){
//					nLineLen = sizeof( szLine ) - 10;
//				}
//				memcpy( szLine, pCompareData, nLineLen - 1 );
//				szLine[nLineLen - 1] = '\0';
//				memcpy( szLine, pCompareData, nLineLen - nEolCodeLen );
//				szLine[nLineLen - nEolCodeLen] = '\0';
//				pCompareData = szLine;

				if( bGrepOutputLine ){
				/* �Y���s */
					k = nLineLen - nEolCodeLen;
					if( k > 1000 ){
						k = 1000;
					}
					pCompareData[k] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* �m�[�}�� */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pCompareData );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						wsprintf( szWork, "�E(%6d,%-5d): %s\r\n", nLine, nColm, pCompareData );
					}
				}else{
				/* �Y������ */
					char* pszHit;
					pszHit = new char[nKeyKen + 1];
					memcpy( pszHit, pszRes, nKeyKen );
					pszHit[nKeyKen] = '\0';
					if( 1 == nGrepOutputStyle ){
					/* �m�[�}�� */
						wsprintf( szWork, "%s(%d,%d)%s: %s\r\n", pszFullPath, nLine, nColm, pszCodeName, pszHit );
					}else
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						wsprintf( szWork, "�E(%6d,%-5d): %s\r\n", nLine, nColm, pszHit );
					}
					delete [] pszHit;
				}
				if( 2 == nGrepOutputStyle ){
				/* WZ�� */
					if( !bOutFileName ){
						cmemMessage.AppendSz( szWork0 );
						bOutFileName = TRUE;
					}
				}
				cmemMessage.AppendSz( szWork );
				++nHitCount;
				++(*pnHitCount);
				//	May 22, 2000 genta
				// if( 0 == ( (*pnHitCount) % 16 ) ){
					::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
				// }
			}
		}
	}

	if( NULL != hgRead ){
		::GlobalUnlock( hgRead );
		::GlobalFree( hgRead );
	}
	_lclose( hFile );
	return nHitCount;
}


/*
	�J�[�\�����O�̒P����擾 �P��̒�����Ԃ��܂�
	�P���؂�
*/
int CEditView::GetLeftWord( CMemory* pcmemWord, int nMaxWordLen )
{
	const char*	pLine;
	int			nLineLen;
	int			nIdx;
	int			nIdxTo;
	int			nLineFrom;
	int			nColmFrom;
	int			nLineTo;
	int			nColmTo;
	CMemory		cmemWord;
	int			nCurLine;
	int			nCharChars;

	nCurLine = m_nCaretPosY;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
	if( NULL == pLine ){
//		return 0;
	}else{
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );
	}
	if( 0 == nIdxTo || NULL == pLine ){
		if( nCurLine <= 0 ){
			return 0;
		}
		nCurLine--;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen );
		if( NULL == pLine ){
			return 0;
		}
		if( pLine[nLineLen - 1] == '\r' || pLine[nLineLen - 1] == '\n' ){
			return 0;
		}
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
//		nIdxTo = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );

		nCharChars = &pLine[nLineLen] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nLineLen] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdxTo = nLineLen;
		nIdx = nIdxTo - nCharChars;

//		nIdx = nIdxTo = nLineLen - 1;
	}else{
		nCharChars = &pLine[nIdxTo] - CMemory::MemCharPrev( pLine, nLineLen, &pLine[nIdxTo] );
		if( 0 == nCharChars ){
			return 0;
		}
		nIdx = nIdxTo - nCharChars;
	}
	if( 1 == nCharChars ){
		if( pLine[nIdx] == SPACE || pLine[nIdx] == TAB ){
			return 0;
		}
	}
	if( 2 == nCharChars ){
		if( (unsigned char)pLine[nIdx	 ] == (unsigned char)0x81 &&
			(unsigned char)pLine[nIdx + 1] == (unsigned char)0x40
		){
			return 0;
		}
	}


//	nIdx = nIdxTo - 1;
//	if( 0 > nIdx ){
//		return 0;
//	}
//	if( pLine[nIdx] == SPACE ||
//		pLine[nIdx] == TAB ||
//		(
//			nIdx > 0 &&
//			pLine[nIdx] == 0x40 &&
//			(unsigned char)pLine[nIdx - 1] == (unsigned char)0x81
//		)
//	){
//		return 0;
//	}
//	while( 0 <= nIdx && ( pLine[nIdx] != SPACE && pLine[nIdx] != TAB ) ){
//		nIdx--;
//	}
//	++nIdx;
//	if( nIdxTo - nIdx > 0 ){
//		if( nMaxWordLen < nIdxTo - nIdx ){
//			return 0;
//		}else{
//			pcmemWord->SetData( &pLine[nIdx], nIdxTo - nIdx );
//			return nIdxTo - nIdx;
//		}
//	}else{
//		return 0;
//	}



	/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
	if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
		nCurLine, nIdx,
		&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, &cmemWord, pcmemWord )
	){
		pcmemWord->Append( &pLine[nIdx], nCharChars );
//		MYTRACE( "==========\n" );
//		MYTRACE( "cmemWord=[%s]\n", cmemWord.GetPtr( NULL ) );
//		MYTRACE( "pcmemWord=[%s]\n", pcmemWord->GetPtr( NULL ) );

		return pcmemWord->GetLength();
	}else{
		return 0;
	}
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int CEditView::IsCurrentPositionSelected(
	int		nCaretPosX,		// �J�[�\���ʒuX
	int		nCaretPosY		// �J�[�\���ʒuY
)
{
	if( FALSE == IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}
	RECT	rcSel;
	POINT	po;


	/* ��`�͈͑I�𒆂� */
	if( m_bBeginBoxSelect ){
		/* 2�_��Ίp�Ƃ����`�����߂� */
		TwoPointToRect(
			&rcSel,
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo			/* �͈͑I���I���� */
		);
		++rcSel.bottom;
		po.x = nCaretPosX;
		po.y = nCaretPosY;
		if( m_bDragSource ){
			if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){ /* Ctrl�L�[��������Ă����� */
				++rcSel.left;
			}else{
				++rcSel.right;
			}
		}
		if( PtInRect( &rcSel, po ) ){
			return 0;
		}
		if( rcSel.top > nCaretPosY ){
			return -1;
		}
		if( rcSel.bottom < nCaretPosY ){
			return 1;
		}
		if( rcSel.left > nCaretPosX ){
			return -1;
		}
		if( rcSel.right < nCaretPosX ){
			return 1;
		}
	}else{
		if( m_nSelectLineFrom > nCaretPosY ){
			return -1;
		}
		if( m_nSelectLineTo < nCaretPosY ){
			return 1;
		}
		if( m_nSelectLineFrom == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrl�L�[��������Ă����� */
					if( m_nSelectColmFrom >= nCaretPosX ){
						return -1;
					}
				}else{
					if( m_nSelectColmFrom > nCaretPosX ){
						return -1;
					}
				}
			}else
			if( m_nSelectColmFrom > nCaretPosX ){
				return -1;
			}
		}
		if( m_nSelectLineTo == nCaretPosY ){
			if( m_bDragSource ){
				if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){	/* Ctrl�L�[��������Ă����� */
					if( m_nSelectColmTo <= nCaretPosX ){
						return 1;
					}
				}else{
					if( m_nSelectColmTo < nCaretPosX ){
						return 1;
					}
				}
			}else
			if( m_nSelectColmTo <= nCaretPosX ){
				return 1;
			}
		}
		return 0;
	}
	return -1;
}

/* �w��J�[�\���ʒu���I���G���A���ɂ��邩 (�e�X�g)
	�y�߂�l�z
	-1	�I���G���A���O�� or ���I��
	0	�I���G���A��
	1	�I���G���A�����
*/
int CEditView::IsCurrentPositionSelectedTEST(
	int		nCaretPosX,		// �J�[�\���ʒuX
	int		nCaretPosY,		// �J�[�\���ʒuY
	int		nSelectLineFrom,
	int		nSelectColmFrom,
	int		nSelectLineTo,
	int		nSelectColmTo
)
{
	if( FALSE == IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return -1;
	}
//	RECT	rcSel;
//	POINT	po;


	if( nSelectLineFrom > nCaretPosY ){
		return -1;
	}
	if( nSelectLineTo < nCaretPosY ){
		return 1;
	}
	if( nSelectLineFrom == nCaretPosY ){
		if( nSelectColmFrom > nCaretPosX ){
			return -1;
		}
	}
	if( nSelectLineTo == nCaretPosY ){
		if( nSelectColmTo <= nCaretPosX ){
			return 1;
		}
	}
	return 0;
}

/* �N���b�v�{�[�h����f�[�^���擾 */
BOOL CEditView::MyGetClipboardData( CMemory& cmemBuf, BOOL* pbColmnSelect )
{
	HGLOBAL		hglb;
	char*		lptstr;

	if( NULL != pbColmnSelect ){
		*pbColmnSelect = FALSE;
	}


	UINT uFormatSakuraClip;
	UINT uFormat;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( !::IsClipboardFormatAvailable( CF_OEMTEXT )
	 && !::IsClipboardFormatAvailable( uFormatSakuraClip )
	){
		return FALSE;
	}
	if ( !::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}

	char	szFormatName[128];

	if( NULL != pbColmnSelect ){
		/* ��`�I���̃e�L�X�g�f�[�^���N���b�v�{�[�h�ɂ��邩 */
		uFormat = 0;
		while( 0 != ( uFormat = ::EnumClipboardFormats( uFormat ) ) ){
			::GetClipboardFormatName( uFormat, szFormatName, sizeof(szFormatName) - 1 );
//			MYTRACE( "szFormatName=[%s]\n", szFormatName );
			if( 0 == lstrcmp( "MSDEVColumnSelect", szFormatName ) ){
				*pbColmnSelect = TRUE;
				break;
			}
		}
	}
	if( ::IsClipboardFormatAvailable( uFormatSakuraClip ) ){
		hglb = ::GetClipboardData( uFormatSakuraClip );
		if (hglb != NULL) {
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetData( lptstr + sizeof(int), *((int*)lptstr) );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}else{
		hglb = ::GetClipboardData( CF_OEMTEXT );
		if( hglb != NULL ){
			lptstr = (char*)::GlobalLock(hglb);
			cmemBuf.SetDataSz( lptstr );
			::GlobalUnlock(hglb);
			::CloseClipboard();
			return TRUE;
		}
	}
	::CloseClipboard();
	return FALSE;
}

/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
BOOL CEditView::MySetClipboardData( const char* pszText, int nTextLen, BOOL bColmnSelect )
{
	HGLOBAL		hgClip;
	char*		pszClip;
	UINT		uFormat;
	/* Windows�N���b�v�{�[�h�ɃR�s�[ */
	if( FALSE == ::OpenClipboard( m_hWnd ) ){
		return FALSE;
	}
	::EmptyClipboard();

	/* �e�L�X�g�`���̃f�[�^ */
	hgClip = ::GlobalAlloc(
		GMEM_MOVEABLE | GMEM_DDESHARE,
		lstrlen( pszText ) + 1
	);
	pszClip = (char*)::GlobalLock( hgClip );
	lstrcpy( pszClip, pszText );
	::GlobalUnlock( hgClip );
	::SetClipboardData( CF_OEMTEXT, hgClip );

	/* �o�C�i���`���̃f�[�^
		(int) �u�f�[�^�v�̒���
		�u�f�[�^�v
	*/
	UINT	uFormatSakuraClip;
	uFormatSakuraClip = ::RegisterClipboardFormat( "SAKURAClip" );
	if( 0 != uFormatSakuraClip ){
		hgClip = ::GlobalAlloc(
			GMEM_MOVEABLE | GMEM_DDESHARE,
			nTextLen + sizeof( int ) + 1
		);
		pszClip = (char*)::GlobalLock( hgClip );
		*((int*)pszClip) = nTextLen;
		memcpy( pszClip + sizeof( int ), pszText, nTextLen );
		::GlobalUnlock( hgClip );
		::SetClipboardData( uFormatSakuraClip, hgClip );
	}


	/* ��`�I���������_�~�[�f�[�^ */
	if( bColmnSelect ){
		uFormat = ::RegisterClipboardFormat( "MSDEVColumnSelect" );
		if( 0 != uFormat ){
			hgClip = ::GlobalAlloc(
				GMEM_MOVEABLE | GMEM_DDESHARE,
				1
			);
			pszClip = (char*)::GlobalLock( hgClip );
			pszClip[0] = '\0';
			::GlobalUnlock( hgClip );
			::SetClipboardData( uFormat, hgClip );
		}
	}
	::CloseClipboard();
	return TRUE;
}






STDMETHODIMP CEditView::DragEnter( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragEnter()\n" );
#endif

	if( TRUE == m_pShareData->m_Common.m_bUseOLE_DragDrop ){	/* OLE�ɂ��h���b�O & �h���b�v���g�� */
	}else{
		return E_INVALIDARG;
	}

	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	if( IsDataAvailable( pDataObject, CF_TEXT )
//	 && NULL != GetGlobalData(pDataObject, CF_TEXT)
	){
		/* �������A�N�e�B�u�y�C���ɂ��� */
		m_pcEditDoc->SetActivePane( m_nMyIndex );

		/* �I���e�L�X�g�̃h���b�O���� */
		m_bDragMode = TRUE;

	//	/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃A�N�e�B�u�v�� */
	//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

		/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
		OnSetFocus();

	//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
	//	OnLBUTTONDOWN( dwKeyState, pt.x, pt.y );

		m_pcDropTarget->m_pDataObject = pDataObject;
		/* Ctrl,ALT,�L�[��������Ă����� */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			*pdwEffect = DROPEFFECT_MOVE;
		}
//		/* �A�N�e�B�u�ɂ��� */
//		ActivateFrameWindow( GetParent( m_hwndParent ) );
//		ActivateFrameWindow( m_hWnd_DropTarget/*GetParent( m_hwndParent )*/ );

//		::SetFocus( m_hWnd_DropTarget );
		::SetFocus( m_hWnd );
	}else{
		return E_INVALIDARG;
//		*pdwEffect = DROPEFFECT_NONE;

	}
//	/* �A�N�e�B�u�ɂ��� */
//	ActivateFrameWindow( ::GetParent( m_hwndParent ) );
//	::PostMessage( ::GetParent( m_hwndParent ), WM_ACTIVATE, MAKELONG( 0, WA_ACTIVE ), 0 );
//	/*
//	||	�������̃��[�U�[������\�ɂ���
	//	||	�u���b�L���O�t�b�N (���b�Z�[�W�z��)
//	*/
//	BlockingHook();

	DragOver( dwKeyState, pt, pdwEffect );
	return S_OK;
}

STDMETHODIMP CEditView::DragOver( DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragOver()\n" );
#endif
//	RECT	rc;
//	POINT	po;
//	po.x = pt.x;
//	po.y = pt.y;
//	::GetWindowRect(m_hWnd, &rc );
//	if( m_bDragSource && PtInRect( &rc, po ) ){
//	}else{
		/* �}�E�X�ړ��̃��b�Z�[�W���� */
//		::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
		::ScreenToClient( m_hWnd, (LPPOINT)&pt );
		OnMOUSEMOVE( dwKeyState, pt.x , pt.y );
//	}

//	MYTRACE( "m_nCaretPosX=%d, m_nCaretPosY=%d\n", m_nCaretPosX, m_nCaretPosY );

	if ( pdwEffect == NULL )
		return E_INVALIDARG;
//	::ScreenToClient( m_hWnd_DropTarget, (LPPOINT)&pt );
//	DWORD dwIndex = LOWORD( ::SendMessage( m_hWnd_DropTarget, EM_CHARFROMPOS, 0, MAKELPARAM( pt.x, pt.y ) ) );
//	if ( dwIndex != (WORD) -1 ){
//		::SendMessage( m_hWnd_DropTarget, EM_SETSEL, dwIndex, dwIndex );
//		::SendMessage( m_hWnd_DropTarget, EM_SCROLLCARET, 0, 0 );
//	}
	if( NULL == m_pcDropTarget->m_pDataObject ){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	if( m_bDragSource
	 && 0 == IsCurrentPositionSelected( /* �w��J�[�\���ʒu���I���G���A���ɂ��邩 */
			m_nCaretPosX,				// �J�[�\���ʒuX
			m_nCaretPosY				// �J�[�\���ʒuY
		)
	){
		*pdwEffect = DROPEFFECT_NONE;
	}else
	/* Ctrl,ALT,�L�[��������Ă����� */
	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
	 || FALSE == m_bDragSource
	){
		*pdwEffect = DROPEFFECT_COPY;
	}else{
		*pdwEffect = DROPEFFECT_MOVE;
	}
	return S_OK;
}



STDMETHODIMP CEditView::DragLeave( void )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::DragLeave()\n" );
#endif

	if( !m_bDragSource ){
		/* ���̓t�H�[�J�X���������Ƃ��̏��� */
		OnKillFocus();
		::SetFocus(NULL);
	}else{
		// 1999.11.15
		OnSetFocus();
		::SetFocus( ::GetParent( m_hwndParent ) );
	}

	m_pcDropTarget->m_pDataObject = NULL;

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	return S_OK;
}

STDMETHODIMP CEditView::Drop( LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect )
{
#ifdef _DEBUG
	MYTRACE( "CEditView::Drop()\n" );
#endif
	BOOL		bBoxSelected;
	BOOL		bMove;
	BOOL		bMoveToPrev;
	RECT		rcSel;
	int			nCaretPosX_Old;
	int			nCaretPosY_Old;
	CMemory		cmemBuf;
	CMemory		cmemClip;
	int			bBeginBoxSelect_Old;
//	int			nSelectLineBgn_Old;			/* �͈͑I���J�n�s(���_) */
//	int			nSelectColBgn_Old;			/* �͈͑I���J�n��(���_) */
	int			nSelectLineBgnFrom_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColBgnFrom_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
	int			nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */
	int			nSelectLineFrom_Old;
	int			nSelectColFrom_Old;
	int			nSelectLineTo_Old;
	int			nSelectColTo_Old;
//	MYTRACE( "CEditView::Drop()\n" );

	if( !m_bDragSource
	 && IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* ���݂̑I��͈͂��I����Ԃɖ߂� */
		DisableSelectArea( TRUE );
	}
	if( pDataObject == NULL || pdwEffect == NULL )
		return E_INVALIDARG;
	*pdwEffect = DROPEFFECT_NONE;
	if( IsDataAvailable(pDataObject, CF_TEXT)){
#ifdef _DEBUG
		MYTRACE( "TRUE == IsDataAvailable()\n" );
#endif
		HGLOBAL		hData = GetGlobalData(pDataObject, CF_TEXT);
#ifdef _DEBUG
		MYTRACE( "%xh == GetGlobalData(pDataObject, CF_TEXT)\n", hData );
#endif
		if (hData == NULL){
			m_pcDropTarget->m_pDataObject = NULL;
			/* �I���e�L�X�g�̃h���b�O���� */
			m_bDragMode = FALSE;
			return E_INVALIDARG;
		}

		DWORD		nSize = 0;
		LPCTSTR lpszSource = (LPCTSTR) ::GlobalLock(hData);

//		MYTRACE( "lpszSource=[%s]\n", lpszSource );

		/* �ړ����R�s�[�� */
		if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL )
		 || FALSE == m_bDragSource
		){
			bMove = FALSE;
			*pdwEffect = DROPEFFECT_COPY;
		}else{
			bMove = TRUE;
			*pdwEffect = DROPEFFECT_MOVE;
		}

		if( m_bDragSource ){
			if( NULL != m_pcOpeBlk ){
			}else{
				m_pcOpeBlk = new COpeBlk;
			}
			bBoxSelected = m_bBeginBoxSelect;
			/* �I��͈͂̃f�[�^���擾 */
//			GetSelectedData( cmemBuf, FALSE, NULL, FALSE );
//			cmemBuf.SetData( lpszSource, lstrlen( lpszSource ) );
			cmemBuf.SetDataSz( lpszSource );

			/* �ړ��̏ꍇ�A�ʒu�֌W���Z�o */
			if( bMove ){
				if( bBoxSelected ){
					/* 2�_��Ίp�Ƃ����`�����߂� */
					TwoPointToRect(
						&rcSel,
						m_nSelectLineFrom,		/* �͈͑I���J�n�s */
						m_nSelectColmFrom,		/* �͈͑I���J�n�� */
						m_nSelectLineTo,		/* �͈͑I���I���s */
						m_nSelectColmTo			/* �͈͑I���I���� */
					);
					++rcSel.bottom;
					if( m_nCaretPosY >= rcSel.bottom ){
						bMoveToPrev = FALSE;
					}else
					if( m_nCaretPosY + rcSel.bottom - rcSel.top < rcSel.top ){
						bMoveToPrev = TRUE;
					}else
					if( m_nCaretPosX < rcSel.left ){
						bMoveToPrev = TRUE;
					}else{
						bMoveToPrev = FALSE;
					}
				}else{
					if( m_nSelectLineFrom > m_nCaretPosY ){
						bMoveToPrev = TRUE;
					}else
					if( m_nSelectLineFrom == m_nCaretPosY ){
						if( m_nSelectColmFrom > m_nCaretPosX ){
							bMoveToPrev = TRUE;
						}else{
							bMoveToPrev = FALSE;
						}
					}else{
						bMoveToPrev = FALSE;
					}
				}
			}
			if( !bMove ){
				/* �R�s�[���[�h */
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( TRUE );
			}
			nCaretPosX_Old = m_nCaretPosX;
			nCaretPosY_Old = m_nCaretPosY;
			if( bMove ){
				if( bMoveToPrev ){
					/* �ړ����[�h & �O�Ɉړ� */
					/* �I���G���A���폜 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}else{
					bBeginBoxSelect_Old = m_bBeginBoxSelect;
//					nSelectLineBgn_Old	= m_nSelectLineBgn;			/* �͈͑I���J�n�s(���_) */
//					nSelectColBgn_Old	= m_nSelectColmBgn;			/* �͈͑I���J�n��(���_) */

					nSelectLineBgnFrom_Old	= m_nSelectLineBgnFrom;	/* �͈͑I���J�n�s(���_) */
					nSelectColBgnFrom_Old	= m_nSelectColmBgnFrom;	/* �͈͑I���J�n��(���_) */
					nSelectLineBgnTo_Old	= m_nSelectLineBgnTo;	/* �͈͑I���J�n�s(���_) */
					nSelectColBgnTo_Old		= m_nSelectColmBgnTo;	/* �͈͑I���J�n��(���_) */

					nSelectLineFrom_Old	= m_nSelectLineFrom;
					nSelectColFrom_Old	= m_nSelectColmFrom;
					nSelectLineTo_Old	= m_nSelectLineTo;
					nSelectColTo_Old	= m_nSelectColmTo;
					/* ���݂̑I��͈͂��I����Ԃɖ߂� */
					DisableSelectArea( TRUE );
				}
			}
			if( FALSE == bBoxSelected ){	/* ��`�͈͑I�� */
				Command_INSTEXT( TRUE, cmemBuf.GetPtr( NULL ), FALSE );
			}else{

				cmemClip.SetDataSz( "" );

				/* �N���b�v�{�[�h����f�[�^���擾 */
				BOOL	bBoxSelectOld;
				MyGetClipboardData( cmemClip, &bBoxSelectOld );

					/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
					MySetClipboardData( cmemBuf.GetPtr( NULL ), cmemBuf.GetLength(), TRUE );

					/* �\��t���i�N���b�v�{�[�h����\��t���j*/
					Command_PASTEBOX();

				/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
				MySetClipboardData( cmemClip.GetPtr( NULL ), cmemClip.GetLength(), bBoxSelectOld );
			}
			if( bMove ){
				if( bMoveToPrev ){
				}else{
					/* �ړ����[�h & ���Ɉړ�*/
					m_bBeginBoxSelect = bBeginBoxSelect_Old;
//					m_nSelectLineBgn = nSelectLineBgn_Old;			/* �͈͑I���J�n�s(���_) */
//					m_nSelectColmBgn = nSelectColBgn_Old;			/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnFrom = nSelectLineBgnFrom_Old;	/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = nSelectColBgnFrom_Old;	/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnTo = nSelectLineBgnTo_Old;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = nSelectColBgnTo_Old;		/* �͈͑I���J�n��(���_) */

					m_nSelectLineFrom = nSelectLineFrom_Old;
					m_nSelectColmFrom = nSelectColFrom_Old;
					m_nSelectLineTo = nSelectLineTo_Old;
					m_nSelectColmTo = nSelectColTo_Old;

					/* �I���G���A���폜 */
					DeleteData( TRUE );
					MoveCursor( nCaretPosX_Old, nCaretPosY_Old, TRUE );
				}
			}
			/* �A���h�D�o�b�t�@�̏��� */
			if( NULL != m_pcOpeBlk ){
				if( 0 < m_pcOpeBlk->GetNum() ){	/* ����̐���Ԃ� */
					/* ����̒ǉ� */
					m_pcEditDoc->m_cOpeBuf.AppendOpeBlk( m_pcOpeBlk );
				}else{
					delete m_pcOpeBlk;
				}
				m_pcOpeBlk = NULL;
			}
		}else{
			HandleCommand( F_INSTEXT, TRUE, (LPARAM)lpszSource, TRUE, 0, 0 );
		}
		::GlobalUnlock(hData);
	}else{
#ifdef _DEBUG
		MYTRACE( "FALSE == IsDataAvailable()\n" );
#endif
	}
	m_pcDropTarget->m_pDataObject = NULL;
//	::SetFocus(NULL);

	/* �I���e�L�X�g�̃h���b�O���� */
	m_bDragMode = FALSE;

	/* �ҏW�E�B���h�E�I�u�W�F�N�g����̃A�N�e�B�u�v�� */
	::SetFocus( ::GetParent( m_hwndParent ) );
	SetActiveWindow( m_hWnd );
//	::PostMessage( m_pShareData->m_hwndTray, MYWM_ACTIVATE_ME, (WPARAM)::GetParent( m_hwndParent ),  0 );

//	::ShowCaret( m_hWnd );
	return S_OK;
}





/* ���݃J�[�\���ʒu�P��܂��͑I��͈͂�茟�����̃L�[���擾 */
void CEditView::GetCurrentTextForSearch( CMemory& cmemCurText )
{

	int				i;
	char			szTopic[_MAX_PATH];
//	CMemory			cmemCurText;
	const char*		pLine;
	int				nLineLen;
	int				nIdx;
	int				nLineFrom;
	int				nColmFrom;
	int				nLineTo;
	int				nColmTo;
	const CLayout*	pcLayout;

	cmemCurText.SetDataSz( "" );
	szTopic[0] = '\0';
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �I��͈͂̃f�[�^���擾 */
		if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
			/* ��������������݈ʒu�̒P��ŏ����� */
			strncpy( szTopic, cmemCurText.GetPtr( NULL ), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = '\0';
		}
	}else{
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr2( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdx = LineColmnToIndex( pLine, nLineLen, m_nCaretPosX );

			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				m_nCaretPosY, nIdx,
				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
			){
				/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen );
				nColmFrom = LineIndexToColmn( pLine, nLineLen, nColmFrom );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen );
				nColmTo = LineIndexToColmn( pLine, nLineLen, nColmTo );
				/* �I��͈͂̕ύX */
//				m_nSelectLineBgn = nLineFrom;		/* �͈͑I���J�n�s(���_) */
//				m_nSelectColmBgn = nColmFrom;		/* �͈͑I���J�n��(���_) */

				m_nSelectLineBgnFrom = nLineFrom;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnFrom = nColmFrom;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnTo = nLineTo;		/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnTo = nColmTo;		/* �͈͑I���J�n��(���_) */

				m_nSelectLineFrom = nLineFrom;
				m_nSelectColmFrom = nColmFrom;
				m_nSelectLineTo = nLineTo;
				m_nSelectColmTo = nColmTo;
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE ) ){
					/* ��������������݈ʒu�̒P��ŏ����� */
					strncpy( szTopic, cmemCurText.GetPtr( NULL ), MAX_PATH - 1 );
					szTopic[MAX_PATH - 1] = '\0';
				}
				/* ���݂̑I��͈͂��I����Ԃɖ߂� */
				DisableSelectArea( FALSE );
			}
		}
	}

	/* ����������͉��s�܂� */
	for( i = 0; i < (int)lstrlen( szTopic ); ++i ){
		if( szTopic[i] == CR || szTopic[i] == LF ){
			szTopic[i] = '\0';
			break;
		}
	}
//	cmemCurText.SetData( szTopic, lstrlen( szTopic ) );
	cmemCurText.SetDataSz( szTopic );
	return;

}



/* �J�[�\���s�A���_�[���C����ON */
void CEditView::CaretUnderLineON( BOOL bDraw )
{
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		return;
	}
	m_nOldUnderLineY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight;
	if( -1 == m_nOldUnderLineY ){
		m_nOldUnderLineY = -2;
	}

	if( bDraw
	 && m_bDrawSWITCH
	 && m_nOldUnderLineY >=m_nViewAlignTop
	 && m_bDoing_UndoRedo == FALSE	/* �A���h�D�E���h�D�̎��s���� */
	){
//		MYTRACE( "���J�[�\���s�A���_�[���C���̕`��\n" );
		/* ���J�[�\���s�A���_�[���C���̕`�� */
		HDC		hdc;
		HPEN	hPen, hPenOld;
		hdc = ::GetDC( m_hWnd );
		hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_colTEXT );
		hPenOld = (HPEN)::SelectObject( hdc, hPen );
		::MoveToEx(
			hdc,
			m_nViewAlignLeft,
			m_nOldUnderLineY,
			NULL
		);
		::LineTo(
			hdc,
			m_nViewCx + m_nViewAlignLeft,
			m_nOldUnderLineY
		);
		::SelectObject( hdc, hPenOld );
		::DeleteObject( hPen );
		::ReleaseDC( m_hWnd, hdc );
		hdc= NULL;
	}
	return;
}



/* �J�[�\���s�A���_�[���C����OFF */
void CEditView::CaretUnderLineOFF( BOOL bDraw )
{
	if( FALSE == m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_UNDERLINE].m_bDisp ){
		return;
	}

//	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
//		m_nOldUnderLineY = -1;
//		return;
//	}

	if( -1 != m_nOldUnderLineY ){
		if( bDraw
		 && m_bDrawSWITCH
		 && m_nOldUnderLineY >=m_nViewAlignTop
		 && m_bDoing_UndoRedo == FALSE	/* �A���h�D�E���h�D�̎��s���� */
		){
//			MYTRACE( "���J�[�\���s�A���_�[���C���̏���\n" );
			/* �J�[�\���s�A���_�[���C���̏��� */
			HDC		hdc;
			HPEN	hPen, hPenOld;
			hdc = ::GetDC( m_hWnd );
			hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
			hPenOld = (HPEN)::SelectObject( hdc, hPen );
			::MoveToEx(
				hdc,
				m_nViewAlignLeft,
				m_nOldUnderLineY,
				NULL
			);
			::LineTo(
				hdc,
				m_nViewCx + m_nViewAlignLeft,
				m_nOldUnderLineY
			);
			::SelectObject( hdc, hPenOld );
			::DeleteObject( hPen );
			::ReleaseDC( m_hWnd, hdc );
		}
		m_nOldUnderLineY = -1;
	}
	return;
}


/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
CEOL CEditView::GetCurrentInsertEOL( void )
{
#if 0
	//	May 12, 2000 genta
	CDocLine* pFirstlineinfo = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(0);
	if( pFirstlineinfo != NULL ){
		enumEOLType t = pFirstlineinfo->m_cEol;
		if( t != EOL_NONE && t != EOL_UNKNOWN )
			return t;
	}
	return EOL_CRLF;

	CEditDoc::FileRead�ɂăR�[�h��ݒ肷��悤�ɂ����̂ŁC�����͍폜�D
#endif
#if 0
	enumEOLType t = m_pcEditDoc->GetNewLineCode();
	char buf[30];
	wsprintf( buf, "EOL: %d", t );
	::MessageBox( NULL, buf, "End of Line", MB_OK );
	return t;
#else
	return m_pcEditDoc->GetNewLineCode();
#endif
}


// �q�v���Z�X�̕W���o�͂����_�C���N�g����
void CEditView::ExecCmd(const char* pszCmd, BOOL bGetStdout )
{
	HANDLE  hFile;
	char	szTempFile[_MAX_PATH+ 1];

	hFile = NULL;
	if( bGetStdout ){
		::GetTempPath( sizeof( szTempFile ) - 1, szTempFile );
		// �e���|�����t�@�C���̃t�@�C�������쐬���܂��B
		::GetTempFileName(
		  szTempFile,	// pointer to directory name for temporary file
		  "skr",		// pointer to filename prefix
		  0,			// number used to create temporary filename
		  szTempFile	// pointer to buffer that receives the new filename
		);
//		MYTRACE( "szTempFile=[%s]\n", szTempFile );

		hFile = CreateFile(
			szTempFile,
			GENERIC_WRITE | GENERIC_READ,
			0,
			NULL, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if ( hFile == INVALID_HANDLE_VALUE ){
			return;
		}
	}

	STARTUPINFO				StartupInfo;
	PROCESS_INFORMATION		ProcessInfo;
	char					szCmd[512];
	CDlgCancel				cDlgCancel;
	if( bGetStdout ){
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
	}

	GetStartupInfo(&StartupInfo);
	StartupInfo.dwFlags |= ( STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW );
	StartupInfo.wShowWindow = bGetStdout?/*SW_SHOW*/SW_HIDE:SW_SHOW;	/* �q�v���Z�X�̃E�B���h�E�\����� */
	if( bGetStdout ){
		StartupInfo.hStdOutput	= hFile;
		StartupInfo.hStdError	= hFile;
	}
	lstrcpy(szCmd, "");
	lstrcat(szCmd, pszCmd);
	const char* pszOpt_C;
	if( bGetStdout ){
		pszOpt_C = "/C";
	}else{
		pszOpt_C = "/K";
	}
	memset( &ProcessInfo, 0, sizeof( PROCESS_INFORMATION ) );
	if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
		wsprintf( szCmd, "command.com %s ", pszOpt_C );  // Win95/98 �Ȃ� command.com
		lstrcat(szCmd, pszCmd);
		if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
			wsprintf(szCmd, "cmd.exe %s ", pszOpt_C );  // Win NT �Ȃ� cmd.exe
			lstrcat(szCmd, pszCmd);
			if( !CreateProcess( NULL, szCmd, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &StartupInfo, &ProcessInfo ) ){
				goto end_of_func;
			}
		}
	}
	if( bGetStdout ){
		/* �Ō�Ƀe�L�X�g��ǉ� */
//		Command_ADDTAIL( "\r\n", 2 );
//		Command_ADDTAIL( szCmd, strlen( szCmd ) );
//		Command_ADDTAIL( "\r\n", 2 );

		m_cShareData.TraceOut( "%s", "\r\n" );
		m_cShareData.TraceOut( "%s", szCmd );
		m_cShareData.TraceOut( "%s", "\r\n" );

	}
	if( bGetStdout ){
		::SetDlgItemText( cDlgCancel.m_hWnd, IDC_STATIC_CMD, szCmd );
	}

	DWORD	dwRes;
	DWORD	swFPOld;
	DWORD	swFPRead;
	DWORD	dwNumberOfBytesRead;
//	DWORD	dwNumberOfBytesWritten;
	swFPRead = 0;
	BOOL	bRes;
	char	szBuffer[1000];
	/* �v���Z�X�I�u�W�F�N�g���V�O�i����ԂɂȂ�܂Ń��[�v���� */
	while(bGetStdout){
		/* �v���Z�X�I�u�W�F�N�g�̏�Ԃ𒲂ׂ� */
		dwRes = ::WaitForSingleObject(ProcessInfo.hProcess, 0 );
		// OutputDebugString( "WaitForSingleObject()\n" );

		if( bGetStdout ){
			/* �t�@�C���|�C���^���ړ����܂� */
			swFPOld = SetFilePointer(
				hFile,						// handle of file
				swFPRead,					// number of bytes to move file pointer
				NULL,						// pointer to high-order word of distance to move
				FILE_BEGIN					// how to move
			);
			while(1){
				bRes = ReadFile(
					hFile,					// handle of file to read
					szBuffer,				// pointer to buffer that receives data
					sizeof( szBuffer ) - 1,	// number of bytes to read
					&dwNumberOfBytesRead,	// pointer to number of bytes read
					NULL					// pointer to structure for data
				);
				swFPRead += dwNumberOfBytesRead;
				szBuffer[dwNumberOfBytesRead] = '\0';
				if( 0 < dwNumberOfBytesRead ){
					/* �Ō�Ƀe�L�X�g��ǉ� */
//					Command_ADDTAIL( szBuffer, dwNumberOfBytesRead );
//					/*�t�@�C���̍Ō�Ɉړ� */
//					Command_GOFILEEND(FALSE);

					m_cShareData.TraceOut( "%s", szBuffer );

				}

				if( dwNumberOfBytesRead < sizeof( szBuffer ) - 1 ){
					break;
				}
			}

//			/* �t�@�C���|�C���^���ړ����܂� */
//			SetFilePointer(
//				hFile,					// handle of file
//				0/*swFPOld*/,			// number of bytes to move file pointer
//				NULL,					// pointer to high-order word of distance to move
//				FILE_END/*FILE_BEGIN*/	// how to move
//			);
		}

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
			break;
		}
		/* ���f�{�^�������`�F�b�N */
		if( cDlgCancel.IsCanceled() ){
			//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂�
			::TerminateProcess(
				ProcessInfo.hProcess,	// handle to the process
				0						// exit code for the process
			);
			/* �Ō�Ƀe�L�X�g��ǉ� */
			const char*		pszText;
			pszText = "\r\n���f���܂����B\r\n";
//			Command_ADDTAIL( pszText, strlen( pszText ) );
//			/*�t�@�C���̍Ō�Ɉړ� */
//			Command_GOFILEEND( FALSE );

			m_cShareData.TraceOut( "%s", pszText );

			break;
		}

		/* �v���Z�X�I�u�W�F�N�g���V�O�i����� */
		if( WAIT_OBJECT_0 == dwRes ){
			break;
		}
	}
end_of_func:;
	if( bGetStdout ){
		if( NULL != ProcessInfo.hThread ){
			CloseHandle( ProcessInfo.hThread );
		}
		if( NULL != ProcessInfo.hProcess ){
			CloseHandle( ProcessInfo.hProcess );
		}
		cDlgCancel.CloseDialog( 0 );
	}
	if( NULL != hFile ){
		::CloseHandle( hFile );
		::DeleteFile( szTempFile );
	}

	return;
}


/*[EOF]*/
