/*!	@file
	@brief ����v���r���[�Ǘ��N���X

	@author YAZAKI
	@date 2002/1/11 �V�K�쐬
*/
/*
	Copyright (C) 2001, Stonee, jepro, genta
	Copyright (C) 2002, YAZAKI, aroka, MIK, genta
	Copyright (C) 2003, genta, �����, ������, KEITA
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2006, ryoji, Moca
	Copyright (C) 2008, nasukoji
	Copyright (C) 2012, ossan (ossan@ongs.net)
	Copyright (C) 2013, Uchi

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

#include "StdAfx.h"
#include "CPrintPreview.h"
#include "uiparts/HandCursor.h"
#include "doc/layout/CLayout.h"
#include "window/CEditWnd.h"
#include "dlg/CDlgCancel.h" /// 2002/2/3 aroka from here
#include "dlg/CDlgInput1.h" /// 2007.02.11 Moca
#include "CEditApp.h"
#include "util/window.h"
#include "util/shell.h"
#include "env/CSakuraEnvironment.h"
// CColorStrategy�͖{����CEditView���K�v�����ACEditWnd.h�������include�ς�
#include "view/colors/CColorStrategy.h"
#include "sakura_rc.h"

using namespace std;

#define MIN_PREVIEW_ZOOM 10
#define MAX_PREVIEW_ZOOM 400

#define		LINE_RANGE_X	48		/* ���������̂P��̃X�N���[���� */
#define		LINE_RANGE_Y	24		/* ���������̂P��̃X�N���[���� */

#define		PAGE_RANGE_X	160		/* ���������̂P��̃y�[�W�X�N���[���� */
#define		PAGE_RANGE_Y	160		/* ���������̂P��̃y�[�W�X�N���[���� */

#define		COMPAT_BMP_BASE     1   /* COMPAT_BMP_SCALE�s�N�Z�����𕡎ʂ����ʃs�N�Z���� */
#define		COMPAT_BMP_SCALE    2   /* �݊�BMP��COMPAT_BMP_BASE�ɑ΂���{��(1�ȏ�̐����{) */

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
	m_pParentWnd( pParentWnd ),
	m_hdcCompatDC( NULL ),			// �ĕ`��p�R���p�`�u��DC
	m_hbmpCompatBMP( NULL ),		// �ĕ`��p������BMP
	m_hbmpCompatBMPOld( NULL ),		// �ĕ`��p������BMP(OLD)
	m_nbmpCompatScale( COMPAT_BMP_BASE )
{
	/* ����p�̃��C�A�E�g���̍쐬 */
	m_pLayoutMgr_Print = new CLayoutMgr;

	/* ����v���r���[ �R���g���[�� �쐬 */
	CreatePrintPreviewControls();

	// �ĕ`��p�R���p�`�u��DC
	HDC hdc = ::GetDC( pParentWnd->GetHwnd() );
	m_hdcCompatDC = ::CreateCompatibleDC( hdc );
	::ReleaseDC( pParentWnd->GetHwnd(), hdc );
}

CPrintPreview::~CPrintPreview()
{
	/* ����v���r���[ �R���g���[�� �j�� */
	DestroyPrintPreviewControls();
	
	/* ����p�̃��C�A�E�g���̍폜 */
	delete m_pLayoutMgr_Print;
	
	/* �t�H���g���L���b�V����ҏW���[�h�ɖ߂� */
	SelectCharWidthCache( CWM_FONT_EDIT, CWM_CACHE_NEUTRAL );

	// 2006.08.17 Moca CompatDC�폜�BCEditWnd����ڐ�
	// �ĕ`��p������BMP
	if( m_hbmpCompatBMP != NULL ){
		// �ĕ`��p������BMP(OLD)
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );
		::DeleteObject( m_hbmpCompatBMP );
	}
	// �ĕ`��p�R���p�`�u��DC
	if( m_hdcCompatDC != NULL ){
		::DeleteDC( m_hdcCompatDC );
	}
}

