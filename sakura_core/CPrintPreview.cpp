/*!	@file
	@brief ����v���r���[�Ǘ��N���X

	@author YAZAKI
	@date 2002/1/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2002, YAZAKI, aroka, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CPrintPreview.h"
#include "CLayout.h"
#include "CEditWnd.h"
#include "charcode.h"
#include "CDlgPrintPage.h"
#include "CDlgCancel.h"/// 2002/2/3 aroka from here
#include "Debug.h"///
#include "etc_uty.h"///
#include <stdio.h>/// 2002/2/3 aroka to here

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		/* ���������̂P��̃X�N���[���� */
#define		LINE_RANGE_Y	24		/* ���������̂P��̃X�N���[���� */

#define		PAGE_RANGE_X	160		/* ���������̂P��̃y�[�W�X�N���[���� */
#define		PAGE_RANGE_Y	160		/* ���������̂P��̃y�[�W�X�N���[���� */

CPrint CPrintPreview::m_cPrint;		//!< ���݂̃v�����^��� 2003.05.02 �����

/*! �R���X�g���N�^
	����v���r���[��\�����邽�߂ɕK�v�ȏ����������A�̈�m�ہB
	�R���g���[�����쐬����B
*/
CPrintPreview::CPrintPreview(CEditWnd* pParentWnd ) :
	m_nPreview_Zoom( 100 ),			/* ����v���r���[�{�� */
	m_nPreviewVScrollPos( 0 ),
	m_nPreviewHScrollPos( 0 ),
	m_nCurPageNum( 0 ),				/* ���݂̃y�[�W */
	m_pParentWnd( pParentWnd )
{
	/* ����p�̃��C�A�E�g���̍쐬 */
	m_pLayoutMgr_Print = new CLayoutMgr;

	/* ����v���r���[ �R���g���[�� �쐬 */
	CreatePrintPreviewControls();
}

CPrintPreview::~CPrintPreview()
{
	/* ����v���r���[ �R���g���[�� �j�� */
	DestroyPrintPreviewControls();
	
	/* ����p�̃��C�A�E�g���̍폜 */
	delete m_pLayoutMgr_Print;
}

/*!	����v���r���[���́AWM_PAINT������
*/
LRESULT CPrintPreview::OnPaint(
	HWND			hwnd,	// handle of window
	UINT			uMsg,	// message identifier
	WPARAM			wParam,	// first message parameter
	LPARAM			lParam 	// second message parameter
)
{
	PAINTSTRUCT		ps;
	HDC				hdcOld = ::BeginPaint( hwnd, &ps );
	HDC				hdc = m_pParentWnd->m_hdcCompatDC;	//	�e�E�B���h�E��ComatibleDC�ɕ`��

	/* ����v���r���[ ����o�[ */
	RECT			rc;
	::GetClientRect( hwnd, &rc );
	::FillRect( hdc, &rc, (HBRUSH)::GetStockObject( GRAY_BRUSH ) );

	int nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}

	/* �v�����^���̕\�� */
	char	szText[1024];
	char	szPaperName[256];
	::SetDlgItemText( m_hwndPrintPreviewBar, IDC_STATIC_PRNDEV, m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName );
	m_cPrint.GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , (char*)szPaperName );
	wsprintf( szText, "%s  %s",
		szPaperName,
		(m_pPrintSetting->m_mdmDevMode.dmOrientation & DMORIENT_LANDSCAPE) ? "��" : "�c"
	);
	::SetDlgItemText( m_hwndPrintPreviewBar, IDC_STATIC_PAPER, szText );

	/* �o�b�N�O���E���h ���[�h��ύX */
	::SetBkMode( hdc, TRANSPARENT );

	/* �}�b�s���O���[�h�̕ύX */
	int nMapModeOld =
	::SetMapMode( hdc, MM_LOMETRIC );
	::SetMapMode( hdc, MM_ANISOTROPIC );

	/* �o�͔{���̕ύX */
	SIZE			sz;
	::GetWindowExtEx( hdc, &sz );
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)( ((long)nCx) * 100L / ((long)m_nPreview_Zoom) );
	nCy = (int)( ((long)nCy) * 100L / ((long)m_nPreview_Zoom) );
	::SetWindowExtEx( hdc, nCx, nCy, &sz );

	/* ����t�H���g�쐬 & �ݒ� */
	m_lfPreviewHan.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	strcpy( m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan );

	m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	strcpy( m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );

	/* ����p���p�t�H���g�ƑS�p�t�H���g�𐶐� */
	HFONT	hFontHan = CreateFontIndirect( &m_lfPreviewHan );
	HFONT	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	/* ����p���p�t�H���g�ɐݒ肵�A�ȑO�̃t�H���g��ێ� */
	HFONT	hFontOld = (HFONT)::SelectObject( hdc, hFontHan );

	/* ����E�B���h�E�̉��ɕ������W���_���ړ� */
	POINT			poViewPortOld;
	::SetViewportOrgEx( hdc, -1 * m_nPreviewHScrollPos, nToolBarHeight + m_nPreviewVScrollPos, &poViewPortOld );

	/* �p���̕`�� */
	int	nDirectY = -1;	//	Y���W�̉����v���X�����ɂ��邽�߁H
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft,
		nDirectY * ( m_nPreview_ViewMarginTop ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth + 1,
		nDirectY * (m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight + 1 )
	);
	/* �}�[�W���g�̕\�� */
	HPEN			hPen, hPenOld;
	hPen = ::CreatePen( PS_SOLID, 0, RGB(127,127,127) );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		nDirectY * ( m_nPreview_ViewMarginTop + m_pPrintSetting->m_nPrintMarginTY ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginRX + 1,
		nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginBY )
	);
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

	::SetTextColor( hdc, RGB( 0, 0, 0 ) );

	RECT cRect;	/* ���̑傫��������킷RECT */
	cRect.left   = m_nPreview_ViewMarginLeft +                             m_pPrintSetting->m_nPrintMarginLX + 5;
	cRect.right  = m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + 5);
	cRect.top    = nDirectY * ( m_nPreview_ViewMarginTop +                              m_pPrintSetting->m_nPrintMarginTY + 5);
	cRect.bottom = nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + 5));

	/* �w�b�_ */
	DrawHeader( hdc, cRect, hFontZen );

	/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
	DrawPageText(
		hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		m_nPreview_ViewMarginTop  + m_pPrintSetting->m_nPrintMarginTY + 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
		m_nCurPageNum,
		hFontZen,
		NULL
	);

	DrawFooter( hdc, cRect, hFontZen );

	/* ����t�H���g���� & �j�� */
	::SelectObject( hdc, hFontOld );
	::DeleteObject( hFontZen );
	::DeleteObject( hFontHan );

	/* �}�b�s���O���[�h�̕ύX */
	::SetMapMode( hdc, nMapModeOld );

	/* �������W���_�����Ƃɖ߂� */
	::SetViewportOrgEx( hdc, poViewPortOld.x, poViewPortOld.y, NULL );


	/* �������c�b�𗘗p�����ĕ`��̏ꍇ�̓������c�b�ɕ`�悵�����e����ʂփR�s�[���� */
	rc = ps.rcPaint;
	::DPtoLP( hdc, (POINT*)&rc, 2 );
	::BitBlt(
		hdcOld,
		ps.rcPaint.left,
		ps.rcPaint.top,
		ps.rcPaint.right - ps.rcPaint.left,
		ps.rcPaint.bottom - ps.rcPaint.top,
		hdc,
		ps.rcPaint.left,
		ps.rcPaint.top,
		SRCCOPY
	);
	::EndPaint( hwnd, &ps );
	return 0L;
}

