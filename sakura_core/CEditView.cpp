//	$Id$
/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author Norio Nakatani
	@date	1998/03/13 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta
	Copyright (C) 2001, GAE, MIK, hor, asa-o, Stonee, Misaka, novice, YAZAKI
	Copyright (C) 2002, YAZAKI, hor, aroka, MIK, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

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
#include "etc_uty.h"
#include "global.h"
#include "CAutoSave.h"
#include "CLayout.h"/// 2002/2/3 aroka
#include "COpe.h"///
#include "COpeBlk.h"///
#include "CDropTarget.h"///
#include "CSplitBoxWnd.h"///
#include "CRegexKeyword.h"///	//@@@ 2001.11.17 add MIK
#include "CMarkMgr.h"///
#include "COsVersionInfo.h"
#include "CDocLine.h"   // 2002.04.09 minfu
#include "CFileLoad.h" // 2002/08/30 Moca
#include "CMemoryIterator.h"	// @@@ 2002.09.28 YAZAKI
#include "my_icmp.h" // 2002/11/30 Moca �ǉ�

#ifndef WM_MOUSEWHEEL
	#define WM_MOUSEWHEEL	0x020A
#endif


#ifndef IMR_RECONVERTSTRING
#define IMR_RECONVERTSTRING             0x0004
#endif // IMR_RECONVERTSTRING

/* 2002.04.09 minfu �ĕϊ����� */
#ifndef IMR_CONFIRMRECONVERTSTRING
#define IMR_CONFIRMRECONVERTSTRING             0x0005
#endif // IMR_CONFIRMRECONVERTSTRING



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

//@@@2002.01.14 YAZAKI static�ɂ��ă������̐ߖ�i(10240+10) * 3 �o�C�g�j
int CEditView::m_pnDx[10240 + 10];

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



//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
CEditView::CEditView() : m_cHistory( new CAutoMarkMgr ) //,
// 20020331 aroka �ĕϊ��Ή� for 95/NT
// 2002.04.09 �R���X�g���N�^�̂Ȃ��Ɉړ����܂����B minfu
//	m_uMSIMEReconvertMsg( ::RegisterWindowMessage( RWM_RECONVERT ) ),
//	m_uATOKReconvertMsg( ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) )
{
	LOGFONT		lf;

	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	m_CurRegexp.Init();

	m_bDrawSWITCH = TRUE;
	m_pcDropTarget = new CDropTarget( this );
	m_bDragSource = FALSE;
	m_bDragMode = FALSE;					/* �I���e�L�X�g�̃h���b�O���� */
	m_bCurSrchKeyMark = FALSE;				/* ���������� */
	//	Jun. 27, 2001 genta
	m_szCurSrchKey[0] = '\0';
	//strcpy( m_szCurSrchKey, "" );			/**/
	m_bCurSrchRegularExp = 0;				/* �����^�u��  1==���K�\�� */
	m_bCurSrchLoHiCase = 0;					/* �����^�u��  1==�p�啶���������̋�� */
	m_bCurSrchWordOnly = 0;					/* �����^�u��  1==�P��̂݌��� */

	m_bExecutingKeyMacro = FALSE;			/* �L�[�{�[�h�}�N���̎��s�� */
	m_bPrevCommand = 0;
	m_nMyIndex = 0;

	//	Dec. 4, 2002 genta
	//	���j���[�o�[�ւ̃��b�Z�[�W�\���@�\��CEditWnd�ֈڊ�

	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();
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

	m_nSrchStartPosX_PHY = -1;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/06/26 ai
	m_nSrchStartPosY_PHY = -1;	/* ����/�u���J�n���̃J�[�\���ʒu  ���s�P�ʍs�̍s�ԍ�(0�J�n) */				// 02/06/26 ai
	m_bSearch = FALSE;			/* ����/�u���J�n�ʒu��o�^���邩 */											// 02/06/26 ai
	m_nBracketPairPosX_PHY = -1;/* �Ί��ʂ̈ʒu ���s�P�ʍs�擪����̃o�C�g��(0�J�n) */	// 02/12/13 ai
	m_nBracketPairPosY_PHY = -1;/* �Ί��ʂ̈ʒu ���s�P�ʍs�̍s�ԍ�(0�J�n) */			// 02/12/13 ai
	m_bDrawSelectArea = FALSE;	/* �I��͈͂�`�悵���� */	// 02/12/13 ai

	m_nCaretWidth = 0;			/* �L�����b�g�̕� */
	m_nCaretHeight = 0;			/* �L�����b�g�̍��� */
	m_bSelectingLock = FALSE;	/* �I����Ԃ̃��b�N */
	m_bBeginSelect = FALSE;		/* �͈͑I�� */
	m_bBeginBoxSelect = FALSE;	/* ��`�͈͑I�� */
	m_bBeginLineSelect = FALSE;	/* �s�P�ʑI�� */
	m_bBeginWordSelect = FALSE;	/* �P��P�ʑI�� */

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
	m_nViewAlignTop += m_pShareData->m_Common.m_nRulerHeight;	/* ���[���[���� */
	m_nOldCaretPosX = 0;	// �O��`�悵�����[���[�̃L�����b�g�ʒu 2002.02.25 Add By KK
	m_nOldCaretWidth = 0;	// �O��`�悵�����[���[�̃L�����b�g��   2002.02.25 Add By KK
	m_bRedrawRuler = true;	// ���[���[�S�̂�`��������=true   2002.02.25 Add By KK
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

	// from here  2002.04.09 minfu OS�ɂ���čĕϊ��̕�����ς���
	//	YAZAKI COsVersionInfo�̃J�v�Z�����͎��܂���B
	COsVersionInfo	cOs;
//	POSVERSIONINFO pOsVer;
	
//	pOsVer =  cOs.GetOsVersionInfo();
	if( cOs.OsDoesNOTSupportReconvert() ){
		// 95 or NT�Ȃ��
		m_uMSIMEReconvertMsg = ::RegisterWindowMessage( RWM_RECONVERT );
		m_uATOKReconvertMsg = ::RegisterWindowMessage( MSGNAME_ATOK_RECONVERT ) ;
		m_uWM_MSIME_RECONVERTREQUEST = ::RegisterWindowMessage("MSIMEReconvertRequest");
		
		m_hAtokModule = LoadLibrary("ATOK10WC.DLL");
		AT_ImmSetReconvertString = NULL;
		if ( NULL != m_hAtokModule ) {
			AT_ImmSetReconvertString =(BOOL (WINAPI *)( HIMC , int ,PRECONVERTSTRING , DWORD  ) ) GetProcAddress(m_hAtokModule,"AT_ImmSetReconvertString");
		}
	}else{ 
		// ����ȊO��OS�̂Ƃ���OS�W�����g�p����
		m_uMSIMEReconvertMsg = 0;
		m_uATOKReconvertMsg = 0 ;
		m_hAtokModule = 0;	//@@@ 2002.04.14 MIK
	}
	// to here  2002.04.10 minfu
	
// 2002/07/22 novice
//	m_bCaretShowFlag = false;
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

	delete m_pcDropTarget;
	m_pcDropTarget = NULL;

	delete m_cHistory;

	delete m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	
	//�ĕϊ� 2002.04.10 minfu
	if(m_hAtokModule)
		FreeLibrary(m_hAtokModule);
	
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

	m_cRegexKeyword = new CRegexKeyword;	//@@@ 2001.11.17 add MIK
	m_cRegexKeyword->RegexKeySetTypes(&(m_pcEditDoc->GetDocumentAttribute()));	//@@@ 2001.11.17 add MIK

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
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
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
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
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
//	AdjustScrollBars();

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

	/* �A���_�[���C�� */
	m_cUnderLine.SetView( this );
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
			//	maybe it is in Unicode
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
			HandleCommand( F_IME_CHAR, TRUE, wParam, 0, 0, 0 );
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
//			return 0L;

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

//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
		if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
		{
			CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
			pcEditView -> ScrollAtV( m_nViewTopLine );
		}
//	2001/06/20 End

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

//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
		if( m_pShareData->m_Common.m_bSplitterWndHScroll )	// �����X�N���[���̓������Ƃ�
		{
			CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x02];
			HDC			hdc = ::GetDC( pcEditView->m_hWnd );
			pcEditView -> ScrollAtH( m_nViewLeftCol );
			m_bRedrawRuler = true; //2002.02.25 Add By KK �X�N���[�������[���[�S�̂�`���Ȃ����B
			DispRuler( hdc );
			::ReleaseDC( m_hWnd, hdc );
		}
//	2001/06/20 End

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
		hdc = ::BeginPaint( hwnd, &ps );
		OnPaint( hdc, &ps, FALSE );
		::EndPaint(hwnd, &ps);
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

	case MYWM_IME_REQUEST:  /* �ĕϊ�  by minfu 2002.03.27 */ // 20020331 aroka
		
		//if( (wParam == IMR_RECONVERTSTRING) &&  IsTextSelected() && ( !m_bBeginBoxSelect) ){
		//	// lParam��IME���p�ӂ����ĕϊ��p�\���̂̃|�C���^�������Ă���B
		//	return RequestedReconversion((PRECONVERTSTRING)lParam);
		//}
		// 2002.04.09 switch case �ɕύX  minfu 
		switch ( wParam ){
		case IMR_RECONVERTSTRING:
			return SetReconvertStruct((PRECONVERTSTRING)lParam, false);
			
		case IMR_CONFIRMRECONVERTSTRING:
			return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, false);
			
		}
		
		return 0L;
	
	default:
// << 20020331 aroka �ĕϊ��Ή� for 95/NT
		if( (m_uMSIMEReconvertMsg && (uMsg == m_uMSIMEReconvertMsg)) 
			|| (m_uATOKReconvertMsg && (uMsg == m_uATOKReconvertMsg))){
		// 2002.04.08 switch case �ɕύX minfu 
			switch ( wParam ){
			case IMR_RECONVERTSTRING:
				return SetReconvertStruct((PRECONVERTSTRING)lParam, true);
				
			case IMR_CONFIRMRECONVERTSTRING:
				return SetSelectionFromReonvert((PRECONVERTSTRING)lParam, true);
				
			}
//		if( uMsg == m_uMSIMEReconvertMsg || uMsg == m_uATOKReconvertMsg){
//			if( (wParam == IMR_RECONVERTSTRING) &&  IsTextSelected() && ( !m_bBeginBoxSelect) ){
//				// lParam��IME���p�ӂ����ĕϊ��p�\���̂̃|�C���^�������Ă���B
//				return RequestedReconversionW((PRECONVERTSTRING)lParam);
//			}
			return 0L;
		}
// >> by aroka

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
	SetParentCaption();	//	[Q] genta �{���ɕK�v�H

	return;
}


/* �L�����b�g�̕\���E�X�V */
void CEditView::ShowEditCaret( void )
{
	const char*		pLine;
	int				nLineLen;
	int				nCaretWidth = 0;
	int				nCaretHeight = 0;
	int				nIdxFrom;
	int				nCharChars;
//	HDC				hdc;
//	const CLayout*	pcLayout;
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
			const CLayout* pcLayout;
			nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
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
		const CLayout* pcLayout;
		nCaretWidth = ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
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

#if 0
	hdc = ::GetDC( m_hWnd );
#endif
	if( m_nCaretWidth == 0 ){
		/* �L�����b�g���Ȃ������ꍇ */
		/* �L�����b�g�̍쐬 */
		::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
		m_bCaretShowFlag = false; // 2002/07/22 novice
	}else{
		if( m_nCaretWidth != nCaretWidth || m_nCaretHeight != nCaretHeight ){
			/* �L�����b�g�͂��邪�A�傫�����ς�����ꍇ */
			/* ���݂̃L�����b�g���폜 */
			::DestroyCaret();

			/* �L�����b�g�̍쐬 */
			::CreateCaret( m_hWnd, (HBITMAP)NULL, nCaretWidth, nCaretHeight );
			m_bCaretShowFlag = false; // 2002/07/22 novice
		}else{
			/* �L�����b�g�͂��邵�A�傫�����ς���Ă��Ȃ��ꍇ */
			/* �L�����b�g���B�� */
			HideCaret_( m_hWnd ); // 2002/07/22 novice
		}
	}
	/* �L�����b�g�̈ʒu�𒲐� */
	int nPosX = m_nViewAlignLeft + (m_nCaretPosX - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	int nPosY = m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight - nCaretHeight;
	::SetCaretPos( nPosX, nPosY );
	if ( m_nViewAlignLeft <= nPosX && m_nViewAlignTop <= nPosY ){
		/* �L�����b�g�̕\�� */
		ShowCaret_( m_hWnd ); // 2002/07/22 novice
	}

	m_nCaretWidth = nCaretWidth;
	m_nCaretHeight = nCaretHeight;	/* �L�����b�g�̍��� */
	SetIMECompFormPos();

#if 0
	2002/05/12 YAZAKI ShowEditCaret�Ŏd�����������B
	//2002.02.27 Add By KK �A���_�[���C���̂������ጸ
	if (m_nOldUnderLineY != m_nViewAlignTop  + (m_nCaretPosY - m_nViewTopLine) * ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight ) + m_nCharHeight) {
		//�A���_�[���C���̕`��ʒu���A�O��̃A���_�[���C���`��ʒu�ƈقȂ��Ă�����A�A���_�[���C����`�������B
		CaretUnderLineOFF(TRUE);
		CaretUnderLineON(TRUE);
	}

	/* ���[���[�`�� */
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );
#endif

	return;
}





/* ���̓t�H�[�J�X���󂯎�����Ƃ��̏��� */
void CEditView::OnSetFocus( void )
{
//NG	/* 1999.11.15 */
//NG	::SetFocus( m_hwndParent );
//NG	::SetFocus( m_hWnd );

	ShowEditCaret();

//	SetIMECompFormPos();	YAZAKI ShowEditCaret�ō�ƍς�
	SetIMECompFormFont();

	/* ���[���̃J�[�\�����O���[���獕�ɕύX���� */
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );

	return;
}





/* ���̓t�H�[�J�X���������Ƃ��̏��� */
void CEditView::OnKillFocus( void )
{
	DestroyCaret();

	/* ���[���[�`�� */
	/* ���[���̃J�[�\����������O���[�ɕύX���� */
	HDC	hdc = ::GetDC( m_hWnd );
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
	case SB_TOP:
		ScrollAtV( 0 );
		break;
	case SB_BOTTOM:
		ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - m_nViewRowNum );
		break;
	default:
		break;
	}
	return;
}




