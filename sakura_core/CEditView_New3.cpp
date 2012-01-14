/*!	@file
	@brief 試験機能？実体無し

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"

#if 0




//#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include "CEditView.h"
#include "Debug.h"
#include "Keycode.h"
#include "Funccode.h"
#include "CRunningTimer.h"
#include "charcode.h"
#include "mymessage.h"
#include "CWaitCursor.h"
#include "CEditWnd.h"
#include "CShareData.h"
#include "CDlgCancel.h"
#include "sakura_rc.h"
#include "etc_uty.h"

#ifdef _DEBUG
//★★★テスト用非公開機能★★★
/* ルーラー描画 */
void CEditView::DispRulerEx( HDC hdc )
{
	if( !m_bDrawSWITCH ){
		return;
	}
	if( !m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_bDisp ){
		return;
	}

	/* 描画処理 */
	HBRUSH		hBrush;
	HBRUSH		hBrushOld;
	HRGN		hRgn;
	RECT		rc;
	RECT		rc2;
	int			i;
	int			nX;
	int			nY;
	LOGFONT		lf;
	HFONT		hFont;
	HFONT		hFontOld;
	char		szColm[32];
//	SIZE		size;
	HPEN		hPen;
	HPEN		hPenOld;
	int			nROP_Old;
	COLORREF	colTextOld;
	int			nToX;
	SIZE		sizFont;

	/* LOGFONTの初期化 */
	memset( &lf, 0, sizeof(LOGFONT) );
	lf.lfHeight			= -11;
	lf.lfWidth			= 5;
	lf.lfEscapement		= 0;
	lf.lfOrientation	= 0;
	lf.lfWeight			= 400;
	lf.lfItalic			= 0;
	lf.lfUnderline		= 0;
	lf.lfStrikeOut		= 0;
	lf.lfCharSet		= 0;
	lf.lfOutPrecision	= 3;
	lf.lfClipPrecision	= 2;
	lf.lfQuality		= 1;
	lf.lfPitchAndFamily	= 34;
	strcpy( lf.lfFaceName, "Arial" );
	hFont = ::CreateFontIndirect( &lf );
	hFontOld = (HFONT)::SelectObject( hdc, hFont );
	::GetTextExtentPoint32( hdc, "X", 1, &sizFont );
	::SetBkMode( hdc, TRANSPARENT );

	/* 背景 */
	hPen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DHILIGHT ) );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
	::MoveToEx( hdc, 0, 0, NULL );
	::LineTo( hdc, m_nViewAlignLeft + m_nViewCx, 0 );
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );


	hPen = ::CreatePen( PS_SOLID, 0, ::GetSysColor( COLOR_3DFACE ) );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
	::MoveToEx( hdc, 0, 1, NULL );
	::LineTo( hdc, m_nViewAlignLeft + m_nViewCx, 1 );
	::MoveToEx( hdc, 0, m_nViewAlignTop - m_nTopYohaku - 2, NULL );
	::LineTo( hdc, m_nViewAlignLeft + m_nViewCx, m_nViewAlignTop - m_nTopYohaku - 2 );
	::MoveToEx( hdc, 0, m_nViewAlignTop - m_nTopYohaku - 3, NULL );
	::LineTo( hdc, m_nViewAlignLeft + m_nViewCx, m_nViewAlignTop - m_nTopYohaku - 3 );
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );


	hPen = ::CreatePen( PS_SOLID, 0, /*RGB( 255, 0, 0 )*/::GetSysColor( COLOR_3DSHADOW ) );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
	::MoveToEx( hdc, 0, m_nViewAlignTop - m_nTopYohaku - 1, NULL );
	::LineTo( hdc, m_nViewAlignLeft + m_nViewCx, m_nViewAlignTop - m_nTopYohaku - 1 );
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );



	rc.left = m_nViewAlignLeft - 2;
	rc.top = 2;
	rc.right = m_nViewAlignLeft + (m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - m_nViewLeftCol) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 2;
	if( rc.right > m_nViewAlignLeft + m_nViewCx + 2 ){
		rc.right = m_nViewAlignLeft + m_nViewCx + 2;
	}
	rc.bottom = m_nViewAlignTop - m_nTopYohaku - 3;
	CSplitBoxWnd::Draw3dRect( hdc,
		rc.left,
		rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top,
		::GetSysColor( COLOR_3DSHADOW ),
		::GetSysColor( COLOR_3DHILIGHT )
	);


	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
	rc2.left = 0;
	rc2.top = 1;
	rc2.right = rc.left;
	rc2.bottom = m_nViewAlignTop - m_nTopYohaku - 1;
	::FillRect( hdc, &rc2, hBrush );
	::DeleteObject( hBrush );

	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_3DFACE ) );
	rc2.left = rc.right;
	rc2.top = 1;
	rc2.right = m_nViewAlignLeft + m_nViewCx + 2;
	rc2.bottom = m_nViewAlignTop - m_nTopYohaku - 1;
	::FillRect( hdc, &rc2, hBrush );
	::DeleteObject( hBrush );


	rc.left++;
	rc.top++;
	rc.right--;
	rc.bottom--;
	CSplitBoxWnd::Draw3dRect( hdc,
		rc.left,
		rc.top,
		rc.right - rc.left,
		rc.bottom - rc.top,
		::GetSysColor( COLOR_3DDKSHADOW ),
		::GetSysColor( COLOR_3DSHADOW )
	);




	hBrush = ::CreateSolidBrush( m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colBACK );
	rc.left++;
	rc.top++;
	rc.right--;
	rc.bottom--;
	::FillRect( hdc, &rc, hBrush );
	::DeleteObject( hBrush );



	nX = m_nViewAlignLeft;
