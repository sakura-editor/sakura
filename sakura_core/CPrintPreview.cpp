/*!	@file
	@brief ����v���r���[�Ǘ��N���X

	@author YAZAKI
	@date 2002/1/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CPrintPreview.h"
#include "CLayout.h"
#include "CEditWnd.h"
#include "charcode.h"
#include "CDlgPrintPage.h"

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		/* ���������̂P��̃X�N���[���� */
#define		LINE_RANGE_Y	24		/* ���������̂P��̃X�N���[���� */

#define		PAGE_RANGE_X	160		/* ���������̂P��̃y�[�W�X�N���[���� */
#define		PAGE_RANGE_Y	160		/* ���������̂P��̃y�[�W�X�N���[���� */

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
	CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , (char*)szPaperName );
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

	HFONT	hFont = CreateFontIndirect( &m_lfPreviewHan );
	HFONT	hFontOld = (HFONT)::SelectObject( hdc, hFont );
	HFONT	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	/* ����E�B���h�E�̉��ɕ������W���_���ړ� */
	POINT			poViewPortOld;
	::SetViewportOrgEx( hdc, -1 * m_nPreviewHScrollPos, nToolBarHeight + m_nPreviewVScrollPos, &poViewPortOld );

	/* �p���̕`�� */
	int	nDirectY = -1;	//	�ȂɁH
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

	HFONT	hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );

	/* �t�@�C�����̕\�� */
	char			szWork[260];
	if( 0 == lstrlen( m_pParentWnd->m_cEditDoc.m_szFilePath ) ){	/* ���ݕҏW���̃t�@�C���̃p�X */
		strcpy( szWork, "(����)" );
	}else{
		char	szFileName[_MAX_FNAME];
		char	szExt[_MAX_EXT];
		_splitpath( m_pParentWnd->m_cEditDoc.m_szFilePath, NULL, NULL, szFileName, szExt );
		wsprintf( szWork, "%s%s", szFileName, szExt );
	}
	::TextOut( hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		nDirectY * ( m_nPreview_ViewMarginTop + m_pPrintSetting->m_nPrintMarginTY ),
		szWork, lstrlen( szWork )
	);
	::SelectObject( hdc, hFontZenOld );

	/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
	DrawPageText(
		hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		m_nPreview_ViewMarginTop  + m_pPrintSetting->m_nPrintMarginTY + 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
		m_nCurPageNum,
		hFontZen,
		NULL
	);
	/* �y�[�W�ԍ��̕\�� */
	wsprintf( szWork, "- %d -", m_nCurPageNum + 1 );
	::TextOut( hdc,
		m_nPreview_ViewMarginLeft
		+ m_nPreview_PaperWidth / 2
		- ( 5 * 10 )
		,
		nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginBY - m_pPrintSetting->m_nPrintFontHeight/* - m_nPreview_PaperOffsetTop*/ ),
		szWork, lstrlen( szWork )
	);

	/* ����t�H���g���� & �j�� */
	::SelectObject( hdc, hFontOld );
	::DeleteObject( hFont );
	::DeleteObject( hFontZen );

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
	::SetMapMode(hdc, MM_LOMETRIC );
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
		::PostMessage( m_pParentWnd->m_hWnd, WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (WPARAM)m_hwndVScrollBar );

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
	m_pPrintSetting->m_mdmDevMode.dmFields |= ( DM_ORIENTATION | DM_PAPERSIZE );
	if( m_pPrintSetting->m_mdmDevMode.dmFields & DM_PAPERLENGTH ){
		m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERLENGTH );
	}
	if( m_pPrintSetting->m_mdmDevMode.dmFields & DM_PAPERWIDTH ){
		m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERWIDTH);
	}
	if( m_pPrintSetting->m_mdmDevMode.dmFields & DM_PAPERLENGTH ){
		m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERLENGTH );
	}
	if( m_pPrintSetting->m_mdmDevMode.dmFields & DM_PAPERWIDTH ){
		m_pPrintSetting->m_mdmDevMode.dmFields &= (~DM_PAPERWIDTH);
	}

	/* ���/�v���r���[�ɕK�v�ȏ����擾 */
	char	szErrMsg[1024];
	if( FALSE == CPrint::GetPrintMetrics(
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
	}else{
		if( m_pPrintSetting->m_nPrintPaperSize != m_pPrintSetting->m_mdmDevMode.dmPaperSize ){
			char	szPaperNameOld[256];
			char	szPaperNameNew[256];
			/* �p���̖��O���擾 */
			CPrint::GetPaperName( m_pPrintSetting->m_nPrintPaperSize , (char*)szPaperNameOld );
			CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , (char*)szPaperNameNew );

			::MYMESSAGEBOX( m_pParentWnd->m_hWnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, GSTR_APPNAME,
				"���݂̃v�����^ %s �ł́A\n�w�肳�ꂽ�p�� %s �͎g�p�ł��܂���B\n���p�\�ȗp�� %s �ɕύX���܂����B",
				m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName,
				szPaperNameOld, szPaperNameNew
			);
		}
		/* ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������ */
		m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
	}

	m_nPreview_ViewMarginLeft = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */
	m_nPreview_ViewMarginTop = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */

	m_bPreview_EnableColms =
		( m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginLX - m_pPrintSetting->m_nPrintMarginRX
		- ( m_pPrintSetting->m_nPrintDansuu - 1 ) * m_pPrintSetting->m_nPrintDanSpace
		- ( m_pPrintSetting->m_nPrintDansuu ) * ( ( m_nPreview_LineNumberColmns /*+ (m_nPreview_LineNumberColmns?1:0)*/ ) * m_pPrintSetting->m_nPrintFontWidth )
		) / m_pPrintSetting->m_nPrintFontWidth / m_pPrintSetting->m_nPrintDansuu;	/* �󎚉\����/�y�[�W */
	m_bPreview_EnableLines = ( m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginTY - m_pPrintSetting->m_nPrintMarginBY ) / ( m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 ) ) - 4;	/* �󎚉\�s��/�y�[�W */

	/* ����p�̃��C�A�E�g�Ǘ����̏����� */
	m_pLayoutMgr_Print->Create( &m_pParentWnd->m_cEditDoc.m_cDocLineMgr );

	/* ����p�̃��C�A�E�g���̕ύX */
	m_pLayoutMgr_Print->SetLayoutInfo(
		m_bPreview_EnableColms,
		m_pPrintSetting->m_bPrintWordWrap,	/* �p�����[�h���b�v������ */
		m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_nTabSpace,
		"",		/* �s�R�����g�f���~�^ */
		"",		/* �s�R�����g�f���~�^2 */
		"",		/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
		"",		/* �u���b�N�R�����g�f���~�^(From) */
		"",		/* �u���b�N�R�����g�f���~�^(To) */
		"",		/* �u���b�N�R�����g�f���~�^2(From) */
		"",		/* �u���b�N�R�����g�f���~�^2(To) */
		0,		/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
		TRUE,	
		NULL,	
		FALSE,	/* �V���O���N�H�[�e�[�V�����������\������ */
		FALSE	/* �_�u���N�H�[�e�[�V�����������\������ */
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
	::SetMapMode( hdc, MM_LOMETRIC ); //MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
	::SetMapMode( hdc, MM_ANISOTROPIC );
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
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ));
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
	HANDLE		hPrinter;
	HDC			hdc;
	char		szJobName[256 + 1];
	char		szProgress[100];
	char		szErrMsg[1024];
	int			nDirectY;
	HFONT		hFont, hFontOld;
	int			i;
	char		szWork[260];
	HFONT		hFontZen, hFontZenOld;
	int			nFrom;
	int			nTo;
	if( 0 == m_nAllPageNum ){
		::MessageBeep( MB_ICONHAND );
		return;
	}

	CDlgPrintPage cCDlgPrintPage;
	cCDlgPrintPage.m_bAllPage = TRUE;
	cCDlgPrintPage.m_nPageMin = 1;
	cCDlgPrintPage.m_nPageMax = m_nAllPageNum;
	cCDlgPrintPage.m_nPageFrom = 1;
	cCDlgPrintPage.m_nPageTo = m_nAllPageNum;
	if( FALSE == cCDlgPrintPage.DoModal( m_pParentWnd->m_hInstance, m_pParentWnd->m_hWnd, NULL ) ){
		return;
	}


	CDlgCancel	cDlgPrinting;
	cDlgPrinting.DoModeless( m_pParentWnd->m_hInstance, m_pParentWnd->m_hWnd, IDD_PRINTING );


	if( 0 == lstrlen( m_pParentWnd->m_cEditDoc.m_szFilePath ) ){	/* ���ݕҏW���̃t�@�C���̃p�X */
		strcpy( szJobName, "����" );
	}else{
		char	szFileName[_MAX_FNAME];
		char	szExt[_MAX_EXT];
		_splitpath( m_pParentWnd->m_cEditDoc.m_szFilePath, NULL, NULL, szFileName, szExt );
		wsprintf( szJobName, "%s%s", szFileName, szExt );
	}
	::SetDlgItemText( cDlgPrinting.m_hWnd, IDC_STATIC_JOBNAME, szJobName );
	::EnableWindow( m_pParentWnd->m_hWnd, FALSE );



	/* ��� �W���u�J�n */
	if( FALSE == CPrint::PrintOpen(
		szJobName,
		&m_pPrintSetting->m_mdmDevMode,	/* �v�����^�ݒ� DEVMODE�p*/
		&hPrinter,
		&hdc,
		szErrMsg						/* �G���[���b�Z�[�W�i�[�ꏊ */
	) ){
//		MYTRACE( "%s\n", szErrMsg );
	}

	hFont = CreateFontIndirect( &m_lfPreviewHan );
	hFontZen = CreateFontIndirect( &m_lfPreviewZen );

	if( FALSE == cCDlgPrintPage.m_bAllPage ){
		nFrom = cCDlgPrintPage.m_nPageFrom;
		nTo   = cCDlgPrintPage.m_nPageTo;
	}else{
		nFrom = 1;
		nTo = m_nAllPageNum;
	}
	for( i = nFrom - 1; i < nTo; ++i ){
		//	Jun. 18, 2001 genta �y�[�W�ԍ��\���̌v�Z�~�X�C��
		sprintf( szProgress, "%d/%d", i - (nFrom - 1) + 1/*i*/, nTo - (nFrom - 1)/*m_nAllPageNum*/ );
		::SetDlgItemText( cDlgPrinting.m_hWnd, IDC_STATIC_PROGRESS, szProgress );

		/* ��� �y�[�W�J�n */
		CPrint::PrintStartPage( hdc );

		/* �}�b�s���O���[�h�̕ύX */
		::SetMapMode(hdc, MM_LOMETRIC );					//MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
		::SetMapMode( hdc, MM_ANISOTROPIC/*MM_HIMETRIC*/ );	//MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
		nDirectY = -1;

		/* ����t�H���g�ݒ� */
		hFontOld = (HFONT)::SelectObject( hdc, hFont );
		/* �t�@�C�����̕\�� */
		if( 0 == lstrlen( m_pParentWnd->m_cEditDoc.m_szFilePath ) ){	/* ���ݕҏW���̃t�@�C���̃p�X */
			strcpy( szWork, "(����)" );
		}else{
			char	szFileName[_MAX_FNAME];
			char	szExt[_MAX_EXT];
			_splitpath( m_pParentWnd->m_cEditDoc.m_szFilePath, NULL, NULL, szFileName, szExt );
			wsprintf( szWork, "%s%s", szFileName, szExt );
		}
		hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );
		::TextOut( hdc,
			m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft ,
			nDirectY * ( m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop ),
			szWork, lstrlen( szWork )
		);
		::SelectObject( hdc, hFontZenOld );
		/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
		DrawPageText(
			hdc,
			m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft ,
			m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop+ 2 * ( m_pPrintSetting->m_nPrintFontHeight + (m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100) ),
			i,
			hFontZen,
			&cDlgPrinting
		);
		/* �y�[�W�ԍ��̕\�� */
		wsprintf( szWork, "- %d -", i + 1 );
		::TextOut( hdc,
//			  m_pPrintSetting->m_nPrintMarginLX
//			- m_nPreview_PaperOffsetLeft
			+ m_nPreview_PaperWidth /* m_nPreview_PaperWidth */ / 2
			- (5 * 10),
			nDirectY * ( m_nPreview_PaperAllHeight /* m_nPreview_PaperAllHeight */ - m_nPreview_PaperOffsetTop /* m_nPreview_PaperOffsetTop */ - m_pPrintSetting->m_nPrintMarginBY - m_pPrintSetting->m_nPrintFontHeight/* - m_nPreview_PaperOffsetTop*/ ),
//TEST			nDirectY * ( m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop ),
			szWork, lstrlen( szWork )
		);
	//	for( i = 0; i < 10; ++i ){
	//		::MoveToEx( hdc, 1000, nDirectY * ( 1000 * i ) , NULL );
	//		::LineTo( hdc, 5000, nDirectY * ( 1000 * i ) );
	//	}
		/* ����t�H���g���� */
		::SelectObject( hdc, hFontOld );
		/* ��� �y�[�W�I�� */
		CPrint::PrintEndPage( hdc );

		/* ���f�{�^�������`�F�b�N */
		if( cDlgPrinting.IsCanceled() ){
			break;
		}
	}
	sprintf( szProgress, "%d/%d", i - nFrom - 1/*i*/, nTo - nFrom + 1/*m_nAllPageNum*/ );
	::SetDlgItemText( cDlgPrinting.m_hWnd, IDC_STATIC_PROGRESS, szProgress );

	/* ��� �W���u�I�� */
	CPrint::PrintClose( hPrinter, hdc );

	::DeleteObject( hFontZen );
	::DeleteObject( hFont );


	::EnableWindow( m_pParentWnd->m_hWnd, TRUE );
	cDlgPrinting.CloseDialog( 0 );

	return;
}

