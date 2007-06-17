/*!	@file
	@brief �^�u�E�B���h�E

	@author MIK
	@date 2003.5.30
	@date 2004.01.27 break�R��Ή��BTCHAR���B�^�u�\���������(?)�̑Ή��B
*/
/*
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2004, Moca, MIK, genta, Kazika
	Copyright (C) 2005, ryoji
	Copyright (C) 2006, ryoji, fon
	Copyright (C) 2007, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "stdafx.h"
#include <windows.h>
#include "CTabWnd.h"
#include "debug.h"
#include "CEditWnd.h"
#include "global.h"
#include "mymessage.h"
#include "etc_uty.h"
#include "charcode.h"
#include "my_tchar.h"	//Unicode�Ή� Moca

//#if(WINVER >= 0x0500)
#ifndef	SPI_GETFOREGROUNDLOCKTIMEOUT
#define SPI_GETFOREGROUNDLOCKTIMEOUT        0x2000
#endif
#ifndef	SPI_SETFOREGROUNDLOCKTIMEOUT
#define SPI_SETFOREGROUNDLOCKTIMEOUT        0x2001
#endif
//#endif

// 2007.04.01 ryoji WM_THEMECHANGED
#ifndef	WM_THEMECHANGED
#define WM_THEMECHANGED		0x031A
#endif

// 2006.01.30 ryoji �^�u�̃T�C�Y�^�ʒu�Ɋւ����`
#define TAB_WINDOW_HEIGHT	24
#define TAB_MARGIN_TOP		3
#define TAB_MARGIN_LEFT		1
#define TAB_MARGIN_RIGHT	47
#define TAB_ITEM_HEIGHT		(TAB_WINDOW_HEIGHT - 5)
#define MAX_TABITEM_WIDTH	200
#define MIN_TABITEM_WIDTH	60
#define CX_SMICON			16
#define CY_SMICON			16
static const RECT rcBtnBase = { 0, 0, 16, 16 };

// 2006.02.01 ryoji �^�u�ꗗ���j���[�p�f�[�^
typedef struct {
	int iItem;
	int iImage;
	TCHAR szText[_MAX_PATH];
} TABMENU_DATA;

/*!	�^�u�ꗗ���j���[�p�f�[�^�� qsort() �R�[���o�b�N����
	@date 2006.02.01 ryoji �V�K�쐬
*/
static int compTABMENU_DATA( const void *arg1, const void *arg2 )
{
	int ret;

	ret = ::lstrcmp( ((TABMENU_DATA*)arg1)->szText, ((TABMENU_DATA*)arg2)->szText );
	if( 0 == ret )
		ret = ((TABMENU_DATA*)arg1)->iItem - ((TABMENU_DATA*)arg2)->iItem;
	return ret;
}


WNDPROC	gm_pOldWndProc = NULL;

/* TabWnd�E�B���h�E���b�Z�[�W�̃R�[���o�b�N�֐� */
LRESULT CALLBACK TabWndProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CTabWnd	*pcTabWnd;

	// Modified by KEITA for WIN64 2003.9.6
	pcTabWnd = (CTabWnd*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	if( pcTabWnd )
	{
		//return
		if( 0L == pcTabWnd->TabWndDispatchEvent( hwnd, uMsg, wParam, lParam ) )
			return 0L;
	}

	if( gm_pOldWndProc )
		return ::CallWindowProc( (WNDPROC)gm_pOldWndProc, hwnd, uMsg, wParam, lParam );
	else
		return ::DefWindowProc( hwnd, uMsg, wParam, lParam );
}

/* ���b�Z�[�W�z�� */
LRESULT CTabWnd::TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 2005.09.01 ryoji �^�u���̃��b�Z�[�W�������ʂɊ֐������A�^�u�����ύX�̏�����ǉ�
	switch( uMsg )
	{
	case WM_LBUTTONDOWN:
		return OnTabLButtonDown( wParam, lParam );

	case WM_LBUTTONUP:
		return OnTabLButtonUp( wParam, lParam );

	case WM_MOUSEMOVE:
		return OnTabMouseMove( wParam, lParam );

	case WM_CAPTURECHANGED:
		return OnTabCaptureChanged( wParam, lParam );

	case WM_RBUTTONDOWN:
		return OnTabRButtonDown( wParam, lParam );

	case WM_RBUTTONUP:
		return OnTabRButtonUp( wParam, lParam );

	case WM_MBUTTONDOWN:
		return OnTabMButtonDown( wParam, lParam );

	case WM_MBUTTONUP:
		return OnTabMButtonUp( wParam, lParam );

	case WM_NOTIFY:
		return OnTabNotify( wParam, lParam );

	case WM_HSCROLL:
		::InvalidateRect( m_hWnd, NULL, TRUE );	// �A�N�e�B�u�^�u�̈ʒu���ς��̂Ńg�b�v�o���h���X�V����	// 2006.03.27 ryoji
		break;

	case WM_THEMECHANGED:
		m_bVisualStyle = ::IsVisualStyle();
		break;

	//default:
	}

	return 1L;	//�f�t�H���g�̃f�B�X�p�b�`�ɂ܂킷
}

/*! �^�u�� WM_LBUTTONDOWN ���� */
LRESULT CTabWnd::OnTabLButtonDown( WPARAM wParam, LPARAM lParam )
{
	// �{�^���������ꂽ�ʒu���m�F����
	TCHITTESTINFO hitinfo;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nSrcTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	if( 0 > nSrcTab )
		return 1L;

	m_eDragState = DRAG_CHECK;	// �h���b�O�̃`�F�b�N���J�n

	// �h���b�O���^�u���L������
	m_nSrcTab = nSrcTab;

	::SetCapture( m_hwndTab );

	return 0L;
}

/*! �^�u�� WM_LBUTTONUP ���� */
LRESULT CTabWnd::OnTabLButtonUp( WPARAM wParam, LPARAM lParam )
{
	TCHITTESTINFO	hitinfo;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	int nSelfTab = FindTabIndexByHWND( m_hwndParent );

	switch( m_eDragState )
	{
	case DRAG_CHECK:
		if ( m_nSrcTab == nDstTab && m_nSrcTab != nSelfTab )
		{
			//�w��̃E�C���h�E���A�N�e�B�u��
			TCITEM	tcitem;
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = (LPARAM)0;
			TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );

			ShowHideWindow( (HWND)tcitem.lParam, TRUE );
		}
		break;

	case DRAG_DRAG:
		// �^�u�̏�����ύX����
		if( ReorderTab( m_nSrcTab, nDstTab ) )
		{
			// �ĕ\�����b�Z�[�W���u���[�h�L���X�g����B
			HWND hwndSel = ( nSelfTab == m_nSrcTab )? m_hwndParent: NULL;
			CShareData::getInstance()->PostMessageToAllEditors( MYWM_TAB_WINDOW_NOTIFY, (WPARAM)TWNT_REFRESH, (LPARAM)hwndSel, m_hwndParent );
		}
		break;

	default:
		break;
	}

	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	return 0L;
}

/*! �^�u�� WM_MOUSEMOVE ���� */
LRESULT CTabWnd::OnTabMouseMove( WPARAM wParam, LPARAM lParam )
{
	TCHITTESTINFO	hitinfo;
	hitinfo.pt.x = LOWORD( (DWORD)lParam );
	hitinfo.pt.y = HIWORD( (DWORD)lParam );
	int nDstTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );

	switch( m_eDragState )
	{
	case DRAG_CHECK:
		// ���̃^�u���痣�ꂽ��h���b�O�J�n
		if( m_nSrcTab == nDstTab )
			break;
		m_eDragState = DRAG_DRAG;
		// �����ɗ�����h���b�O�J�n�Ȃ̂� break ���Ȃ��ł��̂܂� DRAG_DRAG �����ɓ���

	case DRAG_DRAG:
		// �h���b�O���̃}�E�X�J�[�\����\������
		HCURSOR	hCursor;
		if ( 0 > nDstTab || m_nSrcTab == nDstTab )
			hCursor = ::LoadCursor( NULL, IDC_NO );
		else
			hCursor = ::LoadCursor( ::GetModuleHandle(NULL), MAKEINTRESOURCE(IDC_CURSOR_MOVEARROW) );
		::SetCursor( hCursor );
		break;

	default:
		return 1L;
	}

	return 0L;
}

/*! �^�u�� WM_CAPTURECHANGED ���� */
LRESULT CTabWnd::OnTabCaptureChanged( WPARAM wParam, LPARAM lParam )
{
	if( m_eDragState != DRAG_NONE )
		m_eDragState = DRAG_NONE;

	return 0L;
}

/*! �^�u�� WM_RBUTTONDOWN ���� */
LRESULT CTabWnd::OnTabRButtonDown( WPARAM wParam, LPARAM lParam )
{
	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	return 0L;	// 2006.01.28 ryoji OnTabMButtonDown �ɂ��킹�� 0 ��Ԃ��悤�ɕύX
}

/*! �^�u�� WM_RBUTTONUP ���� */
LRESULT CTabWnd::OnTabRButtonUp( WPARAM wParam, LPARAM lParam )
{
	// 2006.01.28 ryoji �^�u�̃J�X�^�����j���[�\���R�}���h�����s����(�֐���)
	return ExecTabCommand( F_CUSTMENU_BASE + CUSTMENU_INDEX_FOR_TABWND, MAKEPOINTS(lParam) );
}