/*!	����v���r���[���́AWM_PAINT������

	@date 2007.02.11 Moca �v���r���[�����炩�ɂ���@�\�D
		�g��`�悵�Ă���k�����邱�ƂŃA���`�G�C���A�X���ʂ��o���D
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
	HDC				hdc = m_hdcCompatDC;	//	�e�E�B���h�E��ComatibleDC�ɕ`��

	/* ����v���r���[ ����o�[ */
	
	// BMP�͂��Ƃŏk���R�s�[����̂Ŋg�債�č�悷��K�v����

	// �N���C�A���g�̈�S�̂��O���[�œh��Ԃ�
	{
		RECT bmpRc;
		::GetClientRect( hwnd, &bmpRc );
		bmpRc.right  = (bmpRc.right  * m_nbmpCompatScale) / COMPAT_BMP_BASE;
		bmpRc.bottom = (bmpRc.bottom * m_nbmpCompatScale) / COMPAT_BMP_BASE;
		::FillRect( hdc, &bmpRc, (HBRUSH)::GetStockObject( GRAY_BRUSH ) );
	}

	// �c�[���o�[���� -> nToolBarHeight
	int nToolBarHeight = 0;
	if( NULL != m_hwndPrintPreviewBar ){
		RECT rc;
		::GetWindowRect( m_hwndPrintPreviewBar, &rc );
		nToolBarHeight = rc.bottom - rc.top;
	}

	// �v�����^���̕\�� -> IDD_PRINTPREVIEWBAR�E���STATIC��
	TCHAR	szText[1024];
	::DlgItem_SetText(
		m_hwndPrintPreviewBar,
		IDC_STATIC_PRNDEV,
		m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName
	);

	// �v�f���̕\�� -> IDD_PRINTPREVIEWBAR�E����STATIC��
	TCHAR	szPaperName[256];
	CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , szPaperName );
	auto_sprintf(
		szText,
		_T("%ts  %ts"),
		szPaperName,
		(m_pPrintSetting->m_mdmDevMode.dmOrientation & DMORIENT_LANDSCAPE) ? _T("��") : _T("�c")
	);
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_PAPER, szText );

	// �o�b�N�O���E���h ���[�h��ύX
	::SetBkMode( hdc, TRANSPARENT );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        �}�b�s���O                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �}�b�s���O���[�h�̕ύX
	int nMapModeOld =
	::SetMapMode( hdc, MM_LOMETRIC );
	::SetMapMode( hdc, MM_ANISOTROPIC );

	// �o�͔{���̕ύX
	SIZE			sz;
	::GetWindowExtEx( hdc, &sz );
	int nCx = sz.cx;
	int nCy = sz.cy;
	nCx = (int)( ((long)nCx) * 100L / ((long)m_nPreview_Zoom) );
	nCy = (int)( ((long)nCy) * 100L / ((long)m_nPreview_Zoom) );
	// ��掞�́A COMPAT_BMP_SCALE/COMPAT_BMP_BASE�{�̍��W (SetWindowExtEx�͋t�Ȃ̂Ŕ��΂ɂȂ�)
	nCx = (nCx * COMPAT_BMP_BASE) / m_nbmpCompatScale;
	nCy = (nCy * COMPAT_BMP_BASE) / m_nbmpCompatScale;
	::SetWindowExtEx( hdc, nCx, nCy, &sz );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �t�H���g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �t�H���g�쐬
	CreateFonts( hdc );
	// ����p���p�t�H���g�ɐݒ肵�A�ȑO�̃t�H���g��ێ�
	HFONT	hFontOld = (HFONT)::SelectObject( hdc, m_hFontHan );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           ���_                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// ����E�B���h�E�̉��ɕ������W���_���ړ�
	POINT poViewPortOld;
	::SetViewportOrgEx(
		hdc,
		((-1 * m_nPreviewHScrollPos) * m_nbmpCompatScale) / COMPAT_BMP_BASE, 
		((nToolBarHeight + m_nPreviewVScrollPos) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
		&poViewPortOld
	);


	// �ȉ� 0.1mm���W�Ń����_�����O

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           �w�i                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �p���̕`��
	int	nDirectY = -1;	//	Y���W�̉����v���X�����ɂ��邽�߁H
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft,
		nDirectY * ( m_nPreview_ViewMarginTop ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth + 1,
		nDirectY * (m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight + 1 )
	);

	// �}�[�W���g�̕\��
	CGraphics gr(hdc);
	gr.SetPen( RGB(128,128,128) ); // 2006.08.14 Moca 127��128�ɕύX
	::Rectangle( hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		nDirectY * ( m_nPreview_ViewMarginTop + m_pPrintSetting->m_nPrintMarginTY ),
		m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - m_pPrintSetting->m_nPrintMarginRX + 1,
		nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - m_pPrintSetting->m_nPrintMarginBY )
	);
	gr.ClearPen();

	::SetTextColor( hdc, RGB( 0, 0, 0 ) );

	RECT cRect;	/* ���̑傫��������킷RECT */
	cRect.left   = m_nPreview_ViewMarginLeft +                             m_pPrintSetting->m_nPrintMarginLX + 5;
	cRect.right  = m_nPreview_ViewMarginLeft + m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + 5);
	cRect.top    = nDirectY * ( m_nPreview_ViewMarginTop +                              m_pPrintSetting->m_nPrintMarginTY + 5);
	cRect.bottom = nDirectY * ( m_nPreview_ViewMarginTop + m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + 5));


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         �e�L�X�g                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int nHeaderHeight = CPrint::CalcHeaderHeight( m_pPrintSetting );

	// �w�b�_
	if( nHeaderHeight ){
		DrawHeaderFooter( hdc, cRect, true );
	}


	CColorStrategy* pStrategyStart = DrawPageTextFirst( m_nCurPageNum );

	// ���/����v���r���[ �y�[�W�e�L�X�g�̕`��
	DrawPageText(
		hdc,
		m_nPreview_ViewMarginLeft + m_pPrintSetting->m_nPrintMarginLX,
		m_nPreview_ViewMarginTop  + m_pPrintSetting->m_nPrintMarginTY + nHeaderHeight*2,
		m_nCurPageNum,
		NULL,
		pStrategyStart
	);

	// �t�b�^
	if( CPrint::CalcFooterHeight( m_pPrintSetting ) ){
		DrawHeaderFooter( hdc, cRect, false );
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          ��n��                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	����O�̃t�H���g�ɖ߂�
	::SelectObject( hdc, hFontOld );

	// �}�b�s���O���[�h�̕ύX
	::SetMapMode( hdc, nMapModeOld );

	//	����p�t�H���g�j��
	DestroyFonts();

	// �������W���_�����Ƃɖ߂�
	::SetViewportOrgEx( hdc, poViewPortOld.x, poViewPortOld.y, NULL );


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ����ʂ֓]��                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// �������c�b�𗘗p�����ĕ`��̏ꍇ�̓������c�b�ɕ`�悵�����e����ʂփR�s�[����
	RECT rc;
	rc = ps.rcPaint;
	::DPtoLP( hdc, (POINT*)&rc, 2 );
	if( 1 == (m_nbmpCompatScale / COMPAT_BMP_BASE) ){
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
	}
	else{
		int stretchModeOld = SetStretchBltMode( hdcOld, STRETCH_HALFTONE );
		::StretchBlt(
			hdcOld,
			ps.rcPaint.left,
			ps.rcPaint.top,
			ps.rcPaint.right - ps.rcPaint.left,
			ps.rcPaint.bottom - ps.rcPaint.top,
			hdc,
			(ps.rcPaint.left * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			(ps.rcPaint.top * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.right - ps.rcPaint.left) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			((ps.rcPaint.bottom - ps.rcPaint.top) * m_nbmpCompatScale) / COMPAT_BMP_BASE,
			SRCCOPY
		);
		SetStretchBltMode( hdcOld, stretchModeOld );
	}
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

	HDC			hdc = ::GetDC( m_pParentWnd->GetHwnd() );
	int nMapModeOld = ::SetMapMode( hdc, MM_LOMETRIC );
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
	if( m_hbmpCompatBMP != NULL ){
		::SelectObject( m_hdcCompatDC, m_hbmpCompatBMPOld );	/* �ĕ`��p�������a�l�o(OLD) */
		::DeleteObject( m_hbmpCompatBMP );
	}
	// 2007.02.11 Moca �v���r���[�����炩�ɂ���
	// Win9x�ł� �����BMP�͍쐬�ł��Ȃ����Ƃ�
	// StretchBlt��STRETCH_HALFTONE�����T�|�[�g�ł���̂� Win2K �ȏ�݂̂ŗL���ɂ���B
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hwndPrintPreviewBar, IDC_CHECK_ANTIALIAS ) &&
			IsWin2000_or_later() ){
		m_nbmpCompatScale = COMPAT_BMP_SCALE;
	}else{
		// Win9x: BASE = SCALE �� 1:1
		m_nbmpCompatScale = COMPAT_BMP_BASE;
	}
	m_hbmpCompatBMP = ::CreateCompatibleBitmap( hdc, (cx * m_nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE,
		(cy * m_nbmpCompatScale + COMPAT_BMP_BASE - 1) / COMPAT_BMP_BASE);
	m_hbmpCompatBMPOld = (HBITMAP)::SelectObject( m_hdcCompatDC, m_hbmpCompatBMP );

	::SetMapMode( hdc, nMapModeOld );

	::ReleaseDC( m_pParentWnd->GetHwnd(), hdc );

	/* ����v���r���[�F�r���[��(�s�N�Z��) */
	m_nPreview_ViewWidth = abs( po.x );
	
	/* ����v���r���[�F�r���[����(�s�N�Z��) */
	m_nPreview_ViewHeight = abs( po.y );
	
	/* ����v���r���[ �X�N���[���o�[������ */
	InitPreviewScrollBar();
	
	/* ����v���r���[ �X�N���[���o�[�̏����� */
	
	m_pParentWnd->SetDragPosOrg(CMyPoint(0,0));
	m_pParentWnd->SetDragMode(true);
	OnMouseMove( 0, MAKELONG( 0, 0 ) );
	m_pParentWnd->SetDragMode(false);
	//	SizeBox���e�X�g
	if( NULL != m_hwndSizeBox ){
		if( wParam == SIZE_MAXIMIZED ){
			::ShowWindow( m_hwndSizeBox, SW_HIDE );
		}else
		if( wParam == SIZE_RESTORED ){
			if( ::IsZoomed( m_pParentWnd->GetHwnd() ) ){
				::ShowWindow( m_hwndSizeBox, SW_HIDE );
			}else{
				::ShowWindow( m_hwndSizeBox, SW_SHOW );
			}
		}else{
			::ShowWindow( m_hwndSizeBox, SW_SHOW );
		}
	}
	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
	return 0L;
}

/*!
	@date 2006.08.14 Moca SB_TOP, SB_BOTTOM�ւ̑Ή�
*/
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
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, &si );
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
	// 2006.08.14 Moca SB_TOP, SB_BOTTOM�ւ̑Ή�
	case SB_TOP:
		nMove = -1 * nNowPos;
		break;
	case SB_BOTTOM:
		nMove = si.nMax - nNowPos;
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
		::ScrollWindowEx( m_pParentWnd->GetHwnd(), 0, nMove, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

/*!
	@date 2006.08.14 Moca SB_LEFT, SB_RIGHT�ւ̑Ή�
*/
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
	si.cbSize = sizeof( si );
	si.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	::GetScrollInfo( hwndScrollBar, SB_CTL, &si );
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
	// 2006.08.14 Moca SB_LEFT, SB_RIGHT�ւ̑Ή�
	case SB_LEFT:
		nMove = -1 * nNowPos;
		break;
	case SB_RIGHT:
		nMove = si.nMax - nNowPos;
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
		::ScrollWindowEx( m_pParentWnd->GetHwnd(), nMove, 0, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
	}
	return 0;
}

LRESULT CPrintPreview::OnMouseMove( WPARAM wParam, LPARAM lParam )
{
	/* ��J�[�\�� */
	SetHandCursor();		// Hand Cursor��ݒ� 2013/1/29 Uchi
	if( !m_pParentWnd->GetDragMode() ){
		return 0;
	}
//	WPARAM		fwKeys = wParam;			// key flags
	int			xPos = LOWORD( lParam );	// horizontal position of cursor
	int			yPos = HIWORD( lParam );	// vertical position of cursor
	RECT		rc;
	GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	POINT		po;
	po.x = xPos;
	po.y = yPos;
	if( !PtInRect( &rc, po ) ){	//	�v���r���[�����`�F�b�N�B
		return 0;
	}

	//	Y��
	SCROLLINFO	siV;
	siV.cbSize = sizeof( siV );
	siV.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndVScrollBar, SB_CTL, &siV );
	int			nMoveY;
	if( m_SCROLLBAR_VERT ){
		int		nNowPosY = GetScrollPos( m_hwndVScrollBar, SB_CTL );
		nMoveY = m_pParentWnd->GetDragPosOrg().y - yPos;

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
	siH.cbSize = sizeof( siH );
	siH.fMask = SIF_PAGE | SIF_POS | SIF_RANGE | SIF_TRACKPOS;
	GetScrollInfo( m_hwndHScrollBar, SB_CTL, &siH );
	int			nMoveX;
	if( m_SCROLLBAR_HORZ ){
		int		nNowPosX = GetScrollPos( m_hwndHScrollBar, SB_CTL );
		nMoveX = m_pParentWnd->GetDragPosOrg().x - xPos;
		
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

	m_pParentWnd->SetDragPosOrg(CMyPoint(xPos,yPos));
	/* �`�� */
	ScrollWindowEx( m_pParentWnd->GetHwnd(), nMoveX, nMoveY, NULL, NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE );
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
		::PostMessageAny( m_pParentWnd->GetHwnd(), WM_VSCROLL, MAKELONG( nScrollCode, 0 ), (LPARAM)m_hwndVScrollBar );

		/* �������̃��[�U�[������\�ɂ��� */
		if( !::BlockingHook( NULL ) ){
			return -1;
		}
	}
	return 0;
}