/* �����X�N���[���o�[���b�Z�[�W���� */
void CEditView::OnHScroll( int nScrollCode, int nPos, HWND hwndScrollBar )
{
	m_bRedrawRuler = true; // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		ScrollAtH( m_nViewLeftCol - 4 );
		break;
	case SB_LINERIGHT:
		ScrollAtH( m_nViewLeftCol + 4 );
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
	case SB_LEFT:
		ScrollAtH( 0 );
		break;
	case SB_RIGHT:
		ScrollAtH( m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - m_nViewColNum );
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
		bi.biClrImportant	= nNumColors;
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

	m_bDrawSelectArea = TRUE;	// 02/12/13 ai

//	MYTRACE( "DrawSelectArea()  m_bBeginBoxSelect=%s\n", m_bBeginBoxSelect?"TRUE":"FALSE" );
	if( m_bBeginBoxSelect ){		/* ��`�͈͑I�� */
		// 2001.12.21 hor ��`�G���A��EOF������ꍇ�ARGN_XOR�Ō��������
		// EOF�ȍ~�̃G���A�����]���Ă��܂��̂ŁA���̏ꍇ��Redraw���g��
		// 2002.02.16 hor �������}�~���邽��EOF�ȍ~�̃G���A�����]�����������x���]���Č��ɖ߂����Ƃɂ���
		//if((m_nViewTopLine+m_nViewRowNum+1>=m_pcEditDoc->m_cLayoutMgr.GetLineCount()) &&
		//   (m_nSelectLineTo+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ||
		//	m_nSelectLineToOld+1 >= m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) ) {
		//	Redraw();
		//	return;
		//}

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

		if( rcNew.left <= rcNew.right ){
			/* ::CombineRgn()�̌��ʂ��󂯎�邽�߂ɁA�K���ȃ��[�W��������� */
			hrgnDraw = ::CreateRectRgnIndirect( &rcNew );

			/* ���I����`�ƐV�I����`�̃��[�W������������� �d�Ȃ肠�������������������܂� */
			if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnOld, hrgnNew, RGN_XOR ) ){
				::PaintRgn( hdc, hrgnDraw );

				// 2002.02.16 hor
				// ������̃G���A��EOF���܂܂��ꍇ��EOF�ȍ~�̕������������܂�
				int  nLastLen;
				int  nLastLine=m_pcEditDoc->m_cLayoutMgr.GetLineCount()-1;
				const char* pLine;
				const CLayout* pcLayout;
				if(m_nViewTopLine+m_nViewRowNum+1>=nLastLine) {
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLastLine, &nLastLen ,&pcLayout);
					if( NULL != pcLayout && EOL_NONE != pcLayout->m_cEol ){
						nLastLine++;
						nLastLen=0;
					}
					if(m_nSelectLineFrom>=nLastLine || m_nSelectLineTo>=nLastLine ||
					   m_nSelectLineFromOld>=nLastLine || m_nSelectLineToOld>=nLastLine){
						rcNew.left = m_nViewAlignLeft + ( m_nViewLeftCol + nLastLen ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
						rcNew.right = m_nViewAlignLeft + m_nViewCx;
						rcNew.top = ( nLastLine - m_nViewTopLine) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
						rcNew.bottom = rcNew.top + ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace );
						hrgnNew = ::CreateRectRgnIndirect( &rcNew );
						if( NULLREGION != ::CombineRgn( hrgnDraw, hrgnDraw, hrgnNew, RGN_AND ) ){
							::PaintRgn( hdc, hrgnDraw );
						}
					}
				}

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
	const char*		pLine;
	int				nLineLen;
	RECT			rcClip;
	int				nSelectFrom;
	int				nSelectTo;
	const CLayout*	pcLayout;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );

	int nPosX = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			nPosX ++;
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn();

	if( nFromLine == nToLine ){
			nSelectFrom = nFromCol;
			nSelectTo	= nToCol;
	}else{
		if( nLineNum == nFromLine ){
			nSelectFrom = nFromCol;
			nSelectTo	= nPosX;
		}else
		if( nLineNum == nToLine ){
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nToCol;
		}else{
			nSelectFrom = pcLayout ? pcLayout->GetIndent() : 0;
			nSelectTo	= nPosX;
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
	//	�K�v�ȂƂ������B
	if ( rcClip.right != rcClip.left ){
		m_cUnderLine.CaretUnderLineOFF( TRUE );	//	YAZAKI

		HRGN hrgnDraw = ::CreateRectRgn( rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );
		::PaintRgn( hdc, hrgnDraw );
		::DeleteObject( hrgnDraw );
	}


//	::Rectangle( hdc, rcClip.left, rcClip.top, rcClip.right + 1, rcClip.bottom + 1);
//	::FillRect( hdc, &rcClip, hBrushTextCol );

//	//	/* �f�o�b�O���j�^�ɏo�� */
//	m_cShareData.TraceOut( "DrawSelectAreaLine() rcClip.left=%d, rcClip.top=%d, rcClip.right=%d, rcClip.bottom=%d\n", rcClip.left, rcClip.top, rcClip.right, rcClip.bottom );

	return;
}





/* �e�L�X�g���I������Ă��邩 */
// 2002/03/29 Azumaiya
// �C�����C���֐��ɕύX(�w�b�_�ɋL�q)�B
/*BOOL CEditView::IsTextSelected( void )
{
	if(
		m_nSelectLineFrom	== -1 ||
		m_nSelectLineTo		== -1 ||
		m_nSelectColmFrom	== -1 ||
		m_nSelectColmTo		== -1
	)
	{
//	if( m_nSelectLineFrom == m_nSelectLineTo &&
//		m_nSelectColmFrom  == m_nSelectColmTo ){
		return FALSE;
	}
	return TRUE;

//	return ~(m_nSelectLineFrom|m_nSelectLineTo|m_nSelectColmFrom|m_nSelectColmTo) >> 31;
}*/


/* �e�L�X�g�̑I�𒆂� */
// 2002/03/29 Azumaiya
// �C�����C���֐��ɕύX(�w�b�_�ɋL�q)�B
/*BOOL CEditView::IsTextSelecting( void )
{
	if( m_bBeginSelect ||
		IsTextSelected()
	){
//		MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d TRUE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
		return TRUE;
	}
//	MYTRACE( "m_bBeginSelect=%d IsTextSelected()=%d FALSE==IsTextSelecting()\n", m_bBeginSelect, IsTextSelected() );
	return FALSE;

//	return m_bSelectingLock|IsTextSelected();
}*/


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
//	2002/05/12 YAZAKI �s�v�Ǝv��ꂽ�̂ŁB
//	if( m_nCaretWidth == 0 ){	/* �L�����b�g���Ȃ������ꍇ */
//	}else{
//		OnKillFocus();
//		OnSetFocus();
//	}
	//	Oct. 11, 2002 genta IME�̃t�H���g���ύX
	SetIMECompFormFont();
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
	//	Sep 18, 2002 genta
	m_nViewAlignLeftNew += m_pShareData->m_Common.m_nLineNumRightSpace;
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
//			OnKillFocus();
			m_cUnderLine.Lock();
			OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
			m_cUnderLine.UnLock();
//			OnSetFocus();
//			DispRuler( hdc );
			ShowEditCaret();
			::ReleaseDC( m_hWnd, hdc );
		}
		m_bRedrawRuler = true;
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
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = nAllLines / nVScrollRate - 1;	/* �S�s�� */
		si.nPage = m_nViewRowNum / nVScrollRate;	/* �\����̍s�� */
		si.nPos  = m_nViewTopLine / nVScrollRate;	/* �\����̈�ԏ�̍s(0�J�n) */
		si.nTrackPos = nVScrollRate;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* �����X�N���[���o�[�̏k�� */
		
		//	Nov. 16, 2002 genta
		//	�c�X�N���[���o�[��Disable�ɂȂ����Ƃ��͕K���S�̂���ʓ��Ɏ��܂�悤��
		//	�X�N���[��������
		if( m_nViewRowNum > nAllLines ){
			ScrollAtV( 0 );
		}
	}
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;

//@@		::GetScrollInfo( m_hwndHScrollBar, SB_CTL, &si );
//@@		if( si.nMax == m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize - 1
//@@		 && si.nPage == (UINT)m_nViewColNum
//@@		 && si.nPos  == m_nViewLeftCol
//@@	   /*&& si.nTrackPos == 1*/ ){
//@@		}else{
			/* �����X�N���[���o�[ */
//			si.cbSize = sizeof( si );
//			si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
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
	@date 2001/10/20 deleted by novice AdjustScrollBar()���ĂԈʒu��ύX
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
//	if (IsTextSelected()) { //2002.02.27 Add By KK �A���_�[���C���̂������ጸ - �����ł̓e�L�X�g�I�����̂݃A���_�[���C���������B
		m_cUnderLine.CaretUnderLineOFF( bDraw );	//	YAZAKI
//	}	2002/04/04 YAZAKI ���y�[�W�X�N���[�����ɃA���_�[���C�����c�����܂܃X�N���[�����Ă��܂����ɑΏ��B

	if( m_bBeginSelect ){	/* �͈͑I�� */
		nCaretMarginY = 0;
	}else{
		//	2001/10/20 novice
		nCaretMarginY = m_nViewRowNum / nCaretMarginRate;
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
			::InvalidateRect( m_hWnd, NULL, TRUE );
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
			}
		}

		/* �X�N���[���o�[�̏�Ԃ��X�V���� */
		AdjustScrollBars(); // 2001/10/20 novice
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
	// ���X�N���[��������������A���[���[�S�̂��ĕ`�� 2002.02.25 Add By KK
	if (nScrollColNum != 0 ){
		//����DispRuler�Ăяo�����ɍĕ`��B�ibDraw=false�̃P�[�X���l�������B�j
		m_bRedrawRuler = true;
	}

	/* �J�[�\���s�A���_�[���C����ON */
	//CaretUnderLineON( bDraw ); //2002.02.27 Del By KK �A���_�[���C���̂������ጸ
	if( bDraw ){
		/* �L�����b�g�̕\���E�X�V */
		ShowEditCaret();

		/* ���[���̍ĕ`�� */
		DispRuler( hdc );

		/* �A���_�[���C���̍ĕ`�� */
		m_cUnderLine.CaretUnderLineON(TRUE);

		/* �L�����b�g�̍s���ʒu��\������ */
		DrawCaretPosInfo();
	}
	::ReleaseDC( m_hWnd, hdc );

//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
	if(nScrollRowNum != 0)
	{
		if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
		{
			CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
			pcEditView -> ScrollAtV( m_nViewTopLine );
		}
	}
	if(nScrollColNum != 0)
	{
		if( m_pShareData->m_Common.m_bSplitterWndHScroll && m_pcEditDoc->m_cSplitterWnd.GetAllSplitRows() == 2 )	// �����X�N���[���̓������Ƃ�
		{
			CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x02];
			HDC			hdc = ::GetDC( pcEditView->m_hWnd );
			pcEditView -> ScrollAtH( m_nViewLeftCol );
			m_bRedrawRuler = true; //2002.02.25 Add By KK �X�N���[�������[���[�S�̂�`���Ȃ����B
			DispRuler( hdc );
			::ReleaseDC( pcEditView->m_hWnd, hdc );
		}
	}
//	2001/06/20 End

// 02/09/18 �Ί��ʂ̋����\�� ai Start
	if( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		DrawBracketPair();
	}
// 02/09/18 �Ί��ʂ̋����\�� ai End

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
	int				nNewX;
	int				nNewY;
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
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nNewY, &nLineLen, &pcLayout );
		if( NULL == pLine ){
			nNewX = nLineLen;
		}else
		/* ���s�ŏI����Ă��邩 */
		if( EOL_NONE != pcLayout->m_cEol.GetLen() ){
			nNewX = 0;
			++nNewY;
		}else{
			nNewX = LineIndexToColmn( pcLayout, nLineLen );
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
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nNewY, &nLineLen, &pcLayout );

		int nPosX = 0;
		int i = 0;
		CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
		while( !it.end() ){
			it.scanNext();
			if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
				i = nLineLen;
				break;
			}
			if( it.getColumn() + it.getColumnDelta() > nNewX ){
				if (nNewX >= (pcLayout ? pcLayout->GetIndent() : 0) && (it.getColumnDelta() > 1) && ((it.getColumn() + it.getColumnDelta() - nNewX) <= it.getColumnDelta() / 2)){
					nPosX += it.getColumnDelta();
				}
				i = it.getIndex();
				break;
			}
			it.addDelta();
		}
		nPosX += it.getColumn();
		if ( it.end() ){
			i = it.getIndex();
			nPosX -= it.getColumnDelta();
		}

		if( i >= nLineLen ){
// From 2001.12.21 hor �t���[�J�[�\��OFF��EOF�̂���s�̒��O���}�E�X�őI���ł��Ȃ��o�O�̏C��
			if( nNewY +1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
				EOL_NONE == pcLayout->m_cEol.GetLen() ){
				nPosX = LineIndexToColmn( pcLayout, nLineLen );
			}else
// To 2001.12.21 hor
			/* �t���[�J�[�\�����[�h�� */
			if( m_pShareData->m_Common.m_bIsFreeCursorMode
			  || ( m_bBeginSelect && m_bBeginBoxSelect )	/* �}�E�X�͈͑I�� && ��`�͈͑I�� */
//			  || m_bDragMode /* OLE DropTarget */
			  || ( m_bDragMode && m_bBeginBoxSelect ) /* OLE DropTarget && ��`�͈͑I�� */
			){
// From 2001.12.21 hor
//				if( nNewY + 1 == m_pcEditDoc->m_cLayoutMgr.GetLineCount() &&
//					pLine[ nLineLen - 1 ] != '\n' && pLine[ nLineLen - 1 ] != '\r'
//				){
//					nPosX = LineIndexToColmn( pLine, nLineLen, nLineLen );
//				}else{
// To 2001.12.21 hor
					nPosX = nNewX;
					if( nPosX < 0 ){
						nPosX = 0;
					}else
					if( nPosX > m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize ){	/* �܂�Ԃ������� */
						nPosX = m_pcEditDoc->GetDocumentAttribute().m_nMaxLineSize;
					}
//				}
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

	/* ����Tip���N������Ă��� */
	if( 0 == m_dwTipTimer ){
		/* ����Tip������ */
		m_cTipWnd.Hide();
		m_dwTipTimer = ::GetTickCount();	/* ����Tip�N���^�C�}�[ */
	}else{
		m_dwTipTimer = ::GetTickCount();		/* ����Tip�N���^�C�}�[ */
	}

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
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
					DWORD dwEffects;
					m_bDragSource = TRUE;
					CDataObject data( cmemCurText.GetPtr() );
					dwEffects = data.DragDrop( TRUE, DROPEFFECT_COPY | DROPEFFECT_MOVE );
					m_bDragSource = FALSE;
//					MYTRACE( "dwEffects=%d\n", dwEffects );
					if( 0 == dwEffects ){
						if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
							/* ���݂̑I��͈͂��I����Ԃɖ߂� */
							DisableSelectArea( TRUE );
							
//@@@ 2002.01.08 YAZAKI �t���[�J�[�\��OFF�ŕ����s�I�����A�s�̌����N���b�N����Ƃ����ɃL�����b�g���u����Ă��܂��o�O�C��
							/* �J�[�\���ړ��B */
							if( yPos >= m_nViewAlignTop && yPos < m_nViewAlignTop  + m_nViewCy ){
								if( xPos >= m_nViewAlignLeft && xPos < m_nViewAlignLeft + m_nViewCx ){
									MoveCursorToPoint( xPos, yPos );
								}else
								if( xPos < m_nViewAlignLeft ){
									MoveCursorToPoint( m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ), yPos );
								}
							}
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
		HideCaret_( m_hWnd ); // 2002/07/22 novice
		/* ���݂̃J�[�\���ʒu����I�����J�n���� */
		BeginSelectArea( );
		m_cUnderLine.CaretUnderLineOFF( TRUE );
		m_cUnderLine.Lock();
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
		}
		else
		if( yPos < m_nViewAlignTop ){
			//	���[���N���b�N
			return;
		}
		else {
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
		HideCaret_( m_hWnd ); // 2002/07/22 novice


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


		/******* ���̎��_�ŕK�� true == IsTextSelected() �̏�ԂɂȂ� ****:*/
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
				if ( Command_SELECTWORD() ){
					m_nSelectLineBgnFrom = m_nSelectLineFrom;	/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = m_nSelectColmFrom;	/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnTo = m_nSelectLineTo;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = m_nSelectColmTo;		/* �͈͑I���J�n��(���_) */
				}
			}else{

				/* �I��̈�`�� */
				DrawSelectArea();


				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				const CLayout* pcLayout;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_nSelectColmFrom );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineFrom, nIdx, &nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );


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
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineTo, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					nIdx = LineColmnToIndex( pcLayout, m_nSelectColmTo );
					/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
					if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
						m_nSelectLineTo, nIdx,
						&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
					){
						/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
						nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
						pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
						nColmTo = LineIndexToColmn( pcLayout, nColmTo );

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

			// 2002.10.07 YAZAKI �܂�Ԃ��s���C���f���g���Ă���Ƃ��ɑI�������������o�O�̑΍�
			Command_GOLINEEND( TRUE, FALSE );
			Command_RIGHT( TRUE, FALSE, FALSE );

			m_nSelectLineBgnTo = m_nSelectLineTo;	/* �͈͑I���J�n�s(���_) */
			m_nSelectColmBgnTo = m_nSelectColmTo;	/* �͈͑I���J�n��(���_) */
		}else{
//			/* ���݂̃J�[�\���ʒu����I�����J�n���� */
//			BeginSelectArea( );
//			m_bBeginLineSelect = FALSE;

			/* URL���N���b�N���ꂽ��I�����邩 */
			if( TRUE == m_pShareData->m_Common.m_bSelectClickedURL ){

				int			nUrlLine;	// URL�̍s(�܂�Ԃ��P��)
				int			nUrlIdxBgn;	// URL�̈ʒu(�s������̃o�C�g�ʒu)
				int			nUrlLen;	// URL�̒���(�o�C�g��)
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
#if 0
					2002/04/03 YAZAKI �s�v�ȏ����ł����B
					m_nSelectLineBgnFrom = nUrlLine;			/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = nUrlIdxBgn;			/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnTo = nUrlLine;				/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = nUrlIdxBgn + nUrlLen;	/* �͈͑I���J�n��(���_) */

					m_nSelectLineFrom =	nUrlLine;
					m_nSelectColmFrom = nUrlIdxBgn;
					m_nSelectLineTo = nUrlLine;
					m_nSelectColmTo = nUrlIdxBgn + nUrlLen;
#endif
					/*
					  �J�[�\���ʒu�ϊ�
					  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
					  �����C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
						2002/04/08 YAZAKI �����ł��킩��₷���B
					*/
					int nColmFrom, nLineFrom, nColmTo, nLineTo;
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nUrlIdxBgn          , nUrlLine, &nColmFrom, &nLineFrom );
					m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( nUrlIdxBgn + nUrlLen, nUrlLine, &nColmTo, &nLineTo );

					m_nSelectLineBgnFrom = nLineFrom;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnFrom = nColmFrom;		/* �͈͑I���J�n��(���_) */
					m_nSelectLineBgnTo = nLineTo;		/* �͈͑I���J�n�s(���_) */
					m_nSelectColmBgnTo = nColmTo;		/* �͈͑I���J�n��(���_) */

					m_nSelectLineFrom = nLineFrom;
					m_nSelectColmFrom = nColmFrom;
					m_nSelectLineTo = nLineTo;
					m_nSelectColmTo = nColmTo;

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
	MY_RUNNINGTIMER( cRunningTimer, "CEditView::IsCurrentPositionURL" );

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
// 2001.12.21 hor �ȉ��A���s����Ȃ��̂ŃR�����g�A�E�g���܂� (�s��////�͂��Ƃ��ƃR�����g�s�ł�)
//	int			nIdx;
//	int			nFuncID;
//	nIdx = 0;
//	/* Ctrl,ALT,�L�[��������Ă����� */
//	if( (SHORT)0x8000 & ::GetKeyState( VK_SHIFT ) ){
//		nIdx |= _SHIFT;
//	}
//	if( (SHORT)0x8000 & ::GetKeyState( VK_CONTROL ) ){
//		nIdx |= _CTRL;
//	}
//	if( (SHORT)0x8000 & ::GetKeyState( VK_MENU ) ){
//		nIdx |= _ALT;
//	}
//	/* �}�E�X�E�N���b�N�ɑΉ�����@�\�R�[�h��m_Common.m_pKeyNameArr[1]�ɓ����Ă��� */
//	nFuncID = m_pShareData->m_pKeyNameArr[1].m_nFuncCodeArr[nIdx];
//	if( nFuncID != 0 ){
//		/* �R�}���h�R�[�h�ɂ�鏈���U�蕪�� */
//		::PostMessage( ::GetParent( m_hwndParent ), WM_COMMAND, MAKELONG( nFuncID, 0 ),  (LPARAM)NULL );
//	}
////	/* �E�N���b�N���j���[ */
////	Command_MENU_RBUTTON();
//	return;
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
					if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
						pszWork = cmemCurText.GetPtr();
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
//							if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
							if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
								/* �Y������L�[������ */
								m_cTipWnd.m_KeyWasHit = TRUE;
								pszWork = pcmemRefText->GetPtr();
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

// 2001.12.21 hor �ȉ��A���s����Ȃ��̂ŃR�����g�A�E�g���܂� (�s��////�͂��Ƃ��ƃR�����g�s�ł�)
////		rc.top += m_nViewAlignTop;
//		RECT rc2;
//		rc2 = rc;
//		rc2.bottom = rc.top + m_nViewAlignTop + ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//		 && 0 < m_nViewTopLine
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//		rc2 = rc;
//		rc2.top = rc.bottom - ( m_pcEditDoc->GetDocumentAttribute().m_nLineSpace + m_nCharHeight );
//		if( PtInRect( &rc2, po )
//			&& m_pcEditDoc->m_cLayoutMgr.GetLineCount() > m_nViewTopLine + m_nViewRowNum
//		){
//			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
//			return;
//		}
//
////		rc.top += 48;
////		rc.bottom -= 48;
////		if( !PtInRect( &rc, po ) ){
////			OnMOUSEMOVE( 0, m_nMouseRollPosXOld, m_nMouseRollPosYOld );
////		}
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
			if( xPos < m_nViewAlignLeft || yPos < m_nViewAlignTop ){	//	2002/2/10 aroka
				/* ���J�[�\�� */
				if( yPos >= m_nViewAlignTop )
					::SetCursor( ::LoadCursor( m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_RVARROW ) ) );
				else
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
			const CLayout* pcLayout;
			if( NULL != ( pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout ) ) ){
				nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );
				/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
				if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
					m_nCaretPosY, nIdx,
					&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
				){
					/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
					nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
					pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
					nColmTo = LineIndexToColmn( pcLayout, nColmTo );

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
						/* �n�_���O���Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
						ChangeSelectAreaByCurrentCursor( nColmFrom, nLineFrom );
					}else
					if( /*0 == nWorkT ||*/ 1 == nWorkT ){
						/* �I�_������Ɉړ��B���݂̃J�[�\���ʒu�ɂ���đI��͈͂�ύX */
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
		if( nRollLineNum > 30 ){	//@@@ YAZAKI 2001.12.31 10��30�ցB
			nRollLineNum = 30;
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
//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
		if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
		{
			m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01].ScrollAtV( m_nViewTopLine );
		}
//	2001/06/20 End
	}
	return 0;
}