/*! �^�u�� WM_MBUTTONDOWN ����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnTabMButtonDown( WPARAM wParam, LPARAM lParam )
{
	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	return 0L;	// �t�H�[�J�X���^�u�Ɉڂ�Ȃ��悤�A�����ł� 0 ��Ԃ�
}

/*! �^�u�� WM_MBUTTONUP ����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnTabMButtonUp( WPARAM wParam, LPARAM lParam )
{
	// �E�B���h�E�����R�}���h�����s����
	return ExecTabCommand( F_WINCLOSE, MAKEPOINTS(lParam) );
}

/*! �^�u�� WM_NOTIFY ����

	@date 2005.09.01 ryoji �֐���
	@date 2006.10.31 ryoji �c�[���`�b�v�̃t���p�X�����ȈՕ\������
*/
LRESULT CTabWnd::OnTabNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMTTDISPINFO	lpnmtdi;
	lpnmtdi = (LPNMTTDISPINFO)lParam;
	if( lpnmtdi->hdr.hwndFrom == m_hwndToolTip )
	{
		switch( lpnmtdi->hdr.code )
		{
		//case TTN_NEEDTEXT:
		case TTN_GETDISPINFOW:
		case TTN_GETDISPINFOA:
			{
				TCITEM	tcitem;

				tcitem.mask   = TCIF_PARAM;
				tcitem.lParam = (LPARAM)NULL;
				if( TabCtrl_GetItem( m_hwndTab, lpnmtdi->hdr.idFrom, &tcitem ) )
				{
					FileInfo*	pfi;
					::SendMessage( (HWND)tcitem.lParam, MYWM_GETFILEINFO, 0, 0 );
					pfi = (FileInfo*)&m_pShareData->m_FileInfo_MYWM_GETFILEINFO;

					if( pfi->m_szPath[0] )
					{
						// �t���p�X�����ȈՕ\������	// 2006.10.31 ryoji
						TCHAR szText[_MAX_PATH];
						CShareData::getInstance()->GetTransformFileNameFast( pfi->m_szPath, szText, _MAX_PATH );
						_tcsncpy( m_szTextTip1, szText, sizeof( m_szTextTip1 ) / sizeof( TCHAR ) );
						m_szTextTip1[ (sizeof( m_szTextTip1 ) / sizeof( TCHAR )) - 1 ] = _T('\0');
					}
					else if( pfi->m_bIsGrep )
					{
						_tcsncpy( m_szTextTip1, pfi->m_szGrepKey, sizeof( m_szTextTip1 ) / sizeof( TCHAR ) );
						m_szTextTip1[ (sizeof( m_szTextTip1 ) / sizeof( TCHAR )) - 1 ] = _T('\0');
					}
					else if( pfi->m_bIsDebug )
					{
						_tcscpy( m_szTextTip1, _T("(�A�E�g�v�b�g�E�C���h�E)") );
					}
					else
					{
						_tcscpy( m_szTextTip1, _T("(����)") );
					}

					if( TTN_GETDISPINFOW == lpnmtdi->hdr.code )
					{
#ifdef UNICODE
						lpnmtdi->lpszText = m_szTextTip1;
						lpnmtdi->hinst    = NULL;
#else
						//UNICODE�̕����񂪗~�����B
						int	Size = _tcslen( m_szTextTip1 );
						m_szTextTip2[ MultiByteToWideChar( CP_ACP, 0, m_szTextTip1, Size, m_szTextTip2, Size ) ] = 0;
						LPNMTTDISPINFOW	lpnmtdiw = (LPNMTTDISPINFOW)lParam;
						lpnmtdiw->lpszText = m_szTextTip2;
						lpnmtdiw->hinst    = NULL;
#endif	//UNICODE
					}
					else
					{
#ifdef UNICODE
						//SJIS�̕����񂪗~�����B
						int	Size = _tcslen( m_szTextTip1 );
						m_szTextTip2[ WideCharToMultiByte( CP_ACP, 0, m_szTextTip1, Size, m_szTextTip2, Size, 0, 0 ) ] = 0;
						LPNMTTDISPINFOA	lpnmtdia = (LPNMTTDISPINFOA)lParam;
						lpnmtdia->lpszText = m_szTextTip2;
						lpnmtdia->hinst    = NULL;
#else
						lpnmtdi->lpszText = m_szTextTip1;
						lpnmtdi->hinst    = NULL;
#endif	//UNICODE
					}

					return 0L;
				}
			}
		}
	}

	return 1L;
}

/*! �^�u�����ύX����
	@date 2005.09.01 ryoji �V�K�쐬

*/
BOOL CTabWnd::ReorderTab( int nSrcTab, int nDstTab )
{
	EditNode	*p;
	int			nCount;
	int			i;
	TCITEM		tcitem;
	HWND		hwndSrc;	// �ړ����E�B���h�E
	HWND		hwndDst;	// �ړ���E�B���h�E

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
		return FALSE;

	// �ړ����^�u�A�ړ���^�u�̃E�B���h�E���擾����
	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = (LPARAM)0;
	TabCtrl_GetItem( m_hwndTab, nSrcTab, &tcitem );
	hwndSrc = (HWND)tcitem.lParam;

	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = (LPARAM)0;
	TabCtrl_GetItem( m_hwndTab, nDstTab, &tcitem );
	hwndDst = (HWND)tcitem.lParam;

	// ���L�f�[�^��ł̃E�B���h�E�쐬�����擾�i�^�u���Ɠ����͂��j
	p = NULL;
	nSrcTab = -1;
	nDstTab = -1;
	nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, TRUE );
	for( i = 0; i < nCount; i++ )
	{
		if( hwndSrc == p[i].m_hWnd )
			nSrcTab = i;
		if( hwndDst == p[i].m_hWnd )
			nDstTab = i;
	}

	if( 0 > nSrcTab || 0 > nDstTab || nSrcTab == nDstTab )
	{
		if( p ) delete []p;
		return FALSE;
	}

	// �^�u�̏��������ւ��邽�߂ɃE�B���h�E�̃C���f�b�N�X�����ւ���
	int nArr0, nArr1;
	int	nIndex;

	nArr0 = p[ nDstTab ].m_nIndex;
	nIndex = m_pShareData->m_pEditArr[ nArr0 ].m_nIndex;
	if( nSrcTab < nDstTab )
	{
		// �^�u���������[�e�[�g
		for( i = nDstTab - 1; i >= nSrcTab; i-- )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	else
	{
		// �^�u�E�������[�e�[�g
		for( i = nDstTab + 1; i <= nSrcTab; i++ )
		{
			nArr1 = p[ i ].m_nIndex;
			m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = m_pShareData->m_pEditArr[ nArr1 ].m_nIndex;
			nArr0 = nArr1;
		}
	}
	m_pShareData->m_pEditArr[ nArr0 ].m_nIndex = nIndex;

	if( p ) delete []p;

	return TRUE;
}

/*! �^�u�� �R�}���h���s����
	@date 2006.01.28 ryoji �V�K�쐬
*/
LRESULT CTabWnd::ExecTabCommand( int nId, POINTS pts )
{
	// �}�E�X�ʒu(pt)�̃^�u���擾����
	TCHITTESTINFO	hitinfo;
	hitinfo.pt.x = pts.x;
	hitinfo.pt.y = pts.y;
	int nTab = TabCtrl_HitTest( m_hwndTab, (LPARAM)&hitinfo );
	if( nTab < 0 )
		return 1L;

	// �ΏۃE�B���h�E���擾����
	TCITEM	tcitem;
	tcitem.mask   = TCIF_PARAM;
	tcitem.lParam = (LPARAM)0;
	if( !TabCtrl_GetItem( m_hwndTab, nTab, &tcitem ) )
		return 1L;
	HWND hWnd = (HWND)tcitem.lParam;

	// �ΏۃE�C���h�E���A�N�e�B�u�ɂ���B
	ShowHideWindow( hWnd, TRUE );

	// �R�}���h��ΏۃE�C���h�E�ɑ���B
	::PostMessage( hWnd, WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );

	return 0L;
}


CTabWnd::CTabWnd()
  : m_bVisualStyle( FALSE ),		// 2007.04.01 ryoji
    m_eDragState( DRAG_NONE ),
    m_bHovering( FALSE ),	//	2006.02.01 ryoji
    m_bListBtnHilighted( FALSE ),	//	2006.02.01 ryoji
    m_bCloseBtnHilighted( FALSE ),	//	2006.10.21 ryoji
    m_eCaptureSrc( CAPT_NONE )	//	2006.11.30 ryoji
{
	strcat( m_szClassInheritances, _T("::CTabWnd") );

	m_pszClassName = _T("CTabWnd");
	/* ���L�f�[�^�\���̂̃A�h���X��Ԃ� */
	m_pShareData = CShareData::getInstance()->GetShareData();

	m_hInstance  = NULL;
	m_hwndParent = NULL;
	m_hwndTab    = NULL;
	m_hFont      = NULL;
	gm_pOldWndProc = NULL;
	m_hwndToolTip = NULL;
	m_hIml = NULL;

	// 2006.02.17 ryoji ImageList_Duplicate() �̃A�h���X���擾����
	// �iIE4.0 �����̊��ł�����\�Ȃ悤�ɓ��I���[�h�j
    HINSTANCE hinst = ::GetModuleHandle(TEXT("comctl32"));
    *(FARPROC*)&m_RealImageList_Duplicate = ::GetProcAddress(hinst, "ImageList_Duplicate");

	return;
}

CTabWnd::~CTabWnd()
{
	return;
}