void CPrintPreview::OnChangePrintSetting( void )
{
	HDC		hdc = ::GetDC( m_pParentWnd->GetHwnd() );
	::SetMapMode( hdc, MM_LOMETRIC ); //MM_HIMETRIC ���ꂼ��̘_���P�ʂ́A0.01 mm �Ƀ}�b�v����܂�
	::SetMapMode( hdc, MM_ANISOTROPIC );

	::EnumFontFamilies(
		hdc,
		NULL,
		(FONTENUMPROC)CPrintPreview::MyEnumFontFamProc,
		(LPARAM)this
	);

	// 2009.08.08 ����ŗp���T�C�Y�A���w�肪�����Ȃ����Ή� syat
	/* DEVMODE�\���̂��ݒ肳��Ă��Ȃ����������̃v�����^��ݒ� */
	if( m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName[0] == L'\0' ){
		GetDefaultPrinterInfo();
	}

	/* ����v���r���[�\����� */
	m_nPreview_LineNumberColumns = 0;	/* �s�ԍ��G���A�̕�(������) */

	/* �s�ԍ���\�����邩 */
	if( m_pPrintSetting->m_bPrintLineNumber ){
		/* �s�ԍ��\���ɕK�v�Ȍ������v�Z */
		m_nPreview_LineNumberColumns = m_pParentWnd->GetActiveView().GetTextArea().DetectWidthOfLineNumberArea_calculate();
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
	TCHAR	szErrMsg[1024];
	if( !m_cPrint.GetPrintMetrics(
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
			TCHAR	szPaperNameOld[256];
			TCHAR	szPaperNameNew[256];
			/* �p���̖��O���擾 */
			CPrint::GetPaperName( m_pPrintSetting->m_nPrintPaperSize , szPaperNameOld );
			CPrint::GetPaperName( m_pPrintSetting->m_mdmDevMode.dmPaperSize , szPaperNameNew );

			TopWarningMessage(
				m_pParentWnd->GetHwnd(),
				_T("���݂̃v�����^ %ts �ł́A\n�w�肳�ꂽ�p�� %ts �͎g�p�ł��܂���B\n")
				_T("���p�\�ȗp�� %ts �ɕύX���܂����B"),
				m_pPrintSetting->m_mdmDevMode.m_szPrinterDeviceName,
				szPaperNameOld,
				szPaperNameNew
			);
		}
	}
	/* ���݂̃y�[�W�ݒ�́A�p���T�C�Y�Ɨp�������𔽉f������(�G���[��A4�c�ɂȂ����ꍇ���l������if���̊O�ֈړ� 2003.07.03 �����) */
	m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
	m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;	// �p�������̔��f�Y����C�� 2003/07/03 �����

	m_nPreview_ViewMarginLeft = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */
	m_nPreview_ViewMarginTop = 8 * 10;		/* ����v���r���[�F�r���[���[�Ɨp���̊Ԋu(1/10mm�P��) */

	/* �s������̕�����(�s�ԍ�����) */
	m_bPreview_EnableColumns = CLayoutInt( CPrint::CalculatePrintableColumns( m_pPrintSetting, m_nPreview_PaperAllWidth, m_nPreview_LineNumberColumns ) );	/* �󎚉\����/�y�[�W */
	/* �c�����̍s�� */
	m_bPreview_EnableLines = CPrint::CalculatePrintableLines( m_pPrintSetting, m_nPreview_PaperAllHeight );			/* �󎚉\�s��/�y�[�W */

	// �󎚉\�̈悪�Ȃ��ꍇ�͈���v���r���[���I������ 2013.5.10 aroka
	if( m_bPreview_EnableColumns == 0 || m_bPreview_EnableLines == 0 ){
		CEditWnd* pcEditWnd = m_pParentWnd;
		pcEditWnd->PrintPreviewModeONOFF();
		pcEditWnd->SendStatusMessage( _T("����y�[�W�ݒ�G���[:�󎚉\�̈悪����܂���") );
		return;
	}

	/* ����p�̃��C�A�E�g�Ǘ����̏����� */
	m_pLayoutMgr_Print->Create( &m_pParentWnd->GetDocument(), &m_pParentWnd->GetDocument().m_cDocLineMgr );

	/* ����p�̃��C�A�E�g���̕ύX */
//	STypeConfig& ref = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute();
	STypeConfig ref = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute();
	ref.m_nMaxLineKetas = 		m_bPreview_EnableColumns;
	ref.m_bWordWrap =			m_pPrintSetting->m_bPrintWordWrap;	/* �p�����[�h���b�v������ */
	//	Sep. 23, 2002 genta LayoutMgr�̒l���g��
	ref.m_nTabSpace =			m_pParentWnd->GetDocument().m_cLayoutMgr.GetTabSpace();

	//@@@ 2002.09.22 YAZAKI
	ref.m_cLineComment.CopyTo(0, L"", -1);	/* �s�R�����g�f���~�^ */
	ref.m_cLineComment.CopyTo(1, L"", -1);	/* �s�R�����g�f���~�^2 */
	ref.m_cLineComment.CopyTo(2, L"", -1);	/* �s�R�����g�f���~�^3 */	//Jun. 01, 2001 JEPRO �ǉ�
	ref.m_cBlockComments[0].SetBlockCommentRule(L"", L"");	/* �u���b�N�R�����g�f���~�^ */
	ref.m_cBlockComments[1].SetBlockCommentRule(L"", L"");	/* �u���b�N�R�����g�f���~�^2 */

	ref.m_nStringType =			0;		/* �������؂�L���G�X�P�[�v���@  0=[\"][\'] 1=[""][''] */
	ref.m_ColorInfoArr[COLORIDX_COMMENT].m_bDisp = false;
	ref.m_ColorInfoArr[COLORIDX_SSTRING].m_bDisp = false;
	ref.m_ColorInfoArr[COLORIDX_WSTRING].m_bDisp = false;
	ref.m_bKinsokuHead = m_pPrintSetting->m_bPrintKinsokuHead,	/* �s���֑����� */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuTail = m_pPrintSetting->m_bPrintKinsokuTail,	/* �s���֑����� */	//@@@ 2002.04.08 MIK
	ref.m_bKinsokuRet = m_pPrintSetting->m_bPrintKinsokuRet,	/* ���s�������Ԃ牺���� */	//@@@ 2002.04.13 MIK
	ref.m_bKinsokuKuto = m_pPrintSetting->m_bPrintKinsokuKuto,	/* ��Ǔ_���Ԃ牺���� */	//@@@ 2002.04.17 MIK
	m_pLayoutMgr_Print->SetLayoutInfo( true, ref );
	m_nAllPageNum = (WORD)((Int)m_pLayoutMgr_Print->GetLineCount() / ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu ));		/* �S�y�[�W�� */
	if( 0 < m_pLayoutMgr_Print->GetLineCount() % ( m_bPreview_EnableLines * m_pPrintSetting->m_nPrintDansuu ) ){
		m_nAllPageNum++;
	}
	if( m_nAllPageNum <= m_nCurPageNum ){	/* ���݂̃y�[�W */
		m_nCurPageNum = 0;
	}

	/* WM_SIZE ���� */
	RECT	rc;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	OnSize( SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
	::ReleaseDC( m_pParentWnd->GetHwnd(), hdc );
	/* �v���r���[ �y�[�W�w�� */
	OnPreviewGoPage( m_nCurPageNum );
	return;
}

/*! @brief �y�[�W�ԍ����ڎw��ɂ��W�����v

	@author Moca
**/
void CPrintPreview::OnPreviewGoDirectPage( void )
{
	const int  INPUT_PAGE_NUM_LEN = 12;

	CDlgInput1 cDlgInputPage;
	TCHAR      szMessage[512];
	TCHAR      szPageNum[INPUT_PAGE_NUM_LEN];
	
	auto_sprintf( szMessage, _T("�\������y�[�W�ԍ����w�肵�Ă��������B(1 - %d)") , m_nAllPageNum );
	auto_sprintf( szPageNum, _T("%d"), m_nCurPageNum + 1 );

	BOOL bDlgInputPageResult=cDlgInputPage.DoModal(
		CEditApp::getInstance()->GetAppInstance(),
		m_hwndPrintPreviewBar, 
		_T("�v���r���[�y�[�W�w��"),
		szMessage,
		INPUT_PAGE_NUM_LEN,
		szPageNum
	);
	if( FALSE != bDlgInputPageResult ){
		int i;
		int nPageNumLen = _tcslen( szPageNum );
		for( i = 0; i < nPageNumLen;  i++ ){
			if( !(_T('0') <= szPageNum[i] &&  szPageNum[i] <= _T('9')) ){
				return;
			}
		}
		int nPage = _ttoi( szPageNum );
		OnPreviewGoPage( nPage - 1 );
	}
}

void CPrintPreview::OnPreviewGoPage( int nPage )
{
	if( m_nAllPageNum <= nPage ){	/* ���݂̃y�[�W */
		nPage = m_nAllPageNum - 1;
	}
	if( 0 > nPage ){				/* ���݂̃y�[�W */
		nPage = 0;
	}
	m_nCurPageNum = (short)nPage;

	//	2008.01.29 nasukoji	���������2���̎�����ł��Ȃ��Ȃ邱�Ƃւ̑Ώ��iSetFocus���ړ��j
	//	2008.02.01 genta : �{�^���̃t�H�[�J�X�����̓���ɂȂ�悤�ɂ��邽�߁C
	//		�O�{�^����Disable�����ֈړ������D
	//		����ł��Ȃ����ۂ́u���ցv��Disable�ɂ��ւ�炸�t�H�[�J�X��^���Ă������߁D
	//		���E�O�ǂ�����C�{�^���L�������t�H�[�J�X�ړ����{�^���������̏��ɂ���
	if( 0 < m_nCurPageNum ){
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

	if( 0 == m_nCurPageNum ){
		//	�ŏ��̃y�[�W�̂Ƃ��́A�O�̃y�[�W�{�^�����I�t�B
		//	Jul. 18, 2001 genta Focus�̂���Window��Disable�ɂ���Ƒ���ł��Ȃ��Ȃ�̂����
		::SetFocus( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_NEXTPAGE ));
		::EnableWindow( ::GetDlgItem( m_hwndPrintPreviewBar, IDC_BUTTON_PREVPAGE ), FALSE );
	}
	wchar_t	szEdit[1024];
	auto_sprintf( szEdit, L"%d/%d��", m_nCurPageNum + 1, m_nAllPageNum );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_PAGENUM, szEdit );

	auto_sprintf( szEdit, L"%d %%", m_nPreview_Zoom );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
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

	wchar_t	szEdit[1024];
	auto_sprintf( szEdit, L"%d %%", m_nPreview_Zoom );
	::DlgItem_SetText( m_hwndPrintPreviewBar, IDC_STATIC_ZOOM, szEdit );

	/* WM_SIZE ���� */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc1 );
	OnSize( SIZE_RESTORED, MAKELONG( rc1.right - rc1.left, rc1.bottom - rc1.top ) );

	/* ����v���r���[ �X�N���[���o�[������ */
	InitPreviewScrollBar();

	/* �ĕ`�� */
	::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
	return;
}