LRESULT CPrintPreview::OnSize( WPARAM wParam, LPARAM lParam )
{
	int	cx = LOWORD( lParam );
	int	cy = HIWORD( lParam );

	/* ����v���r���[ ����o�[ */
	int nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		RECT			rc;
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
		::MoveWindow( m_hwndPrintPreviewBar, 0, 0, cx, nToolBarHeight, TRUE );
	}

	/* ����v���r���[ �����X�N���[���o�[�E�B���h�E */
	int	nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int	nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );
	if( NULL != m_hwndVScrollBar ){
		::MoveWindow( m_hwndVScrollBar, cx - nCxVScroll, nToolBarHeight, nCxVScroll, cy - nCyVScroll - nToolBarHeight, TRUE );
	}
	
	/* ����v���r���[ �����X�N���[���o�[�E�B���h�E */
	int	nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int	nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	if( NULL != m_hwndHScrollBar ){
		::MoveWindow( m_hwndHScrollBar, 0, cy - nCyHScroll, cx - nCxVScroll, nCyHScroll, TRUE );
	}
	
	/* ����v���r���[ �T�C�Y�{�b�N�X�E�B���h�E */
	if( NULL != m_hwndSizeBox){
		::MoveWindow( m_hwndSizeBox, cx - nCxVScroll, cy - nCyHScroll, nCxHScroll, nCyVScroll, TRUE );
	}

	HDC			hdc = ::GetDC( m_pParentWnd->m_hWnd );
	::SetMapMode( hdc, MM_LOMETRIC );
	::SetMapMode( hdc, MM_ANISOTROPIC );

	/* �o�͔{���̕ύX */
	SIZE		sz;
	::GetWindowExtEx( hdc, &sz );
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)( ((long)nCx) * 100L / ((long)m_nPreview_Zoom) );
	nCy = (int)( ((long)nCy) * 100L / ((long)m_nPreview_Zoom) );
	::SetWindowExtEx( hdc, nCx, nCy, &sz );

	/* �r���[�̃T�C�Y */
	POINT		po;
	po.x = m_nPreview_PaperAllWidth + m_nPreview_ViewMarginLeft * 2;
	po.y = m_nPreview_PaperAllHeight + m_nPreview_ViewMarginTop * 2;
	::LPtoDP( hdc, &po, 1 );

	/* �ĕ`��p�������a�l�o */
	if( m_pParentWnd->m_hbmpCompatBMP != NULL ){
		::SelectObject( m_pParentWnd->m_hdcCompatDC, m_pParentWnd->m_hbmpCompatBMPOld );	/* �ĕ`��p�������a�l�o(OLD) */
		::DeleteObject( m_pParentWnd->m_hbmpCompatBMP );
	}
	m_pParentWnd->m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, cx, cy );
	m_pParentWnd->m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_pParentWnd->m_hdcCompatDC, m_pParentWnd->m_hbmpCompatBMP );


	::ReleaseDC( m_pParentWnd->m_hWnd, hdc );

	/* ����v���r���[�F�r���[��(�s�N�Z��) */
	m_nPreview_ViewWidth = abs( po.x );
	
	/* ����v���r���[�F�r���[����(�s�N�Z��) */
	m_nPreview_ViewHeight = abs( po.y );
	
	/* ����v���r���[ �X�N���[���o�[������ */
	InitPreviewScrollBar();
	
	/* ����v���r���[ �X�N���[���o�[�̏����� */
	
	m_pParentWnd->m_nDragPosOrgX = 0;
	m_pParentWnd->m_nDragPosOrgY = 0;
	m_pParentWnd->m_bDragMode = TRUE;
	OnMouseMove( 0, MAKELONG( 0, 0 ) );
	m_pParentWnd->m_bDragMode = FALSE;
	//	SizeBox���e�X�g
	if( NULL != m_hwndSizeBox ){
		if( wParam == SIZE_MAXIMIZED ){
			::ShowWindow( m_hwndSizeBox, SW_HIDE );
		}else
		if( wParam == SIZE_RESTORED ){
			if( ::IsZoomed( m_pParentWnd->m_hWnd ) ){
				::ShowWindow( m_hwndSizeBox, SW_HIDE );
			}else{
				::ShowWindow( m_hwndSizeBox, SW_SHOW );
			}
		}else{
			::ShowWindow( m_hwndSizeBox, SW_SHOW );
		}
	}
	::InvalidateRect( m_pParentWnd->m_hWnd, NULL, TRUE );
	return 0L;
}