/* �E�B���h�E �I�[�v�� */
HWND CTabWnd::Open( HINSTANCE hInstance, HWND hwndParent )
{
	/* ������ */
	m_hInstance  = hInstance;
	m_hwndParent = hwndParent;
	m_hwndTab    = NULL;
	m_hFont      = NULL;
	gm_pOldWndProc = NULL;
	m_hwndToolTip = NULL;
	m_bVisualStyle = ::IsVisualStyle();	// 2007.04.01 ryoji
	m_eDragState = DRAG_NONE;	//	2005.09.29 ryoji
	m_bHovering = FALSE;			// 2006.02.01 ryoji
	m_bListBtnHilighted = FALSE;	// 2006.02.01 ryoji
	m_bCloseBtnHilighted = FALSE;	// 2006.10.21 ryoji
	m_eCaptureSrc = CAPT_NONE;	// 2006.11.30 ryoji

	/* �E�B���h�E�N���X�쐬 */
	RegisterWC(
		/* WNDCLASS�p */
		NULL,								// Handle to the class icon.
		NULL,								//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),	// Handle to the class cursor.
		// 2006.01.30 ryoji �w�i�� WM_PAINT �ŕ`�悷��ق���������Ȃ��i�Ǝv���j
		//(HBRUSH)(COLOR_3DFACE + 1),			// Handle to the class background brush.
		NULL,								// Handle to the class background brush.
		NULL,								// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName						// Pointer to a null-terminated string or is an atom.
	);

	/* ���N���X�����o�Ăяo�� */
	CWnd::Create(
		/* CreateWindowEx()�p */
		0,									// extended window style
		m_pszClassName,						// Pointer to a null-terminated string or is an atom.
		m_pszClassName,						// pointer to window name
		WS_CHILD/* | WS_VISIBLE*/,			// window style	// 2007.03.08 ryoji WS_VISIBLE ����
		// 2006.01.30 ryoji �����z�u������
		// ���^�u��\�� -> �\���ؑւŕҏW�E�B���h�E�ɃS�~���\������邱�Ƃ�����̂ŏ������̓[����
		CW_USEDEFAULT,						// horizontal position of window
		0,									// vertical position of window
		0,									// window width
		TAB_WINDOW_HEIGHT,					// window height
		NULL								// handle to menu, or child-window identifier
	);

	//�^�u�E�C���h�E���쐬����B
	m_hwndTab = ::CreateWindow(
		WC_TABCONTROL,
		_T(""),
		//	2004.05.22 MIK ������TAB�΍��WS_CLIPSIBLINGS�ǉ�
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
		// 2006.01.30 ryoji �����z�u������
		TAB_MARGIN_LEFT,
		TAB_MARGIN_TOP,
		0,
		TAB_WINDOW_HEIGHT,
		m_hWnd,
		(HMENU)NULL,
		m_hInstance,
		(LPVOID)NULL
		);
	if( m_hwndTab )
	{
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hwndTab, GWLP_USERDATA, (LONG_PTR) this );
		gm_pOldWndProc = (WNDPROC)::SetWindowLongPtr( m_hwndTab, GWLP_WNDPROC, (LONG_PTR) TabWndProc );

		//�X�^�C����ύX����B
		UINT lngStyle;
		lngStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
		//	Feb. 14, 2004 MIK �}���`���C�����̕ύX�����߂�
		lngStyle &= ~(TCS_BUTTONS | TCS_MULTILINE);
		lngStyle |= TCS_TABS | TCS_SINGLELINE | TCS_FOCUSNEVER | TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;	// 2006.01.28 ryoji
		//lngStyle &= ~(TCS_BUTTONS | TCS_SINGLELINE);	//2004.01.31
		//lngStyle |= TCS_TABS | TCS_MULTILINE;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lngStyle );
		TabCtrl_SetItemSize( m_hwndTab, MAX_TABITEM_WIDTH, TAB_ITEM_HEIGHT );	// 2006.01.28 ryoji

		/* �\���p�t�H���g */
		/* LOGFONT�̏����� */
		LOGFONT	lf;
		::ZeroMemory( &lf, sizeof(LOGFONT) );
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
		_tcscpy( lf.lfFaceName, _T("�l�r �o�S�V�b�N") );
		m_hFont = ::CreateFontIndirect( &lf );
		
		/* �t�H���g�ύX */
		::SendMessage( m_hwndTab, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0) );

		//�c�[���`�b�v���쐬����B
		//	2005.08.11 ryoji �u�d�˂ĕ\���v��Z-order�����������Ȃ�̂�TOPMOST�w�������
		m_hwndToolTip = ::CreateWindowEx(
			0,
			TOOLTIPS_CLASS,
			NULL,
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			m_hWnd, //m_hwndTab,
			NULL,
			m_hInstance,
			NULL
			);

		// �c�[���`�b�v���}���`���C���\�ɂ���iSHRT_MAX: Win95��INT_MAX���ƕ\������Ȃ��j	// 2007.03.03 ryoji
		::SendMessage( m_hwndToolTip, TTM_SETMAXTIPWIDTH, 0, (LPARAM)SHRT_MAX );

		// �^�u�o�[�Ƀc�[���`�b�v��ǉ�����
		TOOLINFO	ti;
		ti.cbSize      = sizeof( TOOLINFO );
		ti.uFlags      = TTF_SUBCLASS | TTF_IDISHWND;	// TTF_IDISHWND: uId �� HWND �� rect �͖����iHWND �S�́j
		ti.hwnd        = m_hWnd;
		ti.hinst       = m_hInstance;
		ti.uId         = (UINT)m_hWnd;
		ti.lpszText    = NULL;
		ti.rect.left   = 0;
		ti.rect.top    = 0;
		ti.rect.right  = 0;
		ti.rect.bottom = 0;
		::SendMessage( m_hwndToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti );

		// �^�u�o�[���̃^�u�R���g���[���Ƀc�[���`�b�v��ǉ�����
		TabCtrl_SetToolTips( m_hwndTab, m_hwndToolTip );

		// 2006.02.22 ryoji �C���[�W���X�g������������
		InitImageList();

		//TabCtrl_DeleteAllItems( m_hwndTab );
		//::ShowWindow( m_hwndTab, SW_HIDE );
		Refresh();
	}

	return m_hWnd;
}

/* �E�B���h�E �N���[�Y */
void CTabWnd::Close( void )
{
	if( m_hWnd )
	{
		if( gm_pOldWndProc )
		{
			// Modified by KEITA for WIN64 2003.9.6
			::SetWindowLongPtr( m_hwndTab, GWLP_WNDPROC, (LONG_PTR)gm_pOldWndProc );
			gm_pOldWndProc = NULL;
		}
		
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( m_hwndTab, GWLP_USERDATA, (LONG_PTR)NULL );

		if( m_hwndToolTip )
		{
			::DestroyWindow( m_hwndToolTip );
			m_hwndToolTip = NULL;
		}

		::DestroyWindow( m_hWnd );
		m_hWnd = NULL;
	}
}

//WM_SIZE����
LRESULT CTabWnd::OnSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	RECT	rcParent;

	if( NULL == m_hWnd || NULL == m_hwndTab ) return 0L;

	::GetWindowRect( m_hWnd, &rcParent );

	::MoveWindow( m_hwndTab, TAB_MARGIN_LEFT, TAB_MARGIN_TOP, (rcParent.right - rcParent.left) - (TAB_MARGIN_LEFT + TAB_MARGIN_RIGHT), TAB_WINDOW_HEIGHT, TRUE );	// 2005.01.30 ryoji

	LayoutTab();	// 2006.01.28 ryoji �^�u�̃��C�A�E�g��������

	::InvalidateRect( m_hWnd, NULL, FALSE );	//	2006.02.01 ryoji

	return 0L;
}

//WM_DSESTROY����
LRESULT CTabWnd::OnDestroy( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	//�^�u�R���g���[�����폜
	if( m_hwndTab )
	{
		::DestroyWindow( m_hwndTab );
		m_hwndTab = NULL;
	}

	//�\���p�t�H���g
	if( m_hFont )
	{
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}

	// 2006.01.28 ryoji �C���[�W���X�g���폜
	if( NULL != m_hIml )
	{
		ImageList_Destroy( m_hIml );
		m_hIml = NULL;
	}

	::KillTimer( hwnd, 1 );	//	2006.02.01 ryoji

	m_hWnd = NULL;

	return 0L;
}
 
/*! WM_LBUTTONDBLCLK����
	@date 2006.03.26 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnLButtonDblClk( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// �V�K�쐬�R�}���h�����s����
	::SendMessage( m_hwndParent, WM_COMMAND, MAKEWPARAM( F_FILENEW, 0 ), (LPARAM)NULL );
	return 0L;
}

/*!	WM_CAPTURECHANGED����
	@date 2006.11.30 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnCaptureChanged( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( m_eCaptureSrc != CAPT_NONE )
		m_eCaptureSrc = CAPT_NONE;

	return 0L;
}

/*!	WM_LBUTTONDOWN����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.11.30 ryoji �^�u�ꗗ�{�^���N���b�N�֐���p�~���ď�����荞��
	                       ����{�^����Ȃ�L���v�`���[�J�n
*/
LRESULT CTabWnd::OnLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( m_hWnd, &rc );

	// �^�u�ꗗ�{�^����Ȃ�^�u�ꗗ���j���[�i�^�u���j��\������
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( m_hWnd, &pt );
		TabListMenu( pt, FALSE, FALSE );	// �^�u�ꗗ���j���[�i�^�u���j
	}
	else
	{
		// ����{�^����Ȃ�L���v�`���[�J�n
		GetCloseBtnRect( &rc, &rcBtn );
		if( ::PtInRect( &rcBtn, pt ) )
		{
			m_eCaptureSrc = CAPT_CLOSE;	// �L���v�`���[���͕���{�^��
			::SetCapture( m_hWnd );
		}
	}

	return 0L;
}

/*!	WM_LBUTTONUP����
	@date 2006.11.30 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( m_hWnd, &rc );

	if( ::GetCapture() == m_hWnd )	// ���E�B���h�E���}�E�X�L���v�`���[���Ă���?
	{
		if( m_eCaptureSrc == CAPT_CLOSE )	// �L���v�`���[���͕���{�^��?
		{
			// ����{�^����Ȃ�^�u�����
			GetCloseBtnRect( &rc, &rcBtn );
			if( ::PtInRect( &rcBtn, pt ) )
			{
				int nId;
				if( m_pShareData->m_Common.m_bDispTabWnd &&
					!m_pShareData->m_Common.m_bDispTabWndMultiWin &&
					!m_pShareData->m_Common.m_bTab_CloseOneWin			// 2007.02.13 ryoji �����ǉ�
					)
				{
					nId = F_WINCLOSE;	// ����i�^�C�g���o�[�̕���{�^���͕ҏW�̑S�I���j
				}
				else
				{
					nId = F_EXITALLEDITORS;	// �ҏW�̑S�I���i�^�C�g���o�[�̕���{�^���͂P��������j
				}
				::PostMessage( m_hwndParent, WM_COMMAND, MAKEWPARAM( nId, 0 ), (LPARAM)NULL );
			}
		}

		// �L���v�`���[����
		m_eCaptureSrc = CAPT_NONE;
		::ReleaseCapture();
	}

	return 0L;
}

/*!	WM_RBUTTONDOWN����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.11.30 ryoji �^�u�ꗗ�{�^���N���b�N�֐���p�~���ď�����荞��
*/
LRESULT CTabWnd::OnRButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT pt;
	RECT rc;
	RECT rcBtn;

	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	::GetClientRect( m_hWnd, &rc );

	// �^�u�ꗗ�{�^����Ȃ�^�u�ꗗ���j���[�i�t���p�X�j��\������	// 2006.11.30 ryoji
	GetListBtnRect( &rc, &rcBtn );
	if( ::PtInRect( &rcBtn, pt ) )
	{
		pt.x = rcBtn.left;
		pt.y = rcBtn.bottom;
		::ClientToScreen( m_hWnd, &pt );
		TabListMenu( pt, FALSE, TRUE );	// �^�u�ꗗ���j���[�i�t���p�X�j
	}

	return 0L;
}