/*!
	���炩
	�`�F�b�N���A2�{(COMPAT_BMP_SCALE/COMPAT_BMP_BASE)�T�C�Y�Ń����_�����O����
*/
void CPrintPreview::OnCheckAntialias( void )
{
	/* WM_SIZE ���� */
	RECT	rc;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	OnSize( SIZE_RESTORED, MAKELONG( rc.right - rc.left, rc.bottom - rc.top ) );
}



/*!
	���
*/
void CPrintPreview::OnPrint( void )
{
	HDC			hdc;
	TCHAR		szJobName[256 + 1];
	TCHAR		szProgress[100];
	TCHAR		szErrMsg[1024];
	int			nDirectY = -1;
	int			i;
	HFONT		hFontOld;	//	OnPrint�ȑO�̃t�H���g

	if( 0 == m_nAllPageNum ){
		TopWarningMessage( m_pParentWnd->GetHwnd(), _T("�������y�[�W������܂���B") );
		return;
	}

	/* �v�����^�ɓn���W���u���𐶐� */
	if( ! m_pParentWnd->GetDocument().m_cDocFile.GetFilePathClass().IsValidPath() ){	/* ���ݕҏW���̃t�@�C���̃p�X */
		_tcscpy( szJobName, _T("����") );
	}else{
		TCHAR	szFileName[_MAX_FNAME];
		TCHAR	szExt[_MAX_EXT];
		_tsplitpath( m_pParentWnd->GetDocument().m_cDocFile.GetFilePath(), NULL, NULL, szFileName, szExt );
		auto_snprintf_s( szJobName, _countof(szJobName), _T("%ts%ts"), szFileName, szExt );
	}

	/* ����͈͂��w��ł���v�����^�_�C�A���O���쐬 */
	//	2003.05.02 �����
	PRINTDLG pd;
	memset_raw( &pd, 0, sizeof(pd) );
#ifndef _DEBUG
// Debug���[�h�ŁAhwndOwner���w�肷��ƁAWin2000�ł͗�����̂ŁE�E�E
	pd.hwndOwner = m_pParentWnd->GetHwnd();
#endif
	pd.nMinPage = 1;
	pd.nMaxPage = m_nAllPageNum;
	pd.nFromPage = 1;
	pd.nToPage = m_nAllPageNum;
	pd.Flags = PD_ALLPAGES | PD_NOSELECTION | PD_USEDEVMODECOPIESANDCOLLATE;
	if( !m_cPrint.PrintDlg(&pd, &m_pPrintSetting->m_mdmDevMode) ){
		return;
	}
	// ����J�n�y�[�W�ƁA����y�[�W�����m�F
	WORD		nFrom;
	WORD		nNum;
	if( 0 != (pd.Flags & PD_PAGENUMS) ){	// 2003.05.02 �����
		nFrom = pd.nFromPage - 1;
		nNum  = pd.nToPage - nFrom;
	}else{
		nFrom = 0;
		nNum  = m_nAllPageNum;
	}

	/* ����ߒ���\�����āA�L�����Z�����邽�߂̃_�C�A���O���쐬 */
	CDlgCancel	cDlgPrinting;
	cDlgPrinting.DoModeless( CEditApp::getInstance()->GetAppInstance(), m_pParentWnd->GetHwnd(), IDD_PRINTING );
	::DlgItem_SetText( cDlgPrinting.GetHwnd(), IDC_STATIC_JOBNAME, szJobName );
	::DlgItem_SetText( cDlgPrinting.GetHwnd(), IDC_STATIC_PROGRESS, _T("") );	// XPS�Ή� 2013/5/8 Uchi

	/* �e�E�B���h�E�𖳌��� */
	::EnableWindow( m_pParentWnd->GetHwnd(), FALSE );

	/* ��� �W���u�J�n */
	if( !m_cPrint.PrintOpen(
		szJobName,
		&m_pPrintSetting->m_mdmDevMode,	/* �v�����^�ݒ� DEVMODE�p*/
		&hdc,
		szErrMsg						/* �G���[���b�Z�[�W�i�[�ꏊ */
	) ){
//		MYTRACE( _T("%ts\n"), szErrMsg );
	}

	// ����p���p�t�H���g�ƁA����p�S�p�t�H���g���쐬
	CreateFonts( hdc );
	// ���݂̃t�H���g������p���p�t�H���g�ɐݒ聕�ȑO�̃t�H���g��ێ�
	hFontOld = (HFONT)::SelectObject( hdc, m_hFontHan );

	/* ���̑傫��������킷RECT��ݒ� */
	RECT cRect;
	cRect.left   =                             m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft + 5;
	cRect.right  = m_nPreview_PaperAllWidth - (m_pPrintSetting->m_nPrintMarginRX + m_nPreview_PaperOffsetLeft + 5);
	cRect.top    = nDirectY * (                              m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop + 5 );
	cRect.bottom = nDirectY * ( m_nPreview_PaperAllHeight - (m_pPrintSetting->m_nPrintMarginBY + m_nPreview_PaperOffsetTop + 5) );

	/* �w�b�_�E�t�b�^��$p��W�J���邽�߂ɁAm_nCurPageNum��ێ� */
	WORD	nCurPageNumOld = m_nCurPageNum;
	CColorStrategy* pStrategy = DrawPageTextFirst( m_nCurPageNum );
	for( i = 0; i < nNum; ++i ){
		m_nCurPageNum = nFrom + (WORD)i;

		/* ����ߒ���\�� */
		//	Jun. 18, 2001 genta �y�[�W�ԍ��\���̌v�Z�~�X�C��
		auto_sprintf( szProgress, _T("%d/%d"), i + 1, nNum );
		::DlgItem_SetText( cDlgPrinting.GetHwnd(), IDC_STATIC_PROGRESS, szProgress );

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
		::SelectObject( hdc, m_hFontHan );
		//	To Here Jun. 26, 2003 ����� / ������

		int nHeaderHeight = CPrint::CalcHeaderHeight( m_pPrintSetting );

		// �w�b�_���
		if( nHeaderHeight ){
			DrawHeaderFooter( hdc, cRect, true );
		}

		const CLayoutInt	nPageTopLineNum = CLayoutInt( ((nFrom + i) * m_pPrintSetting->m_nPrintDansuu) * m_bPreview_EnableLines );
		const CLayout*		pcPageTopLayout = m_pLayoutMgr_Print->SearchLineByLayoutY( nPageTopLineNum );
		if (m_pPrintSetting->m_bColorPrint
			&& !(i == 0)
			&& pcPageTopLayout->GetLogicOffset() == 0) {
			pStrategy = m_pool->GetStrategyByColor(pcPageTopLayout->GetColorTypePrev());
			m_pool->NotifyOnStartScanLogic();
			if (pStrategy)	pStrategy->InitStrategyStatus();
		}
		// ���/����v���r���[ �y�[�W�e�L�X�g�̕`��
		pStrategy = DrawPageText(
			hdc,
			m_pPrintSetting->m_nPrintMarginLX - m_nPreview_PaperOffsetLeft ,
			m_pPrintSetting->m_nPrintMarginTY - m_nPreview_PaperOffsetTop + nHeaderHeight*2,
			nFrom + i,
			&cDlgPrinting,
			pStrategy
		);

		// �t�b�^���
		if( CPrint::CalcFooterHeight( m_pPrintSetting ) ){
			DrawHeaderFooter( hdc, cRect, false );
		}

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

	//	����p�t�H���g�j��
	DestroyFonts();

	::EnableWindow( m_pParentWnd->GetHwnd(), TRUE );
	cDlgPrinting.CloseDialog( 0 );

	m_nCurPageNum = nCurPageNumOld;

	// ������I�������APreview���甲���� 2003.05.02 �����
	m_pParentWnd->PrintPreviewModeONOFF();
	return;
}


/*! ���/����v���r���[ �w�b�_��t�b�^�̕`��
*/
void CPrintPreview::DrawHeaderFooter( HDC hdc, const CMyRect& rect, bool bHeader )
{
	bool		bFontSetting = (bHeader ? m_pPrintSetting->m_lfHeader.lfFaceName[0] : m_pPrintSetting->m_lfFooter.lfFaceName[0]) != _T('\0');
	const int	nWorkLen = 1024;
	wchar_t		szWork[1024 + 1];
	int			nLen;

	if (bFontSetting) {
		// �t�H���g�w��L��
		HFONT	hFontForce = NULL;
		HFONT	hFontOld = NULL;

		// �t�H���g�쐬
		LOGFONT	lf = (bHeader ? m_pPrintSetting->m_lfHeader : m_pPrintSetting->m_lfFooter);
		lf.lfHeight = -( bHeader ? m_pPrintSetting->m_nHeaderPointSize : m_pPrintSetting->m_nFooterPointSize) * 254 / 720;	// �t�H���g�̃T�C�Y�v�Z(pt->1/10mm)
		hFontForce = ::CreateFontIndirect( &lf );

		// �t�H���g�ݒ�
		hFontOld = (HFONT)::SelectObject( hdc, hFontForce );

		// TextMetric�̎擾
		TEXTMETRIC	tm;
		::GetTextMetrics( hdc, &tm );

		// Y���W�
		int		nY = bHeader ? rect.top : rect.bottom + tm.tmHeight;

		// ����
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_LEFT] : m_pPrintSetting->m_szFooterForm[POS_LEFT],
			szWork, nWorkLen);
		::ExtTextOutW_AnyBuild(
			hdc,
			rect.left,
			nY,
			0,
			NULL,
			szWork,
			wcslen( szWork ),
			NULL
		);

		// ������
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_CENTER] : m_pPrintSetting->m_szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		SIZE	Size;
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W( hdc, szWork, nLen, &Size);		//�e�L�X�g��
		::ExtTextOutW_AnyBuild(
			hdc,
			( rect.right + rect.left - Size.cx) / 2,
			nY,
			0,
			NULL,
			szWork,
			nLen,
			NULL
		);

		// �E��
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_RIGHT] : m_pPrintSetting->m_szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		nLen = wcslen(szWork);
		::GetTextExtentPoint32W( hdc, szWork, nLen, &Size);		//�e�L�X�g��
		::ExtTextOutW_AnyBuild(
			hdc,
			rect.right - Size.cx,
			nY,
			0,
			NULL,
			szWork,
			nLen,
			NULL
		);
		// �t�H���g�̖߂�
		::SelectObject( hdc, hFontOld );
		::DeleteObject( hFontForce );
	}
	else {
		int		nTextWidth;

		// �����Ԋu
		int nDx = m_pPrintSetting->m_nPrintFontWidth;

		// Y���W�
		int nY = nY = bHeader ? rect.top : rect.bottom + m_pPrintSetting->m_nPrintFontHeight;

		// ����
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_LEFT] : m_pPrintSetting->m_szFooterForm[POS_LEFT],
			szWork, nWorkLen);
		nLen = wcslen( szWork );
		Print_DrawLine(
			hdc,
			CMyPoint(
				rect.left,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			CLayoutInt(0)
		);

		// ������
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_CENTER] : m_pPrintSetting->m_szFooterForm[POS_CENTER],
			szWork, nWorkLen);
		nLen = wcslen( szWork );
		nTextWidth = CTextMetrics::CalcTextWidth2(szWork, nLen, nDx); //�e�L�X�g��
		Print_DrawLine(
			hdc,
			CMyPoint(
				( rect.right + rect.left - nTextWidth) / 2,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			CLayoutInt(0)
		);

		// �E��
		CSakuraEnvironment::ExpandParameter(
			bHeader ? m_pPrintSetting->m_szHeaderForm[POS_RIGHT] : m_pPrintSetting->m_szFooterForm[POS_RIGHT],
			szWork, nWorkLen);
		nLen = wcslen( szWork );
		nTextWidth = CTextMetrics::CalcTextWidth2(szWork, nLen, nDx); //�e�L�X�g��
		Print_DrawLine(
			hdc,
			CMyPoint(
				rect.right - nTextWidth,
				nY
			),
			szWork,
			nLen,
			0,
			nLen,
			CLayoutInt(0)
		);
	}
}