LRESULT CPrintPreview::OnVScroll( WPARAM wParam, LPARAM lParam )
{
	int			nPreviewVScrollPos;
	SCROLLINFO	si;
	int			nNowPos;
	int			nMove;
	int			nNewPos;
	int			nScrollCode;
	int			nPos;
	HWND		hwndScrollBar;
	nScrollCode = (int) LOWORD(wParam);
	nPos = (int) HIWORD(wParam);
	hwndScrollBar = (HWND) lParam;
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, (SCROLLINFO*)&si );
	nNowPos = ::GetScrollPos( hwndScrollBar, SB_CTL );
	nNewPos = 0;
	nMove = 0;
	switch( nScrollCode ){
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	default:
		return 0;
	}
	nNewPos = nNowPos + nMove;
	if( nNewPos < 0 ){
		nNewPos = 0;
	}else
	if( nNewPos > (int)(si.nMax - si.nPage + 1) ){
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	nPreviewVScrollPos = -1 * nNewPos;
	if( nPreviewVScrollPos != m_nPreviewVScrollPos ){
		::SetScrollPos( hwndScrollBar, SB_CTL, nNewPos, TRUE);
		m_nPreviewVScrollPos = nPreviewVScrollPos;
		/* �`�� */
		::ScrollWindowEx( m_pParentWnd->m_hWnd, 0, nMove, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

LRESULT CPrintPreview::OnHScroll( WPARAM wParam, LPARAM lParam )
{
	int			nPreviewHScrollPos;
	SCROLLINFO	si;
	int			nNowPos;
	int			nMove;
	int			nNewPos;
	int			nScrollCode;
	int			nPos;
	HWND		hwndScrollBar;
	nScrollCode = (int) LOWORD(wParam);
	nPos = (int) HIWORD(wParam);
	hwndScrollBar = (HWND) lParam;
	si.cbSize = sizeof( SCROLLINFO );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, (SCROLLINFO*)&si );
	nNowPos = ::GetScrollPos( hwndScrollBar, SB_CTL );
	nMove = 0;
	switch( nScrollCode ){
	case SB_LINEUP:
		nMove = -1 * LINE_RANGE_Y;
		break;
	case SB_LINEDOWN:
		nMove = LINE_RANGE_Y;
		break;
	case SB_PAGEUP:
		nMove = -1 * PAGE_RANGE_Y;
		break;
	case SB_PAGEDOWN:
		nMove = PAGE_RANGE_Y;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		nMove = nPos - nNowPos;
		break;
	default:
		return 0;
	}
	nNewPos = nNowPos + nMove;
	if( nNewPos < 0 ){
		nNewPos = 0;
	}else
	if( nNewPos > (int)(si.nMax - si.nPage + 1) ){
		nNewPos = (int)(si.nMax - si.nPage + 1);
	}
	nMove = nNowPos - nNewPos;
	nPreviewHScrollPos = nNewPos;
	if( nPreviewHScrollPos != m_nPreviewHScrollPos ){
		::SetScrollPos( hwndScrollBar, SB_CTL, nNewPos, TRUE);
		m_nPreviewHScrollPos = nPreviewHScrollPos;
		/* �`�� */
		::ScrollWindowEx( m_pParentWnd->m_hWnd, nMove, 0, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

LRESULT CPrintPreview::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	/* ��J�[�\�� */
	::SetCursor( ::LoadCursor( m_pParentWnd->m_hInstance, MAKEINTRESOURCE( IDC_CURSOR_HAND ) ) );
	if( !m_pParentWnd->m_bDragMode ){
		return 0;
	}
//	WPARAM		fwKeys = wParam;			// key flags
	int			xPos = LOWORD( lParam );	// horizontal position of cursor
	int			yPos = HIWORD( lParam );	// vertical position of cursor
	RECT		rc;
	GetClientRect( m_pParentWnd->m_hWnd, &rc );
	POINT		po;
	po.x = xPos;
	po.y = yPos;
	if( !PtInRect( &rc, po ) ){	//	�v���r���[�����`�F�b�N�B
		return 0;
	}

	//	Y��
	SCROLLINFO	siV;
	siV.cbSize = sizeof( SCROLLINFO );
	siV.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndVScrollBar, SB_CTL, (SCROLLINFO*)&siV );
	int			nMoveY;
	if( m_SCROLLBAR_VERT ){
		int		nNowPosY = GetScrollPos( m_hwndVScrollBar, SB_CTL );
		nMoveY = m_pParentWnd->m_nDragPosOrgY - yPos;

		int		nNewPosY = nNowPosY + nMoveY;
		if( nNewPosY < 0 ){
			nNewPosY = 0;
		}else
		if( nNewPosY > (int)(siV.nMax - siV.nPage + 1) ){
			nNewPosY = (int)(siV.nMax - siV.nPage + 1);
		}
		nMoveY = nNowPosY - nNewPosY;
		SetScrollPos( m_hwndVScrollBar, SB_CTL, nNewPosY, TRUE );
		m_nPreviewVScrollPos = -1 * nNewPosY;
	}else{
		nMoveY = 0;
	}

	//	X��
	SCROLLINFO	siH;
	siH.cbSize = sizeof( SCROLLINFO );
	siH.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndHScrollBar, SB_CTL, (SCROLLINFO*)&siH );
	int			nMoveX;
	if( m_SCROLLBAR_HORZ ){
		int		nNowPosX = GetScrollPos( m_hwndHScrollBar, SB_CTL );
		nMoveX = m_pParentWnd->m_nDragPosOrgX - xPos;
		
		int		nNewPosX = nNowPosX + nMoveX;
		if( nNewPosX < 0 ){
			nNewPosX = 0;
		}else
		if( nNewPosX > (int)(siH.nMax - siH.nPage + 1) ){
			nNewPosX = (int)(siH.nMax - siH.nPage + 1);
		}
		nMoveX = nNowPosX - nNewPosX;
		SetScrollPos( m_hwndHScrollBar, SB_CTL, nNewPosX, TRUE );
		m_nPreviewHScrollPos = nNewPosX;
	}else{
		nMoveX = 0;
	}

	m_pParentWnd->m_nDragPosOrgX = xPos;
	m_pParentWnd->m_nDragPosOrgY = yPos;
	/* �`�� */
	ScrollWindowEx( m_pParentWnd->m_hWnd, nMoveX, nMoveY, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	return 0;
}

LRESULT CPrintPreview::OnMouseWheel( WPARAM wParam, LPARAM lParam )
{
//	WORD	fwKeys = LOWORD(wParam);			// key flags
	short	zDelta = (short) HIWORD(wParam);	// wheel rotation
//	short	xPos = (short) LOWORD(lParam);		// horizontal position of pointer
//	short	yPos = (short) HIWORD(lParam);		// vertical position of pointer

	int		nScrollCode;
	if( 0 < zDelta ){
		nScrollCode = SB_LINEUP;
	}else{
		nScrollCode = SB_LINEDOWN;
	}

	int		i;
	for( i = 0; i < 3; ++i ){
		/* ����v���r���[ �����X�N���[���o�[���b�Z�[�W���� WM_VSCROLL */
		::PostMessage( m_pParentWnd->m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (LPARAM)m_hwndVScrollBar );

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return -1;
		}
	}
	return 0;
}

void CPrintPreview::OnChangePrintSetting( void )
{
	HDC		hdc = ::GetDC( m_pParentWnd->m_hWnd );
	::SetMapMode( hdc, MM_LOMETRIC ); //MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
	::SetMapMode( hdc, MM_ANISOTROPIC );

	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)CPrintPreview::MyEnumFontFamProc,
		(LPARAM)this
	);

	/* ����v���r���[�\����� */
	m_nPreview_LineNumberColmns = 0;	/* �s�ԍ��G���A�̕�(������) */

	/* �s�ԍ���\�����邩 */
	if( m_pPrintSetting->m_bPrintLineNumber ){
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		m_nPreview_LineNumberColmns = m_pParentWnd->m_cEditDoc.m_cEditViewArr[0].DetectWidthOfLineNumberArea_calculate();
	}
	/* ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������ */
	m_pPrintSetting->m_mdmDevMode.dmPaperSize = m_pPrintSetting->m_nPrintPaperSize;
	m_pPrintSetting->m_mdmDevMode.dmOrientation = m_pPrintSetting->m_nPrintPaperOrientation;
	// �p���T�C�Y�A�p�������͕ύX�����̂Ńr�b�g�𗧂Ă�
	m_pPrintSetting->m_mdmDevMode.dmFields |= ( DM_ORIENTATION | DM_PAPERSIZE );
	// �p���̒����A���͌��܂��Ă��Ȃ��̂ŁA�r�b�g�����낷
	m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERLENGTH );
	m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERWIDTH);

	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	char	szErrMsg[1024];
	if( FALSE == m_cPrint.GetPrintMetrics(
		&m_pPrintSetting->m_mdmDevMode,	/* �v�����^�ݒ� DEVMODE�p*/
		&m_nPreview_PaperAllWidth,		/* �p���� */
		&m_nPreview_PaperAllHeight,		/* �p������ */
		&m_nPreview_PaperWidth,			/* �p������L���� */
		&m_nPreview_PaperHeight,		/* �p������L������ */
		&m_nPreview_PaperOffsetLeft,	/* ����\�ʒu���[ */
		&m_nPreview_PaperOffsetTop,		/* ����\�ʒu��[ */
		szErrMsg						/* �G���[���b�Z�[�W�i�[�ꏊ */
	) ){
		/* �G���[�̏ꍇ�AA4�c(210mm�~297mm)�ŏ����� */
		m_nPreview_PaperAllWidth = 210 * 10;	/* �p���� */
		m_nPreview_PaperAllHeight = 297 * 10;	/* �p������ */
		m_nPreview_PaperWidth = 210 * 10;		/* �p������L���� */
		m_nPreview_PaperHeight = 297 * 10;		/* �p������L������ */
		m_nPreview_PaperOffsetLeft = 0;			/* ����\�ʒu���[ */
		m_nPreview_PaperOffsetTop = 0;			/* ����\�ʒu��[ */
		// DEVMODE�\���̂�A4�c�ŏ����� 2003.07.03 �����
		m_pPrintSetting->m_mdmDevMode.dmPaperSize = DMPAPER_A4;
		m_pPrintSetting->m_mdmDevMode.dmOrientation = DMORIENT_PORTRAIT;
		m_pPrintSetting->m_mdmDevMode.dmPaperLength = m_nPreview_PaperHeight;
		m_pPrintSetting->m_mdmDevMode.dmPaperWidth = m_nPreview_PaperWidth;
		m_pPrintSetting->m_mdmDevMode.dmFields |= ( DM_ORIENTATION | DM_PAPERSIZE | DM_PAPERLENGTH | DM_PAPERWIDTH);
	}else{
		if( m_pPrintSetting->m_nPrintPaperSize != m_pPrintSetting->m_mdmDevMode.dmPaperSize ){
			char	szPaperNameOld[256];
			char	szPaperNameNew[256];
			/* �p���̖��O���擾 */
			m_cPrint.GetPaperName( m_pPrintSetting->m_nPrintPaperSize , (char*)szPaperNameOld );
			m_cPrint.GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , (char*)szPaperNameNew );

			::MYMESSAGEBOX( m_pParentWnd->m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
				"���݂̃v�����^ %s �ł́A\n�w�肳�ꂽ�p�� %s �͎g�p�ł��܂���B\n���p�\�ȗp�� %s �ɕύX���܂����B",
				m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName,
				szPaperNameOld, szPaperNameNew
			);
		}
	}
	/* ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������(�G���[��A4�c�ɂȂ����ꍇ���l������if���̊O�ֈړ� 2003.07.03 �����) */
	m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
	m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;	// �p�������̔��f�Y����C�� 2003/07/03 �����

	m_nPreview_ViewMarginLeft = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */
	m_nPreview_ViewMarginTop = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */

	m_bPreview_EnableColms =
		( m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginLX - m_pPrintSetting->m_nPrintMarginRX
		- ( m_pPrintSetting->m_nPrintDansuu - 1 ) * m_pPrintSetting->m_nPrintDanSpace
		- ( m_pPrintSetting->m_nPrintDansuu ) * ( ( m_nPreview_LineNumberColmns /*+ (m_nPreview_LineNumberColmns?1:0)*/ ) * m_pPrintSetting->m_nPrintFontWidth )
		) / m_pPrintSetting->m_nPrintFontWidth / m_pPrintSetting->m_nPrintDansuu;	/* �󎚉\����/�y�[�W */
	m_bPreview_EnableLines = ( m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginTY - m_pPrintSetting->m_nPrintMarginBY ) / ( m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 ) ) - 4;	/* �󎚉\�s��/�y�[�W */

	/* ����p�̃��C�A�E�g�Ǘ����̏����� */
	m_pLayoutMgr_Print->Create( &m_pParentWnd->m_cEditDoc, &m_pParentWnd->m_cEditDoc.m_cDocLineMgr );

	/* ����p�̃��C�A�E�g���̕ύX */
