/*!	@file
	@brief �����E�B���h�E�̊Ǘ�

	@author kobake
	@date	2008/04/14 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2010, Moca
	Copyright (C) 2012, ryoji, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CEditView.h"
#include "CRuler.h"
#include "env/DLLSHAREDATA.h"
#include "window/CEditWnd.h"
#include "types/CTypeSupport.h"
#include <limits.h>

/*! �X�N���[���o�[�쐬
	@date 2006.12.19 ryoji �V�K�쐬�iCEditView::Create���番���j
*/
BOOL CEditView::CreateScrollBar()
{
	SCROLLINFO	si;

	/* �X�N���[���o�[�̍쐬 */
	m_hwndVScrollBar = ::CreateWindowEx(
		0L,									/* no extended styles */
		_T("SCROLLBAR"),					/* scroll bar control class */
		NULL,								/* text for window title bar */
		WS_VISIBLE | WS_CHILD | SBS_VERT,	/* scroll bar styles */
		0,									/* horizontal position */
		0,									/* vertical position */
		200,								/* width of the scroll bar */
		CW_USEDEFAULT,						/* default height */
		GetHwnd(),								/* handle of main window */
		(HMENU) NULL,						/* no menu for a scroll bar */
		G_AppInstance(),						/* instance owning this window */
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
	if( GetDllShareData().m_Common.m_sWindow.m_bScrollBarHorz && !m_bMiniMap ){	/* �����X�N���[���o�[���g�� */
		m_hwndHScrollBar = ::CreateWindowEx(
			0L,									/* no extended styles */
			_T("SCROLLBAR"),					/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_HORZ,	/* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(),								/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
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
	if( GetDllShareData().m_Common.m_sWindow.m_nFUNCKEYWND_Place == 0 ){	/* �t�@���N�V�����L�[�\���ʒu�^0:�� 1:�� */
		m_hwndSizeBox = ::CreateWindowEx(
			WS_EX_CONTROLPARENT/*0L*/, 			/* no extended styles */
			_T("SCROLLBAR"),					/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD | SBS_SIZEBOX | SBS_SIZEGRIP, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}else{
		m_hwndSizeBox = ::CreateWindowEx(
			0L, 								/* no extended styles */
			_T("STATIC"),						/* scroll bar control class */
			NULL,								/* text for window title bar */
			WS_VISIBLE | WS_CHILD/* | SBS_SIZEBOX | SBS_SIZEGRIP*/, /* scroll bar styles */
			0,									/* horizontal position */
			0,									/* vertical position */
			200,								/* width of the scroll bar */
			CW_USEDEFAULT,						/* default height */
			GetHwnd(), 							/* handle of main window */
			(HMENU) NULL,						/* no menu for a scroll bar */
			G_AppInstance(),						/* instance owning this window */
			(LPVOID) NULL						/* pointer not needed */
		);
	}
	return TRUE;
}



/*! �X�N���[���o�[�j��
	@date 2006.12.19 ryoji �V�K�쐬
*/
void CEditView::DestroyScrollBar()
{
	if( m_hwndVScrollBar )
	{
		::DestroyWindow( m_hwndVScrollBar );
		m_hwndVScrollBar = NULL;
	}

	if( m_hwndHScrollBar )
	{
		::DestroyWindow( m_hwndHScrollBar );
		m_hwndHScrollBar = NULL;
	}

	if( m_hwndSizeBox )
	{
		::DestroyWindow( m_hwndSizeBox );
		m_hwndSizeBox = NULL;
	}
}

/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[�������s��

	@date 2004.09.11 genta �X�N���[���s����Ԃ��悤�ɁD
		���g�p��hwndScrollBar�����폜�D
*/
CLayoutInt CEditView::OnVScroll( int nScrollCode, int nPos )
{
	CLayoutInt nScrollVal = CLayoutInt(0);

	// nPos 32bit�Ή�
	if( nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION ){
		if( m_hwndVScrollBar ){
			HWND hWndScroll = m_hwndVScrollBar;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			::GetScrollInfo(hWndScroll, SB_CTL, &info);
			nPos = info.nTrackPos;
		}
	}

	switch( nScrollCode ){
	case SB_LINEDOWN:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() + 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() + GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_LINEUP:
//		for( i = 0; i < 4; ++i ){
//			ScrollAtV( GetTextArea().GetViewTopLine() - 1 );
//		}
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - GetDllShareData().m_Common.m_sGeneral.m_nRepeatedScrollLineNum );
		break;
	case SB_PAGEDOWN:
		nScrollVal = ScrollAtV( GetTextArea().GetBottomLine() );
		break;
	case SB_PAGEUP:
		nScrollVal = ScrollAtV( GetTextArea().GetViewTopLine() - GetTextArea().m_nViewRowNum );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtV( CLayoutInt(nPos) );
		break;
	case SB_TOP:
		nScrollVal = ScrollAtV( CLayoutInt(0) );
		break;
	case SB_BOTTOM:
		nScrollVal = ScrollAtV(( m_pcEditDoc->m_cLayoutMgr.GetLineCount() ) - GetTextArea().m_nViewRowNum );
		break;
	default:
		break;
	}
	return nScrollVal;
}

/*! �����X�N���[���o�[���b�Z�[�W����

	@param nScrollCode [in]	�X�N���[����� (Windows����n��������)
	@param nPos [in]		�X�N���[���ʒu(THUMBTRACK�p)
	@retval	���ۂɃX�N���[����������

	@date 2004.09.11 genta �X�N���[��������Ԃ��悤�ɁD
		���g�p��hwndScrollBar�����폜�D
*/
CLayoutInt CEditView::OnHScroll( int nScrollCode, int nPos )
{
	const CLayoutInt nHScrollNum = GetTextMetrics().GetLayoutXDefault(CKetaXInt(4));
	CLayoutInt nScrollVal = CLayoutInt(0);

	// nPos 32bit�Ή�
	if( nScrollCode == SB_THUMBTRACK || nScrollCode == SB_THUMBPOSITION ){
		if( m_hwndHScrollBar ){
			HWND hWndScroll = m_hwndHScrollBar;
			SCROLLINFO info;
			info.cbSize = sizeof(SCROLLINFO);
			info.fMask = SIF_TRACKPOS;
			::GetScrollInfo(hWndScroll, SB_CTL, &info);
			nPos = info.nTrackPos;
		}
	}

	GetRuler().SetRedrawFlag(); // YAZAKI
	switch( nScrollCode ){
	case SB_LINELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - nHScrollNum );
		break;
	case SB_LINERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() + nHScrollNum );
		break;
	case SB_PAGELEFT:
		nScrollVal = ScrollAtH( GetTextArea().GetViewLeftCol() - GetTextArea().m_nViewColNum );
		break;
	case SB_PAGERIGHT:
		nScrollVal = ScrollAtH( GetTextArea().GetRightCol() );
		break;
	case SB_THUMBPOSITION:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE( _T("nPos=%d\n"), nPos );
		break;
	case SB_THUMBTRACK:
		nScrollVal = ScrollAtH( CLayoutInt(nPos) );
//		MYTRACE( _T("nPos=%d\n"), nPos );
		break;
	case SB_LEFT:
		nScrollVal = ScrollAtH( CLayoutInt(0) );
		break;
	case SB_RIGHT:
		//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
		nScrollVal = ScrollAtH( m_pcEditDoc->m_cLayoutMgr.GetMaxLineKetas() - GetTextArea().m_nViewColNum );
		break;
	}
	return nScrollVal;
}

/** �X�N���[���o�[�̏�Ԃ��X�V����

	�^�u�o�[�̃^�u�ؑ֎��� SIF_DISABLENOSCROLL �t���O�ł̗L�����^������������ɓ��삵�Ȃ�
	�i�s���ŃT�C�Y�ύX���Ă��邱�Ƃɂ��e�����H�j�̂� SIF_DISABLENOSCROLL �ŗL���^����
	�̐ؑւɎ��s�����ꍇ�ɂ͋����ؑւ���

	@date 2008.05.24 ryoji �L���^�����̋����ؑւ�ǉ�
	@date 2008.06.08 ryoji �����X�N���[���͈͂ɂԂ牺���]����ǉ�
	@date 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����̃X�N���[���o�[����
*/
void CEditView::AdjustScrollBars()
{
	if( !GetDrawSwitch() ){
		return;
	}


	SCROLLINFO	si;
	bool		bEnable;

	if( NULL != m_hwndVScrollBar ){
		/* �����X�N���[���o�[ */
		const CLayoutInt	nEofMargin = CLayoutInt(2); // EOF�Ƃ��̉��̃}�[�W��
		const CLayoutInt	nAllLines = m_pcEditDoc->m_cLayoutMgr.GetLineCount() + nEofMargin;
		int	nVScrollRate = 1;
#ifdef _WIN64
		/* nAllLines / nVScrollRate < INT_MAX �ƂȂ鐮��nVScrollRate�����߂� */
		// 64bit�ŗp�X�N���[����
		while( nAllLines / nVScrollRate > INT_MAX ){
			++nVScrollRate;
		}
#endif
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)nAllLines / nVScrollRate - 1;	/* �S�s�� */
		si.nPage = (Int)GetTextArea().m_nViewRowNum / nVScrollRate;	/* �\����̍s�� */
		si.nPos  = (Int)GetTextArea().GetViewTopLine() / nVScrollRate;	/* �\����̈�ԏ�̍s(0�J�n) */
		si.nTrackPos = 0;
		::SetScrollInfo( m_hwndVScrollBar, SB_CTL, &si, TRUE );
		m_nVScrollRate = nVScrollRate;				/* �����X�N���[���o�[�̏k�� */
		
		//	Nov. 16, 2002 genta
		//	�c�X�N���[���o�[��Disable�ɂȂ����Ƃ��͕K���S�̂���ʓ��Ɏ��܂�悤��
		//	�X�N���[��������
		//	2005.11.01 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
		bEnable = ( GetTextArea().m_nViewRowNum < nAllLines );
		if( bEnable != (::IsWindowEnabled( m_hwndVScrollBar ) != 0) ){
			::EnableWindow( m_hwndVScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if( !bEnable ){
			ScrollAtV( CLayoutInt(0) );
		}
	}
	if( NULL != m_hwndHScrollBar ){
		/* �����X�N���[���o�[ */
		si.cbSize = sizeof( si );
		si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		si.nMin  = 0;
		si.nMax  = (Int)GetRightEdgeForScrollBar() - 1;		// 2009.08.28 nasukoji	�X�N���[���o�[����p�̉E�[���W���擾
		si.nPage = (Int)GetTextArea().m_nViewColNum;			/* �\����̌��� */
		si.nPos  = (Int)GetTextArea().GetViewLeftCol();		/* �\����̈�ԍ��̌�(0�J�n) */
		si.nTrackPos = 1;
		::SetScrollInfo( m_hwndHScrollBar, SB_CTL, &si, TRUE );

		//	2006.1.28 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
		bEnable = ( GetTextArea().m_nViewColNum < GetRightEdgeForScrollBar() );
		if( bEnable != (::IsWindowEnabled( m_hwndHScrollBar ) != 0) ){
			::EnableWindow( m_hwndHScrollBar, bEnable? TRUE: FALSE );	// SIF_DISABLENOSCROLL �듮�쎞�̋����ؑ�
		}
		if( !bEnable ){
			ScrollAtH( CLayoutInt(0) );
		}
	}
}

/*! �w���[�s�ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[�������s�� (��:������/��:�����)

	@date 2004.09.11 genta �s����߂�l�Ƃ��ĕԂ��悤�ɁD(�����X�N���[���p)
*/
CLayoutInt CEditView::ScrollAtV( CLayoutInt nPos )
{
	CLayoutInt	nScrollRowNum;
	RECT		rcScrol;
	RECT		rcClip;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}
	else if( (m_pcEditDoc->m_cLayoutMgr.GetLineCount() + 2 )- GetTextArea().m_nViewRowNum < nPos ){
		nPos = ( m_pcEditDoc->m_cLayoutMgr.GetLineCount() + CLayoutInt(2) ) - GetTextArea().m_nViewRowNum;
		if( nPos < 0 ){
			nPos = CLayoutInt(0);
		}
	}
	if( GetTextArea().GetViewTopLine() == nPos ){
		return CLayoutInt(0);	//	�X�N���[�������B
	}
	/* �����X�N���[���ʁi�s���j�̎Z�o */
	nScrollRowNum = GetTextArea().GetViewTopLine() - nPos;

	/* �X�N���[�� */
	if( t_abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum ){
		GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		if( nScrollRowNum > 0 ){
			rcScrol.bottom =
				GetTextArea().GetAreaBottom() -
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top = GetTextArea().GetAreaTop();
			rcClip.bottom =
				GetTextArea().GetAreaTop() + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
		}
		else if( nScrollRowNum < 0 ){
			rcScrol.top =
				GetTextArea().GetAreaTop() - (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			GetTextArea().SetViewTopLine( CLayoutInt(nPos) );
			rcClip.left = 0;
			rcClip.right = GetTextArea().GetAreaRight();
			rcClip.top =
				GetTextArea().GetAreaBottom() +
				(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy();
			rcClip.bottom = GetTextArea().GetAreaBottom();
		}
		if( GetDrawSwitch() ){
			RECT rcClip2 = {0,0,0,0};
			ScrollDraw(nScrollRowNum, CLayoutInt(0), rcScrol, rcClip, rcClip2);
			::UpdateWindow( GetHwnd() );
		}
	}

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	GetCaret().ShowEditCaret();

	MiniMapRedraw(false);

	return -nScrollRowNum;	//�������t�Ȃ̂ŕ������]���K�v
}




/*! �w�荶�[���ʒu�փX�N���[��

	@param nPos [in] �X�N���[���ʒu
	@retval ���ۂɃX�N���[���������� (��:�E����/��:������)

	@date 2004.09.11 genta ������߂�l�Ƃ��ĕԂ��悤�ɁD(�����X�N���[���p)
	@date 2008.06.08 ryoji �����X�N���[���͈͂ɂԂ牺���]����ǉ�
	@date 2009.08.28 nasukoji	�u�܂�Ԃ��Ȃ��v�I�����E�ɍs���߂��Ȃ��悤�ɂ���
*/
CLayoutInt CEditView::ScrollAtH( CLayoutInt nPos )
{
	RECT		rcScrol;
	RECT		rcClip2;
	if( nPos < 0 ){
		nPos = CLayoutInt(0);
	}
	//	Aug. 18, 2003 ryoji �ϐ��̃~�X���C��
	//	�E�B���h�E�̕�������߂ċ��������Ƃ��ɕҏW�̈悪�s�ԍ����痣��Ă��܂����Ƃ��������D
	//	Aug. 14, 2005 genta �܂�Ԃ�����LayoutMgr����擾����悤��
	else if( GetRightEdgeForScrollBar() + GetWrapOverhang() - GetTextArea().m_nViewColNum  < nPos ){
		nPos = GetRightEdgeForScrollBar() + GetWrapOverhang() - GetTextArea().m_nViewColNum ;
		//	May 29, 2004 genta �܂�Ԃ������E�B���h�E�����傫���Ƃ���WM_HSCROLL�������
		//	nPos�����̒l�ɂȂ邱�Ƃ�����C���̏ꍇ�ɃX�N���[���o�[����ҏW�̈悪
		//	����Ă��܂��D
		if( nPos < 0 )
			nPos = CLayoutInt(0);
	}
	if( GetTextArea().GetViewLeftCol() == nPos ){
		return CLayoutInt(0);
	}
	/* �����X�N���[���ʁi�������j�̎Z�o */
	const CLayoutInt	nScrollColNum = GetTextArea().GetViewLeftCol() - nPos;

	/* �X�N���[�� */
	if( t_abs( nScrollColNum ) >= GetTextArea().m_nViewColNum /*|| abs( nScrollRowNum ) >= GetTextArea().m_nViewRowNum*/ ){
		GetTextArea().SetViewLeftCol( nPos );
		::InvalidateRect( GetHwnd(), NULL, TRUE );
	}else{
		rcScrol.left = 0;
		rcScrol.right = GetTextArea().GetAreaRight();
		rcScrol.top = GetTextArea().GetAreaTop();
		rcScrol.bottom = GetTextArea().GetAreaBottom();
		int nScrollColPxWidth = GetTextMetrics().GetCharPxWidth(nScrollColNum);
		if( nScrollColNum > 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft();
			rcScrol.right =
				GetTextArea().GetAreaRight() - nScrollColPxWidth;
			rcClip2.left = GetTextArea().GetAreaLeft();
			rcClip2.right = GetTextArea().GetAreaLeft() + nScrollColPxWidth;
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		else if( nScrollColNum < 0 ){
			rcScrol.left = GetTextArea().GetAreaLeft() - nScrollColPxWidth;
			rcClip2.left =
				GetTextArea().GetAreaRight() + nScrollColPxWidth;
			rcClip2.right = GetTextArea().GetAreaRight();
			rcClip2.top = GetTextArea().GetAreaTop();
			rcClip2.bottom = GetTextArea().GetAreaBottom();
		}
		GetTextArea().SetViewLeftCol( nPos );
		if( GetDrawSwitch() ){
			RECT rcClip = {0,0,0,0};
			ScrollDraw(CLayoutInt(0), nScrollColNum, rcScrol, rcClip, rcClip2);
			::UpdateWindow( GetHwnd() );
		}
	}
	//	2006.1.28 aroka ����������C�� (�o�[�������Ă��X�N���[�����Ȃ�)
	// ���AdjustScrollBars���Ă�ł��܂��ƁA��x�ڂ͂����܂ł��Ȃ��̂ŁA
	// GetRuler().DispRuler���Ă΂�Ȃ��B���̂��߁A���������ւ����B
	GetRuler().SetRedrawFlag(); // ���[���[���ĕ`�悷��B
	HDC hdc = ::GetDC( GetHwnd() );
	GetRuler().DispRuler( hdc );
	::ReleaseDC( GetHwnd(), hdc );

	/* �X�N���[���o�[�̏�Ԃ��X�V���� */
	AdjustScrollBars();

	/* �L�����b�g�̕\���E�X�V */
	GetCaret().ShowEditCaret();

	return -nScrollColNum;	//�������t�Ȃ̂ŕ������]���K�v
}


void CEditView::ScrollDraw(CLayoutInt nScrollRowNum, CLayoutInt nScrollColNum, const RECT& rcScroll, const RECT& rcClip, const RECT& rcClip2)
{
	const CTextArea& area = GetTextArea();

	// �w�i�͉�ʂɑ΂��ČŒ肩
	bool bBackImgFixed = IsBkBitmap() &&
		(0 != nScrollRowNum && !m_pTypeData->m_backImgScrollY ||
		 0 != nScrollColNum && !m_pTypeData->m_backImgScrollX);
	if( bBackImgFixed ){
		CMyRect rcBody = area.GetAreaRect();
		rcBody.left = 0; // �s�ԍ����ړ�
		rcBody.top = area.GetRulerHeight();
		InvalidateRect(&rcBody, FALSE);
	}else{
		int nScrollColPxWidth = GetTextMetrics().GetCharPxWidth(nScrollColNum);
		ScrollWindowEx(
			nScrollColPxWidth,	// �����X�N���[����
			(Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),	// �����X�N���[����
			&rcScroll,	/* �X�N���[�������`�̍\���̂̃A�h���X */
			NULL, NULL , NULL, SW_ERASE | SW_INVALIDATE
		);
		// From Here 2007.09.09 Moca �݊�BMP�ɂ���ʃo�b�t�@
		if( m_hbmpCompatBMP ){
			// �݊�BMP���X�N���[�������̂��߂�BitBlt�ňړ�������
			::BitBlt(
				m_hdcCompatDC,
				rcScroll.left + nScrollColPxWidth,
				rcScroll.top  + (Int)nScrollRowNum * GetTextMetrics().GetHankakuDy(),
				rcScroll.right - rcScroll.left, rcScroll.bottom - rcScroll.top,
				m_hdcCompatDC, rcScroll.left, rcScroll.top, SRCCOPY
			);
		}

		if( 0 < area.GetTopYohaku() &&
		  IsBkBitmap() &&
		  (0 != nScrollRowNum && m_pTypeData->m_backImgScrollY || 0 != nScrollColNum && m_pTypeData->m_backImgScrollX) ){
			// Scroll�̂Ƃ��Ƀ��[���[�]���X�V
			CMyRect rcTopYohaku;
			if( CTypeSupport(this, COLORIDX_TEXT).GetBackColor() == CTypeSupport(this, COLORIDX_GYOU).GetBackColor() ){
				rcTopYohaku.left = 0;
			}else{
				rcTopYohaku.left = area.GetLineNumberWidth();
			}
			rcTopYohaku.top  = area.GetRulerHeight();
			rcTopYohaku.right  = area.GetAreaRight();
			rcTopYohaku.bottom = area.GetAreaTop();
			HDC hdcSelf = GetDC();
			HDC hdcBgImg = m_hdcCompatDC ? m_hdcCompatDC : CreateCompatibleDC(hdcSelf);
			HBITMAP hOldBmp = (HBITMAP)::SelectObject(hdcBgImg, m_pcEditDoc->m_hBackImg);
			DrawBackImage(hdcSelf, rcTopYohaku, hdcBgImg);
			SelectObject(hdcBgImg, hOldBmp);
			ReleaseDC(hdcSelf);
			if( !m_hdcCompatDC ){
				DeleteObject(hdcBgImg);
			}
		}
		if( IsBkBitmap() && 0 != nScrollColNum && m_pTypeData->m_backImgScrollX ){
			// �s�ԍ��w�i�̂��߂ɍX�V
			CMyRect rcLineNum;
			area.GenerateLineNumberRect(&rcLineNum);
			InvalidateRect( &rcLineNum, FALSE );
		}
	}
	// �J�[�\���̏c�����e�L�X�g�ƍs�ԍ��̌��Ԃɂ���Ƃ��A�X�N���[�����ɏc���̈���X�V
	if( nScrollColNum != 0 && m_nOldCursorLineX == GetTextArea().GetAreaLeft() - 1 ){
		RECT rcClip3;
		rcClip3.left   = m_nOldCursorLineX - (m_nOldCursorVLineWidth - 1);
		rcClip3.right  = m_nOldCursorLineX + 1;
		rcClip3.top    = GetTextArea().GetAreaTop();
		rcClip3.bottom = GetTextArea().GetAreaBottom();
		InvalidateRect( &rcClip3, FALSE );
	}
	// To Here 2007.09.09 Moca

	if( nScrollRowNum != 0 ){
		InvalidateRect( &rcClip );
		if( nScrollColNum != 0 ){
			RECT lineNumClip;
			GetTextArea().GenerateLineNumberRect(&lineNumClip);
			InvalidateRect( &lineNumClip, FALSE );
		}
	}
	if( nScrollColNum != 0 ){
		InvalidateRect( &rcClip2, FALSE );
	}
}


void CEditView::MiniMapRedraw(bool bUpdateAll)
{
	if( this == &m_pcEditWnd->GetActiveView() && m_pcEditWnd->GetMiniMap().GetHwnd() ){
		CEditView& miniMap = m_pcEditWnd->GetMiniMap();
		CLayoutYInt nViewTop = miniMap.m_nPageViewTop;
		CLayoutYInt nViewBottom = miniMap.m_nPageViewBottom;
		CLayoutYInt nDiff = nViewTop - GetTextArea().GetViewTopLine();
		CLayoutYInt nDrawTopTop;
		CLayoutYInt nDrawTopBottom;
		bool bUpdate = (t_abs(nDiff) > nViewBottom - nViewTop) || bUpdateAll;
		bool bUpdateOne = false;
		if( bUpdate ){
			if( nViewTop == GetTextArea().GetViewTopLine() ){
				// OnSize:�������L�k����
				bUpdateOne = true;
				nDrawTopTop = t_min(nViewBottom, GetTextArea().GetBottomLine());
				nDrawTopBottom = t_max(nViewBottom, GetTextArea().GetBottomLine());
			}else{
				nDrawTopTop = nViewTop;
				nDrawTopBottom = nViewBottom;
			}
		}else{
			if( nDiff < 0 ){
				// ��Ɉړ�
				nDrawTopTop = GetTextArea().GetViewTopLine();
				nDrawTopBottom = nViewTop;
			}else{
				// ���Ɉړ�
				nDrawTopTop = nViewTop;
				nDrawTopBottom = GetTextArea().GetViewTopLine();
			}
		}
		RECT rcMiniMap;
		rcMiniMap.left = 0;
		rcMiniMap.right = miniMap.GetTextArea().GetAreaRight();
		rcMiniMap.top = miniMap.GetTextArea().GenerateYPx(nDrawTopTop);
		rcMiniMap.bottom = miniMap.GetTextArea().GenerateYPx(nDrawTopBottom);
		::InvalidateRect( miniMap.GetHwnd(), &rcMiniMap, FALSE );
		::UpdateWindow( miniMap.GetHwnd() );

		if( bUpdateOne ){
			return;
		}
		CLayoutYInt nDrawBottomTop;
		CLayoutYInt nDrawBottomBottom;
		if( bUpdate ){
			nDrawBottomTop = GetTextArea().GetViewTopLine();
			nDrawBottomBottom = GetTextArea().GetBottomLine();
		}else{
			if( nDiff < 0 ){
				// ��Ɉړ�
				nDrawBottomTop = GetTextArea().GetBottomLine();
				nDrawBottomBottom = nViewBottom;
			}else{
				// ���Ɉړ�
				nDrawBottomTop = nViewBottom;
				nDrawBottomBottom = GetTextArea().GetBottomLine();
			}
		}
		rcMiniMap.left = 0;
		rcMiniMap.right = miniMap.GetTextArea().GetAreaRight();
		rcMiniMap.top = miniMap.GetTextArea().GenerateYPx(nDrawBottomTop);
		rcMiniMap.bottom = miniMap.GetTextArea().GenerateYPx(nDrawBottomBottom);
		::InvalidateRect( miniMap.GetHwnd(), &rcMiniMap, FALSE );
		::UpdateWindow( miniMap.GetHwnd() );
	}
}


/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������
	
	@param line [in] �X�N���[���s�� (��:������/��:�����/0:�������Ȃ�)
	
	@author asa-o
	@date 2001.06.20 asa-o �V�K�쐬
	@date 2004.09.11 genta �֐���

	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������

*/
void CEditView::SyncScrollV( CLayoutInt line )
{
	if( GetDllShareData().m_Common.m_sWindow.m_bSplitterWndVScroll && line != 0 
		&& m_pcEditWnd->IsEnablePane(m_nMyIndex^0x01)
		&& 0 <= m_nMyIndex
	){
		CEditView&	editView = m_pcEditWnd->GetView(m_nMyIndex^0x01);
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		editView.ScrollByV( line );
#else
		editView.ScrollAtV( GetTextArea().GetViewTopLine() );
#endif
	}
}

/*!	���������X�N���[��

	���������X�N���[����ON�Ȃ�΁C�Ή�����E�B���h�E���w��s�������X�N���[������D
	
	@param col [in] �X�N���[������ (��:�E����/��:������/0:�������Ȃ�)
	
	@author asa-o
	@date 2001.06.20 asa-o �V�K�쐬
	@date 2004.09.11 genta �֐���

	@note ����̏ڍׂ͐ݒ��@�\�g���ɂ��ύX�ɂȂ�\��������
*/
void CEditView::SyncScrollH( CLayoutInt col )
{
	if( GetDllShareData().m_Common.m_sWindow.m_bSplitterWndHScroll && col != 0
		&& m_pcEditWnd->IsEnablePane(m_nMyIndex^0x02)
		&& 0 <= m_nMyIndex
	){
		CEditView&	cEditView = m_pcEditWnd->GetView(m_nMyIndex^0x02);
		HDC			hdc = ::GetDC( cEditView.GetHwnd() );
		
#if 0
		//	������ۂ����܂܃X�N���[������ꍇ
		cEditView.ScrollByH( col );
#else
		cEditView.ScrollAtH( GetTextArea().GetViewLeftCol() );
#endif
		GetRuler().SetRedrawFlag(); //2002.02.25 Add By KK �X�N���[�������[���[�S�̂�`���Ȃ����B
		GetRuler().DispRuler( hdc );
		::ReleaseDC( GetHwnd(), hdc );
	}
}

/** �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z
	@date 2008.06.08 ryoji �V�K�쐬
*/
CLayoutInt CEditView::GetWrapOverhang( void ) const
{
	CLayoutInt nMargin = GetTextMetrics().GetLayoutXDefault(CKetaXInt(1));	// �܂�Ԃ��L��
	if (!m_pTypeData->m_bKinsokuHide) {	// �Ԃ牺�����B�����̓X�L�b�v	2012/11/30 Uchi
		if( m_pTypeData->m_bKinsokuRet )
			nMargin += GetTextMetrics().GetLayoutXDefault(CKetaXInt(1));	// ���s�Ԃ牺��
		if( m_pTypeData->m_bKinsokuKuto )
			nMargin += GetTextMetrics().GetLayoutXDefault(CKetaXInt(2));	// ��Ǔ_�Ԃ牺��
	}
	return CLayoutInt( nMargin );
}

/** �u�E�[�Ő܂�Ԃ��v�p�Ƀr���[�̌�������܂�Ԃ��������v�Z����
	@param nViewColNum	[in] �r���[�̌���
	@retval �܂�Ԃ�����
	@date 2008.06.08 ryoji �V�K�쐬
*/
CKetaXInt CEditView::ViewColNumToWrapColNum( CLayoutXInt nViewColNum ) const
{
	// �Ԃ牺���]������������
	CKetaXInt nKeta = CKetaXInt((Int)(nViewColNum - GetWrapOverhang())) / (Int)GetTextMetrics().GetLayoutXDefault();

	// MINLINEKETAS�����̎���MINLINEKETAS�Ő܂�Ԃ��Ƃ���
	if( nKeta < CKetaXInt(MINLINEKETAS) ){
		nKeta = CKetaXInt(MINLINEKETAS);		// �܂�Ԃ����̍ŏ������ɐݒ�
	}

	return nKeta;
}

/*!
	@brief  �X�N���[���o�[����p�ɉE�[���W���擾����

	�u�܂�Ԃ��Ȃ��v
		�t���[�J�[�\����Ԃ̎��̓e�L�X�g�̕������E���փJ�[�\�����ړ��ł���
		�̂ŁA������l�������X�N���[���o�[�̐��䂪�K�v�B
		�{�֐��́A���L�̓��ōł��傫�Ȓl�i�E�[�̍��W�j��Ԃ��B
		�@�E�e�L�X�g�̉E�[
		�@�E�L�����b�g�ʒu
		�@�E�I��͈͂̉E�[
	
	�u�w�茅�Ő܂�Ԃ��v
	�u�E�[�Ő܂�Ԃ��v
		��L�̏ꍇ�܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z

	@return     �E�[�̃��C�A�E�g���W��Ԃ�

	@note   �u�܂�Ԃ��Ȃ��v�I�����́A�X�N���[����ɃL�����b�g�������Ȃ�
	        �Ȃ�Ȃ��l�ɂ��邽�߂ɉE�}�[�W���Ƃ��Ĕ��p3���Œ�ŉ��Z����B

	@date 2009.08.28 nasukoji	�V�K�쐬
*/
CLayoutInt CEditView::GetRightEdgeForScrollBar( void )
{
	// �܂�Ԃ����Ȍ�̂Ԃ牺���]���v�Z
	CLayoutXInt nWidth = m_pcEditDoc->m_cLayoutMgr.GetMaxLineLayout() + GetWrapOverhang();
	
	if( m_pcEditDoc->m_nTextWrapMethodCur == WRAP_NO_TEXT_WRAP ){
		CLayoutInt nRightEdge = m_pcEditDoc->m_cLayoutMgr.GetMaxTextWidth();	// �e�L�X�g�̍ő啝

		// �I��͈͂��� ���� �͈͂̉E�[���e�L�X�g�̕����E��
		if( GetSelectionInfo().IsTextSelected() ){
			// �J�n�ʒu�E�I���ʒu�̂��E���ɂ�����Ŕ�r
			if( GetSelectionInfo().m_sSelect.GetFrom().GetX2() < GetSelectionInfo().m_sSelect.GetTo().GetX2() ){
				if( nRightEdge < GetSelectionInfo().m_sSelect.GetTo().GetX2() )
					nRightEdge = GetSelectionInfo().m_sSelect.GetTo().GetX2();
			}else{
				if( nRightEdge < GetSelectionInfo().m_sSelect.GetFrom().GetX2() )
					nRightEdge = GetSelectionInfo().m_sSelect.GetFrom().GetX2();
			}
		}

		// �t���[�J�[�\�����[�h ���� �L�����b�g�ʒu���e�L�X�g�̕����E��
		if( GetDllShareData().m_Common.m_sGeneral.m_bIsFreeCursorMode && nRightEdge < GetCaret().GetCaretLayoutPos().GetX2() )
			nRightEdge = GetCaret().GetCaretLayoutPos().GetX2();

		// �E�}�[�W�����i3���j���l������nWidth�𒴂��Ȃ��悤�ɂ���
		CLayoutXInt layout3Keta = GetTextMetrics().GetLayoutXDefault(CKetaXInt(3));
		nWidth = t_min( nRightEdge + layout3Keta, nWidth );
	}

	return nWidth;
}