/* ���/����v���r���[ �y�[�W�e�L�X�g�̐F��������
	�ŏ��̃y�[�W�p
	@date 2013.05.19 Moca �V�K�ǉ� 
*/
CColorStrategy* CPrintPreview::DrawPageTextFirst(int nPageNum)
{
	// �y�[�W�g�b�v�̐F�w����擾
	CColorStrategy*	pStrategy = NULL;
	if (m_pPrintSetting->m_bColorPrint) {
		m_pool = CColorStrategyPool::getInstance();
		m_pool->SetCurrentView(&(m_pParentWnd->GetActiveView()));

		const CLayoutInt	nPageTopLineNum = CLayoutInt( (nPageNum * m_pPrintSetting->m_nPrintDansuu) * m_bPreview_EnableLines );
		const CLayout*		pcPageTopLayout = m_pLayoutMgr_Print->SearchLineByLayoutY( nPageTopLineNum );
		const CLogicInt		nPageTopOff = pcPageTopLayout->GetLogicOffset();

		// �y�[�W�g�b�v�̕����s�̐擪������
		while (pcPageTopLayout->GetLogicOffset()) {
			pcPageTopLayout = pcPageTopLayout->GetPrevLayout();
		}

		// �_���s�擪��CColorStrategy�擾
		pStrategy = m_pool->GetStrategyByColor( pcPageTopLayout->GetColorTypePrev() );
		m_pool->NotifyOnStartScanLogic();
		if (pStrategy)	pStrategy->InitStrategyStatus();
		if (nPageTopOff) {
			CStringRef&	csr = pcPageTopLayout->GetDocLineRef()->GetStringRefWithEOL();
			CLogicInt	iLogic;
			for ( iLogic = 0; iLogic < nPageTopOff; ++iLogic) {
				pStrategy = GetColorStrategy( csr, iLogic, pStrategy );
			}
		}
	}
	return pStrategy;
}