/*!	WM_MEASUREITEM����
	@date 2006.02.01 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnMeasureItem( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	MEASUREITEMSTRUCT* lpmis = (MEASUREITEMSTRUCT*)lParam;
	if( lpmis->CtlType == ODT_MENU )
	{
		TABMENU_DATA* pData = (TABMENU_DATA*)lpmis->itemData;

		HDC hdc = ::GetDC( hwnd );
		HFONT hfnt = CreateMenuFont();
		HFONT hfntOld = (HFONT)::SelectObject( hdc, hfnt );
		SIZE size;

		::GetTextExtentPoint32( hdc, pData->szText, ::lstrlen(pData->szText), &size );

		lpmis->itemHeight = ::GetSystemMetrics( SM_CYMENU );
		lpmis->itemWidth = size.cx + CX_SMICON + 8;

		::SelectObject( hdc, hfntOld );
		::DeleteObject( hfnt );
		::ReleaseDC( hwnd, hdc );
	}

	return 0L;
}

/*!	WM_DRAWITEM����
	@date 2006.02.01 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnDrawItem( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	DRAWITEMSTRUCT* lpdis = (DRAWITEMSTRUCT*)lParam;
	if( lpdis->CtlType == ODT_MENU )
	{
		// �^�u�ꗗ���j���[��`�悷��
		TABMENU_DATA* pData = (TABMENU_DATA*)lpdis->itemData;

		HDC hdc = lpdis->hDC;
		RECT rcItem = lpdis->rcItem;

		// ��Ԃɏ]���ăe�L�X�g�Ɣw�i�F�����߂�
		COLORREF clrText;
		COLORREF clrBk;
		if (lpdis->itemState & ODS_SELECTED)
		{
			clrText = ::GetSysColor( COLOR_HIGHLIGHTTEXT );
			clrBk = COLOR_HIGHLIGHT;
		}
		else
		{
			clrText = ::GetSysColor( COLOR_MENUTEXT );
			clrBk = COLOR_MENU;
		}

		// �w�i�`��
		::FillRect( hdc, &rcItem, (HBRUSH)(clrBk + 1) );

		// �A�C�R���`��
		if( NULL != m_hIml && 0 <= pData->iImage )
		{
			int top = rcItem.top + ( rcItem.bottom - rcItem.top - CY_SMICON ) / 2;
			ImageList_Draw( m_hIml, pData->iImage, lpdis->hDC, rcItem.left + 2, top, ILD_TRANSPARENT );
		}

		// �e�L�X�g�`��
		COLORREF clrTextOld = ::SetTextColor( hdc, clrText );
		int iBkModeOld = ::SetBkMode( hdc, TRANSPARENT );
		HFONT hfnt = CreateMenuFont();
		HFONT hfntOld = (HFONT)::SelectObject( hdc, hfnt );
		RECT rcText = rcItem;
		rcText.left += (CX_SMICON + 8);

		::DrawText( hdc, pData->szText, -1, &rcText, DT_SINGLELINE | DT_LEFT | DT_VCENTER );

		::SetTextColor( hdc, clrTextOld );
		::SetBkMode( hdc, iBkModeOld );
		::SelectObject( hdc, hfntOld );
		::DeleteObject( hfnt );

		// �`�F�b�N��ԂȂ�O�g�`��
		if( lpdis->itemState & ODS_CHECKED )
		{
			HPEN hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_HIGHLIGHT ) );
			HBRUSH hbr = (HBRUSH)::GetStockObject( NULL_BRUSH );
			HPEN hpenOld = (HPEN)::SelectObject( hdc, hpen );
			HBRUSH hbrOld = (HBRUSH)::SelectObject( hdc, hbr );

			::Rectangle( hdc, rcItem.left, rcItem.top, rcItem.right, rcItem.bottom );

			::SelectObject( hdc, hpenOld );
			::SelectObject( hdc, hbrOld );
			::DeleteObject( hpen );
		}
	}

	return 0L;
}

/*!	WM_MOUSEMOVE����
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2007.03.05 ryoji �{�^���̏o����Ńc�[���`�b�v���X�V����
*/
LRESULT CTabWnd::OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// �J�[�\�����E�B���h�E���ɓ�������^�C�}�[�N��
	// �E�B���h�E�O�ɏo����^�C�}�[�폜
	POINT pt;
	RECT rc;
	BOOL bHovering;

	pt.x = LOWORD( lParam );
	pt.y = HIWORD( lParam );
	::GetClientRect( hwnd, &rc );
	bHovering = ::PtInRect( &rc, pt );
	if( bHovering != m_bHovering )
	{
		m_bHovering = bHovering;
		if( m_bHovering )
			::SetTimer( hwnd, 1, 200, NULL );
		else
			::KillTimer( hwnd, 1 );
	}

	// �J�[�\�����{�^������o���肷��Ƃ��ɍĕ`��
	RECT rcBtn;
	LPTSTR pszTip = (LPTSTR)-1L;

	GetListBtnRect( &rc, &rcBtn );
	bHovering = ::PtInRect( &rcBtn, pt );
	if( bHovering != m_bListBtnHilighted )
	{
		m_bListBtnHilighted = bHovering;
		::InvalidateRect( hwnd, &rcBtn, FALSE );

		// �c�[���`�b�v�p�̕�����쐬	// 2007.03.05 ryoji
		pszTip = NULL;	// �{�^���̊O�ɏo��Ƃ��͏���
		if( m_bListBtnHilighted )	// �{�^���ɓ����Ă���?
		{
			pszTip = m_szTextTip1;
			_tcscpy( m_szTextTip1, _T("���N���b�N: �^�u���ꗗ\n�E�N���b�N: �p�X���ꗗ") );
		}
	}

	GetCloseBtnRect( &rc, &rcBtn );
	bHovering = ::PtInRect( &rcBtn, pt );
	if( bHovering != m_bCloseBtnHilighted )
	{
		m_bCloseBtnHilighted = bHovering;
		::InvalidateRect( hwnd, &rcBtn, FALSE );

		// �c�[���`�b�v�p�̕�����쐬	// 2007.03.05 ryoji
		pszTip = NULL;	// �{�^���̊O�ɏo��Ƃ��͏���
		if( m_bCloseBtnHilighted )	// �{�^���ɓ����Ă���?
		{
			pszTip = m_szTextTip1;
			if( m_pShareData->m_Common.m_bDispTabWnd &&
				!m_pShareData->m_Common.m_bDispTabWndMultiWin &&
				!m_pShareData->m_Common.m_bTab_CloseOneWin
				)
			{
				_tcscpy( m_szTextTip1, _T("�^�u�����") );
			}
			else
			{
				::LoadString( m_hInstance, F_EXITALLEDITORS, m_szTextTip1, sizeof(m_szTextTip1)/sizeof(TCHAR) );
				m_szTextTip1[sizeof(m_szTextTip1)/sizeof(TCHAR) - 1] = _T('\0');
			}
		}
	}

	// �c�[���`�b�v�X�V	// 2007.03.05 ryoji
	if( pszTip != (LPTSTR)-1L )	// �{�^���ւ̏o���肪������?
	{
		TOOLINFO ti;
		::ZeroMemory( &ti, sizeof(ti) );
		ti.cbSize       = sizeof( TOOLINFO );
		ti.hwnd         = m_hWnd;
		ti.hinst        = m_hInstance;
		ti.uId          = (UINT)m_hWnd;
		ti.lpszText     = pszTip;
		::SendMessage( m_hwndToolTip, TTM_UPDATETIPTEXT, (WPARAM)0, (LPARAM)&ti );
	}

	return 0L;
}

/*!	WM_TIMER����
	@date 2006.02.01 ryoji �V�K�쐬
*/
LRESULT CTabWnd::OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if( wParam == 1 )
	{
		// �J�[�\�����E�B���h�E�O�ɂ���ꍇ�ɂ� WM_MOUSEMOVE �𑗂�
		POINT pt;
		RECT rc;

		::GetCursorPos( &pt );
		::ScreenToClient( hwnd, &pt );
		::GetClientRect( hwnd, &rc );
		if( !::PtInRect( &rc, pt ) )
			::SendMessage( hwnd, WM_MOUSEMOVE, 0, MAKELONG( pt.x, pt.y ) );
	}

	return 0L;
}

/*!	WM_PAINT����

	@date 2005.09.01 ryoji �^�u�̏�ɋ��E����ǉ�
	@date 2006.01.30 ryoji �w�i�`�揈����ǉ��i�w�i�u���V�� NULL �ɕύX�j
	@date 2006.02.01 ryoji �ꗗ�{�^���̕`�揈����ǉ�
	@date 2006.10.21 ryoji ����{�^���̕`�揈����ǉ�
	@date 2007.03.27 ryoji Windows�N���V�b�N�X�^�C���̏ꍇ�̓A�N�e�B�u�^�u�̏㕔�Ƀg�b�v�o���h��`�悷��
*/
LRESULT CTabWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rc;

	hdc = ::BeginPaint( hwnd, &ps );

	// �w�i��`�悷��
	::GetClientRect( hwnd, &rc );
	::FillRect( hdc, &rc, (HBRUSH)(COLOR_3DFACE + 1) );

	// �{�^����`�悷��
	DrawListBtn( hdc, &rc );
	DrawCloseBtn( hdc, &rc );	// 2006.10.21 ryoji �ǉ�

	// �㑤�ɋ��E����`�悷��
	::DrawEdge(hdc, &rc, EDGE_ETCHED, BF_TOP);

	// Windows�N���V�b�N�X�^�C���̏ꍇ�̓A�N�e�B�u�^�u�̏㕔�Ƀg�b�v�o���h��`�悷��	// 2006.03.27 ryoji
	if( !m_bVisualStyle )
	{
		int nCurSel = TabCtrl_GetCurSel( m_hwndTab );
		if( nCurSel >= 0 )
		{
			POINT pt;
			RECT rcCurSel;

			TabCtrl_GetItemRect( m_hwndTab, nCurSel, &rcCurSel );
			pt.x = rcCurSel.left;
			pt.y = 0;
			::ClientToScreen( m_hwndTab, &pt );
			::ScreenToClient( m_hWnd, &pt );
			rcCurSel.right = pt.x + (rcCurSel.right - rcCurSel.left) - 1;
			rcCurSel.left = pt.x + 1;
			rcCurSel.top = rc.top + TAB_MARGIN_TOP - 2;
			rcCurSel.bottom = rc.top + TAB_MARGIN_TOP;

			if( rcCurSel.left < rc.left + TAB_MARGIN_LEFT )
				rcCurSel.left = rc.left + TAB_MARGIN_LEFT;	// ���[���E�l

			HWND hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// �^�u���� Up-Down �R���g���[��
			if( hwndUpDown && ::IsWindowVisible( hwndUpDown ) )
			{
				POINT ptREnd;
				RECT rcUpDown;

				::GetWindowRect( hwndUpDown, &rcUpDown );
				ptREnd.x = rcUpDown.left;
				ptREnd.y = 0;
				::ScreenToClient( m_hWnd, &ptREnd );
				if( rcCurSel.right > ptREnd.x )
					rcCurSel.right = ptREnd.x;	// �E�[���E�l
			}

			if( rcCurSel.left < rcCurSel.right )
			{
				HBRUSH hBr = ::CreateSolidBrush( RGB( 255, 128, 0 ) );
				::FillRect( hdc, &rcCurSel, hBr );
				::DeleteObject( hBr );
			}
		}
	}

	::EndPaint( hwnd, &ps );

	return 0L;
}

/*! WM_NOTIFY����

	@date 2005.09.01 ryoji �E�B���h�E�؂�ւ��� OnTabLButtonUp() �Ɉړ�
*/
LRESULT CTabWnd::OnNotify( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	// 2005.09.01 ryoji �E�B���h�E�؂�ւ��� OnTabLButtonUp() �Ɉړ�
	return 0L;
}