//	Types& ref = m_pParentWnd->m_cEditDoc.GetDocumentAttribute();
	Types ref = m_pParentWnd->m_cEditDoc.GetDocumentAttribute();
	ref.m_nMaxLineSize = 		m_bPreview_EnableColms;
	ref.m_bWordWrap =			m_pPrintSetting->m_bPrintWordWrap;	/* �p�����[�h���b�v������ */
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	ref.m_nTabSpace =			m_pParentWnd->m_cEditDoc.m_cLayoutMgr.GetTabSpace();

	//@@@ 2002.09.22 YAZAKI
	ref.m_cLineComment.CopyTo(0, "", -1);	/* �s�R�����g�f���~�^ */
	ref.m_cLineComment.CopyTo(1, "", -1);	/* �s�R�����g�f���~�^2 */
	ref.m_cLineComment.CopyTo(2, "", -1);	/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
	ref.m_cBlockComment.CopyTo(0, "", "");	/* �u���b�N�R�����g�f���~�^ */
	ref.m_cBlockComment.CopyTo(1, "", "");	/* �u���b�N�R�����g�f���~�^2 */

	ref.m_nStringType =			0;		/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = FALSE;
	ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = FALSE;
	ref.m_bKinsokuHead = m_pPrintSetting->m_bPrintKinsokuHead,	/* �s���֑����� */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuTail = m_pPrintSetting->m_bPrintKinsokuTail,	/* �s���֑����� */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuRet = m_pPrintSetting->m_bPrintKinsokuRet,	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	ref.m_bKinsokuKuto = m_pPrintSetting->m_bPrintKinsokuKuto,	/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	m_pLayoutMgr_Print->SetLayoutInfo(
		TRUE,	
		NULL,	
		ref
	);
	m_nAllPageNum = m_pLayoutMgr_Print->GetLineCount() / ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu );		/* �S�y�[�W�� */
	if( 0 < m_pLayoutMgr_Print->GetLineCount() % ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu ) ){
		m_nAllPageNum++;
	}
	if( m_nAllPageNum <= m_nCurPageNum ){	/* ���݂̃y�[�W */
		m_nCurPageNum = 0;
	}
	int		i;
	for( i = 0; i < ( sizeof( m_pnDx ) / sizeof( m_pnDx[0]) ); ++i ){
		m_pnDx[i] = m_pPrintSetting->m_nPrintFontWidth;
	}
	/* WM_SIZE ���� */
	RECT	rc;
	::GetClientRect( m_pParentWnd->m_hWnd, &rc );
	OnSize( SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
	::ReleaseDC( m_pParentWnd->m_hWnd, hdc );
	/* �v���r���[ �y�[�W�w�� */
	OnPreviewGoPage( m_nCurPageNum );
	return;
}

void CPrintPreview::OnPreviewGoPage( int nPage )
{
	if( m_nAllPageNum <= nPage ){	/* ���݂̃y�[�W */
		nPage = m_nAllPageNum - 1;
	}
	if( 0 > nPage ){				/* ���݂̃y�[�W */
		nPage = 0;
	}
	m_nCurPageNum = nPage;

	if( 0 == m_nCurPageNum ){
		//	�ŏ��̃y�[�W�̂Ƃ��́A�O�̃y�[�W�{�^�����I�t�B
		//	Jul. 18, 2001 genta Focus�̂���Window��Disable�ɂ���Ƒ���ł��Ȃ��Ȃ�̂����
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ));
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ), FALSE );
	}else{
		//	�O�̃y�[�W�{�^�����I��
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ), TRUE );
	}

	if( m_nAllPageNum <= m_nCurPageNum + 1 ){
		//	�Ō�̃y�[�W�̂Ƃ��́A���̃y�[�W�{�^�����I�t�B
		//	Jul. 18, 2001 genta Focus�̂���Window��Disable�ɂ���Ƒ���ł��Ȃ��Ȃ�̂����
		//	Mar. 9, 2003 genta 1�y�[�W���������Ƃ��́u�O�ցv�{�^����Disable����Ă���̂ŁA
		//	�Ō�̃y�[�W�܂ŒB������u�߂�v�Ƀt�H�[�J�X���ڂ��悤��
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDCANCEL ));
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ), FALSE );
	}else{
		//	���̃y�[�W�{�^�����I���B
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ), TRUE );
	}

	char	szEdit[1024];
	wsprintf( szEdit, "%d/%d��", m_nCurPageNum + 1, m_nAllPageNum );
	::SetDlgItemText( m_hwndPrintPreviewBar, IDC_STATIC_PAGENUM, szEdit );

	wsprintf( szEdit, "%d %%", m_nPreview_Zoom );
	::SetDlgItemText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	::InvalidateRect( m_pParentWnd->m_hWnd, NULL, TRUE );
	return;
}

void CPrintPreview::OnPreviewZoom( BOOL bZoomUp )
{
	if( bZoomUp ){
		m_nPreview_Zoom += 10;	/* ����v���r���[�{�� */
		if( MAX_PREVIEW_ZOOM < m_nPreview_Zoom ){
			m_nPreview_Zoom = MAX_PREVIEW_ZOOM;
		}
	}else{
		/* �X�N���[���ʒu�𒲐� */
		m_nPreviewVScrollPos = 0;
		m_nPreviewHScrollPos = 0;

		m_nPreview_Zoom -= 10;	/* ����v���r���[�{�� */
		if( MIN_PREVIEW_ZOOM > m_nPreview_Zoom ){
			m_nPreview_Zoom = MIN_PREVIEW_ZOOM;
		}
	}
	
	//	�k���{�^����ON/OFF
	if( MIN_PREVIEW_ZOOM == m_nPreview_Zoom ){
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMDOWN ), TRUE );
	}
	//	�g��{�^����ON/OFF
	if( MAX_PREVIEW_ZOOM == m_nPreview_Zoom ){
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_ZOOMUP ), TRUE );
	}

	char	szEdit[1024];
	wsprintf( szEdit, "%d %%", m_nPreview_Zoom );
	::SetDlgItemText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	/* WM_SIZE ���� */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->m_hWnd, &rc1 );
	OnSize( SIZE_RESTORED, MAKELONG( rc1.right - rc1.left, rc1.bottom - rc1.top ) );

	/* ����v���r���[ �X�N���[���o�[������ */
	InitPreviewScrollBar();

	/* �ĕ`�� */
	::InvalidateRect( m_pParentWnd->m_hWnd, NULL, TRUE );
	return;
}