/* ���/����v���r���[ �y�[�W�e�L�X�g�̕`��
	DrawPageText�ł́A�s�ԍ����i���p�t�H���g�Łj����B
	�{����Print_DrawLine�ɂ��C��
	@date 2006.08.14 Moca ���ʎ��̂����肾���ƁA�R�[�h�̐��� 
	@date 2013.05.19 Moca �F����������pStrategy���y�[�W���܂����ŗ��p����
*/
CColorStrategy* CPrintPreview::DrawPageText(
	HDC				hdc,
	int				nOffX,
	int				nOffY,
	int				nPageNum,
	CDlgCancel*		pCDlgCancel,
	CColorStrategy* pStrategyStart
)
{
	int				nDirectY = -1;

	const int		nLineHeight = m_pPrintSetting->m_nPrintFontHeight + ( m_pPrintSetting->m_nPrintFontHeight * m_pPrintSetting->m_nPrintLineSpacing / 100 );
	// �i�ƒi�̊Ԋu�̕�
	const int		nDanWidth = (Int)m_bPreview_EnableColumns * m_pPrintSetting->m_nPrintFontWidth + m_pPrintSetting->m_nPrintDanSpace;
	// �s�ԍ��̕�
	const int		nLineNumWidth = m_nPreview_LineNumberColumns * m_pPrintSetting->m_nPrintFontWidth;

	/* ���p�t�H���g�̏����擾�����p�t�H���g�ɐݒ� */

	// �y�[�W�g�b�v�̐F�w����擾
	CColorStrategy*	pStrategy = pStrategyStart;

	int				nDan;	//	�i���J�E���^
	int				i;		//	�s���J�E���^
	for( nDan = 0; nDan < m_pPrintSetting->m_nPrintDansuu; ++nDan ){
		// �{��1���ڂ̍����̍��W(�s�ԍ�������ꍇ�͂��̍��W��荶��)
		const int nBasePosX = nOffX + nDanWidth * nDan + nLineNumWidth * (nDan + 1);
		
		for( i = 0; i < m_bPreview_EnableLines; ++i ){
			if( NULL != pCDlgCancel ){
				/* �������̃��[�U�[������\�ɂ��� */
				if( !::BlockingHook( pCDlgCancel->GetHwnd() ) ){
					return NULL;
				}
			}

			/*	���ݕ`�悵�悤�Ƃ��Ă���s�̕����s���i�܂�Ԃ����ƂɃJ�E���g�����s���j
				�֌W������̂́A
				�u�y�[�W���inPageNum�j�v
				�u�i���im_pPrintSetting->m_nPrintDansuu�j�v
				�u�i����1�̂Ƃ��ɁA1�y�[�W������ɉ��s���邩�im_bPreview_EnableLines�j�v
			*/
			const CLayoutInt nLineNum = CLayoutInt( (nPageNum * m_pPrintSetting->m_nPrintDansuu + nDan) * m_bPreview_EnableLines + i );
			const CLayout*	pcLayout = m_pLayoutMgr_Print->SearchLineByLayoutY( nLineNum );
			if( NULL == pcLayout ){
				break;
			}
			/* �s�ԍ���\�����邩 */
			if( m_pPrintSetting->m_bPrintLineNumber ){
				wchar_t		szLineNum[64];	//	�s�ԍ�������B
				/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				if( m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_bLineNumIsCRLF ){
					/* �_���s�ԍ��\�����[�h */
					if( 0 != pcLayout->GetLogicOffset() ){ //�܂�Ԃ����C�A�E�g�s
						wcscpy( szLineNum, L" " );
					}else{
						_itow( pcLayout->GetLogicLineNo() + 1, szLineNum, 10 );	/* �Ή�����_���s�ԍ� */
					}
				}
				else{
					/* �����s(���C�A�E�g�s)�ԍ��\�����[�h */
					_itow( (Int)nLineNum + 1, szLineNum, 10 );
				}

				/* �s�ԍ���؂�  0=�Ȃ� 1=�c�� 2=�C�� */
				if( 2 == m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_nLineTermType ){
					wchar_t szLineTerm[2];
					szLineTerm[0] = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_cLineTermChar;	/* �s�ԍ���؂蕶�� */
					szLineTerm[1] = L'\0';
					wcscat( szLineNum, szLineTerm );
				}
				else{
					wcscat( szLineNum, L" " );
				}

				//������
				const int nLineCols = wcslen( szLineNum );

				//�����Ԋu�z��𐶐�
				vector<int> vDxArray;
				const int* pDxArray = CTextMetrics::GenerateDxArray(&vDxArray, szLineNum, nLineCols, m_pPrintSetting->m_nPrintFontWidth);

				ApiWrap::ExtTextOutW_AnyBuild(
					hdc,
					nBasePosX - nLineCols * m_pPrintSetting->m_nPrintFontWidth,
					nDirectY * ( nOffY + nLineHeight * i + ( m_pPrintSetting->m_nPrintFontHeight - m_nAscentHan ) ),
					0,
					NULL,
					szLineNum,
					nLineCols,
					pDxArray
				);
			}

			const int nLineLen = pcLayout->GetLengthWithoutEOL();
			if( 0 == nLineLen ){
				continue;
			}

			// �����s���̐F�w����擾
			if (m_pPrintSetting->m_bColorPrint
				&& !(nDan == 0 && i == 0)
				&& pcLayout->GetLogicOffset() == 0) {
				pStrategy = m_pool->GetStrategyByColor(pcLayout->GetColorTypePrev());
				m_pool->NotifyOnStartScanLogic();
				if (pStrategy)	pStrategy->InitStrategyStatus();
			}
			// ����^�v���r���[ �s�`��
			pStrategy = Print_DrawLine(
				hdc,
				CMyPoint(
					nBasePosX,
					nDirectY * ( nOffY + nLineHeight * i )
				),
				pcLayout->GetDocLineRef()->GetPtr(),	// pcLayout->GetPtr(),
				(Int)pcLayout->GetDocLineRef()->GetLengthWithEOL(),
				(Int)pcLayout->GetLogicOffset(),
				nLineLen,
				pcLayout->GetIndent(), // 2006.05.16 Add Moca. ���C�A�E�g�C���f���g�����炷�B
				m_pPrintSetting->m_bColorPrint ? pcLayout : NULL,
				pStrategy
			);
		}

		// 2006.08.14 Moca �s�ԍ����c���̏ꍇ��1�x�Ɉ���
		if( m_pPrintSetting->m_bPrintLineNumber &&
				1 == m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_nLineTermType ){
			// �c���͖{���ƍs�ԍ��̌���1���̒��S�ɍ�悷��(��ʍ��ł́A�E�l��)
			::MoveToEx( hdc,
				nBasePosX - (m_pPrintSetting->m_nPrintFontWidth / 2 ),
				nDirectY * nOffY,
				NULL );
			::LineTo( hdc,
				nBasePosX - (m_pPrintSetting->m_nPrintFontWidth / 2 ),
				nDirectY * ( nOffY + nLineHeight * i )
			);
		}
	}
	return pStrategy;
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
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc );
	cx = rc.right - rc.left;
	cy = rc.bottom - rc.top - nToolBarHeight;

	if( NULL != m_hwndVScrollBar ){
		/* �����X�N���[���o�[ */
		si.cbSize = sizeof( si );
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

/*! ����^�v���r���[ �s�`��
	@param[in] nIndent �s���܂�Ԃ��C���f���g����

	@date 2006.08.14 Moca   �܂�Ԃ��C���f���g��������ɔ��f�����悤��
	@date 2007.08    kobake �@�B�I��UNICODE��
	@date 2007.12.12 kobake �S�p�t�H���g�����f����Ă��Ȃ������C��
*/
CColorStrategy* CPrintPreview::Print_DrawLine(
	HDC				hdc,
	POINT			ptDraw, //!< �`����W�BHDC�����P�ʁB
	const wchar_t*	pLine,
	int				nDocLineLen,
	int				nLineStart,
	int				nLineLen,
	CLayoutInt		nIndent,  // 2006.08.14 Moca �ǉ�
	const CLayout*	pcLayout,	//!< �F�t�pLayout
	CColorStrategy*	pStrategyStart
)
{
	if (nLineLen == 0) {
		return pStrategyStart;
	}

	/*	pLine���X�L�������āA���p�����͔��p�����ł܂Ƃ߂āA�S�p�����͑S�p�����ł܂Ƃ߂ĕ`�悷��B
	*/

	//�����Ԋu
	int nDx = m_pPrintSetting->m_nPrintFontWidth;

	//�^�u���擾
	CLayoutInt nTabSpace = m_pParentWnd->GetDocument().m_cLayoutMgr.GetTabSpace(); //	Sep. 23, 2002 genta LayoutMgr�̒l���g��

	//�����Ԋu�z��𐶐�
	vector<int> vDxArray;
	const int* pDxArray = CTextMetrics::GenerateDxArray(
		&vDxArray,
		pLine + nLineStart,
		nLineLen,
		nDx,
		(Int)nTabSpace,
		(Int)nIndent
	);

	int nBgnLogic = nLineStart;	// TAB��W�J����O�̃o�C�g���ŁApLine�̉��o�C�g�ڂ܂ŕ`�悵�����H
	int iLogic;					// pLine�̉������ڂ��X�L�����H
	CLayoutInt nLayoutX = nIndent;	// TAB��W�J������̃o�C�g���ŁA�e�L�X�g�̉��o�C�g�ڂ܂ŕ`�悵�����H

	//�����픻��t���O
	int nKind     = 0; //0:���p 1:�S�p 2:�^�u
	int nKindLast = 2; //���O��nKind���

	// �F�ݒ�	2012-03-07 ossan
	CStringRef cStringLine( pLine, nDocLineLen );
	CColorStrategy* pStrategy = pStrategyStart;
	CColorStrategy*	pStrategyLast = (CColorStrategy*)-1;

	for( iLogic = nLineStart; iLogic < nLineStart + nLineLen; 
			++iLogic, nKindLast = nKind, pStrategyLast = pStrategy ){
		//�����̎��
		if(pLine[iLogic]==WCODE::TAB){
			nKind = 2;
		}
		else if(WCODE::IsHankaku(pLine[iLogic])){
			nKind = 0;
		}
		else{
			nKind = 1;
		}

		pStrategy = pcLayout ? GetColorStrategy(cStringLine, iLogic, pStrategy) : NULL;

		// �^�u�����o�� or ������(�S�p�^���p)�̋��E or �F�w��̋��E
		if (nKind != nKindLast || pStrategyLast != pStrategy) {
			//iLogic�̒��O�܂ł�`��
			if ( 0 < iLogic - nBgnLogic ) {
				Print_DrawBlock(
					hdc,
					ptDraw,		//!< �`����W�BHDC�����P�ʁB
					pLine + nLineStart,
					iLogic - nBgnLogic,
					nKindLast,
					pcLayout,	//!< �F�ݒ�pLayout
					pStrategyLast,
					nBgnLogic - nLineStart,
					nLayoutX,
					nDx,
					pDxArray
				);

				//���i��
				if (nKindLast == 2) {
					nLayoutX += ( nTabSpace - nLayoutX % nTabSpace )
						+ nTabSpace * (iLogic - nBgnLogic - 1);
				}
				else{
					int		nIncrement = 0;
					for (int i = nBgnLogic - nLineStart; i < iLogic - nLineStart; i++) {
						nIncrement += pDxArray[i];
					}
					nLayoutX += CLayoutInt(nIncrement/nDx);
				}
				//���W�b�N�i��
				nBgnLogic = iLogic;
			}
		}
	}

	//�c���`��
	if (0 < nLineStart + nLineLen - nBgnLogic) {
		Print_DrawBlock(
			hdc,
			ptDraw,		//!< �`����W�BHDC�����P�ʁB
			pLine + nLineStart,
			nLineStart + nLineLen - nBgnLogic,
			nKindLast,
			pcLayout,	//!< �F�ݒ�pLayout
			pStrategyLast,
			nBgnLogic - nLineStart,
			nLayoutX,
			nDx,
			pDxArray
		);
	}

	//�t�H���g���� (���p) �ɖ߂�
	::SelectObject( hdc, m_hFontHan );

	//�F�����ɖ߂�	2012-03-07 ossan
	if (pcLayout) {
		int nColorIdx = ToColorInfoArrIndex(COLORIDX_TEXT);
		if (-1 != nColorIdx) {
			const ColorInfo& info = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx];
			::SetTextColor(hdc, info.m_colTEXT);
//			::SetBkColor(hdc, info.m_colBACK);
		}
	}

	return pStrategy;
}