void CTabWnd::TabWindowNotify( WPARAM wParam, LPARAM lParam )
{
	if( NULL == m_hwndTab ) return;

	bool	bFlag = false;	//�O�񉽂��^�u���Ȃ��������H
	int		nCount;
	int		nIndex;
	HWND	hwndUpDown;
	DWORD nScrollPos;

	BreakDrag();	// 2006.01.28 ryoji �h���b�O��Ԃ���������(�֐���)

	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( nCount <= 0 )
	{
		bFlag = true;
		//�ŏ��̂Ƃ��͂��łɑ��݂���E�C���h�E�̏����o�^����K�v������B
		if( wParam == TWNT_ADD ) wParam = TWNT_REFRESH;
	}

	switch( wParam )
	{
	case TWNT_ADD:	//�E�C���h�E�o�^
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 == nIndex )
		{
			TCITEM	tcitem;
			TCHAR	szName[1024];

			_tcscpy( szName, _T("(����)") );

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName;
			tcitem.lParam  = (LPARAM)lParam;

			// 2006.01.28 ryoji �^�u�ɃA�C�R���C���[�W��ǉ�����
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( NULL );

			TabCtrl_InsertItem( m_hwndTab, nCount, &tcitem );
			nIndex = nCount;
		}

		//TabCtrl_SetCurSel( m_hwndTab, nIndex );

		if( (HWND)lParam == m_hwndParent )
		{
			//�����Ȃ�A�N�e�B�u��
			ShowHideWindow( m_hwndParent, TRUE );
			//�����ɗ����Ƃ������Ƃ͂��łɃA�N�e�B�u
			//�R�}���h���s���̃A�E�g�v�b�g�Ŗ�肪����̂ŃA�N�e�B�u�ɂ���

			TabCtrl_SetCurSel( m_hwndTab, nIndex );

			// �����ȊO���B��
			HideOtherWindows( m_hwndParent );
		}
//		else
//		{
//			//�����ɗp���Ȃ���ΉB���B
//			ShowHideWindow( m_hwndParent, FALSE );
//		}
		break;

	case TWNT_DEL:	//�E�C���h�E�폜
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			int nArr;
			nArr = GetFirstOpenedWindow();
			if( -1 != nArr )
			{
				//���̃E�C���h�E�������Ȃ�A�N�e�B�u��
				if( m_pShareData->m_pEditArr[ nArr ].m_hWnd == m_hwndParent )
				{
					if( //TRUE  == m_pShareData->m_Common.m_bDispTabWnd	//2004.02.02
					 //&& FALSE == m_pShareData->m_Common.m_bDispTabWndMultiWin
					 /*&&*/ FALSE == ::IsWindowVisible( m_hwndParent ) )
					{
						ShowHideWindow( m_hwndParent, TRUE );
						ForceActiveWindow( m_hwndParent );
					}
				}
			}
			TabCtrl_DeleteItem( m_hwndTab, nIndex );

			// 2005.09.01 ryoji �X�N���[���ʒu����
			// �i�E�[�̂ق��̃^�u�A�C�e�����폜�����Ƃ��A�X�N���[���\�Ȃ̂ɉE�ɗ]�����ł��邱�Ƃւ̑΍�j
			hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// �^�u���� Up-Down �R���g���[��
			if( hwndUpDown != NULL )
			{
				nScrollPos = LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) );

				// ���݈ʒu nScrollPos �Ɖ�ʕ\���Ƃ���v������
				::SendMessage( m_hwndTab, WM_HSCROLL, MAKEWPARAM(SB_THUMBPOSITION, LOWORD( nScrollPos ) ), (LPARAM)NULL );	// �ݒ�ʒu�Ƀ^�u���X�N���[��
			}
		}
		break;

	case TWNT_ORDER:	//�E�C���h�E�����ύX
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			//TabCtrl_SetCurSel( m_hwndTab, nIndex );

			if( (HWND)lParam == m_hwndParent )
			{
				//�����Ȃ�A�N�e�B�u��
				if( FALSE == ::IsWindowVisible( m_hwndParent ) )
				{
					ShowHideWindow( m_hwndParent, TRUE );
				}
				//�����ɗ����Ƃ������Ƃ͂��łɃA�N�e�B�u

				// ���^�u�A�C�e���������I�ɉ��ʒu�ɂ��邽�߂ɁA
				// ���^�u�A�C�e���I��O�Ɉꎞ�I�ɉ�ʍ��[�̃^�u�A�C�e����I������
				hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// �^�u���� Up-Down �R���g���[��
				nScrollPos = ( hwndUpDown != NULL )? LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) ): 0;
				TabCtrl_SetCurSel( m_hwndTab, nScrollPos );
				TabCtrl_SetCurSel( m_hwndTab, nIndex );

				// �����ȊO���B��
				// �i�A���ؑ֎��� TWNT_ORDER ����ʔ����E�������āH��ʂ����ׂď����Ă��܂����肷��̂�h���j
				HideOtherWindows( m_hwndParent );
			}
//			else
//			{
//				//�����ɗp���Ȃ���ΉB���B
//				ShowHideWindow( m_hwndParent, FALSE );
//			}
		}
		else
		{
			//�w��̃E�C���h�E���Ȃ��̂ōĕ\��
			Refresh();
		}
		break;

	case TWNT_FILE:	//�t�@�C�����ύX
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if( -1 != nIndex )
		{
			TCITEM	tcitem;
			CRecent	cRecentEditNode;
			TCHAR	szName[1024];
			//	Jun. 19, 2004 genta
			TCHAR		szName_amp[sizeof(szName)/sizeof(szName[0]) * 2];
			EditNode	*p;

			cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
			p = (EditNode*)cRecentEditNode.GetItem( cRecentEditNode.FindItem( (const char*)&lParam ) );
			if( p && p->m_szTabCaption[0] )
			{
				_tcsncpy( szName, p->m_szTabCaption, (sizeof( szName ) / sizeof( TCHAR )) );
				szName[ (sizeof( szName ) / sizeof( TCHAR )) - 1 ] = _T('\0');
				//	Jun. 19, 2004 genta &�̃G�X�P�[�v
				dupamp( szName, szName_amp );
			}
			else
			{
				_tcscpy( szName_amp, _T("(����)") );
			}

			tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
			tcitem.pszText = szName_amp;
			tcitem.lParam  = (LPARAM)lParam;

			// 2006.01.28 ryoji �^�u�̃A�C�R���C���[�W��ύX����
			tcitem.mask |= TCIF_IMAGE;
			tcitem.iImage = GetImageIndex( p );

			TabCtrl_SetItem( m_hwndTab, nIndex, &tcitem );

			cRecentEditNode.Terminate();
		}
		else
		{
			//�w��̃E�C���h�E���Ȃ��̂ōĕ\��
			Refresh();
		}
		break;

	case TWNT_REFRESH:	//�ĕ\��
		Refresh();
		break;

	//Start 2004.07.14 Kazika �ǉ�
	//�^�u���[�h�L���ɂȂ����ꍇ�A�܂Ƃ߂��鑤�̃E�B���h�E�͉B���
	case TWNT_MODE_ENABLE:
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if (-1 != nIndex)
		{
			if( (HWND)lParam == m_hwndParent )
			{
				//�����Ȃ�A�N�e�B�u��
				//ShowHideWindow( (HWND)lParam, TRUE );
				//�����͂��Ƃ��ƃA�N�e�B�u�̂͂��c�c

				// �����ȊO���B��
				HideOtherWindows( m_hwndParent );
			}
//			else
//			{
//				//�����ɗp���Ȃ���ΉB���B
//				ShowHideWindow( m_hwndParent, FALSE );
//			}
			//TabCtrl_SetCurSel( m_hwndTab, nIndex );
		}
		break;
	//End 2004.07.14 Kazika

	//Start 2004.08.27 Kazika �ǉ�
	//�^�u���[�h�����ɂȂ����ꍇ�A�B��Ă����E�B���h�E�͕\����ԂƂȂ�
	case TWNT_MODE_DISABLE:
		nIndex = FindTabIndexByHWND( (HWND)lParam );
		if (-1 != nIndex)
		{
			if( (HWND)lParam != m_hwndParent )
			{
				//�\����ԂƂ���(�t�H�A�O���E���h�ɂ͂��Ȃ�)
				TabWnd_ActivateFrameWindow( m_hwndParent, false );
			}
			//TabCtrl_SetCurSel( m_hwndTab, nIndex );
		}
		break;
	//End 2004.08.27 Kazika

	case TWNT_WNDPL_ADJUST:	// �E�B���h�E�ʒu���킹	// 2007.04.03 ryoji
		AdjustWindowPlacement();
		return;

	default:
		break;
	}

	//�^�u�̕\���E��\����؂肩����B
	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( nCount <= 0 )
	{
		::ShowWindow( m_hwndTab, SW_HIDE );
	}
	else
	{
		if( bFlag ) ::ShowWindow( m_hwndTab, SW_SHOW );
	}

	LayoutTab();	// 2006.01.28 ryoji �^�u�̃��C�A�E�g��������

	//�X�V
	::InvalidateRect( m_hwndTab, NULL, TRUE );
	::InvalidateRect( m_hWnd, NULL, TRUE );		// 2006.10.21 ryoji �^�u���{�^���ĕ`��̂��߂ɒǉ�

	return;
}

/*! �w��̃E�C���h�E�n���h���������^�u�ʒu��T�� */
int CTabWnd::FindTabIndexByHWND( HWND hWnd )
{
	int		i;
	int		nCount;
	TCITEM	tcitem;

	if( NULL == m_hwndTab ) return -1;

	nCount = TabCtrl_GetItemCount( m_hwndTab );
	for( i = 0; i < nCount; i++ )
	{
		tcitem.mask   = TCIF_PARAM;
		tcitem.lParam = (LPARAM)0;
		TabCtrl_GetItem( m_hwndTab, i, &tcitem );
		
		if( (HWND)tcitem.lParam == hWnd ) return i;
	}

	return -1;
}