void CPrintPreview::OnPrint( void )
{
	HDC			hdc;
	char		szJobName[256 + 1];
	char		szProgress[100];
	char		szErrMsg[1024];
	int			nDirectY = -1;
	int			i;
	HFONT		hFontOld;	//	OnPrint�ȑO�̃t�H���g
	HFONT		hFontHan;	//	����p���p�t�H���g
	HFONT		hFontZen;	//	����p�S�p�t�H���g

	if( 0 == m_nAllPageNum ){
		::MYMESSAGEBOX( m_pParentWnd->m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
			"�������y�[�W������܂���B"
		);
		return;
	}

	/* �v�����^�ɓn���W���u���𐶐� */
	if( ! m_pParentWnd->m_cEditDoc.IsFilePathAvailable() ){	/* ���ݕҏW���̃t�@�C���̃p�X */
		strcpy( szJobName, "����" );
	}else{
		char	szFileName[_MAX_FNAME];
		char	szExt[_MAX_EXT];
		_splitpath( m_pParentWnd->m_cEditDoc.GetFilePath(), NULL, NULL, szFileName, szExt );
		wsprintf( szJobName, "%s%s", szFileName, szExt );
	}

	/* ����͈͂��w��ł���v�����^�_�C�A���O���쐬 */
	//	2003.05.02 �����
	PRINTDLG pd;
	memset( &pd, 0, sizeof(PRINTDLG) );
#ifndef _DEBUG
// Debug���[�h�ŁAhwndOwner���w�肷��ƁAWin2000�ł͗�����̂ŁE�E�E
	pd.hwndOwner = m_pParentWnd->m_hWnd;
#endif
	pd.nMinPage = 1;
	pd.nMaxPage = m_nAllPageNum;
	pd.nFromPage = 1;
	pd.nToPage = m_nAllPageNum;
	pd.Flags = PD_ALLPAGES | PD_NOSELECTION | PD_USEDEVMODECOPIESANDCOLLATE;
	if (FALSE == m_cPrint.PrintDlg( &pd, &m_pPrintSetting->m_mdmDevMode )) {
		return;
	}
	// ����J�n�y�[�W�ƁA����y�[�W�����m�F
	int			nFrom;
	int			nNum;
	if( 0 != (pd.Flags & PD_PAGENUMS) ){	// 2003.05.02 �����
		nFrom = pd.nFromPage - 1;
		nNum  = pd.nToPage - nFrom;
	}else{
		nFrom = 0;
		nNum  = m_nAllPageNum;
	}

	/* ����ߒ���\�����āA�L�����Z�����邽�߂̃_�C�A���O���쐬 */
	CDlgCancel	cDlgPrinting;
	cDlgPrinting.DoModeless( m_pParentWnd->m_hInstance, m_pParentWnd->m_hWnd, IDD_PRINTING );
	::SetDlgItemText( cDlgPrinting.m_hWnd, IDC_STATIC_JOBNAME, szJobName );

	/* �e�E�B���h�E�𖳌��� */
	::EnableWindow( m_pParentWnd->m_hWnd, FALSE );

	/* ��� �W���u�J�n */
	if( FALSE == m_cPrint.PrintOpen(
		szJobName,
		&m_pPrintSetting->m_mdmDevMode,	/* �v�����^�ݒ� DEVMODE�p*/
		&hdc,
		szErrMsg						/* �G���[���b�Z�[�W�i�[�ꏊ */
	) ){
//		MYTRACE( "%s\n", szErrMsg );
	}

	/* ����p���p�t�H���g�ƁA����p�S�p�t�H���g���쐬 */
	hFontHan = CreateFontIndirect( &m_lfPreviewHan );
	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	// ���݂̃t�H���g������p���p�t�H���g�ɐݒ聕�ȑO�̃t�H���g��ێ�
	hFontOld = (HFONT)::SelectObject( hdc, hFontHan );

	/* ���̑傫��������킷RECT��ݒ� */
	RECT cRect;
	cRect.left   =                             m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft + 5;
	cRect.right  = m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + m_nPreview_PaperOffsetLeft + 5);
	cRect.top    = nDirectY * (                              m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop + 5 );
	cRect.bottom = nDirectY * ( m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + m_nPreview_PaperOffsetTop + 5) );

	/* �w�b�_�E�t�b�^��$p��W�J���邽�߂ɁAm_nCurPageNum��ێ� */
	int nCurPageNumOld = m_nCurPageNum;
	for( i = 0; i < nNum; ++i ){
		m_nCurPageNum = nFrom + i;

		/* ����ߒ���\�� */
		//	Jun. 18, 2001 genta �y�[�W�ԍ��\���̌v�Z�~�X�C��
		sprintf( szProgress, "%d/%d", i + 1, nNum );
		::SetDlgItemText( cDlgPrinting.m_hWnd, IDC_STATIC_PROGRESS, szProgress );

		/* ��� �y�[�W�J�n */
		m_cPrint.PrintStartPage( hdc );

		//	From Here Jun. 26, 2003 ����� / ������
		//	Windows 95/98�ł�StartPage()�֐��̌Ăяo�����ɁA�����̓��Z�b�g����Ċ���l�֖߂�܂��D
		//	���̂Ƃ��J���҂͎��̃y�[�W�̈�����n�߂�O�ɃI�u�W�F�N�g��I���������C
		//	�}�b�s���O���[�h��������x�ݒ肵�Ȃ���΂Ȃ�܂���
		//	Windows NT/2000�ł�StartPage�ł������̓��Z�b�g����܂���D
		
		/* �}�b�s���O���[�h�̕ύX */
		::SetMapMode( hdc, MM_LOMETRIC );		//���ꂼ��̘_���P�ʂ́A0.1 mm �Ƀ}�b�v����܂�
		::SetMapMode( hdc, MM_ANISOTROPIC );	//�_���P�ʂ́A�C�ӂɃX�P�[�����O���ꂽ����̔C�ӂ̒P�ʂɃ}�b�v����܂�

		// ���݂̃t�H���g������p���p�t�H���g�ɐݒ�
		::SelectObject( hdc, hFontHan );
		//	To Here Jun. 26, 2003 ����� / ������

		/* �w�b�_��� */
		DrawHeader( hdc, cRect, hFontZen );

		/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
		DrawPageText(
			hdc,
			m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft ,
			m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop+ 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
			nFrom + i,
			hFontZen,
			&cDlgPrinting
		);

		/* �t�b�^��� */
		DrawFooter( hdc, cRect, hFontZen );

		/* ��� �y�[�W�I�� */
		m_cPrint.PrintEndPage( hdc );

		/* ���f�{�^�������`�F�b�N */
		if( cDlgPrinting.IsCanceled() ){
			break;
		}
	}
	//	����O�̃t�H���g�ɖ߂� 2003.05.02 ����� hdc����̑O�ɏ���������ύX
	::SelectObject( hdc, hFontOld );

	/* ��� �W���u�I�� */
	m_cPrint.PrintClose( hdc );

	//	����p�t�H���g���폜�B
	::DeleteObject( hFontZen );
	::DeleteObject( hFontHan );

	::EnableWindow( m_pParentWnd->m_hWnd, TRUE );
	cDlgPrinting.CloseDialog( 0 );

	m_nCurPageNum = nCurPageNumOld;

	// ������I�������APreview���甲���� 2003.05.02 �����
	m_pParentWnd->PrintPreviewModeONOFF();
	return;
}

/*! ���/����v���r���[ �w�b�_�̕`��
*/
void CPrintPreview::DrawHeader( HDC hdc, RECT& rect, HFONT hFontZen )
{
	/*	�w�b�_	*/
	const int nHeaderWorkLen = 1024;
	char      szHeaderWork[1024 + 1];
	
	/* ���� */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_LEFT], szHeaderWork, nHeaderWorkLen);
	Print_DrawLine( hdc,
		rect.left,
		rect.top,
		szHeaderWork, lstrlen( szHeaderWork ),
		hFontZen
	);

	/* ������ */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_CENTER], szHeaderWork, nHeaderWorkLen);
	Print_DrawLine( hdc,
		( rect.right + rect.left - lstrlen( szHeaderWork ) * m_pPrintSetting->m_nPrintFontWidth) / 2,
		rect.top,
		szHeaderWork, lstrlen( szHeaderWork ),
		hFontZen
	);
	
	/* �E�� */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szHeaderForm[POS_RIGHT], szHeaderWork, nHeaderWorkLen);
	Print_DrawLine( hdc,
		rect.right - lstrlen( szHeaderWork ) * m_pPrintSetting->m_nPrintFontWidth,
		rect.top,
		szHeaderWork, lstrlen( szHeaderWork ),
		hFontZen
	);
}