/* ���݂̃J�[�\���ʒu����I�����J�n���� */
void CEditView::BeginSelectArea( void )
{
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
		m_bDrawSelectArea = FALSE;	// 02/12/13 ai
	}
	m_bSelectingLock	 = FALSE;	/* �I����Ԃ̃��b�N */
	m_nSelectLineFromOld = 0;		/* �͈͑I���J�n�s */
	m_nSelectColmFromOld = 0; 		/* �͈͑I���J�n�� */
	m_nSelectLineToOld = 0;			/* �͈͑I���I���s */
	m_nSelectColmToOld = 0;			/* �͈͑I���I���� */
	m_bBeginBoxSelect = FALSE;		/* ��`�͈͑I�� */
	m_bBeginLineSelect = FALSE;		/* �s�P�ʑI�� */
	m_bBeginWordSelect = FALSE;		/* �P��P�ʑI�� */

	// 2002.02.16 hor ���O�̃J�[�\���ʒu�����Z�b�g
	m_nCaretPosX_Prev=m_nCaretPosX;

	//	From Here Dec. 6, 2000 genta
	//	To Here Dec. 6, 2000 genta

	/* �J�[�\���s�A���_�[���C����ON */
	m_cUnderLine.CaretUnderLineON( bDraw );
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

	//	2002/04/08 YAZAKI �R�[�h�̏d����r��
	ChangeSelectAreaByCurrentCursorTEST(
		nCaretPosX,
		nCaretPosY, 
		m_nSelectLineFrom,
		m_nSelectColmFrom,
		m_nSelectLineTo,
		m_nSelectColmTo
	);
#if 0
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
#endif
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
			if ( nCaretPosY == m_nSelectLineBgnFrom && nCaretPosX == m_nSelectColmBgnFrom ){
				nSelectLineTo = m_nSelectLineBgnTo;	//	m_nSelectLineBgnTo;
				nSelectColmTo = m_nSelectColmBgnTo;	//	m_nSelectColmBgnTo;
			}
			else {
				nSelectLineTo = nCaretPosY;	//	m_nSelectLineBgnTo;
				nSelectColmTo = nCaretPosX;	//	m_nSelectColmBgnTo;
			}
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
		ShowCaret_( m_hWnd ); // 2002/07/22 novice

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
		m_cUnderLine.UnLock();
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
	HideCaret_( m_hWnd ); // 2002/07/22 novice
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
	int				nPosX = 0;
	int				nPosY = m_nCaretPosY;
	int				i;
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
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				nLineCols = LineIndexToColmn( pcLayout, nLineLen );
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
					nScrollLines = MoveCursor( nPosX, nPosY, m_bDrawSWITCH /* TRUE */ ); // YAZAKI.
				}
			}
//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
			if(nScrollLines != 0)
			{
				if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
				{
					CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
					pcEditView -> ScrollAtV( m_nViewTopLine );
				}
			}