/*! �^�u���X�g���ĕ\������

	@date 2004.06.19 genta &���܂܂�Ă���t�@�C�������������\������Ȃ�
	@date 2006.02.06 ryoji �I���^�u���w�肷��HWND��������т��̏����͕s�v�Ȃ̂ō폜�i���E�B���h�E���펞�I���j
*/
void CTabWnd::Refresh( void )
{
	int			i;
	int			nIndex;
	TCITEM		tcitem;
	TCHAR		szName[1024];
	//	Jun. 19, 2004 genta
	TCHAR		szName_amp[sizeof(szName)/sizeof(szName[0]) * 2];
	EditNode	*p;
	int			nCount;
	HWND		hwndUpDown;
	DWORD		nScrollPos;

	if( NULL == m_hwndTab ) return;

	// 2005.09.01 ryoji ���݂̃^�u�̃X�N���[���ʒu���L��
	hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );	// �^�u���� Up-Down �R���g���[��
	nScrollPos = ( hwndUpDown != NULL )? LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) ): 0;

	::SendMessage( m_hwndTab, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0 );	// 2005.09.01 ryoji �ĕ`��֎~

	TabCtrl_DeleteAllItems( m_hwndTab );

	p = NULL;
	nCount = CShareData::getInstance()->GetOpenedWindowArr( &p, TRUE );

	nIndex = -1;
	for( i = 0; i < nCount; i++ )
	{
		if( m_hwndParent == p[ i ].m_hWnd ) nIndex = i;

		if( p[ i ].m_szTabCaption[0] )
		{
			_tcsncpy( szName, p[ i ].m_szTabCaption, (sizeof( szName ) / sizeof( TCHAR )) );
			szName[ (sizeof( szName ) / sizeof( TCHAR )) - 1 ] = _T('\0');
			//	Jun. 19, 2004 genta &�̃G�X�P�[�v
			dupamp( szName, szName_amp );
		}
		else
		{
			_tcscpy( szName_amp, _T("(����)") );
		}

		tcitem.mask    = TCIF_TEXT | TCIF_PARAM;
		tcitem.pszText = szName_amp;
		tcitem.lParam  = (LPARAM)p[ i ].m_hWnd;

		// 2006.01.28 ryoji �^�u�ɃA�C�R����ǉ�����
		tcitem.mask |= TCIF_IMAGE;
		tcitem.iImage = GetImageIndex( &p[ i ] );

		TabCtrl_InsertItem( m_hwndTab, i, &tcitem );
	}

	if( p ) delete [] p;

	::SendMessage( m_hwndTab, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0 );	// 2005.09.01 ryoji �ĕ`�拖��

	// �Ȍ�̑���͕\����ԂŎ��s����
	if( -1 != nIndex )
	{
		TabCtrl_SetCurSel( m_hwndTab, nIndex );
	}
	else if( 0 < nCount )
	{
		TabCtrl_SetCurSel( m_hwndTab, 0 );
	}

	// 2005.09.01 ryoji �^�u�̃X�N���[���ʒu�𕜌�
	if( hwndUpDown )
	{
		hwndUpDown = ::FindWindowEx( m_hwndTab, NULL, UPDOWN_CLASS, 0 );
		if( hwndUpDown != NULL )
		{
			::SendMessage( hwndUpDown, UDM_SETPOS, (WPARAM)0, MAKELPARAM( LOWORD( nScrollPos ), 0 ) );					// Up-Down �R���g���[���Ɉʒu��ݒ�
			nScrollPos = LOWORD( ::SendMessage( hwndUpDown, UDM_GETPOS, (WPARAM)0, (LPARAM)0 ) );							// ���ۂɐݒ肳�ꂽ�ʒu���擾
			::SendMessage( m_hwndTab, WM_HSCROLL, MAKEWPARAM( SB_THUMBPOSITION, LOWORD( nScrollPos ) ), (LPARAM)NULL );	// �ݒ�ʒu�Ƀ^�u���X�N���[��
		}
	}

	return;
}


/*!	�ҏW�E�B���h�E�̈ʒu���킹

	@author ryoji
	@date 2007.04.03 �V�K�쐬
*/
void CTabWnd::AdjustWindowPlacement( void )
{
	// �^�u�܂Ƃߕ\���̏ꍇ�͕ҏW�E�B���h�E�̕\���ʒu�𕜌�����
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		HWND hwnd = m_hwndParent;	// ���g�̕ҏW�E�B���h�E
		WINDOWPLACEMENT wp = m_pShareData->m_TabWndWndpl;
		if( wp.length == sizeof( WINDOWPLACEMENT ) && !::IsWindowVisible( hwnd ) )	// ��������Ƃ����������p��
		{
			// �Ȃ�ׂ���ʂ���O�ɏo�����ɉ�������
			// Note. ��A�N�e�B�u�X���b�h������s����̂ł���΃A�N�e�B�u���w��ł���O�ɂ͏o�Ȃ�
			// Note. SW_xxxxx �̒��ɂ́u�A�N�e�B�u�������̍ő剻�v�w��͑��݂��Ȃ�
			// Note. �s���̏�Ԃ��炢���Ȃ��O�ɏo�Ă��܂��Ǝ��̂悤�Ȍ��ۂ��N����
			//  �E��ʕ`�悳���ہA�N���C�A���g�̈�S�̂��ꎞ�I�ɐ^�����ɂȂ�iVista Aero�j
			//  �E�ő剻�ؑցiSW_SHOWMAXIMIZED�j�̍ہA�ȑO�ɒʏ�\����������ʂ̃X�e�[�^�X�o�[��t�@���N�V�����L�[���ꎞ�I�ɒʏ�T�C�Y�ŕ\�������

			// �E�B���h�E��w��ɔz�u����
			int nIdx = GetFirstOpenedWindow();
			if( nIdx >= 0 )
			{
				// Note. WS_EX_TOPMOST �ɂ��Ă� hwndInsertAfter �E�B���h�E�̏�Ԃ������p�����
				HWND hwndInsertAfter = m_pShareData->m_pEditArr[nIdx].m_hWnd;
				::SetWindowPos( hwnd, hwndInsertAfter, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );
			}

			if( wp.showCmd == SW_SHOWMAXIMIZED && ::IsZoomed( hwnd ) )
			{
				WINDOWPLACEMENT wpCur;
				wpCur.length = sizeof( WINDOWPLACEMENT );
				::GetWindowPlacement( hwnd, &wpCur );
				if( !::EqualRect( &wp.rcNormalPosition, &wpCur.rcNormalPosition ) )
				{
					// �E�B���h�E�̒ʏ�T�C�Y���ړI�̃T�C�Y�ƈ���Ă���Ƃ��͈�U�ʏ�T�C�Y�ŕ\�����Ă���ő剻����
					// Note. �}���`���j�^�ňȑO�ɕʃ��j�^�ōő剻����Ă�����ʂ͈�U�ʏ�T�C�Y�ɖ߂��Ă����Ȃ��ƌ��̕ʃ��j�^���ɕ\������Ă��܂�
					wp.showCmd = SW_SHOWNOACTIVATE;
					::SetWindowPlacement( hwnd, &wp );	// �ʏ�T�C�Y�\��
					wp.showCmd = SW_SHOWMAXIMIZED;
				}
				else
				{
					wp.showCmd = SW_SHOWNA;
				}
			}
			else if( wp.showCmd != SW_SHOWMAXIMIZED )
			{
				wp.showCmd = SW_SHOWNOACTIVATE;
			}
			::SetWindowPlacement( hwnd, &wp );	// �ʒu�𕜌�����
			::UpdateWindow( hwnd );	// �����`��
		}
	}
}

void CTabWnd::ShowHideWindow( HWND hwnd, BOOL bDisp )
{
	if( NULL == hwnd ) return;

	if( bDisp )
	{
		if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
		{
			if( m_pShareData->m_bEditWndChanging )
				return;	// �ؑւ̍Œ�(busy)�͗v���𖳎�����
			m_pShareData->m_bEditWndChanging = TRUE;	// �ҏW�E�B���h�E�ؑ֒�ON	2007.04.03 ryoji

			// �ΏۃE�B���h�E�̃X���b�h�Ɉʒu���킹���˗�����	// 2007.04.03 ryoji
			DWORD dwResult;
			::SendMessageTimeout( hwnd, MYWM_TAB_WINDOW_NOTIFY, TWNT_WNDPL_ADJUST, (LPARAM)NULL,
				SMTO_ABORTIFHUNG | SMTO_BLOCK, 10000, &dwResult );
		}

		TabWnd_ActivateFrameWindow( hwnd );

		m_pShareData->m_bEditWndChanging = FALSE;	// �ҏW�E�B���h�E�ؑ֒�OFF	2007.04.03 ryoji
	}
	else
	{
		if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
		{
			::ShowWindow( hwnd, SW_HIDE );
		}
	}

	return;
}

/*!	���̕ҏW�E�B���h�E���B��

	@param hwndExclude [in] ��\�������珜�O����E�B���h�E

	@author ryoji
	@date 2007.05.17 �V�K�쐬
*/
void CTabWnd::HideOtherWindows( HWND hwndExclude )
{
	if( m_pShareData->m_Common.m_bDispTabWnd && !m_pShareData->m_Common.m_bDispTabWndMultiWin )
	{
		HWND hwnd;
		int	i;
		for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
		{
			hwnd = m_pShareData->m_pEditArr[i].m_hWnd;
			if( CShareData::getInstance()->IsEditWnd( hwnd ) )
			{
				if( hwnd != hwndExclude && ::IsWindowVisible( hwnd ) )
				{
					::ShowWindow( hwnd, SW_HIDE );
				}
			}
		}
	}
}

int CTabWnd::GetFirstOpenedWindow( void )
{
	int	i;

	for( i = 0; i < m_pShareData->m_nEditArrNum; i++ )
	{
		if( CShareData::getInstance()->IsEditWnd( m_pShareData->m_pEditArr[i].m_hWnd ) )
		{
			return i;
		}
	}

	return -1;
}

/*! �E�C���h�E�������I�ɑO�ʂɎ����Ă��� */
void CTabWnd::ForceActiveWindow( HWND hwnd )
{
	int		nId1;
	int		nId2;
	DWORD	dwTime;

	nId2 = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL );
	nId1 = ::GetWindowThreadProcessId( hwnd, NULL );

	::AttachThreadInput( nId1, nId2, TRUE );

	::SystemParametersInfo( SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &dwTime, 0 );
	::SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)0, 0 );

	//�E�B���h�E���t�H�A�O���E���h�ɂ���
	::SetForegroundWindow( hwnd );
	::BringWindowToTop( hwnd );

	::SystemParametersInfo( SPI_SETFOREGROUNDLOCKTIMEOUT, 0, (LPVOID)dwTime, 0 );

	::AttachThreadInput( nId1, nId2, FALSE );
}

/*!	�A�N�e�B�u�ɂ���

	@param hwnd [in] �ΏۃE�B���h�E�̃E�B���h�E�n���h��
	@param bForeground [in] true: active and forground / false: active

	@date 2004.08.27 Kazika ����bForeground�ǉ��BbForeground��false�̏ꍇ�̓E�B���h�E���t�H�A�O���E���h�ɂ��Ȃ��B
	@date 2005.11.05 ryoji Grep�_�C�A���O���t�H�[�J�X������Ȃ��悤�ɂ��邽�߁C
		�ΏۃE�B���h�E�̃v���Z�X�����Ƀt�H�A�O���E���h�Ȃ牽�����Ȃ��悤�ɂ���D
*/
void CTabWnd::TabWnd_ActivateFrameWindow( HWND hwnd, bool bForeground )
{
	if ( bForeground )
	{
		// 2005.11.05 ryoji �ΏۃE�B���h�E�̃v���Z�X�����Ƀt�H�A�O���E���h�Ȃ�ؑւ��ς݂Ȃ̂ŉ������Ȃ��ł���
		DWORD dwPid1, dwPid2;
		::GetWindowThreadProcessId( hwnd, &dwPid1 );
		::GetWindowThreadProcessId( ::GetForegroundWindow(), &dwPid2 );
		if( dwPid1 == dwPid2 ){
			return;
		}

		if( ::IsIconic( hwnd ) )
		{
			::ShowWindow( hwnd, SW_RESTORE );	// Nov. 7. 2003 MIK �A�C�R�����͌��̃T�C�Y�ɖ߂�
			return;
		}
		else if( ::IsZoomed( hwnd ) )
		{
			::ShowWindow( hwnd, SW_MAXIMIZE );
		}
		else
		{
			::ShowWindow( hwnd, SW_SHOW );
		}

		::SetForegroundWindow( hwnd );
		::BringWindowToTop( hwnd );
	}
	else
	{
		// 2005.09.01 ryoji ::ShowWindow( hwnd, SW_SHOWNA ) ���Ɣ�\������\���ɐ؂�ւ��Ƃ��� Z-order �����������Ȃ邱�Ƃ�����̂� ::SetWindowPos �ɕύX
		::SetWindowPos( hwnd, NULL,0,0,0,0,
						SWP_SHOWWINDOW | SWP_NOACTIVATE
						| SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER );
	}

	return;
}