/*! ���/����v���r���[ �t�b�^�̕`��
*/
void CPrintPreview::DrawFooter( HDC hdc, RECT& rect, HFONT hFontZen )
{
	/*	�t�b�^	*/
	const int nFooterWorkLen = 1024;
	char      szFooterWork[1024 + 1];
	
	/* ���� */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_LEFT], szFooterWork, nFooterWorkLen);
	//	����x���W
	Print_DrawLine( hdc,
		rect.left,
		rect.bottom + m_pPrintSetting->m_nPrintFontHeight,
		szFooterWork, lstrlen( szFooterWork ),
		hFontZen
	);

	/* ������ */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_CENTER], szFooterWork, nFooterWorkLen);
	Print_DrawLine( hdc,
		( rect.right + rect.left - lstrlen( szFooterWork ) * m_pPrintSetting->m_nPrintFontWidth) / 2,
		rect.bottom + m_pPrintSetting->m_nPrintFontHeight,
		szFooterWork, lstrlen( szFooterWork ),
		hFontZen
	);
	
	/* �E�� */
	m_pParentWnd->m_cEditDoc.ExpandParameter(m_pPrintSetting->m_szFooterForm[POS_RIGHT], szFooterWork, nFooterWorkLen);
	Print_DrawLine( hdc,
		rect.right - lstrlen( szFooterWork ) * m_pPrintSetting->m_nPrintFontWidth,
		rect.bottom + m_pPrintSetting->m_nPrintFontHeight,
		szFooterWork, lstrlen( szFooterWork ),
		hFontZen
	);
}

/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`��
	DrawPageText�ł́A�s�ԍ����i���p�t�H���g�Łj����B
	�{����Print_DrawLine�ɂ��C��
*/
void CPrintPreview::DrawPageText(
	HDC				hdc,
	int				nOffX,
	int				nOffY,
	int				nPageNum,
	HFONT			hFontZen,
	CDlgCancel*		pCDlgCancel
)
{
	int				nDirectY = -1;
	int				nLineCols;
	const char*		pLine;
	int				nLineLen;
	TEXTMETRIC		tm;

	int				nLineHeight = m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 );

	/* ���p�t�H���g�̏����擾�����p�t�H���g�ɐݒ� */
	::GetTextMetrics( hdc, &tm );
	int nAscentHan = tm.tmAscent;

	int				nDan;	//	�i���J�E���^
	int				i;	//	�s���J�E���^
	for( nDan = 0; nDan < m_pPrintSetting->m_nPrintDansuu; ++nDan ){
		for( i = 0; i < m_bPreview_EnableLines; ++i ){
			if( NULL != pCDlgCancel ){
				/* �������̃��[�U�[������\�ɂ��� */
				if( !::BlockingHook( pCDlgCancel->m_hWnd ) ){
					return;
				}
			}

			/*	���ݕ`�悵�悤�Ƃ��Ă���s�̕����s���i�܂�Ԃ����ƂɃJ�E���g�����s���j
				�֌W������̂́A
				�u�y�[�W���inPageNum�j�v
				�u�i���im_pPrintSetting->m_nPrintDansuu�j�v
				�u�i����1�̂Ƃ��ɁA1�y�[�W������ɉ��s���邩�im_bPreview_EnableLines�j�v
			*/
			int nLineNum = nPageNum * ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu ) + m_bPreview_EnableLines * nDan + i;
			pLine = m_pLayoutMgr_Print->GetLineStr( nLineNum, &nLineLen );
			if( NULL == pLine ){
				break;
			}
			/* �s�ԍ���\�����邩 */
			if( m_pPrintSetting->m_bPrintLineNumber ){
				char			szLineNum[64];	//	�s�ԍ�������B
				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				if( m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_bLineNumIsCRLF ){
					/* �_���s�ԍ��\�����[�h */
					const CLayout*	pcLayout = m_pLayoutMgr_Print->Search( nLineNum );
					if( NULL == pcLayout || 0 != pcLayout->m_nOffset ){
						strcpy( szLineNum, " " );
					}else{
						_itoa( pcLayout->m_nLinePhysical + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
					}
				}else{
					/* �����s(���C�A�E�g�s)�ԍ��\�����[�h */
					_itoa( nLineNum + 1, szLineNum, 10 );
				}
				/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
				if( 2 == m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_nLineTermType ){
					char szLineTerm[2];
					wsprintf( szLineTerm, "%c", m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_cLineTermChar );	/* �s�ԍ���؂蕶�� */
					strcat( szLineNum, szLineTerm );
				}else{
					strcat( szLineNum, " " );
				}
				nLineCols = lstrlen( szLineNum );
				::ExtTextOut( hdc,
					nOffX +
					( m_bPreview_EnableColms * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace )
					* nDan +
					m_nPreview_LineNumberColmns * m_pPrintSetting->m_nPrintFontWidth * (nDan) +
					( m_nPreview_LineNumberColmns - nLineCols) * m_pPrintSetting->m_nPrintFontWidth,
					nDirectY * ( nOffY + nLineHeight * i + ( m_pPrintSetting->m_nPrintFontHeight - nAscentHan ) ), 0, NULL,
					szLineNum, nLineCols, m_pnDx
				);
				/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
				if( 1 == m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_nLineTermType ){
					::MoveToEx( hdc,
						nOffX +
						( m_bPreview_EnableColms * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace )
						* nDan +
						m_nPreview_LineNumberColmns * m_pPrintSetting->m_nPrintFontWidth * (nDan) +
						( m_nPreview_LineNumberColmns ) * m_pPrintSetting->m_nPrintFontWidth
						- (m_pPrintSetting->m_nPrintFontWidth / 2 )
						,
						nDirectY * ( nOffY + nLineHeight * i ),
						NULL );
					::LineTo( hdc,
						nOffX +
						( m_bPreview_EnableColms * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace )
						* nDan +
						m_nPreview_LineNumberColmns * m_pPrintSetting->m_nPrintFontWidth * (nDan) +
						( m_nPreview_LineNumberColmns ) * m_pPrintSetting->m_nPrintFontWidth
						- (m_pPrintSetting->m_nPrintFontWidth / 2 )
						,
						nDirectY * ( nOffY + nLineHeight * i + nLineHeight )
					);
				}

			}

			if( 0 <= nLineLen - 1 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ){
				nLineLen--;
				if( 0 <= nLineLen - 1 && ( pLine[nLineLen - 1] == CR || pLine[nLineLen - 1] == LF ) ){
					nLineLen--;
				}
			}
			if( 0 == nLineLen ){
				continue;
			}

			/* ����^�v���r���[ �s�`�� */
			Print_DrawLine(
				hdc,
				nOffX + ( m_bPreview_EnableColms * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace ) * nDan + m_nPreview_LineNumberColmns * m_pPrintSetting->m_nPrintFontWidth * (nDan + 1),
				nDirectY * ( nOffY + nLineHeight * i ),
				pLine,
				nLineLen,
				hFontZen
			);
		}
	}
	return;
}