//	2001/06/20 End
			return nScrollLines;
		}
	}else{
		/* �J�[�\�����e�L�X�g�ŏ�[�s�ɂ��邩 */
		if( m_nCaretPosY + nMoveLines < 0 ){
			nMoveLines = - m_nCaretPosY;
		}
		if( nMoveLines >= 0 ){
//	2001/06/20 Start by asa-o:	�����E�B���h�E�̃X�N���[���̓���
			if(nScrollLines != 0)
			{
				if( m_pShareData->m_Common.m_bSplitterWndVScroll )	// �����X�N���[���̓������Ƃ�
				{
					CEditView*	pcEditView = &m_pcEditDoc->m_cEditViewArr[m_nMyIndex^0x01];
					pcEditView -> ScrollAtV( m_nViewTopLine );
				}
			}
//	2001/06/20 End
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
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY + nMoveLines, &nLineLen, &pcLayout );
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > pcLayout->GetLengthWithoutEOL() ){
			i = nLineLen;
			break;
		}
		if( it.getColumn() + it.getColumnDelta() > m_nCaretPosX_Prev ){
			i = it.getIndex();
			break;
		}
		it.addDelta();
	}
	nPosX += it.getColumn();
	if ( it.end() ){
		i = it.getIndex();
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
	nScrollLines = MoveCursor( nPosX, m_nCaretPosY + nMoveLines, m_bDrawSWITCH /* TRUE */ ); // YAZAKI.
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
		return;	//	�X�N���[�������B
	}
	/* �����X�N���[���ʁi�s���j�̎Z�o */
	nScrollRowNum = m_nViewTopLine - nPos;

	/* �X�N���[�� */
	if( abs( nScrollRowNum ) >= m_nViewRowNum ){
		m_nViewTopLine = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
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
		if( m_bDrawSWITCH ){
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
	RECT		rcClip2;
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

	/* �X�N���[�� */
	if( abs( nScrollColNum ) >= m_nViewColNum /*|| abs( nScrollRowNum ) >= m_nViewRowNum*/ ){
		m_nViewLeftCol = nPos;
		::InvalidateRect( m_hWnd, NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = m_nViewCx + m_nViewAlignLeft;
		rcScrol.top = m_nViewAlignTop;
		rcScrol.bottom = m_nViewCy + m_nViewAlignTop;
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
	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	ShowEditCaret();

	/* ���[���[�ĕ`�� */
	HDC hdc = ::GetDC( m_hWnd );
	DispRuler( hdc );
	::ReleaseDC( m_hWnd, hdc );
	return;
}

/* �I��͈͂̃f�[�^���擾 */
/* ���펞��TRUE,�͈͖��I���̏ꍇ��FALSE��Ԃ� */
BOOL CEditView::GetSelectedData(
		CMemory&	cmemBuf,
		BOOL		bLineOnly,
		const char*	pszQuote,			/* �擪�ɕt������p�� */
		BOOL		bWithLineNumber,	/* �s�ԍ���t�^���� */
		BOOL		bAddCRLFWhenCopy,	/* �܂�Ԃ��ʒu�ŉ��s�L�������� */
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

		//<< 2002/04/18 Azumaiya
		// �T�C�Y�������v�̂��Ƃ��Ă����B
		// ���\��܂��Ɍ��Ă��܂��B
		int i = rcSel.bottom - rcSel.top;

		// �ŏ��ɍs�����̉��s�ʂ��v�Z���Ă��܂��B
		int nBufSize = strlen(CRLF) * i;

		// ���ۂ̕����ʁB
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( rcSel.top, &nLineLen, &pcLayout );
		for(; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext)
		{
			pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;
			nLineLen = pcLayout->m_nLength;
			if( NULL != pLine )
			{
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

				nBufSize += nIdxTo - nIdxFrom;
			}
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ��܂��Ɍ����e�ʂ����ɃT�C�Y�����炩���ߊm�ۂ��Ă����B
		cmemBuf.AllocBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		nRowNum = 0;
		for( nLineNum = rcSel.top; nLineNum <= rcSel.bottom; ++nLineNum ){
//			if( nRowNum > 0 ){
//				cmemBuf.AppendSz( CRLF );
//				if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
//					break;
//				}
//			}
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left  );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );
				//2002.02.08 hor
				// pLine��NULL�̂Ƃ�(��`�G���A�̒[��EOF�݂̂̍s���܂ނƂ�)�͈ȉ����������Ȃ�
				if( nIdxTo - nIdxFrom > 0 ){
					if( pLine[nIdxTo - 1] == '\n' || pLine[nIdxTo - 1] == '\r' ){
						cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom - 1 );
					}else{
						cmemBuf.Append( &pLine[nIdxFrom], nIdxTo - nIdxFrom );
					}
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

		//<< 2002/04/18 Azumaiya
		//  ���ꂩ��\��t���Ɏg���̈�̑�܂��ȃT�C�Y���擾����B
		//  ��܂��Ƃ������x���ł��̂ŁA�T�C�Y�v�Z�̌덷���i�e�ʂ𑽂����ς�����Ɂj���\�o��Ǝv���܂����A
		// �܂��A�����D��Ƃ������ƂŊ��ق��Ă��������B
		//  ���ʂȗe�ʊm�ۂ��o�Ă��܂��̂ŁA�����������x���グ�����Ƃ���ł����E�E�E�B
		//  �Ƃ͂����A�t�ɏ��������ς��邱�ƂɂȂ��Ă��܂��ƁA���Ȃ葬�x���Ƃ���v���ɂȂ��Ă��܂��̂�
		// �����Ă��܂��Ƃ���ł����E�E�E�B
		m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nSelectLineFrom, &nLineLen, &pcLayout );
		int nBufSize = 0;
		int i = m_nSelectLineTo - m_nSelectLineFrom;
		// �擪�Ɉ��p����t����Ƃ��B
		if ( NULL != pszQuote )
		{
			nBufSize += strlen(pszQuote);
		}

		// �s�ԍ���t����B
		if ( bWithLineNumber )
		{
			nBufSize += nLineNumCols;
		}

		// ���s�R�[�h�ɂ��āB
		if ( neweol == EOL_UNKNOWN )
		{
			nBufSize += strlen(CRLF);
		}
		else
		{
			nBufSize += appendEol.GetLen();
		}

		// ���ׂĂ̍s�ɂ��ē��l�̑��������̂ŁA�s���{����B
		nBufSize *= i;

		// ���ۂ̊e�s�̒����B
		for (; i != 0 && pcLayout != NULL; i--, pcLayout = pcLayout->m_pNext )
		{
			nBufSize += pcLayout->m_nLength + appendEol.GetLen();
			if( bLineOnly ){	/* �����s�I���̏ꍇ�͐擪�̍s�̂� */
				break;
			}
		}

		// ���ׂ������������o�b�t�@������Ă����B
		cmemBuf.AllocBuffer(nBufSize);
		//>> 2002/04/18 Azumaiya

		for( nLineNum = m_nSelectLineFrom; nLineNum <= m_nSelectLineTo; ++nLineNum ){
//			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen );
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL == pLine ){
				break;
			}
			if( nLineNum == m_nSelectLineFrom ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom = LineColmnToIndex( pcLayout, m_nSelectColmFrom );
			}else{
				nIdxFrom = 0;
			}
			if( nLineNum == m_nSelectLineTo ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxTo = LineColmnToIndex( pcLayout, m_nSelectColmTo );
			}else{
				nIdxTo = nLineLen;
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
					if( bAddCRLFWhenCopy ||  /* �܂�Ԃ��s�ɉ��s��t���ăR�s�[ */
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
//	OnKillFocus();
	OnPaint( hdc, &ps, TRUE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );
	/* �I��͈͂��N���b�v�{�[�h�ɃR�s�[ */
	/* �I��͈͂̃f�[�^���擾 */
	/* ���펞��TRUE,�͈͖��I���̏ꍇ�͏I������ */
	if( FALSE == GetSelectedData(
		cmemBuf,
		FALSE,
		pszQuote, /* ���p�� */
		bWithLineNumber, /* �s�ԍ���t�^���� */
		m_pShareData->m_Common.m_bAddCRLFWhenCopy /* �܂�Ԃ��ʒu�ɉ��s�L�������� */
	) ){
		::MessageBeep( MB_ICONHAND );
		return;
	}
	/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
	MySetClipboardData( cmemBuf.GetPtr(), cmemBuf.GetLength(), FALSE );


//	/* Windows�N���b�v�{�[�h�ɃR�s�[ */
//	hgClip = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_DDESHARE, cmemBuf.GetLength() + 1 );
//	pszClip = (char*)::GlobalLock( hgClip );
//	memcpy( pszClip, cmemBuf.GetPtr(), cmemBuf.GetLength() + 1 );
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
			const CLayout* pcLayout;
			nDelPosNext = nIdxFrom;
			nDelLenNext	= nIdxTo - nIdxFrom;
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
				nIdxFrom	= LineColmnToIndex( pcLayout, rcSel.left );
				nIdxTo		= LineColmnToIndex( pcLayout, rcSel.right );

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
				pLine2 = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineNum + 1, &nLineLen2, &pcLayout );
				nPosX = LineIndexToColmn( pcLayout, nDelPos );
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
					pcOpe		/* �ҏW����v�f COpe */
//					FALSE,
//					FALSE
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
					cmemBuf.GetPtr(),
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
		GetSelectedData( cmemBuf, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy );

		/* �@�\��ʂɂ��o�b�t�@�̕ϊ� */
		ConvMemory( &cmemBuf, nFuncCode );

//		/* �I���G���A���폜 */
//		DeleteData( FALSE );

		int nCaretPosYOLD=m_nCaretPosY;

		/* �f�[�^�u�� �폜&�}���ɂ��g���� */
		ReplaceData_CEditView(
			m_nSelectLineFrom,		/* �͈͑I���J�n�s */
			m_nSelectColmFrom,		/* �͈͑I���J�n�� */
			m_nSelectLineTo,		/* �͈͑I���I���s */
			m_nSelectColmTo,		/* �͈͑I���I���� */
			NULL,					/* �폜���ꂽ�f�[�^�̃R�s�[(NULL�\) */
			cmemBuf.GetPtr(),		/* �}������f�[�^ */ // 2002/2/10 aroka CMemory�ύX
			cmemBuf.GetLength(),		/* �}������f�[�^�̒��� */ // 2002/2/10 aroka CMemory�ύX
			FALSE/*TRUEbRedraw*/
		);

		// From Here 2001.12.03 hor
		//	�I���G���A�̕���
		m_nSelectLineFrom	=	nSelectLineFromOld;	/* �͈͑I���J�n�s */
		m_nSelectColmFrom	=	nSelectColFromOld;	/* �͈͑I���J�n�� */
		m_nSelectLineTo		=	m_nCaretPosY;		/* �͈͑I���I���s */
		m_nSelectColmTo		=	m_nCaretPosX;		/* �͈͑I���I���� */
		if(nCaretPosYOLD==m_nSelectLineFrom) {
			MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );
		}else{
			MoveCursor( m_nSelectColmTo, m_nSelectLineTo, TRUE );
		}
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
		RedrawAll();
		// To Here 2001.12.03 hor

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
			cmemBuf.GetPtr(),
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

	m_pcEditDoc->SetModified(true);	/* �ύX�t���O */

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
	case F_TOHANKAKU: pCMemory->ToHankaku( 0x0 ); break;					/* �S�p�����p */
	case F_TOHANKATA: pCMemory->ToHankaku( 0x01 ); break;					/* �S�p�J�^�J�i�����p�J�^�J�i */	// Aug. 29, 2002 ai
	case F_TOZENEI: pCMemory->ToZenkaku( 2, 0 );				/* 2== �p����p				*/ break;	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOHANEI: pCMemory->ToHankaku( 0x4 );						/* 2== �p����p				*/ break;	/* ���p�p�����S�p�p�� */			//July. 30, 2001 Misaka
	case F_TOZENKAKUKATA: pCMemory->ToZenkaku( 0, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�Ђ灨�S�p�E�J�^�J�i */	//Sept. 17, 2000 jepro �������u���p���S�p�J�^�J�i�v����ύX
	case F_TOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 0 );			/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�{�S�J�^���S�p�E�Ђ炪�� */	//Sept. 17, 2000 jepro �������u���p���S�p�Ђ炪�ȁv����ύX
	case F_HANKATATOZENKAKUKATA: pCMemory->ToZenkaku( 0, 1 );	/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�J�^�J�i */
	case F_HANKATATOZENKAKUHIRA: pCMemory->ToZenkaku( 1, 1 );	/* 1== �Ђ炪�� 0==�J�^�J�i */ break;	/* ���p�J�^�J�i���S�p�Ђ炪�� */
	case F_CODECNV_EMAIL:		pCMemory->JIStoSJIS(); break;		/* E-Mail(JIS��SJIS)�R�[�h�ϊ� */
	case F_CODECNV_EUC2SJIS:	pCMemory->EUCToSJIS(); break;		/* EUC��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODE2SJIS:pCMemory->UnicodeToSJIS(); break;	/* Unicode��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UNICODEBE2SJIS: pCMemory->UnicodeBEToSJIS(); break;	/* UnicodeBE��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2JIS:	pCMemory->SJIStoJIS();break;		/* SJIS��JIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2EUC: 	pCMemory->SJISToEUC();break;		/* SJIS��EUC�R�[�h�ϊ� */
	case F_CODECNV_UTF82SJIS:	pCMemory->UTF8ToSJIS();break;		/* UTF-8��SJIS�R�[�h�ϊ� */
	case F_CODECNV_UTF72SJIS:	pCMemory->UTF7ToSJIS();break;		/* UTF-7��SJIS�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF7:	pCMemory->SJISToUTF7();break;		/* SJIS��UTF-7�R�[�h�ϊ� */
	case F_CODECNV_SJIS2UTF8:	pCMemory->SJISToUTF8();break;		/* SJIS��UTF-8�R�[�h�ϊ� */
	case F_CODECNV_AUTO2SJIS:	pCMemory->AUTOToSJIS();break;		/* �������ʁ�SJIS�R�[�h�ϊ� */
	case F_TABTOSPACE:
		pCMemory->TABToSPACE(
			//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace()
		);break;	/* TAB���� */
	case F_SPACETOTAB:	//#### Stonee, 2001/05/27
		pCMemory->SPACEToTAB(
			//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
			m_pcEditDoc->m_cLayoutMgr.GetTabSpace()
		);
		break;		/* �󔒁�TAB */
	case F_LTRIM:	Command_TRIM2( pCMemory , TRUE  );break;	// 2001.12.03 hor
	case F_RTRIM:	Command_TRIM2( pCMemory , FALSE );break;	// 2001.12.03 hor
	}
	return;

}



/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI CDocLine��
*/
int CEditView::LineColmnToIndex( const CDocLine* pcDocLine, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}


/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1
	
	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineColmnToIndex( const CLayout* pcLayout, int nColumn )
{
	int i2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	return i2;
}



/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver0 */
/* �w�肳�ꂽ�����A�s���Z���ꍇ��pnLineAllColLen�ɍs�S�̂̕\��������Ԃ� */
/* ����ȊO�̏ꍇ��pnLineAllColLen�ɂO���Z�b�g����
	
	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineColmnToIndex2( const CLayout* pcLayout, int nColumn, int& pnLineAllColLen )
{
	pnLineAllColLen = 0;

	int i2 = 0;
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getColumn() + it.getColumnDelta() > nColumn ){
			break;
		}
		it.addDelta();
	}
	i2 += it.getIndex();
	if( i2 >= pcLayout->GetLength() ){
		nPosX2 += it.getColumn();
		pnLineAllColLen = nPosX2;
	}
	return i2;
}





/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI CLayout���K�v�ɂȂ�܂����B
*/
int CEditView::LineIndexToColmn( const CLayout* pcLayout, int nIndex )
{
	//	�ȉ��Aiterator��
	int nPosX2 = 0;
	CMemoryIterator<CLayout> it( pcLayout, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
}


/*
||	�w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ�
||
||	@@@ 2002.09.28 YAZAKI CDocLine��
*/
int CEditView::LineIndexToColmn( const CDocLine* pcDocLine, int nIndex )
{
	int nPosX2 = 0;
	CMemoryIterator<CDocLine> it( pcDocLine, m_pcEditDoc->m_cLayoutMgr.GetTabSpace() );
	while( !it.end() ){
		it.scanNext();
		if ( it.getIndex() + it.getIndexDelta() > nIndex ){
			break;
		}
		it.addDelta();
	}
	nPosX2 += it.getColumn();
	return nPosX2;
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


	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->m_CMenuDrawer.ResetContents();

	/* �E�N���b�N���j���[�̒�`�̓J�X�^�����j���[�z���0�Ԗ� */
	nMenuIdx = 0;
//	if( nMenuIdx < 0 || MAX_CUSTOM_MENU	<= nMenuIdx ){
//		return 0;
//	}
//	if( 0 == m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx] ){
//		return 0;
//	}

	//	Oct. 3, 2001 genta
	CFuncLookup& FuncLookup = m_pcEditDoc->m_cFuncLookup;

	hMenu = ::CreatePopupMenu();
	for( i = 0; i < m_pShareData->m_Common.m_nCustMenuItemNumArr[nMenuIdx]; ++i ){
		if( 0 == m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i] ){
			::AppendMenu( hMenu, MF_SEPARATOR, 0, NULL );
		}else{
			//	Oct. 3, 2001 genta
			FuncLookup.Funccode2Name( m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
//			::LoadString( m_hInstance, m_pShareData->m_Common.m_nCustMenuItemFuncArr[nMenuIdx][i], szLabel, 256 );
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
						if( GetSelectedData( cmemCurText, TRUE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
							pszWork = cmemCurText.GetPtr();
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
	//							if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pShareData->m_Common.m_szKeyWordHelpFile ) ){
								if( m_cDicMgr.Search( cmemCurText.GetPtr(), &pcmemRefText, m_pcEditDoc->GetDocumentAttribute().m_szKeyWordHelpFile ) ){
									/* �Y������L�[������ */
									m_cTipWnd.m_KeyWasHit = TRUE;
									pszWork = pcmemRefText->GetPtr();
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
							pszWork = m_cTipWnd.m_cInfo.GetPtr();
							// 2002.05.25 Moca &�̍l����ǉ� 
							char*	pszShortOut = new char[160 + 1];
							if( 80 < lstrlen( pszWork ) ){
								char*	pszShort = new char[80 + 1];
								memcpy( pszShort, pszWork, 80 );
								pszShort[80] = '\0';
								dupamp( (const char*)pszShort, pszShortOut );
								delete [] pszShort;
							}else{
								dupamp( (const char*)pszWork, pszShortOut );
							}
							::InsertMenu( hMenu, 0, MF_BYPOSITION, IDM_COPYDICINFO, pszShortOut );
							delete [] pszShortOut;
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
	::DestroyMenu( hMenu );
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

	char			szText[64];
	HWND			hwndFrame;
	unsigned char*	pLine;
	int				nLineLen;
	int				nIdxFrom;
	int				nCharChars;
	CEditWnd*		pCEditWnd;
	const CLayout*	pcLayout;
#if 0
	char*			pCodeNameArr[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Uni ",
		"UTF-8",
		"UTF-7"
	};
#endif
	// 2002.05.26 Moca  gm_pszCodeNameArr_2 ���g��
	const char* pCodeName = gm_pszCodeNameArr_2[m_pcEditDoc->m_nCharCode];
#if 0
	2002/04/08 YAZAKI �R�[�h�̏d�����폜
	char*			pCodeNameArr2[] = {
		"SJIS",
		"JIS ",
		"EUC ",
		"Unicode",
		"UTF-8",
		"UTF-7"
	};
#endif
//	int	nCodeNameArrNum = sizeof( pCodeNameArr ) / sizeof( pCodeNameArr[0] );

	hwndFrame = ::GetParent( m_hwndParent );
	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	/* �J�[�\���ʒu�̕����R�[�h */
//	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen );
	pLine = (unsigned char*)m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );

	//	May 12, 2000 genta
	//	���s�R�[�h�̕\����ǉ�
	//	From Here
	CEOL cNlType = m_pcEditDoc->GetNewLineCode();
	const char *nNlTypeName = cNlType.GetName();
	//	To Here

	int nPosX, nPosY;
	if( m_pcEditDoc->GetDocumentAttribute().m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
		if (pcLayout && pcLayout->m_nOffset){
			char* pLine = pcLayout->m_pCDocLine->GetPtr();
			int nLineLen = m_nCaretPosX_PHY;	//	 - pcLayout->m_nOffset;
			nPosX = 0;
			int i;
			//	Oct. 4, 2002 genta
			//	�����ʒu�̃J�E���g���@������Ă����̂��C��
			for( i = 0; i < nLineLen; ){
				int nCharChars = CMemory::MemCharNext( (const char *)pLine, nLineLen, (const char *)&pLine[i] ) - (const char *)&pLine[i];
				if ( nCharChars == 1 && pLine[i] == TAB ){
					//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
					nPosX += m_pcEditDoc->m_cLayoutMgr.GetActualTabSpace( nPosX );
					++i;
				}
				else {
					nPosX += nCharChars;
					i += nCharChars;
				}
			}
			nPosX ++;	//	�␳
		}
		else {
			nPosX = m_nCaretPosX + 1;
		}
		nPosY = m_nCaretPosY_PHY + 1;
	}
	else {
		nPosX = m_nCaretPosX + 1;
		nPosY = m_nCaretPosY + 1;
	}

	/* �X�e�[�^�X���������o�� */
	if( NULL == pCEditWnd->m_hwndStatusBar ){
		/* �E�B���h�E�E��ɏ����o�� */
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ�
		//	From Here
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
			if( nIdxFrom >= nLineLen ){
				/* szText */
				wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );	//Oct. 31, 2000 JEPRO //Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
			}else{
				if( nIdxFrom < nLineLen - (pcLayout->m_cEol.GetLen()?1:0) ){
					nCharChars = CMemory::MemCharNext( (char *)pLine, nLineLen, (char *)&pLine[nIdxFrom] ) - (char *)&pLine[nIdxFrom];
				}else{
					nCharChars = pcLayout->m_cEol.GetLen();
				}
				switch( nCharChars ){
				case 1:
					/* szText */
					wsprintf( szText, "%s(%s)   [%02x]%6d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom], nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				case 2:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x]%6d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , nPosY, nPosX);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				case 4:
					/* szText */
					wsprintf( szText, "%s(%s) [%02x%02x%02x%02x]%d�F%d", pCodeName, nNlTypeName, pLine[nIdxFrom],  pLine[nIdxFrom + 1] , pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] , nPosY, nPosX);//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
					break;
				default:
					/* szText */
					wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
				}
			}
		}else{
			/* szText */
			wsprintf( szText, "%s(%s)       %6d�F%d", pCodeName, nNlTypeName, nPosY, nPosX );//Oct. 31, 2000 JEPRO ���j���[�o�[�ł̕\������ߖ�
		}
		//	To Here
		//	Dec. 4, 2002 genta ���j���[�o�[�\����CEditWnd���s��
		m_pcEditDoc->m_pcEditWnd->PrintMenubarMessage( szText );
	}else{
		/* �X�e�[�^�X�o�[�ɏ�Ԃ������o�� */
		char	szText_1[64];
		char	szText_3[32]; // szText_2 => szTest_3 �ɕύX 64�o�C�g������Ȃ� 2002.06.05 Moca 
		char	szText_6[16]; // szText_5 => szTest_6 �ɕύX 64�o�C�g������Ȃ� 2002.06.05 Moca
		wsprintf( szText_1, "%6d �s %5d ��", nPosY, nPosX );	//Oct. 30, 2000 JEPRO �疜�s���v���

		nCharChars = 0;
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdxFrom = LineColmnToIndex( pcLayout, m_nCaretPosX );
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
			wsprintf( szText_3, "%02x  ", pLine[nIdxFrom] );
		}else
		if( 2 == nCharChars ){
			wsprintf( szText_3, "%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1] );
		}else
		if( 4 == nCharChars ){
			wsprintf( szText_3, "%02x%02x%02x%02x", pLine[nIdxFrom],  pLine[nIdxFrom + 1], pLine[nIdxFrom + 2],  pLine[nIdxFrom + 3] );
		}else{
			wsprintf( szText_3, "    " );
		}

		if( m_pShareData->m_Common.m_bIsINSMode ){
			strcpy( szText_6, "�}��" );
		}else{
			strcpy( szText_6, "�㏑" );
		}
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 0 | SBT_NOBORDERS, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 1 | 0, (LPARAM) (LPINT)szText_1 );
		//	May 12, 2000 genta
		//	���s�R�[�h�̕\����ǉ��D���̔ԍ���1�����炷
		//	From Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 2 | 0, (LPARAM) (LPINT)nNlTypeName );
		//	To Here
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 3 | 0, (LPARAM) (LPINT)szText_3 );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 4 | 0, (LPARAM) (LPINT)gm_pszCodeNameArr_1[m_pcEditDoc->m_nCharCode] );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 5 | SBT_OWNERDRAW, (LPARAM) (LPINT)"" );
		::SendMessage( pCEditWnd->m_hwndStatusBar, SB_SETTEXT, 6 | 0, (LPARAM) (LPINT)szText_6 );
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

	/* �t�H���g���ς���Ă��邩������Ȃ��̂ŁA�J�[�\���ړ� */
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




/* �t�H�[�J�X�ړ����̍ĕ`��

	@date 2001/06/21 asa-o �u�X�N���[���o�[�̏�Ԃ��X�V����v�u�J�[�\���ړ��v�폜
*/
void CEditView::RedrawAll( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	/* �ĕ`�� */
	hdc = ::GetDC( m_hWnd );

//	OnKillFocus();

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */
//	OnSetFocus();
	::ReleaseDC( m_hWnd, hdc );

	/* �L�����b�g�̕\�� */
	ShowEditCaret();

	/* �L�����b�g�̍s���ʒu��\������ */
	DrawCaretPosInfo();

	/* �e�E�B���h�E�̃^�C�g�����X�V */
	SetParentCaption();

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	return;
}

// 2001/06/21 Start by asa-o �ĕ`��
void CEditView::Redraw( void )
{
	HDC			hdc;
	PAINTSTRUCT	ps;

	hdc = ::GetDC( m_hWnd );

	::GetClientRect( m_hWnd, &ps.rcPaint );

	OnPaint( hdc, &ps, FALSE );	/* �������c�b���g�p���Ă�����̂Ȃ��ĕ`�� */

	::ReleaseDC( m_hWnd, hdc );

	return;
}
// 2001/06/21 End

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
//	�L�����b�g�̕��E�����̓R�s�[���Ȃ��B2002/05/12 YAZAKI
//	pView->m_nCaretWidth			= m_nCaretWidth;		/* �L�����b�g�̕� */
//	pView->m_nCaretHeight			= m_nCaretHeight;		/* �L�����b�g�̍��� */

	/* �L�[��� */
	pView->m_bSelectingLock			= m_bSelectingLock;		/* �I����Ԃ̃��b�N */
	pView->m_bBeginSelect			= m_bBeginSelect;		/* �͈͑I�� */
	pView->m_bBeginBoxSelect		= m_bBeginBoxSelect;	/* ��`�͈͑I�� */

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
	int			nGrepCharSet,	// 2002/09/21 Moca �����R�[�h�Z�b�g�I��
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
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
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp	cRegexp;
	CMemory		cmemMessage;
	CMemory		cmemWork;
	int			nWork;
	int*		pnKey_CharCharsArr;
	pnKey_CharCharsArr = NULL;

	/*
	|| �o�b�t�@�T�C�Y�̒���
	*/
	cmemMessage.AllocBuffer( 4000 );



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
//	int					nGrepCharSet;
//	BOOL				bGrepOutputLine;

//	pGrepParam				= (GrepParam*)dwGrepParam;

//	pCEditView				= (CEditView*)pGrepParam->pCEditView;
//	cmGrepKey				= *pGrepParam->pcmGrepKey;
//	cmGrepFile				= *pGrepParam->pcmGrepFile;
//	cmGrepFolder			= *pGrepParam->pcmGrepFolder;
//	bGrepSubFolder			= pGrepParam->bGrepSubFolder;
//	bGrepLoHiCase			= pGrepParam->bGrepLoHiCase;
//	bGrepRegularExp			= pGrepParam->bGrepRegularExp;
//	nGrepCharSet			= pGrepParam->nGrepCharSet;
//	bGrepOutputLine			= pGrepParam->bGrepOutputLine;

	m_bDoing_UndoRedo		= TRUE;


	/* �A���h�D�o�b�t�@�̏��� */
	if( NULL != m_pcOpeBlk ){	/* ����u���b�N */
//@@@2002.2.2 YAZAKI NULL����Ȃ��Ɛi�܂Ȃ��̂ŁA�Ƃ肠�����R�����g�B��NULL�̂Ƃ��́Anew COpeBlk����B
//		while( NULL != m_pcOpeBlk ){}
//		delete m_pcOpeBlk;
//		m_pcOpeBlk = NULL;
	}
	else {
		m_pcOpeBlk = new COpeBlk;
	}

	m_bCurSrchKeyMark = TRUE;								/* ����������̃}�[�N */
	strcpy( m_szCurSrchKey, pcmGrepKey->GetPtr() );	/* ���������� */
	m_bCurSrchRegularExp = bGrepRegularExp;					/* �����^�u��  1==���K�\�� */
	m_bCurSrchLoHiCase = bGrepLoHiCase;						/* �����^�u��  1==�p�啶���������̋�� */
	/* ���K�\�� */

	//	From Here Jun. 27 genta
	/*
		Grep���s���ɓ������Č����E��ʐF�����p���K�\���o�b�t�@��
		����������D�����Grep�������ʂ̐F�������s�����߁D
	*/
	if( m_bCurSrchRegularExp ){
		//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
		if( !InitRegexp( m_hWnd, m_CurRegexp, true ) ){
			return 0;
		}

		/* �����p�^�[���̃R���p�C�� */
		int nFlag = 0x00;
		nFlag |= m_bCurSrchLoHiCase ? 0x01 : 0x00;
		m_CurRegexp.Compile( m_szCurSrchKey, nFlag );
	}
	//	To Here Jun. 27 genta

//�܂� m_bCurSrchWordOnly = m_pShareData->m_Common.m_bWordOnly;	/* �����^�u��  1==�P��̂݌��� */

//	cDlgCancel.Create( m_hInstance, m_hwndParent );
//	hwndCancel = cDlgCancel.Open( MAKEINTRESOURCE(IDD_GREPRUNNING) );
	hwndCancel = cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_GREPRUNNING );

	::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, 0, FALSE );
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, " " );	// 2002/09/09 Moca add

	pszWork = pcmGrepKey->GetPtr();
	strcpy( szKey, pszWork );

	strcpy( m_pcEditDoc->m_szGrepKey, szKey );
	m_pcEditDoc->m_bGrepMode = TRUE;

//	::SendMessage( ::GetParent( m_hwndParent ), WM_SETICON, ICON_BIG, (LPARAM)::LoadIcon( m_hInstance, IDI_QUESTION ) );
//2002.02.08 Grep�A�C�R�����傫���A�C�R���Ə������A�C�R����ʁX�ɂ���B
	HICON	hIconBig, hIconSmall;
	//	Dec, 2, 2002 genta �A�C�R���ǂݍ��ݕ��@�ύX
	hIconBig = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, false );
	hIconSmall = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );

	//	Sep. 10, 2002 genta
	//	CEditWnd�ɐV�݂����֐����g���悤��
	CEditWnd*	pCEditWnd = m_pcEditDoc->m_pcEditWnd;	//	Sep. 10, 2002 genta
	pCEditWnd->SetWindowIcon( hIconSmall, ICON_SMALL );
	pCEditWnd->SetWindowIcon( hIconBig, ICON_BIG );

	pszWork = pcmGrepFolder->GetPtr();
	strcpy( szPath, pszWork );
	nDummy = lstrlen( szPath );
	/* �t�H���_�̍Ōオ�u���p����'\\'�v�łȂ��ꍇ�́A�t������ */
	nCharChars = &szPath[nDummy] - CMemory::MemCharPrev( szPath, nDummy, &szPath[nDummy] );
	if( 1 == nCharChars && szPath[nDummy - 1] == '\\' ){
	}else{
		strcat( szPath, "\\" );
	}
	strcpy( szFile, pcmGrepFile->GetPtr( /* &nDummy */ ) );



	/* �Ō�Ƀe�L�X�g��ǉ� */
	cmemMessage.AppendSz( "\r\n����������  " );
	if( 0 < lstrlen( szKey ) ){
		CMemory cmemWork2;
		cmemWork2.SetDataSz( szKey );
		if( m_pcEditDoc->GetDocumentAttribute().m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
			cmemWork2.Replace_j( "\\", "\\\\" );
			cmemWork2.Replace_j( "\'", "\\\'" );
			cmemWork2.Replace_j( "\"", "\\\"" );
		}else{
			cmemWork2.Replace_j( "\'", "\'\'" );
			cmemWork2.Replace_j( "\"", "\"\"" );
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


	if( bWordOnly ){
	/* �P��P�ʂŒT�� */
		cmemMessage.AppendSz( "    (�P��P�ʂŒT��)\r\n" );
	}

	if( bGrepLoHiCase ){
		pszWork = "    (�p�啶������������ʂ���)\r\n";
	}else{
		pszWork = "    (�p�啶������������ʂ��Ȃ�)\r\n";
	}
	cmemMessage.AppendSz( pszWork );

	if( bGrepRegularExp ){
		cmemMessage.AppendSz( "    (���K�\��)\r\n" );
	}

	if( CODE_AUTODETECT == nGrepCharSet ){
		cmemMessage.AppendSz( "    (�����R�[�h�Z�b�g�̎�������)\r\n" );
	}else if( 0 <= nGrepCharSet && nGrepCharSet < CODE_CODEMAX ){
		cmemMessage.AppendSz( "    (�����R�[�h�Z�b�g�F" );
		cmemMessage.AppendSz( gm_pszCodeNameArr_1[nGrepCharSet] );
		cmemMessage.AppendSz( ")\r\n" );
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
//@@@ 2002.01.03 YAZAKI Grep����̓J�[�\����Grep���O�̈ʒu�ɓ�����
	int tmp_PosY_PHY = m_pcEditDoc->m_cLayoutMgr.GetLineCount();
	if( 0 < nWork ){
		Command_ADDTAIL( pszWork, nWork );
	}
	if( bGrepRegularExp ){
		if( !InitRegexp( m_hWnd, cRegexp, true ) ){
			return 0;
		}
		/* �����p�^�[���̃R���p�C�� */
		int nFlag = 0x00;
		nFlag |= bGrepLoHiCase ? 0x01 : 0x00;
		if( !cRegexp.Compile( szKey, nFlag ) ){
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
		bGrepRegularExp, nGrepCharSet,
		bGrepOutputLine, bWordOnly, nGrepOutputStyle, &cRegexp, 0, &nHitCount
	) ){
		wsprintf( szPath, "���f���܂����B\r\n", nHitCount );
		Command_ADDTAIL( szPath, lstrlen( szPath ) );
	}
	wsprintf( szPath, "%d ����������܂����B\r\n", nHitCount );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
//	Command_GOFILEEND( FALSE );
#ifdef _DEBUG
	wsprintf( szPath, "��������: %d�~���b\r\n", cRunningTimer.Read() );
	Command_ADDTAIL( szPath, lstrlen( szPath ) );
//	Command_GOFILEEND( FALSE );
#endif
	MoveCursor( 0, tmp_PosY_PHY, TRUE );	//	�J�[�\����Grep���O�̈ʒu�ɖ߂�

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
	m_pcEditDoc->SetModified(false,false);

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
	int			nGrepCharSet,
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
	int			nGrepOutputStyle,
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp*	pRegexp,	/*!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
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
						bGrepRegularExp, nGrepCharSet,
						bGrepOutputLine, bWordOnly, nGrepOutputStyle, pRegexp, nNest + 1, pnHitCount
					) ){
						goto cancel_return;
					}
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURPATH, pszPath );	//@@@ 2002.01.10 add �T�u�t�H���_����߂��Ă�����...
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
					w32fd.cFileName, szPath2,
//					pszFile, szPath2,
					bGrepSubFolder, bGrepLoHiCase,
					bGrepRegularExp, nGrepCharSet,
					bGrepOutputLine, bWordOnly, nGrepOutputStyle,
					pRegexp, nNest, pnHitCount, szPath2, cmemMessage
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
	::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, " " );	// 2002/09/09 Moca add
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



/*!	@brief Grep���ʂ��\�z����

	@param pWork [out] Grep�o�͕�����D�[���ȃ������̈��\�ߊm�ۂ��Ă������ƁD
		�Œ��� �{��1024 byte�{�t�@�C���� _MAX_PATH byte�{�s�E���ʒu�\���̒������K�v�D

	pWork�͏[���ȃ������̈�������Ă���R�g
	@date 2002/08/29 Moca �o�C�i���[�f�[�^�ɑΉ� pnWorkLen �ǉ�
*/
void CEditView::SetGrepResult(
	/* �f�[�^�i�[�� */
	char*		pWork,
	int*		pnWorkLen,			/*!< [out] Grep�o�͕�����̒��� */
	/* �}�b�`�����t�@�C���̏�� */
	const char*		pszFullPath,	/*!< [in] �t���p�X */
	const char*		pszCodeName,	/*!< [in] �����R�[�h���D" [SJIS]"�Ƃ� */
	/* �}�b�`�����s�̏�� */
	int			nLine,				/*!< [in] �}�b�`�����s�ԍ�(1�`) */
	int			nColm,				/*!< [in] �}�b�`�������ԍ�(1�`) */
	const char*		pCompareData,	/*!< [in] �s�̕����� */
	int			nLineLen,			/*!< [in] �s�̕�����̒��� */
	int			nEolCodeLen,		/*!< [in] EOL�̒��� */
	/* �}�b�`����������̏�� */
	const char*		pMatchData,		/*!< [in] �}�b�`���������� */
	int			nMatchLen,			/*!< [in] �}�b�`����������̒��� */
	/* �I�v�V���� */
	BOOL		bGrepOutputLine,	/*!< [in] 0: �Y�������̂�, !0: �Y���s */
	int			nGrepOutputStyle	/*!< [in] 1: Normal, 2: WZ��(�t�@�C���P��) */
)
{

	int nWorkLen = 0;
	const char * pDispData;
	int k;
	bool bEOL = true;

	if( 1 == nGrepOutputStyle ){
	/* �m�[�}�� */
		nWorkLen = ::wsprintf( pWork, "%s(%d,%d)%s: ", pszFullPath, nLine, nColm, pszCodeName );
	}else
	if( 2 == nGrepOutputStyle ){
	/* WZ�� */
		nWorkLen = ::wsprintf( pWork, "�E(%6d,%-5d): ", nLine, nColm );
	}

	if( bGrepOutputLine ){
	/* �Y���s */
		pDispData = pCompareData;
		k = nLineLen - nEolCodeLen;
		if( 1000 < k ){
			k = 1000;
		}
	}else{
	/* �Y������ */
		pDispData = pMatchData;
		k = nMatchLen;
		if( 2000 < k ){
			k = 2000;
		}
		// �Y�������ɉ��s���܂ޏꍇ�͂��̉��s�R�[�h�����̂܂ܗ��p����(���̍s�ɋ�s�����Ȃ�)
		if( pMatchData[ k -1 ] == '\r' || pMatchData[ k - 1 ] == '\n' ){
			bEOL = false;
		}
	}

	memcpy( &pWork[nWorkLen], pDispData, k );
	nWorkLen += k;
	if( bEOL ){
		memcpy( &pWork[nWorkLen], "\r\n", 2 );
		nWorkLen = nWorkLen + 2;
	}
	*pnWorkLen = nWorkLen;
}

/*!
	Grep���s (CFileLoad���g�����e�X�g��)
	@retval -1 GREP�̃L�����Z��
	@retval ����ȊO �q�b�g��(�t�@�C���������̓t�@�C����)
	@date 2002/08/30 Moca CFileLoad���g�����e�X�g��
*/
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
	int			nGrepCharSet,
	BOOL		bGrepOutputLine,
	BOOL		bWordOnly,
	int			nGrepOutputStyle,
	//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
	CBregexp*	pRegexp,	/*!< [in] ���K�\���R���p�C���f�[�^�B���ɃR���p�C������Ă���K�v������ */
	int			nNest,
	int*		pnHitCount, //!< [i/o] �q�b�g���̑S���v
	const char*	pszFullPath,
	CMemory&	cmemMessage
)
{
	int		nHitCount;
//	char	szLine[16000];
	char	szWork[3000];
	char	szWork0[_MAX_PATH + 100];
	int		nLine;
	int		nWorkLen;
	const char*	pszRes; // 2002/08/29 const�t��
	int		nCharCode;
	const char*	pCompareData; // 2002/08/29 const�t��
	const char*	pszCodeName; // 2002/08/29 const�t��
	int		nColm;
	BOOL	bOutFileName;
	bOutFileName = FALSE;
	int		nLineLen;
	const	char*	pLine;
	CEOL	cEol;
	int		nEolCodeLen;
	CFileLoad	cfl;
	int		nOldPercent = 0;

	int	nKeyKen = lstrlen( pszKey );

	//	�����ł͐��K�\���R���p�C���f�[�^�̏������͕s�v

	nCharCode = nGrepCharSet;
	pszCodeName = "";
	if( CODE_AUTODETECT == nGrepCharSet ){
		/*
		|| �t�@�C���̓��{��R�[�h�Z�b�g����
		||
		|| �y�߂�l�z
		||	SJIS	0
		||	JIS		1
		||	EUC		2
		||	Unicode	3
		||	�G���[	-1
		||	�ڍׂ�enumCodeType(global.h) ���Q��
		*/
		nCharCode = CMemory::CheckKanjiCodeOfFile( pszFullPath );
		if( -1 == nCharCode ){
			wsprintf( szWork, "�����R�[�h�̔��ʏ����ŃG���[ [%s]\r\n", pszFullPath );
			Command_ADDTAIL( szWork, lstrlen( szWork ) );
			return 0;
		}
		pszCodeName = gm_pszCodeNameArr_3[nCharCode];
	}
	nHitCount = 0;
	nLine = 0;

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �s���[�v�̑O���炱���Ɉړ�
	if( 0 == nKeyKen ){
		if( 1 == nGrepOutputStyle ){
		/* �m�[�}�� */
			wsprintf( szWork0, "%s%s\r\n", pszFullPath, pszCodeName );
		}else{
		/* WZ�� */
			wsprintf( szWork0, "��\"%s\"%s\r\n", pszFullPath, pszCodeName );
		}
		cmemMessage.AppendSz( szWork0 );
		++(*pnHitCount);
		::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
		return 1;
	}
	wsprintf( szWork0, "��\"%s\"%s\r\n", pszFullPath, pszCodeName );


	try{
	// �t�@�C�����J��
	// FileClose�Ŗ����I�ɕ��邪�A���Ă��Ȃ��Ƃ��̓f�X�g���N�^�ŕ���
	cfl.FileOpen( pszFullPath, nCharCode, 0 );

//	/* �������̃��[�U�[������\�ɂ��� */
	if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
		return -1;
	}
	/* ���f�{�^�������`�F�b�N */
	if( pcDlgCancel->IsCanceled() ){
		return -1;
	}

	/* ���������������[���̏ꍇ�̓t�@�C���������Ԃ� */
	// 2002/08/29 �t�@�C���I�[�v���̎�O�ֈړ�

	// ���� : cfl.ReadLine �� throw ����\��������
	while( NULL != ( pLine = cfl.ReadLine( &nLineLen, &cEol ) ) ){
		nEolCodeLen = cEol.GetLen();
		++nLine;
		pCompareData = pLine;

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( pcDlgCancel->m_hWnd ) ){
			return -1;
		}
		if( 0 == nLine % 64 ){
			/* ���f�{�^�������`�F�b�N */
			if( pcDlgCancel->IsCanceled() ){
				return -1;
			}
			// 2002/08/30 Moca �i�s��Ԃ�\������(5MB�ȏ�)
			if( 5000000 < cfl.GetFileSize() ){
				int nPercent = cfl.GetPercent();
				if( 5 <= nPercent - nOldPercent ){
					nOldPercent = nPercent;
					::wsprintf( szWork, "%s (%3d%%)", pszFile, nPercent );
					::SetDlgItemText( hwndCancel, IDC_STATIC_CURFILE, szWork );
				}
			}
		}

		/* ���K�\������ */
		if( bGrepRegularExp ){
			BREGEXP*	pRegexpData;	//	���K�\���R���p�C���f�[�^
			int nColmPrev = 0;

			while (1){	//	���邮���
				//	Jun. 27, 2001 genta	���K�\�����C�u�����̍����ւ�
				if( pRegexp->GetMatchInfo( pCompareData, nLineLen, 0, &pRegexpData ) ){

					//	�p�^�[������
					nColm = pRegexpData->startp[0] - pCompareData + 1;
					int matchlen = pRegexpData->endp[0] - pRegexpData->startp[0];

					/* Grep���ʂ��AszWork�Ɋi�[���� */
					SetGrepResult(
						szWork, &nWorkLen,
						pszFullPath, pszCodeName,
						nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
						pRegexpData->startp[0], matchlen,
						bGrepOutputLine, nGrepOutputStyle
					);
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						if( !bOutFileName ){
							cmemMessage.AppendSz( szWork0 );
							bOutFileName = TRUE;
						}
					}
					cmemMessage.Append( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					if ( !bGrepOutputLine ) {
						//	�T���n�߂�ʒu��␳
						pCompareData += nColm;
						nLineLen -= nColm;
						nColmPrev += nColm;
						continue;
					}
				}
				break;
			}
		}else
		/* �P��̂݌��� */
		if( bWordOnly ){
			/*
				2002/02/23 Norio Nakatani
				�P��P�ʂ�Grep�������I�Ɏ����B�P���WhereCurrentWord()�Ŕ��ʂ��Ă܂��̂ŁA
				�p�P���C/C++���ʎq�Ȃǂ̌��������Ȃ�q�b�g���܂��B

				2002/03/06 YAZAKI
				Grep�ɂ����������B
				WhereCurrentWord�ŒP��𒊏o���āA���̒P�ꂪ������Ƃ����Ă��邩��r����B
			*/
			int nNextWordFrom = 0;
			int nNextWordFrom2;
			int nNextWordTo2;
			while (1) {	//	���邮��܂킷�B
				if( TRUE ==
					CDocLineMgr::WhereCurrentWord_2( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom2, &nNextWordTo2 , NULL, NULL )
				){
					if( nKeyKen == nNextWordTo2 - nNextWordFrom2 ){
						// const char* pData = pCompareData;	// 2002/2/10 aroka CMemory�ύX , 2002/08/29 Moca pCompareData��const���ɂ��s�v?
						/* 1==�啶���������̋�� */
						if( (FALSE == bGrepLoHiCase && 0 == _memicmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) ) ||
							(TRUE  == bGrepLoHiCase && 0 ==	  memcmp( &(pCompareData[nNextWordFrom2]) , pszKey, nKeyKen ) )
						){
							/* Grep���ʂ��AszWork�Ɋi�[���� */
							SetGrepResult(
								szWork, &nWorkLen,
								pszFullPath, pszCodeName,
								//	Jun. 25, 2002 genta
								//	���ʒu��1�n�܂�Ȃ̂�1�𑫂��K�v������
								nLine, nNextWordFrom2 + 1, pCompareData, nLineLen, nEolCodeLen,
								pszKey, nKeyKen,
								bGrepOutputLine, nGrepOutputStyle
							);
							if( 2 == nGrepOutputStyle ){
							/* WZ�� */
								if( !bOutFileName ){
									cmemMessage.AppendSz( szWork0 );
									bOutFileName = TRUE;
								}
							}

							cmemMessage.Append( szWork, nWorkLen );
							++nHitCount;
							++(*pnHitCount);
							//	May 22, 2000 genta
							// if( 0 == ( (*pnHitCount) % 16 ) ){
								::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
							// }
						}
					}
					/* ���݈ʒu�̍��E�̒P��̐擪�ʒu�𒲂ׂ� */
					if( !CDocLineMgr::SearchNextWordPosition( pCompareData, nLineLen, nNextWordFrom, &nNextWordFrom, FALSE ) ){
						break;	//	���̒P�ꂪ�����B
					}
				}
				else {
					//	�P�ꂪ������Ȃ������B
					break;
				}
			}
		}
		else {
			/* �����񌟍� */
			int nColmPrev = 0;
			while (1){	//	���邮���
				if( NULL != ( pszRes = CDocLineMgr::SearchString(
					(const unsigned char *)pCompareData, nLineLen,
					0,
					(const unsigned char *)pszKey, nKeyKen,
					pnKey_CharCharsArr,
					bGrepLoHiCase
				) ) ){
					nColm = pszRes - pCompareData + 1;

					/* Grep���ʂ��AszWork�Ɋi�[���� */
					SetGrepResult(
						szWork, &nWorkLen,
						pszFullPath, pszCodeName,
						nLine, nColm + nColmPrev, pCompareData, nLineLen, nEolCodeLen,
						pszKey, nKeyKen,
						bGrepOutputLine, nGrepOutputStyle
					);
					if( 2 == nGrepOutputStyle ){
					/* WZ�� */
						if( !bOutFileName ){
							cmemMessage.AppendSz( szWork0 );
							bOutFileName = TRUE;
						}
					}

					cmemMessage.Append( szWork, nWorkLen );
					++nHitCount;
					++(*pnHitCount);
					//	May 22, 2000 genta
					if( 0 == ( (*pnHitCount) % 16 ) || *pnHitCount < 16 ){
						::SetDlgItemInt( hwndCancel, IDC_STATIC_HITCOUNT, *pnHitCount, FALSE );
					}
					
					if ( !bGrepOutputLine ) {
						//	�T���n�߂�ʒu��␳
						pCompareData += nColm;
						nLineLen -= nColm;
						nColmPrev += nColm;
						continue;
					}
				}
				break;
			}
		}
	}

	// �t�@�C���𖾎��I�ɕ��邪�A�����ŕ��Ȃ��Ƃ��̓f�X�g���N�^�ŕ��Ă���
	cfl.FileClose();
	} // try
	catch( CError_FileOpen ){
		wsprintf( szWork, "file open error [%s]\r\n", pszFullPath );
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
		return 0;
	}
	catch( CError_FileRead ){
		wsprintf( szWork, "CEditView::DoGrepFile() �t�@�C���̓ǂݍ��ݒ��ɃG���[���������܂����B\r\n");
		Command_ADDTAIL( szWork, lstrlen( szWork ) );
	} // ��O�����I���

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
	const CLayout* pcLayout;

	nCurLine = m_nCaretPosY;
	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nCurLine, &nLineLen, &pcLayout );
	if( NULL == pLine ){
//		return 0;
		nIdxTo = 0;
	}else{
		/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� Ver1 */
		nIdxTo = LineColmnToIndex( pcLayout, m_nCaretPosX );
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
//		MYTRACE( "cmemWord=[%s]\n", cmemWord.GetPtr() );
//		MYTRACE( "pcmemWord=[%s]\n", pcmemWord->GetPtr() );

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
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
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
	if( !IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
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
	if( IsDataAvailable(pDataObject, CF_TEXT) ){
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
				Command_INSTEXT( TRUE, cmemBuf.GetPtr(), FALSE );
			}else{

				cmemClip.SetDataSz( "" );

				/* �N���b�v�{�[�h����f�[�^���擾 */
				BOOL	bBoxSelectOld;
				MyGetClipboardData( cmemClip, &bBoxSelectOld );

					/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
					MySetClipboardData( cmemBuf.GetPtr(), cmemBuf.GetLength(), TRUE );

					/* �\��t���i�N���b�v�{�[�h����\��t���j*/
					Command_PASTEBOX();

				/* �N���b�v�{�[�h�Ƀf�[�^��ݒ� */
				MySetClipboardData( cmemClip.GetPtr(), cmemClip.GetLength(), bBoxSelectOld );
			}
			if( bMove ){
				if( bMoveToPrev ){
				}else{
					/* �ړ����[�h & ���Ɉړ�*/
					m_bBeginBoxSelect = bBeginBoxSelect_Old;
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
//	const CLayout*	pcLayout;

	cmemCurText.SetDataSz( "" );
	szTopic[0] = '\0';
	if( IsTextSelected() ){	/* �e�L�X�g���I������Ă��邩 */
		/* �I��͈͂̃f�[�^���擾 */
		if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
			/* ��������������݈ʒu�̒P��ŏ����� */
			strncpy( szTopic, cmemCurText.GetPtr(), _MAX_PATH - 1 );
			szTopic[_MAX_PATH - 1] = '\0';
		}
	}else{
		const CLayout*	pcLayout;
		pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( m_nCaretPosY, &nLineLen, &pcLayout );
		if( NULL != pLine ){
			/* �w�肳�ꂽ���ɑΉ�����s�̃f�[�^���̈ʒu�𒲂ׂ� */
			nIdx = LineColmnToIndex( pcLayout, m_nCaretPosX );

			/* ���݈ʒu�̒P��͈̔͂𒲂ׂ� */
			if( m_pcEditDoc->m_cLayoutMgr.WhereCurrentWord(
				m_nCaretPosY, nIdx,
				&nLineFrom, &nColmFrom, &nLineTo, &nColmTo, NULL, NULL )
			){
				/* �w�肳�ꂽ�s�̃f�[�^���̈ʒu�ɑΉ����錅�̈ʒu�𒲂ׂ� */
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineFrom, &nLineLen, &pcLayout );
				nColmFrom = LineIndexToColmn( pcLayout, nColmFrom );
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLineTo, &nLineLen, &pcLayout );
				nColmTo = LineIndexToColmn( pcLayout, nColmTo );
				/* �I��͈͂̕ύX */
				m_nSelectLineBgnFrom = nLineFrom;	/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnFrom = nColmFrom;	/* �͈͑I���J�n��(���_) */
				m_nSelectLineBgnTo = nLineTo;		/* �͈͑I���J�n�s(���_) */
				m_nSelectColmBgnTo = nColmTo;		/* �͈͑I���J�n��(���_) */

				m_nSelectLineFrom = nLineFrom;
				m_nSelectColmFrom = nColmFrom;
				m_nSelectLineTo = nLineTo;
				m_nSelectColmTo = nColmTo;
				/* �I��͈͂̃f�[�^���擾 */
				if( GetSelectedData( cmemCurText, FALSE, NULL, FALSE, m_pShareData->m_Common.m_bAddCRLFWhenCopy ) ){
					/* ��������������݈ʒu�̒P��ŏ����� */
					strncpy( szTopic, cmemCurText.GetPtr(), MAX_PATH - 1 );
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
void CCaretUnderLine::CaretUnderLineON( BOOL bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineON( bDraw );
}



/* �J�[�\���s�A���_�[���C����OFF */
void CCaretUnderLine::CaretUnderLineOFF( BOOL bDraw )
{
	if( m_nLockCounter ) return;	//	���b�N����Ă����牽���ł��Ȃ��B
	m_pcEditView->CaretUnderLineOFF( bDraw );
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
			/* �J�[�\���s�A���_�[���C���̏����i�������j */
#if 1
			PAINTSTRUCT ps;
			ps.rcPaint.left = m_nViewAlignLeft;
			ps.rcPaint.right = m_nViewAlignLeft + m_nViewCx;
			ps.rcPaint.top = m_nOldUnderLineY;
			ps.rcPaint.bottom = m_nOldUnderLineY;
			HDC hdc = ::GetDC( m_hWnd );
			m_cUnderLine.Lock();
			//	�s�{�ӂȂ���I�������o�b�N�A�b�v�B
			int nSelectLineFrom = m_nSelectLineFrom;
			int nSelectLineTo = m_nSelectLineTo;
			int nSelectColmFrom = m_nSelectColmFrom;
			int nSelectColmTo = m_nSelectColmTo;
			m_nSelectLineFrom = -1;
			m_nSelectLineTo = -1;
			m_nSelectColmFrom = -1;
			m_nSelectColmTo = -1;
			OnPaint( hdc, &ps, FALSE );
			//	�I�����𕜌�
			m_nSelectLineFrom = nSelectLineFrom;
			m_nSelectLineTo = nSelectLineTo;
			m_nSelectColmFrom = nSelectColmFrom;
			m_nSelectColmTo = nSelectColmTo;
			m_cUnderLine.UnLock();
			ReleaseDC( m_hWnd, hdc );
#else
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
#endif
		}
		m_nOldUnderLineY = -1;
	}
	return;
}


#if 0
/* ���݁AEnter�Ȃǂő}��������s�R�[�h�̎�ނ��擾 */
CEOL CEditView::GetCurrentInsertEOL( void )
{
	return m_pcEditDoc->GetNewLineCode();
}
#endif

//	From Here Jun. 30, 2001 GAE
//////////////////////////////////////////////////////////////////////
/*! �q�v���Z�X�̕W���o�͂����_�C���N�g����
	��ƃt�@�C�����쐬
	��ƃt�@�C���̃n���h����W���o�͐�ɐݒ肵����ԂŁA�q�v���Z�X���N������
	�q�v���Z�X���I���܂��̓��[�U�[�ɂ�蒆�f�����܂Ń��[�v
	��ƃt�@�C����ǂݍ���
	�I������  (��n��)
	��ƃt�@�C�����폜
*/
/*
�t�@�C������ă��_�C���N�g����
	�y���_�z
	�q�v���Z�X���s���Ƀ��A���^�C���ɏo�͂����o��������
	�q�v���Z�X�Ƃ̓ǂݏ����̓��������܂����Ȃ��ƌ��ʂ��������ǂ߂Ȃ��E�E�E
	�q�v���Z�X���I������܂ő҂��Ă���A���ʂ�ǂݏo���Ɛ��������ʂ����o����B

	�y�����z
	�e�q�œ����t�@�C���n���h�����g���Ă��邩��t�@�C���|�C���^�ʒu��
	�Œ��ꒃ�ɂȂ�̂�������Ȃ����Ǐڍׂ͕s��

	�y�����z
	��̃t�@�C���ɑ΂���n���h�������B
	�q�ɓn���n���h���ƁA�e���ǂރn���h����ʁX�ɂ��鎖�ŁA�o�͌��ʂ̓��e�̖��͉��������B

	2001/06/23 N.Nakatani �������C��  �ł��܂������Ɠ����Ȃ�

*/
//////////////////////////////////////////////////////////////////////
void CEditView::ExecCmd( const char* pszCmd, BOOL bGetStdout )
{
	#define	IsKanji1(x) ((unsigned char)((x^0x20)-0xA1)<=0x3B)
	char				cmdline[1024];
	HANDLE				hStdOutWrite, hStdOutRead;
	PROCESS_INFORMATION	pi;
	ZeroMemory( &pi, sizeof(PROCESS_INFORMATION) );
	CDlgCancel				cDlgCancel;

	//�q�v���Z�X�̕W���o�͂Ɛڑ�����p�C�v���쐬
	SECURITY_ATTRIBUTES	sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	if( CreatePipe( &hStdOutRead, &hStdOutWrite, &sa, 1000 ) == FALSE ) {
		//�G���[�B�΍�����
		return;
	}
	//�p���s�\�ɂ���
	DuplicateHandle( GetCurrentProcess(), hStdOutRead,
				GetCurrentProcess(), NULL,
				0, FALSE, DUPLICATE_SAME_ACCESS );

	//CreateProcess�ɓn��STARTUPINFO���쐬
	STARTUPINFO	sui;
	ZeroMemory( &sui, sizeof(STARTUPINFO) );
	sui.cb = sizeof(STARTUPINFO);
	if( bGetStdout ) {
		sui.dwFlags = STARTF_USESHOWWINDOW;
		sui.wShowWindow = SW_HIDE;
		sui.dwFlags |=  STARTF_USESTDHANDLES;
		sui.hStdInput = GetStdHandle( STD_INPUT_HANDLE );
		sui.hStdOutput = hStdOutWrite;
		sui.hStdError = hStdOutWrite;
	}

	//�R�}���h���C�����s
	strcpy( cmdline, pszCmd );
	if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
				CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
		//���s�Ɏ��s�����ꍇ�A�R�}���h���C���x�[�X�̃A�v���P�[�V�����Ɣ��f����
		// command(9x) �� cmd(NT) ���Ăяo��

		//OS�o�[�W�����擾
		COsVersionInfo cOsVer;
		//�R�}���h���C��������쐬
		wsprintf( cmdline, "%s %s%s",
				( cOsVer.IsWin32NT() ? "cmd.exe" : "command.com" ),
				( bGetStdout ? "/C " : "/K " ), pszCmd );
		if( CreateProcess( NULL, cmdline, NULL, NULL, TRUE,
					CREATE_NEW_CONSOLE, NULL, NULL, &sui, &pi ) == FALSE ) {
			MessageBox( NULL, cmdline, "�R�}���h���s�͎��s���܂����B", MB_OK | MB_ICONEXCLAMATION );
			goto finish;
		}
	}

	if( bGetStdout ) {
		DWORD	read_cnt;
		DWORD	new_cnt;
		char	work[1024];
		char	tmp;
		int		bufidx = 0;
		int		j;
		BOOL	bLoopFlag = TRUE;

		//���f�_�C�A���O�\��
		cDlgCancel.DoModeless( m_hInstance, m_hwndParent, IDD_EXECRUNNING );
		//���s�����R�}���h���C����\��
		CShareData::getInstance()->TraceOut( "%s", "\r\n" );
		CShareData::getInstance()->TraceOut( "%s", pszCmd );
		CShareData::getInstance()->TraceOut( "%s", "\r\n" );
		//���s���ʂ̎�荞��
		do {
			//�������̃��[�U�[������\�ɂ���
			if( !::BlockingHook( cDlgCancel.m_hWnd ) ){
				break;
			}
			//���f�{�^�������`�F�b�N
			if( cDlgCancel.IsCanceled() ){
				//�w�肳�ꂽ�v���Z�X�ƁA���̃v���Z�X�������ׂẴX���b�h���I�������܂��B
				::TerminateProcess( pi.hProcess, 0 );
				//�Ō�Ƀe�L�X�g��ǉ�
				const char* pszText = "\r\n���f���܂����B\r\n";
				CShareData::getInstance()->TraceOut( "%s", pszText );
				break;
			}
			//�v���Z�X���I�����Ă��Ȃ����m�F
			if( WaitForSingleObject( pi.hProcess, 0 ) == WAIT_OBJECT_0 ) {
				//�I�����Ă���΃��[�v�t���O��FALSE�Ƃ���
				//���������[�v�̏I�������� �v���Z�X�I�� && �p�C�v����
				bLoopFlag = FALSE;
			}
			new_cnt = 0;
			if( PeekNamedPipe( hStdOutRead, NULL, 0, NULL, &new_cnt, NULL ) ) {	//�p�C�v�̒��̓ǂݏo���ҋ@���̕��������擾
				if( new_cnt > 0 ) {												//�ҋ@���̂��̂�����
					if( new_cnt >= sizeof(work)-2 ) {							//�p�C�v����ǂݏo���ʂ𒲐�
						new_cnt = sizeof(work)-2;
					}
					ReadFile( hStdOutRead, &work[bufidx], new_cnt, &read_cnt, NULL );	//�p�C�v����ǂݏo��
					read_cnt += bufidx;													//work���̎��ۂ̃T�C�Y�ɂ���
					if( read_cnt == 0 ) {
						continue;
					}
					//�ǂݏo������������`�F�b�N����
					// \r\n �� \r �����Ƃ������̑��o�C�g�������o�͂���̂�h���K�v������
					//@@@ 2002.1.24 YAZAKI 1�o�C�g��肱�ڂ��\�����������B
					for( j=0; j<(int)read_cnt/*-1*/; j++ ) {
						if( IsKanji1(work[j]) ) {
							j++;
						} else {
							if( work[j] == '\r' && work[j+1] == '\n' ) {
								j++;
							} else if( work[j] == '\n' && work[j+1] == '\r' ) {
								j++;
							}
						}
					}
					if( j == (int)read_cnt ) {	//�҂�����o�͂ł���ꍇ
						work[read_cnt] = 0;
						CShareData::getInstance()->TraceOut( "%s", work );
						bufidx = 0;
					} else {
						tmp = work[read_cnt-1];
						work[read_cnt-1] = 0;
						CShareData::getInstance()->TraceOut( "%s", work );
						work[0] = tmp;
						bufidx = 1;
					}
				}
			}
			Sleep(0);
		} while( bLoopFlag || new_cnt > 0 );
	}


finish:
	//�I������
	CloseHandle( hStdOutWrite );
	CloseHandle( hStdOutRead );
	if( pi.hProcess ) CloseHandle( pi.hProcess );
	if( pi.hThread ) CloseHandle( pi.hThread );
	#undef IsKanji1
}
//	To Here Jun. 30, 2001 GAE

/*!
	�����^�u���^�u�b�N�}�[�N�������̏�Ԃ��X�e�[�^�X�o�[�ɕ\������

	@date 2002.01.26 hor �V�K�쐬
	@date 2002.12.04 genta ���̂�CEditWnd�ֈړ�
*/
void CEditView::SendStatusMessage( const char* msg )
{
	m_pcEditDoc->m_pcEditWnd->SendStatusMessage( msg );
}
// �g��Ȃ��Ȃ�܂��� minfu 2002.04.10 
///*  IME����̍ĕϊ��v���ɉ����� minfu 2002.03.27 */
//LRESULT CEditView::RequestedReconversion(PRECONVERTSTRING pReconv)
//{
//	CMemory cmemBuf;
//	int nlen;
//	
//	/* �I��͈͂̃f�[�^���擾 */
//	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, FALSE ) ){
//		::MessageBeep( MB_ICONHAND );
//		return 0L;
//	}
//	
//	/* pReconv�\���̂� �l���Z�b�g */
//	nlen =  cmemBuf.GetLength();
//	if ( pReconv != NULL ) {    
//		pReconv->dwSize = sizeof(RECONVERTSTRING) + nlen + 1;
//		pReconv->dwVersion = 0;
//		pReconv->dwStrLen = nlen ;
//		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
//		pReconv->dwCompStrLen = nlen;
//		pReconv->dwCompStrOffset = 0;
//		pReconv->dwTargetStrLen = nlen;
//		pReconv->dwTargetStrOffset = 0 ;
//		
//		strncpy ( (char *)(pReconv + 1), cmemBuf.GetPtr( NULL ), nlen);
//	
//	}/* pReconv ��NULL�̂Ƃ��̓T�C�Y��Ԃ��̂� */
//	
//	/* RECONVERTSTRING�\���̂̃T�C�Y���߂�l */
//	return nlen + sizeof(RECONVERTSTRING);
//
//}
//
///*  IME����̍ĕϊ��v���ɉ����� for 95/NT */ // 20020331 aroka
//LRESULT CEditView::RequestedReconversionW(PRECONVERTSTRING pReconv)
//{
//	CMemory cmemBuf;
//	int nlen;
//	
//	/* �I��͈͂̃f�[�^���擾 */
//	if( FALSE == GetSelectedData( cmemBuf, FALSE, NULL, FALSE, FALSE ) ){
//		::MessageBeep( MB_ICONHAND );
//		return 0L;
//	}
//	
//	/* cmemBuf �� UNICODE �ɕϊ� */
//	cmemBuf.SJISToUnicode();
//	/* pReconv�\���̂� �l���Z�b�g */
//	nlen =  cmemBuf.GetLength();
//	if ( pReconv != NULL ) {
//		pReconv->dwSize = sizeof(RECONVERTSTRING) + nlen  + sizeof(wchar_t);
//		pReconv->dwVersion = 0;
//		pReconv->dwStrLen = nlen/sizeof(wchar_t) ;
//		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
//		pReconv->dwCompStrLen = nlen/sizeof(wchar_t);
//		pReconv->dwCompStrOffset = 0;
//		pReconv->dwTargetStrLen = nlen/sizeof(wchar_t);
//		pReconv->dwTargetStrOffset = 0 ;
//		
//		wcsncpy ( (wchar_t *)(pReconv + 1), (wchar_t *)cmemBuf.GetPtr(), nlen/sizeof(wchar_t) );
//	
//	}/* pReconv ��NULL�̂Ƃ��̓T�C�Y��Ԃ��̂� */
//	
//	/* RECONVERTSTRING�\���̂̃T�C�Y���߂�l */
//	return nlen + sizeof(RECONVERTSTRING);
//
//}

//  2002.04.09 minfu from here
/*�ĕϊ��p �J�[�\���ʒu����O��200byte�����o����RECONVERTSTRING�𖄂߂� */
/*  ����  pReconv RECONVERTSTRING�\���̂ւ̃|�C���^�B                     */
/*        bUnicode true�Ȃ��UNICODE�ō\���̂𖄂߂�                      */
/*  �߂�l   RECONVERTSTRING�̃T�C�Y                                      */
LRESULT CEditView::SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode)
{
	const char	*pLine;
	int 		nLineLen;
	int			nCurrentLine /* , nCurrentColumn */ ;
//	int			nCurLogicalLine;
	
	//�s�̒��ōĕϊ���API�ɂ킽���Ƃ��镶����̊J�n�ʒu�ƒ����i�l��������j
	int			nReconvIndex, nReconvLen, nReconvLenWithNull ;
	
	//�s�̒��ōĕϊ��̒��ڂ��镶�߂Ƃ��镶����̊J�n�ʒu�A�I���ʒu�A�����i�Ώە�����j
	int			nSelectedIndex, nSelectedEndIndex, nSelectedLen;
	
	int			nSelectColumnFrom /* , nSelectColumnTo */ ;
	int			nSelectLineFrom, nSelectLineTo;
	
	DWORD		dwReconvTextLen;
	DWORD		dwCompStrOffset, dwCompStrLen;
	
//	int			nCurrentIndex;
	CMemory		cmemBuf1;
	const char*		pszReconv;
	CDocLine*	pcCurDocLine;
	
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;
	
	//��`�I�𒆂͉������Ȃ�
	if( m_bBeginBoxSelect )
		return 0;
	
//	char sz[1024];
	if( IsTextSelected() ){
		//�e�L�X�g���I������Ă���Ƃ�
		nSelectColumnFrom = m_nSelectColmFrom;
		nSelectLineFrom   = m_nSelectLineFrom;
		
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nSelectColmFrom, m_nSelectLineFrom, &nSelectedIndex, &nCurrentLine);
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nSelectColmTo, m_nSelectLineTo, &nSelectedEndIndex, &nSelectLineTo);
		
		//�I��͈͂������s�̎���
		if (nSelectLineTo != nCurrentLine){
			//�s���܂łɐ���
			pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(nCurrentLine);
			nSelectedEndIndex = pcCurDocLine->m_pLine->GetLength();
		}
		
		nSelectedLen = nSelectedEndIndex - nSelectedIndex;
		
	}else{
		//�e�L�X�g���I������Ă��Ȃ��Ƃ�
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys(m_nCaretPosX ,m_nCaretPosY , &nSelectedIndex, &nCurrentLine);
		nSelectedLen = 0;
	}
	
	pcCurDocLine = m_pcEditDoc->m_cDocLineMgr.GetLineInfo(nCurrentLine);
	
	if (NULL == pcCurDocLine )
		return 0;
	
	nLineLen = pcCurDocLine->m_pLine->GetLength() - pcCurDocLine->m_cEol.GetLen() ; //���s�R�[�h���̂���������
	
	if ( 0 == nLineLen )
		return 0;
	
	pLine = pcCurDocLine->m_pLine->GetPtr();

	//�ĕϊ��l��������J�n
	nReconvIndex = 0;
	if ( nSelectedIndex > 200 ) {
		const char* pszWork = pLine;
		while( (nSelectedIndex - nReconvIndex) > 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvIndex = pszWork - pLine ;
		}
	}
	
	//�ĕϊ��l��������I��
	nReconvLen = nLineLen - nReconvIndex;
	if ( (nReconvLen + nReconvIndex - nSelectedIndex) > 200 ){
		const char* pszWork = pLine + nSelectedIndex;
		nReconvLen = nSelectedIndex - nReconvIndex;
		while( ( nReconvLen + nReconvIndex - nSelectedIndex) <= 200 ){
			pszWork = ::CharNext( pszWork);
			nReconvLen = pszWork - (pLine + nReconvIndex) ;
		}
	}
	
	//�Ώە�����̒���
	if ( nSelectedIndex + nSelectedLen > nReconvIndex + nReconvLen ){
		nSelectedLen = nReconvIndex + nReconvLen - nSelectedIndex;
	}
	
	pszReconv =  pLine + nReconvIndex;
	
	if(bUnicode){
		
		//�l��������̊J�n����Ώە�����̊J�n�܂�
		if( nSelectedIndex - nReconvIndex > 0 ){
			cmemBuf1.SetData(pszReconv, nSelectedIndex - nReconvIndex);
			cmemBuf1.SJISToUnicode();
			dwCompStrOffset = cmemBuf1.GetLength();  //Offset ��byte
		}else{
			dwCompStrOffset = 0;
		}
		
		//�Ώە�����̊J�n����Ώە�����̏I���܂�
		if (nSelectedLen > 0 ){
			cmemBuf1.SetData(pszReconv + nSelectedIndex, nSelectedLen);  
			cmemBuf1.SJISToUnicode();
			dwCompStrLen = cmemBuf1.GetLength() / sizeof(wchar_t);
		}else{
			dwCompStrLen = 0;
		}
		
		//�l�������񂷂ׂ�
		cmemBuf1.SetData(pszReconv , nReconvLen );
		cmemBuf1.SJISToUnicode();
		
		dwReconvTextLen =  cmemBuf1.GetLength() / sizeof(wchar_t);
		nReconvLenWithNull =  cmemBuf1.GetLength()  + sizeof(wchar_t);
		
		pszReconv = cmemBuf1.GetPtr();
		
	}else{
		dwReconvTextLen = nReconvLen;
		nReconvLenWithNull = nReconvLen + 1;
		dwCompStrOffset = nSelectedIndex - nReconvIndex;
		dwCompStrLen    = nSelectedLen;
	}
	
	if ( NULL != pReconv) {
		//�ĕϊ��\���̂̐ݒ�
		pReconv->dwSize = sizeof(RECONVERTSTRING) + nReconvLenWithNull ;
		pReconv->dwVersion = 0;
		pReconv->dwStrLen = dwReconvTextLen ;
		pReconv->dwStrOffset = sizeof(RECONVERTSTRING) ;
		pReconv->dwCompStrLen = dwCompStrLen;
		pReconv->dwCompStrOffset = dwCompStrOffset;
		pReconv->dwTargetStrLen = dwCompStrLen;
		pReconv->dwTargetStrOffset = dwCompStrOffset;
		
		CopyMemory( (void *)(pReconv + 1), (void *)pszReconv , nReconvLenWithNull );
		*((char *)(pReconv + 1) + nReconvLenWithNull - 1 ) = 0;
		
	}
	
	// �ĕϊ����̕ۑ�
	m_nLastReconvIndex = nReconvIndex;
	m_nLastReconvLine  = nCurrentLine;
	
	return sizeof(RECONVERTSTRING) + nReconvLenWithNull;

}