/*! �^�u�̃��C�A�E�g��������
	@date 2006.01.28 ryoji �V�K�쐬
*/
void CTabWnd::LayoutTab( void )
{
	// �A�C�R���̕\����؂�ւ���
	HIMAGELIST hImg = TabCtrl_GetImageList( m_hwndTab );
	if( NULL == hImg && m_pShareData->m_Common.m_bDispTabIcon )
	{
		if( NULL != InitImageList() )
			Refresh();
	}
	else if( NULL != hImg && !m_pShareData->m_Common.m_bDispTabIcon )
	{
		InitImageList();
	}

	// �^�u�̃A�C�e�����̓�����؂�ւ���
	UINT lStyle;
	lStyle = (UINT)::GetWindowLongPtr( m_hwndTab, GWL_STYLE );
	if( (lStyle & TCS_FIXEDWIDTH) && !m_pShareData->m_Common.m_bSameTabWidth )
	{
		lStyle &= ~(TCS_FIXEDWIDTH | TCS_FORCELABELLEFT);
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
		return;
	}
	else if( !(lStyle & TCS_FIXEDWIDTH) && m_pShareData->m_Common.m_bSameTabWidth )
	{
		lStyle |= TCS_FIXEDWIDTH | TCS_FORCELABELLEFT;
		::SetWindowLongPtr( m_hwndTab, GWL_STYLE, lStyle );
	}

	if( !m_pShareData->m_Common.m_bSameTabWidth )
		return;	// �A�C�e�����̒����͕s�v

	// �^�u�̃A�C�e�����𒲐�����
	RECT rcTab;
	int nCount;
	int cx;

	::GetClientRect( m_hwndTab, &rcTab );
	nCount = TabCtrl_GetItemCount( m_hwndTab );
	if( 0 < nCount )
	{
		cx = (rcTab.right - rcTab.left - 8) / nCount;
		if( MAX_TABITEM_WIDTH < cx )
			cx = MAX_TABITEM_WIDTH;
		else if( MIN_TABITEM_WIDTH > cx )
			cx = MIN_TABITEM_WIDTH;
		TabCtrl_SetItemSize( m_hwndTab, cx, TAB_ITEM_HEIGHT );
	}
}

/*! �C���[�W���X�g�̏���������
	@date 2006.02.22 ryoji �V�K�쐬
*/
HIMAGELIST CTabWnd::InitImageList( void )
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	hImlNew = NULL;
	if( m_pShareData->m_Common.m_bDispTabIcon )
	{
		// �V�X�e���C���[�W���X�g���擾����
		// ���F������ɍ����ւ��ė��p����A�C�R���ɂ͎��O�ɃA�N�Z�X���Ă����Ȃ��ƃC���[�W������Ȃ�
		//     �����ł́u�t�H���_������A�C�R���v�A�u�t�H���_���J�����A�C�R���v�������ւ��p�Ƃ��ė��p
		//     WinNT4.0 �ł� SHGetFileInfo() �̑������ɓ������w�肷��Ɠ����C���f�b�N�X��Ԃ��Ă��邱�Ƃ�����H

		hImlSys = (HIMAGELIST)::SHGetFileInfo( _T(".0"), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );
		if( NULL == hImlSys )
			goto l_end;
		m_iIconApp = sfi.iIcon;

		hImlSys = (HIMAGELIST)::SHGetFileInfo( _T(".1"), FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES | SHGFI_OPENICON );
		if( NULL == hImlSys )
			goto l_end;
		m_iIconGrep = sfi.iIcon;

		// �V�X�e���C���[�W���X�g�𕡐�����
		hImlNew = ImageList_Duplicate( hImlSys );
		if( NULL == hImlNew )
			goto l_end;
		ImageList_SetBkColor( hImlNew, CLR_NONE );

		// �C���[�W���X�g�ɃA�v���P�[�V�����A�C�R���� Grep�A�C�R����o�^����
		// �i���p���Ȃ��A�C�R���ƍ����ւ���j
		m_hIconApp = GetAppIcon( m_hInstance, ICON_DEFAULT_APP, FN_APP_ICON, true );
		ImageList_ReplaceIcon( hImlNew, m_iIconApp, m_hIconApp );
		m_hIconGrep = GetAppIcon( m_hInstance, ICON_DEFAULT_GREP, FN_GREP_ICON, true );
		ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );
	}

l_end:
	// �^�u�ɐV�����A�C�R���C���[�W��ݒ肷��
	TabCtrl_SetImageList( m_hwndTab, hImlNew );

	// �V�����C���[�W���X�g���L������
	if( NULL != m_hIml )
		ImageList_Destroy( m_hIml );
	m_hIml = hImlNew;

	return m_hIml;	// �V�����C���[�W���X�g��Ԃ�
}

/*! �C���[�W���X�g�̃C���f�b�N�X�擾����
	@date 2006.01.28 ryoji �V�K�쐬
*/
int CTabWnd::GetImageIndex( EditNode* pNode )
{
	SHFILEINFO sfi;
	HIMAGELIST hImlSys;
	HIMAGELIST hImlNew;

	if( NULL == m_hIml )
		return -1;	// �C���[�W���X�g���g���Ă��Ȃ�

	if( pNode )
	{
		if( pNode->m_szFilePath[0] )
		{
			// �g���q�����o��
			TCHAR szExt[_MAX_EXT];
			_tsplitpath( pNode->m_szFilePath, NULL, NULL, NULL, szExt );

			// �g���q�Ɋ֘A�t����ꂽ�A�C�R���C���[�W�̃C���f�b�N�X���擾����
			hImlSys = (HIMAGELIST)::SHGetFileInfo( szExt, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES );
			if( NULL == hImlSys )
				return -1;
			if( ImageList_GetImageCount( m_hIml ) > sfi.iIcon )
				return sfi.iIcon;	// �C���f�b�N�X��Ԃ�

			// �V�X�e���C���[�W���X�g�𕡐�����
			hImlNew = ImageList_Duplicate( hImlSys );
			if( NULL == hImlNew )
				return -1;
			ImageList_SetBkColor( hImlNew, CLR_NONE );

			// �C���[�W���X�g�ɃA�v���P�[�V�����A�C�R���� Grep�A�C�R����o�^����
			// �i���p���Ȃ��A�C�R���ƍ����ւ���j
			ImageList_ReplaceIcon( hImlNew, m_iIconApp, m_hIconApp );
			ImageList_ReplaceIcon( hImlNew, m_iIconGrep, m_hIconGrep );

			// �^�u�ɃA�C�R���C���[�W��ݒ肷��
			if( m_pShareData->m_Common.m_bDispTabIcon )
				TabCtrl_SetImageList( m_hwndTab, hImlNew );

			// �V�����C���[�W���X�g���L������
			ImageList_Destroy( m_hIml );
			m_hIml = hImlNew;

			return sfi.iIcon;	// �C���f�b�N�X��Ԃ�
		}
		else if( pNode->m_bIsGrep )
			return m_iIconGrep;	// grep�A�C�R���̃C���f�b�N�X��Ԃ�
	}

	return m_iIconApp;	// �A�v���P�[�V�����A�C�R���̃C���f�b�N�X��Ԃ�
}

/*! �C���[�W���X�g�̕�������
	@date 2006.02.17 ryoji �V�K�쐬
*/
HIMAGELIST CTabWnd::ImageList_Duplicate( HIMAGELIST himl )
{
	// �{���� ImageList_Duplicate() ������΂�����Ăяo��
	HIMAGELIST hImlNew;
	if( m_RealImageList_Duplicate )
	{
		hImlNew = m_RealImageList_Duplicate( himl );
		if( NULL != hImlNew )
			return hImlNew;
		m_RealImageList_Duplicate = NULL;	// 2006.06.20 ryoji ���s���͑�֏����ɐ؂�ւ�
	}

	// �{���� ImageList_Duplicate() �̑�֏���
	// �V�����C���[�W���X�g���쐬���ăA�C�R���P�ʂŃR�s�[����
	//�i���̏ꍇ�A���F�A�C�R�����Y��ɂ͕\������Ȃ���������Ȃ��j
	hImlNew = ImageList_Create( CX_SMICON, CY_SMICON, ILC_COLOR32 | ILC_MASK, 4, 4 );
	if( hImlNew )
	{
		ImageList_SetBkColor( hImlNew, CLR_NONE );
		int nCount = ImageList_GetImageCount( himl );
		int i;
		for( i = 0; i < nCount; i++ )
		{
			HICON hIcon = ImageList_GetIcon( himl, i, ILD_TRANSPARENT );
			if( NULL == hIcon )
			{
				ImageList_Destroy( hImlNew );
				return NULL;
			}
			int iIcon = ImageList_AddIcon( hImlNew, hIcon );
			::DestroyIcon( hIcon );
			if( 0 > iIcon )
			{
				ImageList_Destroy( hImlNew );
				return NULL;
			}
		}
	}
	return hImlNew;
}

/*! �{�^���w�i�`�揈��
	@date 2006.10.21 ryoji �V�K�쐬
*/
void CTabWnd::DrawBtnBkgnd( HDC hdc, const LPRECT lprcBtn, BOOL bBtnHilighted )
{
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	if( bBtnHilighted )
	{
		hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_HIGHLIGHT ) );
		hbr = (HBRUSH)::GetSysColorBrush( COLOR_MENU );
		hpenOld = (HPEN)::SelectObject( hdc, hpen );
		hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
		::Rectangle( hdc, lprcBtn->left, lprcBtn->top, lprcBtn->right, lprcBtn->bottom );
		::SelectObject( hdc, hpenOld );
		::SelectObject( hdc, hbrOld );
		::DeleteObject( hpen );
	}
}

/*! �ꗗ�{�^���`�揈��
	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.10.21 ryoji �w�i�`����֐��Ăяo���ɕύX
*/
void CTabWnd::DrawListBtn( HDC hdc, const LPRECT lprcClient )
{
	const POINT ptBase[4] = { {4, 8}, {7, 11}, {8, 11}, {11, 8} };	// �`��C���[�W�`��
	POINT pt[4];
	int i;
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	RECT rcBtn;
	GetListBtnRect( lprcClient, &rcBtn );
	DrawBtnBkgnd( hdc, &rcBtn, m_bListBtnHilighted );	// 2006.10.21 ryoji

	int nIndex = m_bListBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( nIndex ) );
	hbr = (HBRUSH)::GetSysColorBrush( nIndex );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
	for( i = 0; i < sizeof(ptBase)/sizeof(ptBase[0]); i++ )
	{
		pt[i].x = ptBase[i].x + rcBtn.left;
		pt[i].y = ptBase[i].y + rcBtn.top;
	}
	::Polygon( hdc, pt, sizeof(pt)/sizeof(pt[0]) );
	::SelectObject( hdc, hpenOld );
	::SelectObject( hdc, hbrOld );
	::DeleteObject( hpen );
}

