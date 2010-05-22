/*!	@file
	@brief �c�[���`�b�v

	@author Norio Nakatani
	@date 1998/10/30 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, asa-o
	Copyright (C) 2002, GAE
	Copyright (C) 2005, D.S.Koba
	Copyright (C) 2006, ryoji, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CTipWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"


/* CTipWnd�N���X �f�X�g���N�^ */
CTipWnd::CTipWnd()
: CWnd(_T("::CTipWnd"))
{
	m_pszClassName = _T("CTipWnd");
	m_KeyWasHit = FALSE;	/* �L�[���q�b�g������ */
	return;
}



/* CTipWnd�N���X �f�X�g���N�^ */
CTipWnd::~CTipWnd()
{
	if( NULL != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}
	return;
}



/* ������ */
void CTipWnd::Create( HINSTANCE hInstance, HWND hwndParent )
{
	/* �E�B���h�E�N���X�쐬 */
	ATOM atWork;
	atWork = RegisterWC(
		hInstance,
		/* WNDCLASS�p */
		NULL,// Handle to the class icon.
		NULL,	//Handle to a small icon
		::LoadCursor( NULL, IDC_ARROW ),// Handle to the class cursor.
		(HBRUSH)/*NULL*/(COLOR_INFOBK + 1),// Handle to the class background brush.
		NULL/*MAKEINTRESOURCE( MYDOCUMENT )*/,// Pointer to a null-terminated character string that specifies the resource name of the class menu, as the name appears in the resource file.
		m_pszClassName// Pointer to a null-terminated string or is an atom.
	);

	/* ���N���X�����o�Ăяo�� */
	// 2006.01.09 ryoji ������Ԃ�s���ɂ���
	//	�����I�ɂ͌����Ȃ�CTipWnd���őO�ʂɂ���Ɣ��f����Ă��܂��ꍇ�����邽��
	CWnd::Create(
		hInstance,
		hwndParent,
		WS_EX_TOOLWINDOW, // extended window style	// 2002/2/3 GAE
		m_pszClassName,	// Pointer to a null-terminated string or is an atom.
		m_pszClassName, // pointer to window name
		WS_POPUP | WS_CLIPCHILDREN | WS_BORDER, // window style
		CW_USEDEFAULT, // horizontal position of window
		0, // vertical position of window
		CW_USEDEFAULT, // window width
		0, // window height
		NULL // handle to menu, or child-window identifier
	);

	if( NULL != m_hFont ){
		::DeleteObject( m_hFont );
		m_hFont = NULL;
	}

	m_hFont = ::CreateFontIndirect( &(CShareData::getInstance()->GetShareData()->m_Common.m_sHelper.m_lf_kh) );
	return;
}

/*!	CreateWindow�̌�

	CWnd::AfterCreateWindow�ŃE�B���h�E��\������悤�ɂȂ��Ă���̂�
	�����Ȃ����邽�߂̋�֐�

	@date 2006.01.09 genta �V�K�쐬
*/
void CTipWnd::AfterCreateWindow( void )
{
}

/* Tip��\�� */
void CTipWnd::Show( int nX, int nY, const TCHAR* szText, RECT* pRect )
{
	HDC		hdc;
	RECT	rc;

	if( NULL != szText ){
		m_cInfo.SetString( szText );
	}
	const TCHAR* pszInfo = m_cInfo.GetStringPtr();

	hdc = ::GetDC( GetHwnd() );

	// �T�C�Y���v�Z�ς�	2001/06/19 asa-o
	if(pRect != NULL)
	{
		rc = *pRect;
	}
	else
	{
		/* �E�B���h�E�̃T�C�Y�����߂� */
		ComputeWindowSize( hdc, m_hFont, pszInfo, &rc );
	}

	::ReleaseDC( GetHwnd(), hdc );

	::MoveWindow( GetHwnd(), nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE );
	::ShowWindow( GetHwnd(), SW_SHOWNA );
	return;

}