/*! ����^�v���r���[ �u���b�N�`��
	@param[in] 

	@date 2013.05.01 Uchi Print_DrawLine����؂�o��
*/
void CPrintPreview::Print_DrawBlock(
	HDC				hdc,
	POINT			ptDraw,		//!< �`����W�BHDC�����P�ʁB
	const wchar_t*	pPhysicalLine,
	int				nBlockLen,	// iLogic - nBgnLogic
	int				nKind,
	const CLayout*	pcLayout,	//!< �F�ݒ�pLayout
	const CColorStrategy*	pStrategy,
	int				nBgnPhysical,	// nBgnLogic - nLineStart
	CLayoutInt		nLayoutX,
	int				nDx,
	const int*		pDxArray
)
{
	if (nKind == 2 && pcLayout == NULL) {
		// TAB�̓J���[�Ŗ�����Έ󎚕s�v
		return;
	}
	HFONT	hFont = (nKind == 1 ? m_hFontZen : m_hFontHan);
	// �F�ݒ�
	if (pcLayout) {
		int nColorIdx = ToColorInfoArrIndex( pStrategy ? pStrategy->GetStrategyColor() : COLORIDX_TEXT );
		if (-1 != nColorIdx) {
			const ColorInfo& info = m_pParentWnd->GetDocument().m_cDocType.GetDocumentAttribute().m_ColorInfoArr[nColorIdx];
			if (nKind == 2 && !info.m_bUnderLine) {
				// TAB�͉�����������Έ󎚕s�v
				return;
			}
			if (info.m_bBoldFont)
				if (info.m_bUnderLine)	hFont = (nKind == 1 ? m_hFontZen_bu: m_hFontHan_bu);	// �����A����
				else					hFont = (nKind == 1 ? m_hFontZen_b : m_hFontHan_b);		// ����
			else
				if (info.m_bUnderLine)	hFont = (nKind == 1 ? m_hFontZen_u : m_hFontHan_u);		// ����
			//	else					hFont = (nKind == 1 ? m_hFontZen   : m_hFontHan);		// �W��
			::SetTextColor( hdc, info.m_colTEXT);
//			::SetBkColor( hdc, info.m_colBACK);
		}
	}
	::SelectObject( hdc, hFont );
	::ExtTextOutW_AnyBuild(
		hdc,
		ptDraw.x + (Int)nLayoutX * nDx,
		ptDraw.y - ( m_pPrintSetting->m_nPrintFontHeight - (nKind == 1 ? m_nAscentZen : m_nAscentHan) ),
		0,
		NULL,
		&pPhysicalLine[nBgnPhysical],
		nBlockLen,
		&pDxArray[nBgnPhysical]
	);
}

/*! �w�胍�W�b�N�ʒu��CColorStrategy���擾
	@param[in] 

	@date 2013.05.01 Uchi �V�K�쐬
*/
CColorStrategy* CPrintPreview::GetColorStrategy(
	const CStringRef&	cStringLine,
	int					iLogic,
	CColorStrategy*		pStrategy
)
{
	if (pStrategy) {
		if (pStrategy->EndColor(cStringLine, iLogic)) {
			pStrategy = NULL;
		}
	}
	if (!pStrategy) {
		for (int i=0; i < m_pool->GetStrategyCount(); i++) {
			if (m_pool->GetStrategy(i)->BeginColor(cStringLine, iLogic)) {
				pStrategy = m_pool->GetStrategy(i);
				break;
			}
		}
	}

	return pStrategy;
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
void CPrintPreview::SetPreviewFontHan( const LOGFONT* lf )
{
	m_lfPreviewHan = *lf;

	//	PrintSetting����R�s�[
	m_lfPreviewHan.lfHeight			= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth			= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy(m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan);

	SelectCharWidthCache( CWM_FONT_PRINT, CWM_CACHE_LOCAL );
	InitCharWidthCache( m_lfPreviewHan, CWM_FONT_PRINT );
}

void CPrintPreview::SetPreviewFontZen( const LOGFONT* lf )
{
	m_lfPreviewZen = *lf;
	//	PrintSetting����R�s�[
	m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy(m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );
}

int CALLBACK CPrintPreview::MyEnumFontFamProc(
	ENUMLOGFONT*	pelf,		// pointer to logical-font data
	NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
	int				nFontType,	// type of font
	LPARAM			lParam 		// address of application-defined data
)
{
	CPrintPreview* pCPrintPreview = (CPrintPreview*)lParam;
	if( 0 == _tcscmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceHan ) ){
		pCPrintPreview->SetPreviewFontHan(&pelf->elfLogFont);
	}
	if( 0 == _tcscmp( pelf->elfLogFont.lfFaceName, pCPrintPreview->m_pPrintSetting->m_szPrintFontFaceZen ) ){
		pCPrintPreview->SetPreviewFontZen(&pelf->elfLogFont);
	}

	return 1;
}