/*! ����{�^���`�揈��
	@date 2006.10.21 ryoji �V�K�쐬
*/
void CTabWnd::DrawCloseBtn( HDC hdc, const LPRECT lprcClient )
{
	const POINT ptBase1[6][2] =	// [x]�`��C���[�W�`��i����6�{�j
	{
		{{4, 5}, {12, 13}},
		{{4, 4}, {13, 13}},
		{{5, 4}, {13, 12}},
		{{11, 4}, {3, 12}},
		{{12, 4}, {3, 13}},
		{{12, 5}, {4, 13}}
	};
	const POINT ptBase2[10][2] = // [xx]�`��C���[�W�`��i��`10�j
	{
		{{3, 4}, {5, 6}},
		{{6, 4}, {8, 6}},
		{{4, 6}, {7, 10}},
		{{3, 10}, {5, 12}},
		{{6, 10}, {8, 12}},
		{{9, 4}, {11, 6}},
		{{12, 4}, {14, 6}},
		{{10, 6}, {13, 10}},
		{{9, 10}, {11, 12}},
		{{12, 10}, {14, 12}}
	};

	POINT pt[2];
	int i;
	HPEN hpen, hpenOld;
	HBRUSH hbr, hbrOld;

	RECT rcBtn;
	GetCloseBtnRect( lprcClient, &rcBtn );

	// �{�^���̍����ɃZ�p���[�^��`�悷��	// 2007.02.27 ryoji
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DSHADOW ) );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	::MoveToEx( hdc, rcBtn.left - 4, rcBtn.top + 1, NULL );
	::LineTo( hdc, rcBtn.left - 4, rcBtn.bottom - 1 );
	::SelectObject( hdc, hpenOld );
	::DeleteObject( hpen );

	DrawBtnBkgnd( hdc, &rcBtn, m_bCloseBtnHilighted );

	int nIndex = m_bCloseBtnHilighted? COLOR_MENUTEXT: COLOR_BTNTEXT;
	hpen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( nIndex ) );
	hbr = (HBRUSH)::GetSysColorBrush( nIndex );
	hpenOld = (HPEN)::SelectObject( hdc, hpen );
	hbrOld = (HBRUSH)::SelectObject( hdc, hbr );
	if( m_pShareData->m_Common.m_bDispTabWnd &&
		!m_pShareData->m_Common.m_bDispTabWndMultiWin &&
		!m_pShareData->m_Common.m_bTab_CloseOneWin			// 2007.02.13 ryoji �����ǉ��i�E�B���h�E�̕���{�^���͑S������j
		)
	{
		// [x]��`��i����6�{�j
		for( i = 0; i < sizeof(ptBase1)/sizeof(ptBase1[0]); i++ )
		{
			pt[0].x = ptBase1[i][0].x + rcBtn.left;
			pt[0].y = ptBase1[i][0].y + rcBtn.top;
			pt[1].x = ptBase1[i][1].x + rcBtn.left;
			pt[1].y = ptBase1[i][1].y + rcBtn.top;
			::MoveToEx( hdc, pt[0].x, pt[0].y, NULL );
			::LineTo( hdc, pt[1].x, pt[1].y );
		}
	}
	else
	{
		 // [xx]��`��i��`10�j
		for( i = 0; i < sizeof(ptBase2)/sizeof(ptBase2[0]); i++ )
		{
			pt[0].x = ptBase2[i][0].x + rcBtn.left;
			pt[0].y = ptBase2[i][0].y + rcBtn.top;
			pt[1].x = ptBase2[i][1].x + rcBtn.left;
			pt[1].y = ptBase2[i][1].y + rcBtn.top;
			::Rectangle( hdc, pt[0].x, pt[0].y, pt[1].x, pt[1].y );
		}
	}
	::SelectObject( hdc, hpenOld );
	::SelectObject( hdc, hbrOld );
	::DeleteObject( hpen );
}

/*! �ꗗ�{�^���̋�`�擾����
	@date 2006.02.01 ryoji �V�K�쐬
*/
void CTabWnd::GetListBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT + 4, lprcClient->top + TAB_MARGIN_TOP + 2 );
}

/*! ����{�^���̋�`�擾����
	@date 2006.10.21 ryoji �V�K�쐬
*/
void CTabWnd::GetCloseBtnRect( const LPRECT lprcClient, LPRECT lprc )
{
	*lprc = rcBtnBase;
	::OffsetRect(lprc, lprcClient->right - TAB_MARGIN_RIGHT + 4 + (rcBtnBase.right - rcBtnBase.left) + 7, lprcClient->top + TAB_MARGIN_TOP + 2 );
}


/*!	�^�u�ꗗ�\������

	@param pt [in] �\���ʒu
	@param bSel [in] �\���ؑփ��j���[��ǉ�����
	@param bFull [in] �p�X���ŕ\������ibSel��TRUE�̏ꍇ�͖����j

	@date 2006.02.01 ryoji �V�K�쐬
	@date 2006.03.23 fon OnListBtnClick����ړ�(�s����//>���ύX��)
	@date 2006.10.31 ryoji ���j���[�̃t���p�X�����ȈՕ\������
	@date 2007.02.28 ryoji �^�u���ꗗ�^�p�X���ꗗ�̕\�������j���[���g�Ő؂�ւ���
*/
LRESULT CTabWnd::TabListMenu( POINT pt, BOOL bSel/* = TRUE*/, BOOL bFull/* = FALSE*/ )
{
	bool bRepeat;

	if( bSel )
		bFull = m_pShareData->m_Common.m_bTab_ListFull;

	do
	{
		int nCount = TabCtrl_GetItemCount( m_hwndTab );
		if( 0 >= nCount )
			return 0L;

		// �e�^�u�A�C�e�����烁�j���[�ɕ\�����镶�����擾����
		TABMENU_DATA* pData = new TABMENU_DATA[nCount];
		CRecent	cRecentEditNode;
		cRecentEditNode.EasyCreate( RECENT_FOR_EDITNODE );
		TCITEM tcitem;
		int i;
		for( i = 0; i < nCount; i++ )
		{
			tcitem.mask = TCIF_PARAM | TCIF_TEXT | TCIF_IMAGE;
			tcitem.lParam = (LPARAM)0;
			tcitem.pszText = pData[i].szText;
			tcitem.cchTextMax = sizeof(pData[i].szText);
			TabCtrl_GetItem( m_hwndTab, i, &tcitem );
			pData[i].szText[sizeof(pData[i].szText) - 1] = _T('\0');	// �o�b�t�@�s���̏ꍇ�ւ̑΍�
			pData[i].iItem = i;
			pData[i].iImage = tcitem.iImage;

			// �p�X���w��Ȃ烁�j���[�ɕ\�����镶������t���p�X���ɒu��������
			if( bFull )
			{
				EditNode *pNode = (EditNode*)cRecentEditNode.GetItem( cRecentEditNode.FindItem( (const char*)&tcitem.lParam ) );
				if( pNode && pNode->m_szFilePath[0] )
				{
					// �t���p�X�����ȈՕ\������	// 2006.10.31 ryoji
					TCHAR szText[_MAX_PATH];
					TCHAR szText_amp[_MAX_PATH * 2];
					CShareData::getInstance()->GetTransformFileNameFast( pNode->m_szFilePath, szText, _MAX_PATH );
					dupamp( szText, szText_amp );	// &��&&�ɒu��������
					::lstrcpyn( pData[i].szText, szText_amp, sizeof(pData[i].szText) );
				}
			}
		}
		cRecentEditNode.Terminate();

		// �\�������Ń\�[�g����
		if(m_pShareData->m_Common.m_bSortTabList){							// 2006.03.23 fon �ύX
			qsort( pData, nCount, sizeof(pData[0]), compTABMENU_DATA );
		}

		// ���j���[���쐬����
		// 2007.02.28 ryoji �\���ؑւ����j���[�ɒǉ�
		int iTabSel = TabCtrl_GetCurSel( m_hwndTab );
		int iMenuSel = -1;
		UINT uFlags = MF_BYPOSITION | (m_hIml? MF_OWNERDRAW:  MF_STRING);
		HMENU hMenu = ::CreatePopupMenu();
		int iAdd = 0;
		if( bSel )	// �\���ؑփ��j���[��ǉ�����
		{
			::InsertMenu( hMenu, 0, MF_BYPOSITION | MF_STRING, 999, bFull? _T("�^�u���ꗗ�ɐؑւ���(&W)"): _T("�p�X���ꗗ�ɐؑւ���(&W)") );
			::InsertMenu( hMenu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
			iAdd = 2;
		}
		for( i = 0; i < nCount; i++ )
		{
			::InsertMenu( hMenu, i + iAdd, uFlags, pData[i].iItem + 100, m_hIml? (LPCTSTR)&pData[i]: pData[i].szText );
			if( pData[i].iItem == iTabSel )
				iMenuSel = i + iAdd;
		}

		// �I���^�u�ɑΉ����郁�j���[���`�F�b�N��Ԃɂ���
		if( iAdd <= iMenuSel && iMenuSel < nCount + iAdd )
			::CheckMenuRadioItem( hMenu, iAdd, nCount + iAdd - 1, iMenuSel, MF_BYPOSITION );

		// ���j���[��\������
		// 2006.04.21 ryoji �}���`���j�^�Ή��̏C��
		RECT rcWork;
		GetMonitorWorkRect( pt, &rcWork );	// ���j�^�̃��[�N�G���A
		int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
									( pt.x > rcWork.left )? pt.x: rcWork.left,
									( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
									0, m_hWnd, NULL);
		::DestroyMenu( hMenu );

		// ���j���[�I�����ꂽ�^�u�̃E�C���h�E���A�N�e�B�u�ɂ���
		bRepeat = false;
		if( 999 == nId )	// �\���ؑ�
		{
			bFull = !bFull;
			bRepeat = true;
		}
		else if( 100 <= nId )
		{
			tcitem.mask   = TCIF_PARAM;
			tcitem.lParam = (LPARAM)0;
			TabCtrl_GetItem( m_hwndTab, nId - 100, &tcitem );

			ShowHideWindow( (HWND)tcitem.lParam, TRUE );
		}

		delete []pData;

	} while( bRepeat );

	if( bSel )
		m_pShareData->m_Common.m_bTab_ListFull = bFull;

	return 0L;
}


/*[EOF]*/