/*�ĕϊ��p �G�f�B�^��̑I��͈͂�ύX���� 2002.04.09 minfu */
LRESULT CEditView::SetSelectionFromReonvert(PRECONVERTSTRING pReconv, bool bUnicode){
	
	DWORD		dwOffset, dwLen;
	CMemory		cmemBuf;
//	char		sz[2000];
	
	// �ĕϊ���񂪕ۑ�����Ă��邩
	if ( (m_nLastReconvIndex < 0) || (m_nLastReconvLine < 0))
		return 0;

	if ( IsTextSelected()) 
		DisableSelectArea( TRUE );
	
	if (bUnicode){
		
		//�l��������̊J�n����Ώە�����̊J�n�܂�
		if( pReconv->dwCompStrOffset > 0){
			cmemBuf.SetData((const char *)((const wchar_t *)(pReconv + 1)), 
								pReconv->dwCompStrOffset ); 
			cmemBuf.UnicodeToSJIS();
			dwOffset = cmemBuf.GetLength();
			
		}else
			dwOffset = 0;

		//�Ώە�����̊J�n����Ώە�����̏I���܂�
		if( pReconv->dwCompStrLen > 0 ){
			cmemBuf.SetData((const char *)(const wchar_t *)(pReconv + 1) + pReconv->dwCompStrOffset , 
								pReconv->dwCompStrLen * sizeof(wchar_t)); 
			cmemBuf.UnicodeToSJIS();
			dwLen = cmemBuf.GetLength();
			
		}else
			dwLen = 0;
		
	}else{
		dwOffset = pReconv->dwCompStrOffset ;
		dwLen =  pReconv->dwCompStrLen;
	}
	
	//�I���J�n�̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(m_nLastReconvIndex + dwOffset 
												, m_nLastReconvLine, &m_nSelectColmFrom, &m_nSelectLineFrom);
	//�I���I���̈ʒu���擾
	m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log(m_nLastReconvIndex + dwOffset + dwLen
												, m_nLastReconvLine, &m_nSelectColmTo, &m_nSelectLineTo);

	/* �P��̐擪�ɃJ�[�\�����ړ� */
	MoveCursor( m_nSelectColmFrom, m_nSelectLineFrom, TRUE );

	//�I��͈͍ĕ`�� 
	DrawSelectArea();

	// �ĕϊ����̔j��
	m_nLastReconvIndex = -1;
	m_nLastReconvLine  = -1;

	return 1;

}