/*!
	����v���r���[�ɕK�v�ȃR���g���[�����쐬����
*/
void CPrintPreview::CreatePrintPreviewControls( void )
{
	/* ����v���r���[ ����o�[ */
	m_hwndPrintPreviewBar = ::CreateDialogParam(
		CEditApp::getInstance()->GetAppInstance(),				// handle to application instance
		MAKEINTRESOURCE( IDD_PRINTPREVIEWBAR ),				// identifies dialog box template name
		m_pParentWnd->GetHwnd(),							// handle to owner window
		CPrintPreview::PrintPreviewBar_DlgProc,	// pointer to dialog box procedure
		(LPARAM)this
	);

	/* �c�X�N���[���o�[�̍쐬 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles			*/
		_T("SCROLLBAR"),						/* scroll bar control class		*/
		NULL,								/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->GetHwnd(),								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		CEditApp::getInstance()->GetAppInstance(),						/* instance owning this window	*/
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
		_T("SCROLLBAR"),						/* scroll bar control class		*/
		NULL,								/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles			*/
		0,									/* horizontal position			*/
		0,									/* vertical position			*/
		200,								/* width of the scroll bar		*/
		CW_USEDEFAULT,						/* default height				*/
		m_pParentWnd->GetHwnd(),								/* handle of main window		*/
		(HMENU) NULL,						/* no menu for a scroll bar		*/
		CEditApp::getInstance()->GetAppInstance(),						/* instance owning this window	*/
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
		_T("SCROLLBAR"),										/* scroll bar control class		*/
		NULL,												/* text for window title bar	*/
		WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles			*/
		0,													/* horizontal position			*/
		0,													/* vertical position			*/
		200,												/* width of the scroll bar		*/
		CW_USEDEFAULT,										/* default height				*/
		m_pParentWnd->GetHwnd(), 											/* handle of main window		*/
		(HMENU) NULL,										/* no menu for a scroll bar 	*/
		CEditApp::getInstance()->GetAppInstance(),										/* instance owning this window	*/
		(LPVOID) NULL										/* pointer not needed			*/
	);
	::ShowWindow( m_hwndPrintPreviewBar, SW_SHOW );


	/* WM_SIZE ���� */
	RECT		rc1;
	::GetClientRect( m_pParentWnd->GetHwnd(), &rc1 );
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
		// 2007.02.11 Moca WM_INIT��DispatchEvent_PPB���ĂԂ悤��
		pCPrintPreview = ( CPrintPreview* )lParam;
		if( NULL != pCPrintPreview ){
			return pCPrintPreview->DispatchEvent_PPB( hwndDlg, uMsg, wParam, lParam );
		}
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
// IDD_PRINTPREVIEWBAR
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



	switch( uMsg ){

	case WM_INITDIALOG:
		// 2007.02.11 Moca DWLP_USER�ݒ�͕s�v
		//// Modified by KEITA for WIN64 2003.9.6
		//::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		{
			if( IsWin2000_or_later() ){
				::EnableWindow( ::GetDlgItem(hwndDlg, IDC_CHECK_ANTIALIAS), TRUE );
			}
		}
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
				memset_raw( &pd, 0, sizeof(pd) );
				pd.Flags = PD_PRINTSETUP | PD_NONETWORKBUTTON;
				pd.hwndOwner = m_pParentWnd->GetHwnd();
				if (m_cPrint.PrintDlg( &pd, &m_pPrintSetting->m_mdmDevMode )) {
					// �p���T�C�Y�Ɨp�������𔽉f������ 2003.05.03 �����
					m_pPrintSetting->m_nPrintPaperSize = m_pPrintSetting->m_mdmDevMode.dmPaperSize;
					m_pPrintSetting->m_nPrintPaperOrientation = m_pPrintSetting->m_mdmDevMode.dmOrientation;
					/* ����v���r���[ �X�N���[���o�[������ */
					InitPreviewScrollBar();
					OnChangePrintSetting();
					::InvalidateRect( m_pParentWnd->GetHwnd(), NULL, TRUE );
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
			//From Here 2007.02.11 Moca �_�C���N�g�W�����v����уA���`�G�C���A�X
			case IDC_BUTTON_DIRECTPAGE:
				OnPreviewGoDirectPage( );
				break;
			case IDC_CHECK_ANTIALIAS:
				OnCheckAntialias();
				break;
			//To Here 2007.02.11 Moca
			case IDC_BUTTON_HELP:
				/* ����v���r���[�̃w���v */
				//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
				MyWinHelp( hwndDlg, CEditApp::getInstance()->GetHelpFilePath(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_PRINT_PREVIEW) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
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



// ����p�t�H���g���쐬����
void CPrintPreview::CreateFonts( HDC hdc )
{
	LOGFONT	lf;
	TEXTMETRIC	tm;

	// ����p���p�t�H���g���쐬 -> m_hFontHan
	m_lfPreviewHan.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
	m_lfPreviewHan.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
	_tcscpy( m_lfPreviewHan.lfFaceName, m_pPrintSetting->m_szPrintFontFaceHan );
	m_hFontHan	= CreateFontIndirect( &m_lfPreviewHan );
	if (m_pPrintSetting->m_bColorPrint) {
		lf = m_lfPreviewHan;	lf.lfWeight = FW_BOLD;
		m_hFontHan_b	= CreateFontIndirect( &lf );		// ����
		lf = m_lfPreviewHan;							lf.lfUnderline = true;
		m_hFontHan_u	= CreateFontIndirect( &lf );		// ����
		lf = m_lfPreviewHan;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
		m_hFontHan_bu	= CreateFontIndirect( &lf );		// �����A����
	}
#ifdef _DEEBUG
	else {
		m_hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
	}
#endif
	// ���p�����̃A�Z���g�i�������j���擾
	::SelectObject( hdc, m_hFontHan );
	::GetTextMetrics( hdc, &tm );
	m_nAscentHan = tm.tmAscent;

	// ����p�S�p�t�H���g���쐬 -> m_hFontZen
	if (auto_strcmp(m_pPrintSetting->m_szPrintFontFaceHan, m_pPrintSetting->m_szPrintFontFaceZen)) {
		m_lfPreviewZen.lfHeight	= m_pPrintSetting->m_nPrintFontHeight;
		m_lfPreviewZen.lfWidth	= m_pPrintSetting->m_nPrintFontWidth;
		_tcscpy( m_lfPreviewZen.lfFaceName, m_pPrintSetting->m_szPrintFontFaceZen );
		m_hFontZen	= CreateFontIndirect( &m_lfPreviewZen );
		if (m_pPrintSetting->m_bColorPrint) {
			lf = m_lfPreviewZen;	lf.lfWeight = FW_BOLD;
			m_hFontZen_b	= CreateFontIndirect( &lf );		// ����
			lf = m_lfPreviewZen;							lf.lfUnderline = true;
			m_hFontZen_u	= CreateFontIndirect( &lf );		// ����
			lf = m_lfPreviewZen;	lf.lfWeight = FW_BOLD;	lf.lfUnderline = true;
			m_hFontZen_bu	= CreateFontIndirect( &lf );		// �����A����
		}
#ifdef _DEEBUG
		else {
			m_hFontHan_b  = m_hFontHan_u  = m_hFontHan_bu = NULL;
		}
#endif
		// �S�p�����̃A�Z���g�i�������j���擾
		::SelectObject( hdc, m_hFontZen );
		::GetTextMetrics( hdc, &tm );
		m_nAscentZen = tm.tmAscent;
	}
	else {
		// ���p�S�p�����t�H���g
		m_hFontZen		= m_hFontHan;
		m_hFontZen_b	= m_hFontHan_b;		// ����
		m_hFontZen_u	= m_hFontHan_u;		// ����
		m_hFontZen_bu	= m_hFontHan_bu;	// �����A����
		m_nAscentZen	= m_nAscentHan;		// �S�p�����̃A�Z���g
	}
}

// ����p�t�H���g��j������
void CPrintPreview::DestroyFonts()
{
	if (m_hFontZen != m_hFontHan) {
		::DeleteObject( m_hFontZen );
		if (m_hFontZen_b) {
			::DeleteObject( m_hFontZen_b );
			::DeleteObject( m_hFontZen_u );
			::DeleteObject( m_hFontZen_bu );
		}
	}
	::DeleteObject( m_hFontHan );
	if (m_hFontHan_b) {
		::DeleteObject( m_hFontHan_b );
		::DeleteObject( m_hFontHan_u );
		::DeleteObject( m_hFontHan_bu );
	}
#ifdef _DEEBUG
	m_hFontHan = m_hFontHan_b = m_hFontHan_u = m_hFontHan_bu =
	m_hFontZen = m_hFontZen_b = m_hFontZen_u = m_hFontZen_bu = NULL;
#endif
}