/* ����v���r���[ �X�N���[���o�[������ */
void CPrintPreview::InitPreviewScrollBar( void )
{
	SCROLLINFO	si;
	RECT		rc;
	int			cx, cy;
	int			nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}
	::GetClientRect( m_pParentWnd->m_hWnd, &rc );
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top - nToolBarHeight;

	if( NULL != m_hwndVScrollBar ){
		/* �����X�N���[���o�[ */
		si.cbSize = sizeof( SCROLLINFO );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		if( m_nPreview_ViewHeight <= cy - nToolBarHeight ){
			si.nMax  = cy - nToolBarHeight;			/* �S�� */
			si.nPage = cy - nToolBarHeight;			/* �\����̌��� */
			si.nPos  = -1 * m_nPreviewVScrollPos;	/* �\����̈�ԍ��̈ʒu */
			si.nTrackPos = 0;
			m_SCROLLBAR_VERT = FALSE;
		}else{
			si.nMax  = m_nPreview_ViewHeight;		/* �S�� */
			si.nPage = cy - nToolBarHeight;			/* �\����̌��� */
			si.nPos  = -1 * m_nPreviewVScrollPos;	/* �\����̈�ԍ��̈ʒu */
			si.nTrackPos = 100;
			m_SCROLLBAR_VERT = TRUE;
		}
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	}
	/* ����v���r���[ �����X�N���[���o�[�E�B���h�E�n���h�� */
	if( NULL != m_hwndHScrollBar ){
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		/* �����X�N���[���o�[ */
//		si.cbSize = sizeof( si );
//		si.fMask = SIF_ALL;
		si.nMin  = 0;
		if( m_nPreview_ViewWidth <= cx ){
			si.nMax  = cx;							/* �S�� */
			si.nPage = cx;							/* �\����̌��� */
			si.nPos  = m_nPreviewHScrollPos;		/* �\����̈�ԍ��̈ʒu */
			si.nTrackPos = 0;
			m_SCROLLBAR_HORZ = FALSE;
		}else{
			si.nMax  = m_nPreview_ViewWidth;		/* �S�� */
			si.nPage = cx;							/* �\����̌��� */
			si.nPos  = m_nPreviewHScrollPos;		/* �\����̈�ԍ��̈ʒu */
			si.nTrackPos = 100;
			m_SCROLLBAR_HORZ = TRUE;
		}
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
	}
	return;
}

/* ����^�v���r���[ �s�`�� */
void CPrintPreview::Print_DrawLine(
	HDC			hdc,
	int			x,
	int			y,
	const char*	pLine,
	int			nLineLen,
	HFONT		hFontZen
)
{
#define MODE_SINGLEBYTE 1
#define MODE_DOUBLEBYTE 2

	int			nCharChars;
	int			nPreviousChars = MODE_SINGLEBYTE;
	TEXTMETRIC	tm;

	/* �S�p�����̃A�Z���g�i�������j���擾 */
	HFONT	hFontHan = (HFONT)::SelectObject( hdc, hFontZen );
	::GetTextMetrics( hdc, &tm );
	int nAscentZen = tm.tmAscent;

	/* ���p�����̃A�Z���g�i�������j���擾 */
	//	���p�����p�t�H���g���I������Ă��邱�Ƃ����҂��Ă���B
	::SelectObject( hdc, hFontHan );
	::GetTextMetrics( hdc, &tm );
	int nAscentHan = tm.tmAscent;

	/*	pLine���X�L�������āA���p�����͔��p�����ł܂Ƃ߂āA�S�p�����͑S�p�����ł܂Ƃ߂ĕ`�悷��B
	*/
	int			nPosX = 0;	//	TAB��W�J������̃o�C�g���ŁA�e�L�X�g�̉��o�C�g�ڂ܂ŕ`�悵�����H
	int			nBgn = 0;	//	TAB��W�J����O�̃o�C�g���ŁApLine�̉��o�C�g�ڂ܂ŕ`�悵�����H
	int			i;			//	pLine�̉������ڂ��X�L�����H
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	int			nTabSpace = m_pParentWnd->m_cEditDoc.m_cLayoutMgr.GetTabSpace();

	for( i = 0; i < nLineLen; ++i ){
		nCharChars = CMemory::MemCharNext( pLine, nLineLen, &pLine[i] ) - &pLine[i];
		if( 0 == nCharChars ){	//	0�o�C�g�����H
			nCharChars = 1;
		}
		if( nCharChars == 1 ){	//	1�o�C�g����
			if( MODE_SINGLEBYTE == nPreviousChars ){
				//	��������1�o�C�g�����B
				if( TAB == pLine[i] ){	//	�^�u����
					if( 0 < i - nBgn ){
						//	�^�u�����̑O�܂ł�`��B
						::ExtTextOut(
							hdc,
							x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
							y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentHan ),
							0,
							NULL,
							&pLine[nBgn], i - nBgn,
							m_pnDx
						);
						nPosX += ( i - nBgn );	//	���ۂ̕������ƁA�o�C�g���̍��𖄂߂�B
					}
					nPosX += ( nTabSpace - nPosX % nTabSpace );
					nBgn = i + 1;	//	�X�L�������̎��̃o�C�g
				}
			}else{
				/*	2�o�C�g��������A1�o�C�g�����ɐ؂�ւ�����Ƃ�
					2�o�C�g�����̉��`��B
				*/
				if( 0 < i - nBgn ){
					::SelectObject( hdc, hFontZen );
					::ExtTextOut(
						hdc,
						x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
						y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentZen ),
						0,
						NULL,
						&pLine[nBgn], i - nBgn,
						m_pnDx
					);
					::SelectObject( hdc, hFontHan );
					nPosX += ( i - nBgn );
				}
				nBgn = i;
				nPreviousChars = MODE_SINGLEBYTE;
				if( TAB == pLine[i] ){	//	�^�u����
					nPosX += ( nTabSpace - nPosX % nTabSpace );
					nBgn = i + 1;
				}
			}
		}else{	//	1�o�C�g����
			if( MODE_SINGLEBYTE == nPreviousChars ){
				if( 0 < i - nBgn ){
					::ExtTextOut(
						hdc,
						x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
						y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentHan ),
						0,
						NULL,
						&pLine[nBgn], i - nBgn,
						m_pnDx
					);
					nPosX += ( i - nBgn );
				}
				nBgn = i;
				nPreviousChars = MODE_DOUBLEBYTE;
			}
			++i;
		}
	}
	
	if( 0 < i - nBgn ){
		if( MODE_DOUBLEBYTE == nPreviousChars ){	/*	2�o�C�g�����`��	*/
			::SelectObject( hdc, hFontZen );
			::ExtTextOut(
				hdc,
				x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
				y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentZen ),
				0,
				NULL,
				&pLine[nBgn], i - nBgn,
				m_pnDx
			);
			::SelectObject( hdc, hFontHan );
		}else{				/* 1�o�C�g�����`��B�t�H���g�͕W����1�o�C�g�����p�B	*/
			::ExtTextOut(
				hdc,
				x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
				y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentHan ),
				0,
				NULL,
				&pLine[nBgn], i - nBgn,
				m_pnDx
			);
		}
	}
	return;
}

/*	����v���r���[�t�H���g�i���p�j��ݒ肷��
	typedef struct tagLOGFONT {
	   LONG lfHeight; 
	   LONG lfWidth; 
	   LONG lfEscapement; 
	   LONG lfOrientation; 
	   LONG lfWeight; 
	   BYTE lfItalic; 
	   BYTE lfUnderline; 
	   BYTE lfStrikeOut; 
	   BYTE lfCharSet; 
	   BYTE lfOutPrecision; 
	   BYTE lfClipPrecision; 
	   BYTE lfQuality; 
	   BYTE lfPitchAndFamily; 
	   TCHAR lfFaceName[LF_FACESIZE]; 
	} LOGFONT;
*/
void CPrintPreview::SetPreviewFontHan( LOGFONT* lf )
{
	m_lfPreviewHan = *lf;

	//	PrintSetting����R�s�[
	m_lfPreviewHan.lfHeight			= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth			= m_pPrintSetting->m_nPrintFontWidth;
	strcpy(m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan);
}

void CPrintPreview::SetPreviewFontZen( LOGFONT* lf )
{
	m_lfPreviewZen = *lf;
	//	PrintSetting����R�s�[
	m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	strcpy(m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );
}