/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`�� */
void CPrintPreview::DrawPageText(
	HDC				hdc,
	int				nOffX,
	int				nOffY,
	int				nPageNum,
	HFONT			hFontZen,
	CDlgCancel*		pCDlgCancel
)
{
	int				nDirectY;
	int				nLineCols;
	const char*		pLine;
	int				nLineLen;
	TEXTMETRIC		tm;
	int				nAscent;
	int				nAscentZen;
	HFONT			hFontZenOld;

	nDirectY = -1;

	int				nLineHeight = m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 );


	::GetTextMetrics( hdc, &tm );
	nAscent = tm.tmAscent;
	hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );
	::GetTextMetrics( hdc, &tm );
	nAscentZen = tm.tmAscent;
	::SelectObject( hdc, hFontZenOld );

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
					nDirectY * ( nOffY + nLineHeight * i + ( m_pPrintSetting->m_nPrintFontHeight - nAscent ) ), 0, NULL,
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
		si.fMask = SIF_ALL;
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
		si.fMask = SIF_ALL;
		/* �����X�N���[���o�[ */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL;
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
	HFONT		hFontZenOld;
	TEXTMETRIC	tm;
	int			nAscent;
	int			nAscentZen;

	/* ���p�����̃A�Z���g�i�������j���擾 */
	//	���p�����p�t�H���g���I������Ă��邱�Ƃ����҂��Ă���B
	::GetTextMetrics( hdc, &tm );
	nAscent = tm.tmAscent;

	/* �S�p�����̃A�Z���g�i�������j���擾 */
	hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );
	::GetTextMetrics( hdc, &tm );
	nAscentZen = tm.tmAscent;

	//	�t�H���g�𔼊p�ɖ߂��Ă����B
	::SelectObject( hdc, hFontZenOld );


	/*	pLine���X�L�������āA���p�����͔��p�����ł܂Ƃ߂āA�S�p�����͑S�p�����ł܂Ƃ߂ĕ`�悷��B
	*/
	int			nPosX = 0;	//	TAB��W�J������̃o�C�g���ŁA�e�L�X�g�̉��o�C�g�ڂ܂ŕ`�悵�����H
	int			nBgn = 0;	//	TAB��W�J����O�̃o�C�g���ŁApLine�̉��o�C�g�ڂ܂ŕ`�悵�����H
	int			i;			//	pLine�̉������ڂ��X�L�����H
	int			nTabSpace = m_pParentWnd->m_cEditDoc.GetDocumentAttribute().m_nTabSpace;	//	TAB��

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
							y - ( m_pPrintSetting->m_nPrintFontHeight - nAscent ),
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
					hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );
					::ExtTextOut(
						hdc,
						x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
						y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentZen ),
						0,
						NULL,
						&pLine[nBgn], i - nBgn,
						m_pnDx
					);
					::SelectObject( hdc, hFontZenOld );
					nPosX += ( i - nBgn );
				}
				nBgn = i;
				nPreviousChars = MODE_SINGLEBYTE;
				if( TAB == pLine[i] ){	//	�^�u����
					nPosX += ( nTabSpace - nPosX % nTabSpace );
					nBgn = i + 1;
				}
			}
		}else{	//	2�o�C�g����
			if( MODE_SINGLEBYTE == nPreviousChars ){
				if( 0 < i - nBgn ){
					::ExtTextOut(
						hdc,
						x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
						y - ( m_pPrintSetting->m_nPrintFontHeight - nAscent ),
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
			hFontZenOld = (HFONT)::SelectObject( hdc, hFontZen );
			::ExtTextOut(
				hdc,
				x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
				y - ( m_pPrintSetting->m_nPrintFontHeight - nAscentZen ),
				0,
				NULL,
				&pLine[nBgn], i - nBgn,
				m_pnDx
			);
			::SelectObject( hdc, hFontZenOld );
		}else{				/* 1�o�C�g�����`��B�t�H���g�͕W����1�o�C�g�����p�B	*/
			::ExtTextOut(
				hdc,
				x + nPosX * m_pPrintSetting->m_nPrintFontWidth,
				y - ( m_pPrintSetting->m_nPrintFontHeight - nAscent ),
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
	si.fMask = SIF_ALL;
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
	si.fMask = SIF_ALL;
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
BOOL CALLBACK CPrintPreview::PrintPreviewBar_DlgProc(
	HWND hwndDlg,	// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
)
{
	CPrintPreview* pCPrintPreview;
	switch( uMsg ){
	case WM_INITDIALOG:
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );
		return TRUE;
	default:
		pCPrintPreview = ( CPrintPreview* )::GetWindowLong( hwndDlg, DWL_USER );
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
BOOL CPrintPreview::DispatchEvent_PPB(
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
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );
		return TRUE;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID			= LOWORD(wParam);	/* ����ID�A�R���g���[��ID �܂��̓A�N�Z�����[�^ID */
		hwndCtl		= (HWND) lParam;	/* �R���g���[���̃n���h�� */
		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
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
			break;
		}
	}
	return FALSE;
}