// 2002/07/22 novice
/*!
	�L�����b�g�̕\��
*/
void CEditView::ShowCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == false ){
		::ShowCaret( hwnd );
		m_bCaretShowFlag = true;
	}
}


/*!
	�L�����b�g�̔�\��
*/
void CEditView::HideCaret_( HWND hwnd )
{
	if ( m_bCaretShowFlag == true ){
		::HideCaret( hwnd );
		m_bCaretShowFlag = false;
	}
}


/*!
	�Ί��ʂ̋����\��
	@date 2002/09/18 ai
*/
void CEditView::DrawBracketPair( void )
{
	const char*	pLine;
	int			nLineLen;
	int			nCol;
	int			nLine;
	int			OutputX;
	int			nLeft;
	int			nTop;
	int			mode;
	HDC			hdc;
	COLORREF	crBackOld;
	COLORREF	crTextOld;
	HFONT		hFontOld;

	hdc = ::GetDC( m_hWnd );
	Types *TypeDataPtr = &( m_pcEditDoc->GetDocumentAttribute() );

	/**********************************/
	/** �O��̑Ί��ʂ̕\�������ɖ߂� **/
	/**********************************/
	if( ( 0 <= m_nBracketPairPosX_PHY ) && ( 0 <= m_nBracketPairPosY_PHY ) )
	{
		// �����ʒu���烌�C�A�E�g�ʒu�֕ϊ�
		m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nBracketPairPosX_PHY, m_nBracketPairPosY_PHY, &nCol, &nLine );

		if ( ( nCol < m_nViewLeftCol ) || ( nCol > m_nViewLeftCol + m_nViewColNum )
			|| ( nLine < m_nViewTopLine ) || ( nLine > m_nViewTopLine + m_nViewRowNum ) )
		{
			/* �\���̈�O�̏ꍇ�͂Ȃɂ����Ȃ� */
		}
		else
		{
			if( ( m_bDrawSelectArea == FALSE )
				|| ( ( m_bDrawSelectArea == TRUE ) && ( 0 != IsCurrentPositionSelected( nCol, nLine ) ) ) ){
				/****************************/
				/** �Ί��ʂ̋����\���̏��� **/
				/****************************/
				const CLayout* pcLayout;
				int nColorIndex;
				pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLine, &nLineLen, &pcLayout );
				if( NULL != pLine ){
					OutputX = LineColmnToIndex( pcLayout, nCol );
					nColorIndex = GetColorIndex( hdc, pcLayout, OutputX );
					//char buf[256];
					//wsprintf( buf, "nColorIndex = %d, pLine[%d] = '%c'", nColorIndex, OutputX, pLine[OutputX] );
					//SendStatusMessage( buf );
					//::SetBkMode( hdc, TRANSPARENT );
					hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
					m_hFontOld = NULL;
					crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
					crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );

					SetCurrentColor( hdc, nColorIndex );

					nLeft = (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
					nTop  = ( nLine - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
					DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );

					if( NULL != m_hFontOld ){
						::SelectObject( hdc, m_hFontOld );
						m_hFontOld = NULL;
					}
					::SetTextColor( hdc, crTextOld );
					::SetBkColor( hdc, crBackOld );
					::SelectObject( hdc, hFontOld );

					if( nLine == m_nCaretPosY ){
						m_cUnderLine.CaretUnderLineON( TRUE );
					}
				}
			}
		}
		m_nBracketPairPosX_PHY = -1;
		m_nBracketPairPosY_PHY = -1;
	}

	if( IsTextSelected() || ( ( m_nBracketPairPosX_PHY != -1 ) && ( m_nBracketPairPosY_PHY != -1 ) ) )
	{	// �I�𒆖��́A�Ί��ʂ̋����\���������ς݂łȂ��ꍇ�͋����\�������Ȃ�
		::ReleaseDC( m_hWnd, hdc );
		return;
	}

	/*******************/
	/** �Ί��ʂ̕\��  **/
	/*******************/
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	mode = 2;
	// �Ή����銇�ʂ����������ꍇ�ɑΊ��ʂ̕\�����s�Ȃ�
	if( SearchBracket( m_nCaretPosX, m_nCaretPosY, &nCol, &nLine, &mode ) )
	{
		if ( ( nCol < m_nViewLeftCol ) || ( nCol > m_nViewLeftCol + m_nViewColNum )
			|| ( nLine < m_nViewTopLine ) || ( nLine > m_nViewTopLine + m_nViewRowNum ) )
		{
			/* �\���̈�O�̏ꍇ�͂Ȃɂ����Ȃ� */
		}
		else
		{
			/**********************/
			/** �Ί��ʂ̋����\�� **/
			/**********************/
			//::SetBkMode( hdc, TRANSPARENT );
			hFontOld = (HFONT)::SelectObject( hdc, m_hFont_HAN );
			m_hFontOld = NULL;
			crBackOld = ::SetBkColor(	hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colBACK );
			crTextOld = ::SetTextColor( hdc, TypeDataPtr->m_ColorInfoArr[COLORIDX_TEXT].m_colTEXT );

			SetCurrentColor( hdc, COLORIDX_BRACKET_PAIR );

			// ���݈ʒu�̊��ʂ̋����\��
			const CLayout* pcLayout;
			int LogX, LogY;
			if( 0 == ( mode & 4 ) )
			{	// �J�[�\���̌�������ʒu
				LogX = m_nCaretPosX;
				LogY = m_nCaretPosY;
			}
			else
			{	// �J�[�\���̑O�������ʒu
				m_pcEditDoc->m_cLayoutMgr.CaretPos_Phys2Log( m_nCaretPosX_PHY - m_nCharSize, m_nCaretPosY_PHY, &LogX, &LogY );
			}

			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( LogY, &nLineLen, &pcLayout );
			if( NULL != pLine ){
				OutputX = LineColmnToIndex( pcLayout, LogX );
				nLeft = (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + LogX * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				nTop  = ( LogY - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
				HideCaret_( m_hWnd );	// �L�����b�g����u������̂�h�~
				DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
				ShowCaret_( m_hWnd );	// �L�����b�g����u������̂�h�~
			}

			// �Ί��ʂ̋����\��
			pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( nLine, &nLineLen, &pcLayout );
			if( NULL != pLine )
			{
				OutputX = LineColmnToIndex( pcLayout, nCol );
				nLeft = (m_nViewAlignLeft - m_nViewLeftCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace )) + nCol * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
				nTop  = ( nLine - m_nViewTopLine ) * ( m_nCharHeight + m_pcEditDoc->GetDocumentAttribute().m_nLineSpace ) + m_nViewAlignTop;
				DispText( hdc, nLeft, nTop, &pLine[OutputX], m_nCharSize );
			}

			::SetTextColor( hdc, crTextOld );
			::SetBkColor( hdc, crBackOld );
			::SelectObject( hdc, hFontOld );

			m_cUnderLine.CaretUnderLineON( TRUE );

			// ���C�A�E�g�ʒu���畨���ʒu�֕ϊ�(�����\���ʒu���L��)
			m_pcEditDoc->m_cLayoutMgr.CaretPos_Log2Phys( nCol, nLine, &m_nBracketPairPosX_PHY, &m_nBracketPairPosY_PHY );
		}
	}

	::ReleaseDC( m_hWnd, hdc );

	return;
}