/* �E�B���h�E�̃T�C�Y�����߂� */
void CTipWnd::ComputeWindowSize(
	HDC				hdc,
	HFONT			hFont,
	const TCHAR*	pszText,
	RECT*			pRect
)
{
	int		nTextLength;
	int		nCurMaxWidth;
	int		nCurHeight;
	int		nBgn;
	RECT	rc;
	HFONT	hFontOld;
	int		i;
	int		nCharChars;

	hFontOld = (HFONT)::SelectObject( hdc, hFont );

	nCurMaxWidth = 0;
	nCurHeight = 0;
	nTextLength = _tcslen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeT::GetSizeOfChar( pszText, nTextLength, i );
		if( ( 1 == nCharChars && _T('\\') == pszText[i] && _T('n') == pszText[i + 1]) || _T('\0') == pszText[i] ){
			if( 0 < i - nBgn ){
				TCHAR*	pszWork = new TCHAR[i - nBgn + 1];
				auto_memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = _T('\0');

				rc.left = 0;
				rc.top = 0;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = 0;
				::DrawText( hdc, pszWork, _tcslen(pszWork), &rc,
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
				delete [] pszWork;
				if( nCurMaxWidth < rc.right ){
					nCurMaxWidth = rc.right;
				}
			}else{
				::DrawText( hdc, _T(" "), 1, &rc,
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}
			nCurHeight += rc.bottom;

			nBgn = i + 2;
		}
		if( 2 == nCharChars ){
			++i;
		}
	}

	pRect->left = 0;
	pRect->top = 0;
	pRect->right = nCurMaxWidth + 4;
	pRect->bottom = nCurHeight + 2;

	::SelectObject( hdc, hFontOld );

	return;


}


/* �E�B���h�E�̃e�L�X�g��\�� */
void CTipWnd::DrawTipText(
	HDC				hdc,
	HFONT			hFont,
	const TCHAR*	pszText
)
{
	int			nTextLength;
	int			nCurMaxWidth;
	int			nCurHeight;
	int			nBgn;
	RECT		rc;
	HFONT		hFontOld;
	int			i;
	int			nBkMode_Old;
	COLORREF	colText_Old;
	int			nCharChars;

	nBkMode_Old = ::SetBkMode( hdc, TRANSPARENT );
	hFontOld = (HFONT)::SelectObject( hdc, hFont );
	colText_Old = ::SetTextColor( hdc, ::GetSysColor( COLOR_INFOTEXT ) );

	nCurMaxWidth = 0;
	nCurHeight = 0;
	nTextLength = _tcslen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
//		nCharChars = &pszText[i] - CMemory::MemCharPrev( pszText, nTextLength, &pszText[i] );
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CNativeT::GetSizeOfChar( pszText, nTextLength, i );
		if( ( 1 == nCharChars && _T('\\') == pszText[i] && _T('n') == pszText[i + 1]) || _T('\0') == pszText[i] ){
			if( 0 < i - nBgn ){
				TCHAR*	pszWork;
				pszWork = new TCHAR[i - nBgn + 1];
				auto_memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = _T('\0');

				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText( hdc, pszWork, _tcslen(pszWork), &rc,
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
				delete [] pszWork;
				if( nCurMaxWidth < rc.right ){
					nCurMaxWidth = rc.right;
				}
			}else{
				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText( hdc, _T(" "), 1, &rc,
					DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
			}

			nBgn = i + 2;
		}
		if( 2 == nCharChars ){
			++i;
		}
	}


	::SetTextColor( hdc, colText_Old );
	::SelectObject( hdc, hFontOld );
	::SetBkMode( hdc, nBkMode_Old );

	return;


}



/* Tip������ */
void CTipWnd::Hide( void )
{
	::ShowWindow( GetHwnd(), SW_HIDE );
//	::DestroyWindow( GetHwnd() );
	return;
}




/* �`�揈�� */
LRESULT CTipWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param )
{
	PAINTSTRUCT	ps;
	RECT		rc;
	HDC			hdc = ::BeginPaint(	hwnd, &ps );
	::GetClientRect( hwnd, &rc );

	/* �E�B���h�E�̃e�L�X�g��\�� */
	DrawTipText( hdc, m_hFont, m_cInfo.GetStringPtr() );

	::EndPaint(	hwnd, &ps );
	return 0L;
}


// 2001/06/19 Start by asa-o: �E�B���h�E�̃T�C�Y�𓾂�
void CTipWnd::GetWindowSize(LPRECT pRect)
{
	const TCHAR*	pszText;

	HDC		hdc = ::GetDC( GetHwnd() );

	pszText = m_cInfo.GetStringPtr();

	// �E�B���h�E�̃T�C�Y�𓾂�
	ComputeWindowSize( hdc, m_hFont, pszText , pRect );

	ReleaseDC(GetHwnd(), hdc); //2007.10.10 kobake ReleaseDC�������Ă����̂��C��
}

// 2001/06/19 End