int CALLBACK CPrintPreview::MyEnumFontFamProc(
	ENUMLOGFONT*	pelf,		// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 		// address of application-defined data
)
{
	CPrintPreview* pCPrintPreview = (CPrintPreview*)lParam;
	if( 0 == strcmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceHan ) ){
		pCPrintPreview->SetPreviewFontHan(&pelf->elfLogFont);
	}
	if( 0 == strcmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceZen ) ){
		pCPrintPreview->SetPreviewFontZen(&pelf->elfLogFont);
	}

	return 1;
}

/*!
	����v���r���[�ɕK�v�ȃR���g���[�����쐬����
*/
void CPrintPreview::CreatePrintPreviewControls( void )
{
	int			nCxHScroll = ::GetSystemMetrics( SM_CXHSCROLL );
	int			nCyHScroll = ::GetSystemMetrics( SM_CYHSCROLL );
	int			nCxVScroll = ::GetSystemMetrics( SM_CXVSCROLL );
	int			nCyVScroll = ::GetSystemMetrics( SM_CYVSCROLL );

	/* ����v���r���[ ����o�[ */
	m_hwndPrintPreviewBar = ::CreateDialogParam(
		m_pParentWnd->m_hInstance,							// handle to application instance
		MAKEINTRESOURCE( IDD_PRINTPREVIEWBAR ),				// identifies dialog box template name
		m_pParentWnd->m_hWnd,								// handle to owner window
		(DLGPROC)CPrintPreview::PrintPreviewBar_DlgProc,	// pointer to dialog box procedure
		(LPARAM)this
	);

	/* �c�X�N���[���o�[�̍쐬 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles			*/
		"SCROLLBAR",						/* scroll bar control class		*/
		(LPSTR) NULL,						/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->m_hWnd,								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		m_pParentWnd->m_hInstance,						/* instance owning this window	*/
		(LPVOID) NULL						/* pointer not needed			*/
	);
	SCROLLINFO	si;
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndVScrollBar, SB_CTL, TRUE );

	/* ���X�N���[���o�[�̍쐬 */
	m_hwndHScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles			*/
		"SCROLLBAR",						/* scroll bar control class		*/
		(LPSTR) NULL,						/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->m_hWnd,								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		m_pParentWnd->m_hInstance,						/* instance owning this window	*/
		(LPVOID) NULL						/* pointer not needed			*/
	);
	si.cbSize = sizeof( si );
	si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
	si.nMin	 = 0;
	si.nMax	 = 29;
	si.nPage = 10;
	si.nPos	 = 0;
	si.nTrackPos = 1;
	::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );
	::ShowScrollBar( m_hwndHScrollBar, SB_CTL, TRUE );

	/* �T�C�Y�{�b�N�X�̍쐬 */
	m_hwndSizeBox = ::CreateWindowEx(
		WS_EX_CONTROLPARENT/*0L*/, 							/* no extended styles			*/
		"SCROLLBAR",										/* scroll bar control class		*/
		(LPSTR) NULL,										/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles			*/
		0,													/* horizontal position			*/
		0,													/* vertical position			*/
		200,												/* width of the scroll bar		*/
		CW_USEDEFAULT,										/* default height				*/
		m_pParentWnd->m_hWnd, 											/* handle of main window		*/
		(HMENU) NULL,										/* no menu for a scroll bar 	*/
		m_pParentWnd->m_hInstance,										/* instance owning this window	*/
		(LPVOID) NULL										/* pointer not needed			*/
	);
	::ShowWindow( m_hwndPrintPreviewBar, SW_SHOW );


	/* WM_SIZE ���� */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->m_hWnd, &rc1 );
	OnSize( SIZE_RESTORED, MAKELONG( rc1.right - rc1.left, rc1.bottom - rc1.top ) );
	return;
}


/*!
	����v���r���[�ɕK�v�������R���g���[����j������
*/
void CPrintPreview::DestroyPrintPreviewControls( void )
{
	/* ����v���r���[ ����o�[ �폜 */
	if ( m_hwndPrintPreviewBar ){
		::DestroyWindow( m_hwndPrintPreviewBar );
		m_hwndPrintPreviewBar = NULL;
	}

	/* ����v���r���[ �����X�N���[���o�[�E�B���h�E �폜 */
	if( m_hwndVScrollBar ){
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}
	/* ����v���r���[ �����X�N���[���o�[�E�B���h�E �폜 */
	if( m_hwndHScrollBar ){
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}
	/* ����v���r���[ �T�C�Y�{�b�N�X�E�B���h�E �폜 */
	if ( m_hwndSizeBox ){
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}

/* �_�C�A���O�v���V�[�W�� */
INT_PTR CALLBACK CPrintPreview::PrintPreviewBar_DlgProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
)
{
	CPrintPreview* pCPrintPreview;
	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		return TRUE;
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPrintPreview = ( CPrintPreview* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPrintPreview ){
			return pCPrintPreview->DispatchEvent_PPB( hwndDlg, uMsg, wParam, lParam );
		}else{
			return FALSE;
		}
	}
}

/* ����v���r���[ ����o�[�Ƀt�H�[�J�X�𓖂Ă� */
void CPrintPreview::SetFocusToPrintPreviewBar( void )
{
	if( NULL != m_hwndPrintPreviewBar ){
		::SetFocus( m_hwndPrintPreviewBar );
	}
}

/* ����v���r���[ ����o�[ �_�C�A���O�̃��b�Z�[�W���� */
INT_PTR CPrintPreview::DispatchEvent_PPB(
	HWND				hwndDlg,	// handle to dialog box
	UINT				uMsg,		// message
	WPARAM				wParam,		// first message parameter
	LPARAM				lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	CMemory				cMemBuf;
	char				szHelpFile[_MAX_PATH];



	switch( uMsg ){

	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID�A�R���g���[��ID �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_PRINTERSELECT:
				// From Here 2003.05.03 �����
				// PRINTDLG��������
				PRINTDLG	pd;
				memset( &pd, 0, sizeof(PRINTDLG) );
				pd.Flags = PD_PRINTSETUP | PD_NONETWORKBUTTON;
				pd.hwndOwner = m_pParentWnd->m_hWnd;
				if (TRUE == m_cPrint.PrintDlg( &pd, &m_pPrintSetting->m_mdmDevMode )) {
					// �p���T�C�Y�Ɨp�������𔽉f������ 2003.05.03 �����
					m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
					m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;
					/* ����v���r���[ �X�N���[���o�[������ */
					InitPreviewScrollBar();
					OnChangePrintSetting();
					::InvalidateRect( m_pParentWnd->m_hWnd, NULL, TRUE );
				}
				// To Here 2003.05.03 �����
				break;
			case IDC_BUTTON_PRINTSETTING:
				m_pParentWnd->OnPrintPageSetting();
				break;
			case IDC_BUTTON_ZOOMUP:
				/* �v���r���[�g��k�� */
				OnPreviewZoom( TRUE );
				break;
			case IDC_BUTTON_ZOOMDOWN:
				/* �v���r���[�g��k�� */
				OnPreviewZoom( FALSE );
				break;
			case IDC_BUTTON_PREVPAGE:
				/* �O�y�[�W */
				OnPreviewGoPreviousPage( );
				break;
			case IDC_BUTTON_NEXTPAGE:
				/* ���y�[�W */
				OnPreviewGoNextPage( );
				break;

			case IDC_BUTTON_HELP:
				/* �w���v�t�@�C���̃t���p�X��Ԃ� */
				::GetHelpFilePath( szHelpFile );
				/* ����v���r���[�̃w���v */
				//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
				::WinHelp( hwndDlg, szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PREVIEW) );
				break;
			case IDOK:
				/* ������s */
				OnPrint();
				return TRUE;
			case IDCANCEL:
				/* ����v���r���[���[�h�̃I��/�I�t */
				m_pParentWnd->PrintPreviewModeONOFF();
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		break;	/* WM_COMMAND */
	}
	return FALSE;
}