/*! �w��ʒu��ColorIndex�̎擾
	CEditView::DispLineNew�����ɂ�������CEditView::DispLineNew��
	�C�����������ꍇ�́A�������C�����K�v�B

	@par nCOMMENTMODE
	�֐������ŏ�ԑJ�ڂ̂��߂Ɏg����ϐ�nCOMMENTMODE�Ə�Ԃ̊֌W�B
 - COLORIDX_TEXT     : �e�L�X�g
 - COLORIDX_COMMENT  : �s�R�����g
 - COLORIDX_BLOCK1   : �u���b�N�R�����g1
 - COLORIDX_SSTRING  : �V���O���R�[�e�[�V����
 - COLORIDX_WSTRING  : �_�u���R�[�e�[�V����
 - COLORIDX_KEYWORD1 : �����L�[���[�h1
 - COLORIDX_CTRLCODE : �R���g���[���R�[�h
 - COLORIDX_DIGIT    : ���p���l
 - COLORIDX_BLOCK2   : �u���b�N�R�����g2
 - COLORIDX_KEYWORD2 : �����L�[���[�h2
 - COLORIDX_URL      : URL
 - COLORIDX_SEARCH   : ����
 - 1000: ���K�\���L�[���[�h
 	�F�w��SetCurrentColor���ĂԂƂ���COLORIDX_*�l�����Z����̂ŁA
 	1000�`COLORIDX_LAST�܂ł͐��K�\���Ŏg�p����B
*/
int CEditView::GetColorIndex(
		HDC						hdc,
		const CLayout*			pcLayout,
		int						nCol
)
{
	//	May 9, 2000 genta
	Types	*TypeDataPtr = &(m_pcEditDoc->GetDocumentAttribute());

	const char*				pLine;	//@@@ 2002.09.22 YAZAKI
	int						nLineLen;
	int						nLineBgn;
	int						nBgn;
	int						nPos;
	int						nCharChars;
	int						nCharChars_2;
	int						nCOMMENTMODE;
	int						nCOMMENTMODE_OLD;
	int						nCOMMENTEND;
	int						nCOMMENTEND_OLD;
	const CLayout*			pcLayout2;
	int						i, j;
	int						nIdx;
	int						nUrlLen;
	BOOL					bSearchStringMode;
	BOOL					bSearchFlg;			// 2002.02.08 hor
	int						nSearchStart;		// 2002.02.08 hor
	int						nSearchEnd;
	bool					bKeyWordTop = true;	//	Keyword Top
	int						nColorIndex;

//@@@ 2001.11.17 add start MIK
	int		nMatchLen;
	int		nMatchColor;
//@@@ 2001.11.17 add end MIK

	bSearchStringMode = FALSE;
	bSearchFlg	= TRUE;	// 2002.02.08 hor
	nSearchStart= -1;	// 2002.02.08 hor
	nSearchEnd	= -1;	// 2002.02.08 hor

	/* �_���s�f�[�^�̎擾 */
	if( NULL != pcLayout ){
		// 2002/2/10 aroka CMemory�ύX
		nLineLen = pcLayout->m_pCDocLine->m_pLine->GetLength() - pcLayout->m_nOffset;
		pLine = pcLayout->m_pCDocLine->m_pLine->GetPtr() + pcLayout->m_nOffset;
		nCOMMENTMODE = pcLayout->m_nTypePrev;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
		nCOMMENTEND = 0;
		pcLayout2 = pcLayout;

	}else{
		pLine = NULL;
		nLineLen = 0;
		nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
		nCOMMENTEND = 0;
		pcLayout2 = NULL;
	}

	/* ���݂̐F���w�� */
	//@SetCurrentColor( hdc, nCOMMENTMODE );
	nColorIndex = nCOMMENTMODE;	// 02/12/18 ai

	nBgn = 0;
	nPos = 0;
	nLineBgn = 0;
	nCharChars = 0;

	if( NULL != pLine ){

		//@@@ 2001.11.17 add start MIK
		if( TypeDataPtr->m_bUseRegexKeyword )
		{
			m_cRegexKeyword->RegexKeyLineStart();
		}
		//@@@ 2001.11.17 add end MIK

		while( nPos < nCol ){	// 02/12/18 ai

			nBgn = nPos;
			nLineBgn = nBgn;

			while( nPos - nLineBgn <= nCol ){	// 02/12/18 ai
				/* ����������̐F���� */
				if( TRUE == m_bCurSrchKeyMark	/* ����������̃}�[�N */
				 && TypeDataPtr->m_ColorInfoArr[COLORIDX_SEARCH].m_bDisp ){
searchnext:;
				// 2002.02.08 hor ���K�\���̌���������}�[�N������������
					if(!bSearchStringMode && (!m_bCurSrchRegularExp || (bSearchFlg && nSearchStart < nPos))){
						bSearchFlg=IsSearchString( pLine, nLineLen, nPos, &nSearchStart, &nSearchEnd );
					}
					if( !bSearchStringMode && bSearchFlg && nSearchStart==nPos
					){
						nBgn = nPos;
						bSearchStringMode = TRUE;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, COLORIDX_SEARCH ); // 2002/03/13 novice
						nColorIndex = COLORIDX_SEARCH;	// 02/12/18 ai
					}else
					if( bSearchStringMode
					 && nSearchEnd == nPos
					){
						nBgn = nPos;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						bSearchStringMode = FALSE;
						goto searchnext;
					}
				}

				if( nPos >= nLineLen - pcLayout2->m_cEol.GetLen() ){
					goto end_of_line;
				}
				SEARCH_START:;
				switch( nCOMMENTMODE ){
				case COLORIDX_TEXT: // 2002/03/13 novice
//@@@ 2001.11.17 add start MIK
					//���K�\���L�[���[�h
					if( TypeDataPtr->m_bUseRegexKeyword
					 && m_cRegexKeyword->RegexIsKeyword( pLine, nPos, nLineLen, &nMatchLen, &nMatchColor )
					 /*&& TypeDataPtr->m_ColorInfoArr[nMatchColor].m_bDisp*/ )
					{
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = 1000 + nMatchColor;	/* �F�w�� */	//@@@ 2002.01.04 upd
						nCOMMENTEND = nPos + nMatchLen;  /* �L�[���[�h������̏I�[���Z�b�g���� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );	//@@@ 2002.01.04
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}
					else
//@@@ 2001.11.17 add end MIK
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cLineComment.Match( nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;

						nCOMMENTMODE = COLORIDX_COMMENT;	/* �s�R�����g�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
					}else
					//	Mar. 15, 2000 genta
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 0, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK1;	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(0) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI

//#ifdef COMPILE_BLOCK_COMMENT2	//@@@ 2001.03.10 by MIK
					}else
					if( TypeDataPtr->m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp &&
						TypeDataPtr->m_cBlockComment.Match_CommentFrom( 1, nPos, nLineLen, pLine )	//@@@ 2002.09.22 YAZAKI
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_BLOCK2;	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos + (int)lstrlen( TypeDataPtr->m_cBlockComment.getBlockCommentFrom(1) ), nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
//#endif
					}else
					if( pLine[nPos] == '\'' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp  /* �V���O���N�H�[�e�[�V�����������\������ */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_SSTRING;	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice

						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( pLine[nPos] == '"' &&
						TypeDataPtr->m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp	/* �_�u���N�H�[�e�[�V�����������\������ */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_WSTRING;	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_URL].m_bDisp			/* URL��\������ */
					 && ( TRUE == IsURL( &pLine[nPos], nLineLen - nPos, &nUrlLen ) )	/* �w��A�h���X��URL�̐擪�Ȃ��TRUE�Ƃ��̒�����Ԃ� */
					){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_URL;	/* URL���[�h */ // 2002/03/13 novice
						nCOMMENTEND = nPos + nUrlLen;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 Start by MIK: ���p���l�������\��
					}else if( bKeyWordTop && TypeDataPtr->m_ColorInfoArr[COLORIDX_DIGIT].m_bDisp
						&& (i = IsNumber( pLine, nPos, nLineLen )) > 0 )		/* ���p������\������ */
					{
						/* �L�[���[�h������̏I�[���Z�b�g���� */
						i = nPos + i;
						/* ���݂̐F���w�� */
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_DIGIT;	/* ���p���l�ł��� */ // 2002/03/13 novice
						nCOMMENTEND = i;
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
//@@@ 2001.02.17 End by MIK: ���p���l�������\��
					}else
					if( bKeyWordTop && TypeDataPtr->m_nKeyWordSetIdx != -1 && /* �L�[���[�h�Z�b�g */
						TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD1].m_bDisp &&  /* �����L�[���[�h��\������ */ // 2002/03/13 novice
						IS_KEYWORD_CHAR( pLine[nPos] )
					){
						//	Mar 4, 2001 genta comment out
						/* �L�[���[�h������̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							if( IS_KEYWORD_CHAR( pLine[i] ) ){
							}else{
								break;
							}
						}
						/* �L�[���[�h���o�^�P��Ȃ�΁A�F��ς��� */
						j = i - nPos;
						/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */
						nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(		//MIK UPDATE 2000.12.01 binary search
							TypeDataPtr->m_nKeyWordSetIdx,
							&pLine[nPos],
							j
						);
						if( nIdx != -1 ){
							/* ���݂̐F���w�� */
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_KEYWORD1;	/* �����L�[���[�h1 */ // 2002/03/13 novice
							nCOMMENTEND = i;
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
						}else{		//MIK START ADD 2000.12.01 second keyword & binary search
							if(TypeDataPtr->m_nKeyWordSetIdx2 != -1 && /* �L�[���[�h�Z�b�g */							//MIK 2000.12.01 second keyword
								TypeDataPtr->m_ColorInfoArr[COLORIDX_KEYWORD2].m_bDisp)									//MIK
							{																							//MIK
								/* ���Ԗڂ̃Z�b�g����w��L�[���[�h���T�[�` �����Ƃ���-1��Ԃ� */						//MIK
								nIdx = m_pShareData->m_CKeyWordSetMgr.SearchKeyWord2(									//MIK 2000.12.01 binary search
									TypeDataPtr->m_nKeyWordSetIdx2 ,													//MIK
									&pLine[nPos],																		//MIK
									j																					//MIK
								);																						//MIK
								if( nIdx != -1 ){																		//MIK
									/* ���݂̐F���w�� */																//MIK
									nBgn = nPos;																		//MIK
									nCOMMENTMODE = COLORIDX_KEYWORD2;	/* �����L�[���[�h2 */ // 2002/03/13 novice		//MIK
									nCOMMENTEND = i;																	//MIK
									if( !bSearchStringMode ){															//MIK
										//@SetCurrentColor( hdc, nCOMMENTMODE );										//MIK
										nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
									}																					//MIK
								}																						//MIK
							}																							//MIK
						}			//MIK END
					}
					//	From Here Mar. 4, 2001 genta
					if( IS_KEYWORD_CHAR( pLine[nPos] ))	bKeyWordTop = false;
					else								bKeyWordTop = true;
					//	To Here
					break;
// 2002/03/13 novice
				case COLORIDX_URL:		/* URL���[�h�ł��� */
				case COLORIDX_KEYWORD1:	/* �����L�[���[�h1 */
				case COLORIDX_DIGIT:	/* ���p���l�ł��� */  //@@@ 2001.02.17 by MIK
				case COLORIDX_KEYWORD2:	/* �����L�[���[�h2 */	//MIK
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_CTRLCODE:	/* �R���g���[���R�[�h */ // 2002/03/13 novice
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = nCOMMENTMODE_OLD;
						nCOMMENTEND = nCOMMENTEND_OLD;
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;

				case COLORIDX_COMMENT:	/* �s�R�����g�ł��� */ // 2002/03/13 novice
					break;
				case COLORIDX_BLOCK1:	/* �u���b�N�R�����g1�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 0, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_BLOCK2:	/* �u���b�N�R�����g2�ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* ���̕����s�Ƀu���b�N�R�����g�̏I�[�����邩 */
						nCOMMENTEND = TypeDataPtr->m_cBlockComment.Match_CommentTo( 1, nPos, nLineLen, pLine );	//@@@ 2002.09.22 YAZAKI
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_SSTRING:	/* �V���O���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �V���O���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\'' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '\'' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				case COLORIDX_WSTRING:	/* �_�u���N�H�[�e�[�V����������ł��� */ // 2002/03/13 novice
					if( 0 == nCOMMENTEND ){
						/* �_�u���N�H�[�e�[�V����������̏I�[�����邩 */
						int i;
						nCOMMENTEND = nLineLen;
						for( i = nPos/* + 1*/; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( TypeDataPtr->m_nStringType == 0 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '\\' ){
									++i;
								}else
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									nCOMMENTEND = i + 1;
									break;
								}
							}else
							if( TypeDataPtr->m_nStringType == 1 ){	/* �������؂�L���G�X�P�[�v���@ 0=[\"][\'] 1=[""][''] */
								if( 1 == nCharChars_2 && pLine[i] == '"' ){
									if( i + 1 < nLineLen && pLine[i + 1] == '"' ){
										++i;
									}else{
										nCOMMENTEND = i + 1;
										break;
									}
								}
							}
							if( 2 == nCharChars_2 ){
								++i;
							}
						}
					}else
					if( nPos == nCOMMENTEND ){
						nBgn = nPos;
						nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
						/* ���݂̐F���w�� */
						if( !bSearchStringMode ){
							//@SetCurrentColor( hdc, nCOMMENTMODE );
							nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
						}
						goto SEARCH_START;
					}
					break;
				default:	//@@@ 2002.01.04 add start
					if( nCOMMENTMODE >= 1000 && nCOMMENTMODE <= 1099 ){	//���K�\���L�[���[�h1�`10
						if( nPos == nCOMMENTEND ){
							nBgn = nPos;
							nCOMMENTMODE = COLORIDX_TEXT; // 2002/03/13 novice
							/* ���݂̐F���w�� */
							if( !bSearchStringMode ){
								//@SetCurrentColor( hdc, nCOMMENTMODE );
								nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
							}
							goto SEARCH_START;
						}
					}
					break;	//@@@ 2002.01.04 add end
				}
				if( pLine[nPos] == TAB ){
					nBgn = nPos + 1;
					nCharChars = 1;
				}else
				if( (unsigned char)pLine[nPos] == 0x81 && (unsigned char)pLine[nPos + 1] == 0x40	//@@@ 2001.11.17 upd MIK
				 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )	//@@@ 2002.01.04
				{	//@@@ 2001.11.17 add MIK	//@@@ 2002.01.04
					nBgn = nPos + 2;
					nCharChars = 2;
				}
				//���p�󔒁i���p�X�y�[�X�j��\�� 2002.04.28 Add by KK 
				else if (pLine[nPos] == ' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp 
					 && (nCOMMENTMODE < 1000 || nCOMMENTMODE > 1099) )
				{
					nBgn = nPos + 1;
					nCharChars = 1;
				}
				else{
					nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[nPos] ) - &pLine[nPos];
					if( 0 == nCharChars ){
						nCharChars = 1;
					}
					if( !bSearchStringMode
					 && 1 == nCharChars
					 && COLORIDX_CTRLCODE != nCOMMENTMODE // 2002/03/13 novice
					 && TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* �R���g���[���R�[�h��F���� */
					 &&	(
								//	Jan. 23, 2002 genta �x���}��
							( (unsigned char)pLine[nPos] <= (unsigned char)0x1F ) ||
							( (unsigned char)'~' < (unsigned char)pLine[nPos] && (unsigned char)pLine[nPos] < (unsigned char)'�' ) ||
							( (unsigned char)'�' < (unsigned char)pLine[nPos] )
						)
					 && pLine[nPos] != TAB && pLine[nPos] != CR && pLine[nPos] != LF
					){
						nBgn = nPos;
						nCOMMENTMODE_OLD = nCOMMENTMODE;
						nCOMMENTEND_OLD = nCOMMENTEND;
						nCOMMENTMODE = COLORIDX_CTRLCODE;	/* �R���g���[���R�[�h ���[�h */ // 2002/03/13 novice
						/* �R���g���[���R�[�h��̏I�[��T�� */
						for( i = nPos + 1; i <= nLineLen - 1; ++i ){
							nCharChars_2 = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
							if( 0 == nCharChars_2 ){
								nCharChars_2 = 1;
							}
							if( nCharChars_2 != 1 ){
								break;
							}
							if( (
								//	Jan. 23, 2002 genta �x���}��
								( (unsigned char)pLine[i] <= (unsigned char)0x1F ) ||
									( (unsigned char)'~' < (unsigned char)pLine[i] && (unsigned char)pLine[i] < (unsigned char)'�' ) ||
									( (unsigned char)'�' < (unsigned char)pLine[i] )
								) &&
								pLine[i] != TAB && pLine[i] != CR && pLine[i] != LF
							){
							}else{
								break;
							}
						}
						nCOMMENTEND = i;
						/* ���݂̐F���w�� */
						//@SetCurrentColor( hdc, nCOMMENTMODE );
						nColorIndex = nCOMMENTMODE;	// 02/12/18 ai
					}
				}
				nPos+= nCharChars;
			} //end of while( nPos - nLineBgn < pcLayout2->m_nLength ){
			if( nPos >= nCol ){	// 02/12/18 ai
				break;
			}
		}

end_of_line:;

	}

//@end_of_func:;
	return nColorIndex;
}

/*[EOF]*/