//	nY = m_nViewAlignTop - m_nTopYohaku - 2;
	nY = (rc.top + rc.bottom) / 2;

//	hPen = ::CreatePen( PS_SOLID, 0, RGB( 0, 0, 0 ) );
	hPen = ::CreatePen( PS_SOLID, 0, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );
	hPenOld = (HPEN)::SelectObject( hdc, hPen );
//	colTextOld = ::SetTextColor( hdc, RGB( 0, 0, 0 ) );
	colTextOld = ::SetTextColor( hdc, m_pcEditDoc->GetDocumentAttribute().m_ColorInfoArr[COLORIDX_RULER].m_colTEXT );

	nToX = m_nViewAlignLeft + m_nViewCx;

	nToX = m_nViewAlignLeft + (m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize() - m_nViewLeftCol) * ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	if( nToX > m_nViewAlignLeft + m_nViewCx ){
		nToX = m_nViewAlignLeft + m_nViewCx;
	}

//	::MoveToEx( hdc, m_nViewAlignLeft, nY + 1, NULL );
//	::LineTo( hdc, nToX/*m_nViewAlignLeft + m_nViewCx*/, nY + 1 );


	for( i = m_nViewLeftCol;
		i <= m_nViewLeftCol + m_nViewColNum + 1
//	 && i <= m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
	 && i < m_pcEditDoc->m_cLayoutMgr.GetMaxLineSize();
		i++
	){
		if( 0 < i && 0 == ( (i) % 10 ) ){
//			::MoveToEx( hdc, nX, rc.top, NULL );
//			::LineTo( hdc, nX, rc.bottom );
//			::MoveToEx( hdc, nX, nY - 6, NULL );
//			::LineTo( hdc, nX, nY + 7 );

			::MoveToEx( hdc, nX, m_nViewAlignTop - m_nTopYohaku - 3, NULL );
			::LineTo( hdc, nX, m_nViewAlignTop - m_nTopYohaku - 5 );

//			::MoveToEx( hdc, nX, 0, NULL );
//			::LineTo( hdc, nX, 2 );


			itoa( (i) / 10, szColm, 10 );

			SIZE sz;
			int nColmLen = lstrlen( szColm );
			// 文字列の幅と高さを計算します。
			::GetTextExtentPoint32( hdc, szColm, nColmLen, &sz );
//			::TextOut( hdc, nX + 2 + 0, nY - (sizFont.cy / 2), szColm, nColmLen );
			::TextOut( hdc, nX - ( sz.cx / 2 ) + 1, nY - (sizFont.cy / 2), szColm, nColmLen );
		}else
		if( 0 < i && 0 == ( (i) % 5 ) ){
			::MoveToEx( hdc, nX, nY - 1, NULL );
			::LineTo( hdc, nX, nY + 2 );
		}else{
//			::MoveToEx( hdc, nX, nY , NULL );
//			::LineTo( hdc, nX, nY + 1 );
		}
		nX += ( m_nCharWidth  + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace );
	}
	::SetTextColor( hdc, colTextOld );
	::SelectObject( hdc, hPenOld );
	::DeleteObject( hPen );

//	return;

	if( m_nViewLeftCol <= m_nCaretPosX
	 && m_nViewLeftCol + m_nViewColNum + 2 >= m_nCaretPosX
	){
		if( 0 == m_nCaretWidth ){
			hBrush = ::CreateSolidBrush( RGB( 128, 128, 128 ) );
		}else{
			hBrush = ::CreateSolidBrush( RGB( 0, 0, 0 ) );
		}
		rc.left = m_nViewAlignLeft + ( m_nCaretPosX - m_nViewLeftCol ) * ( m_nCharWidth + m_pcEditDoc->GetDocumentAttribute().m_nColmSpace ) + 1;
//		rc.top = 0;
		rc.right = rc.left + m_nCharWidth;
//		rc.bottom = m_nViewAlignTop - m_nTopYohaku - 1;
		nROP_Old = ::SetROP2( hdc, R2_NOTXORPEN );
		hRgn = ::CreateRectRgnIndirect( &rc );
		hBrushOld = (HBRUSH)::SelectObject( hdc, hBrush );
		::PaintRgn( hdc, hRgn );
		::SelectObject( hdc, hBrushOld );
		::DeleteObject( hRgn );
//		::FillRect( hdc, &rc, hBrush );
		::DeleteObject( hBrush );
		::SetROP2( hdc, nROP_Old );
	}
/***
	rc.left = 0;
	rc.top = 0;
	rc.right = m_nViewAlignLeft + m_nViewCx;
	rc.bottom = m_nViewAlignTop - m_nTopYohaku;
	CSplitBoxWnd::Draw3dRect(
		hdc,
		rc.left, rc.top, rc.right, rc.bottom,
		::GetSysColor( COLOR_3DHILIGHT ),
		::GetSysColor( COLOR_3DSHADOW )
	);
***/
	::SelectObject( hdc, hFontOld );
	::DeleteObject( hFont );
	return;
}
#endif //#ifdef _DEBUG
#endif //#if 0


/*[EOF]*/
