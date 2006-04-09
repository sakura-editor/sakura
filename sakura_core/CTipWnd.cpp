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
#include "CShareData.h"


/* CTipWnd�N���X �f�X�g���N�^ */
CTipWnd::CTipWnd()
{
	strcat( m_szClassInheritances, "::CTipWnd" );
	m_pszClassName = "CTipWnd";
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

	/* ������ */
	m_hInstance = hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	m_hwndParent = hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */

	/* �E�B���h�E�N���X�쐬 */
	ATOM atWork;
	atWork = RegisterWC(
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
		/* CreateWindowEx()�p */
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

	m_hFont = ::CreateFontIndirect( &(CShareData::getInstance()->GetShareData()->m_Common.m_lf_kh) );
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
void CTipWnd::Show( int nX, int nY, char* szText, RECT* pRect )
{
	HDC		hdc;
	RECT	rc;
	char*	pszInfo;

	if( NULL != szText ){
		m_cInfo.SetData( szText, strlen( szText ) );
	}
	pszInfo = m_cInfo.GetPtr();

	hdc = ::GetDC( m_hWnd );

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

	::ReleaseDC( m_hWnd, hdc );

	::MoveWindow( m_hWnd, nX, nY, rc.right + 8, rc.bottom + 8/*nHeight*/, TRUE );
	::ShowWindow( m_hWnd, SW_SHOWNA );
	return;

}

/* �E�B���h�E�̃T�C�Y�����߂� */
void CTipWnd::ComputeWindowSize(
		HDC			hdc,
		HFONT		hFont,
		const char*	pszText,
		RECT*		pRect
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
	nTextLength = strlen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pszText, nTextLength, i );
		if( ( 1 == nCharChars && '\\' == pszText[i] && 'n' == pszText[i + 1]) || '\0' == pszText[i] ){
			if( 0 < i - nBgn ){
				char*	pszWork;
				pszWork = new char[i - nBgn + 1];
				memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = '\0';

				rc.left = 0;
				rc.top = 0;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = 0;
				::DrawText( hdc, pszWork, strlen(pszWork), &rc,
					DT_CALCRECT | DT_EXTERNALLEADING | DT_EXPANDTABS | DT_WORDBREAK /*| DT_TABSTOP | (0x0000ff00 & ( 4 << 8 ))*/
				);
				delete [] pszWork;
				if( nCurMaxWidth < rc.right ){
					nCurMaxWidth = rc.right;
				}
			}else{
				::DrawText( hdc, " ", 1, &rc,
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
		HDC			hdc,
		HFONT		hFont,
		const char*	pszText
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
	nTextLength = strlen( pszText );
	nBgn = 0;
	for( i = 0; i <= nTextLength; ++i ){
//		nCharChars = &pszText[i] - CMemory::MemCharPrev( pszText, nTextLength, &pszText[i] );
		// 2005-09-02 D.S.Koba GetSizeOfChar
		nCharChars = CMemory::GetSizeOfChar( (const char *)pszText, nTextLength, i );
		if( ( 1 == nCharChars && '\\' == pszText[i] && 'n' == pszText[i + 1]) || '\0' == pszText[i] ){
			if( 0 < i - nBgn ){
				char*	pszWork;
				pszWork = new char[i - nBgn + 1];
				memcpy( pszWork, &pszText[nBgn], i - nBgn );
				pszWork[i - nBgn] = '\0';

				rc.left = 4;
				rc.top = 4 + nCurHeight;
				rc.right = ::GetSystemMetrics( SM_CXSCREEN );
				rc.bottom = rc.top + 200;
				nCurHeight += ::DrawText( hdc, pszWork, strlen(pszWork), &rc,
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
				nCurHeight += ::DrawText( hdc, " ", 1, &rc,
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
	::ShowWindow( m_hWnd, SW_HIDE );
//	::DestroyWindow( m_hWnd );
	return;
}




/* �`�揈�� */
LRESULT CTipWnd::OnPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM l_Param )
{
	HDC			hdc;
	PAINTSTRUCT	ps;
	RECT		rc;
	char*		pszText;
//	int			nTextLen;

	hdc = ::BeginPaint(	hwnd, &ps );
	pszText = m_cInfo.GetPtr();
	::GetClientRect( hwnd, &rc );
	/* �E�B���h�E�̃e�L�X�g��\�� */
	DrawTipText( hdc, m_hFont, pszText );

	::EndPaint(	hwnd, &ps );
	return 0L;


}


// 2001/06/19 Start by asa-o: �E�B���h�E�̃T�C�Y�𓾂�
void CTipWnd::GetWindowSize(LPRECT pRect)
{
	HDC			hdc;
	const char*	pszText;

	hdc = ::GetDC( m_hWnd );

	pszText = m_cInfo.GetPtr();

	// �E�B���h�E�̃T�C�Y�𓾂�
	ComputeWindowSize( hdc, m_hFont, pszText , pRect );
}

// 2001/06/19 End


/*[EOF]*/
