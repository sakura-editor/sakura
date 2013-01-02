/*! @file
	@brief �����F�^�w�i�F����_�C�A���O

	@author ryoji
	@date 2006/04/26 �쐬
*/
/*
	Copyright (C) 2006, ryoji

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
#include "CDlgSameColor.h"
#include "util/shell.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/colors/CColorStrategy.h"
#include "sakura_rc.h"
#include "sakura.hh"

static const DWORD p_helpids[] = {	// 2006.10.10 ryoji
	IDOK,						HIDOK_SAMECOLOR,						// OK
	IDCANCEL,					HIDCANCEL_SAMECOLOR,					// �L�����Z��
	IDC_BUTTON_HELP,			HIDC_BUTTON_SAMECOLOR_HELP,				// �w���v
	IDC_LIST_COLORS,			HIDC_LIST_SAMECOLOR_COLORS,				// �ύX�Ώۂ̐F
	IDC_BUTTON_SELALL,			HIDC_BUTTON_SAMECOLOR_SELALL,			// �S�`�F�b�N
	IDC_BUTTON_SELNOTING,		HIDC_BUTTON_SAMECOLOR_SELNOTING,		// �S����
	IDC_LIST_ITEMINFO,			HIDC_LIST_SAMECOLOR_ITEMINFO,			// �I�𒆂̐F�ɑΉ����鍀�ڂ̃��X�g
	IDC_STATIC_COLOR,			HIDC_STATIC_COLOR,						// ����F
	0, 0
};

LPVOID CDlgSameColor::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

CDlgSameColor::CDlgSameColor() :
	m_wpColorStaticProc(NULL),
	m_wpColorListProc(NULL),
	m_wID(0),
	m_pTypes(NULL),
	m_cr(0)
{
	return;
}

CDlgSameColor::~CDlgSameColor()
{
	return;
}

/*!
	�W���ȊO�̃��b�Z�[�W��ߑ�����
*/
INT_PTR CDlgSameColor::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );
	switch( wMsg ){
	case WM_COMMAND:
		// �F�I�����X�g�{�b�N�X�̑I�����ύX���ꂽ�ꍇ�̏���
		if( IDC_LIST_COLORS == LOWORD(wParam) && LBN_SELCHANGE == HIWORD(wParam) ){
			OnSelChangeListColors( (HWND)lParam );
		}
		break;

	case WM_CTLCOLORLISTBOX:
		{
			// ���ڃ��X�g�̔w�i�F��ݒ肷�鏈��
			HWND hwndLB = (HWND)lParam;
			if( IDC_LIST_ITEMINFO == ::GetDlgCtrlID( hwndLB ) ){
				HDC hdcLB = (HDC)wParam;
				::SetTextColor( hdcLB, ::GetSysColor( COLOR_WINDOWTEXT ) );
				::SetBkMode( hdcLB, TRANSPARENT );
				return (INT_PTR)::GetSysColorBrush( COLOR_BTNFACE );
			}
		}
		break;

	default:
		break;
	}
	return result;
}

/*! ���[�_���_�C�A���O�̕\��
	@param wID [in] �^�C�v�ʐݒ�_�C�A���O�ŉ����ꂽ�{�^��ID
	@param pTypes  [in/out] �^�C�v�ʐݒ�f�[�^
	@param cr [in] �w��F

	@date 2006.04.26 ryoji �V�K�쐬
*/
int CDlgSameColor::DoModal( HINSTANCE hInstance, HWND hwndParent, WORD wID, STypeConfig* pTypes, COLORREF cr )
{
	m_wID = wID;
	m_pTypes = pTypes;
	m_cr = cr;

	(void)CDialog::DoModal( hInstance, hwndParent, IDD_SAMECOLOR, (LPARAM)NULL );

	return TRUE;
}

/*! WM_INITDIALOG ����
	@date 2006.04.26 ryoji �V�K�쐬
*/
BOOL CDlgSameColor::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL bRet = CDialog::OnInitDialog( hwndDlg, wParam, lParam );

	HWND hwndStatic = ::GetDlgItem( GetHwnd(), IDC_STATIC_COLOR );
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_COLORS );

	// �w��F�X�^�e�B�b�N�A�F�I�����X�g���T�u�N���X��
	::SetWindowLongPtr( hwndStatic, GWLP_USERDATA, (LONG_PTR)this );
	m_wpColorStaticProc = (WNDPROC)::SetWindowLongPtr( hwndStatic, GWLP_WNDPROC, (LONG_PTR)ColorStatic_SubclassProc );
	::SetWindowLongPtr( hwndList, GWLP_USERDATA, (LONG_PTR)this );
	m_wpColorListProc = (WNDPROC)::SetWindowLongPtr( hwndList, GWLP_WNDPROC, (LONG_PTR)ColorList_SubclassProc );


	WCHAR szText[30];
	int nItem;
	int i;

	switch( m_wID )	// �^�C�v�ʐݒ�_�C�A���O�ŉ����ꂽ�{�^��ID
	{
	case IDC_BUTTON_SAMETEXTCOLOR:
		// �^�C�v�ʐݒ肩�當���F���d�����Ȃ��悤�Ɏ��o��
		::SetWindowText( GetHwnd(), _T("�����F����") );
		for( i = 0; i < COLORIDX_LAST; ++i ){
			if( m_cr != m_pTypes->m_ColorInfoArr[i].m_colTEXT ){
				_ultow( m_pTypes->m_ColorInfoArr[i].m_colTEXT, szText, 10 );
				if( LB_ERR == List_FindStringExact( hwndList, -1, szText ) ){
					nItem = ::List_AddString( hwndList, szText );
					List_SetItemData( hwndList, nItem, FALSE ); 
				}
			}
		}
		break;

	case IDC_BUTTON_SAMEBKCOLOR:
		// �^�C�v�ʐݒ肩��w�i�F���d�����Ȃ��悤�Ɏ��o��
		::SetWindowText( GetHwnd(), _T("�w�i�F����") );
		for( i = 0; i < COLORIDX_LAST; ++i ){
			if( 0 != (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
				continue;
			if( m_cr != m_pTypes->m_ColorInfoArr[i].m_colBACK ){
				_ultow( m_pTypes->m_ColorInfoArr[i].m_colBACK, szText, 10 );
				if( LB_ERR == List_FindStringExact( hwndList, -1, szText ) ){
					nItem = ::List_AddString( hwndList, szText );
					List_SetItemData( hwndList, nItem, FALSE ); 
				}
			}
		}
		break;

	default:
		CloseDialog( IDCANCEL );
		break;
	}

	if( 0 < List_GetCount( hwndList ) ){
		List_SetCurSel( hwndList, 0 );
		OnSelChangeListColors( hwndList );
	}

	return bRet;
}

/*! BN_CLICKED ����
	@date 2006.04.26 ryoji �V�K�쐬
*/
BOOL CDlgSameColor::OnBnClicked( int wID )
{
	HWND hwndList = ::GetDlgItem( GetHwnd(), IDC_LIST_COLORS );
	int nItemNum = List_GetCount( hwndList );
	BOOL bCheck;
	int i;
	int j;

	switch( wID ){
	case IDC_BUTTON_HELP:
		// �w���v	// 2006.10.07 ryoji
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, HLP000316 );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDC_BUTTON_SELALL:
	case IDC_BUTTON_SELNOTING:
		// �S�I���^�S�����̏���
		bCheck = (wID == IDC_BUTTON_SELALL);
		for( i = 0; i < nItemNum; ++i ){
			List_SetItemData( hwndList, i, bCheck );
		}
		::InvalidateRect( hwndList, NULL, TRUE );
		break;

	case IDOK:
		// �^�C�v�ʐݒ肩��I��F�Ɠ��F�̂��̂����o���Ďw��F�Ɉꊇ�ύX����
		WCHAR szText[30];
		LPWSTR pszStop;
		COLORREF cr;

		for( i = 0; i < nItemNum; ++i ){
			bCheck = (BOOL)List_GetItemData( hwndList, i );
			if( bCheck ){
				List_GetText( hwndList, i, szText );
				cr = wcstoul( szText, &pszStop, 10 );

				switch( m_wID )
				{
				case IDC_BUTTON_SAMETEXTCOLOR:
					for( j = 0; j < COLORIDX_LAST; ++j ){
						if( cr == m_pTypes->m_ColorInfoArr[j].m_colTEXT ){
							m_pTypes->m_ColorInfoArr[j].m_colTEXT = m_cr;
						}
					}
					break;

				case IDC_BUTTON_SAMEBKCOLOR:
					for( j = 0; j < COLORIDX_LAST; ++j ){
						if( cr == m_pTypes->m_ColorInfoArr[j].m_colBACK ){
							m_pTypes->m_ColorInfoArr[j].m_colBACK = m_cr;
						}
					}
					break;

				default:
					break;
				}
			}
		}
		break;

	case IDCANCEL:
		break;
	}
	return CDialog::OnBnClicked( wID );
}

/*! WM_DRAWITEM ����
	@date 2006.04.26 ryoji �V�K�쐬
*/
BOOL CDlgSameColor::OnDrawItem( WPARAM wParam, LPARAM lParam )
{
	LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;	// ���ڕ`����
	if( IDC_LIST_COLORS != pDis->CtlID )	// �I�[�i�[�`��ɂ��Ă���̂͐F�I�����X�g����
		return TRUE;

	//�`��Ώ�
	CGraphics gr(pDis->hDC);

	//
	// �F�I�����X�g�̕`�揈��
	//
	RECT		rc;
	WCHAR		szText[30];
	LPWSTR		pszStop;
	COLORREF	cr;

	List_GetText( pDis->hwndItem, pDis->itemID, szText );
	cr = wcstoul( szText, &pszStop, 10 );

	rc = pDis->rcItem;

	// �A�C�e����`�h��Ԃ�
	::FillRect( gr, &pDis->rcItem, ::GetSysColorBrush( COLOR_WINDOW ) );

	// �A�C�e�����I�����
	if( pDis->itemState & ODS_SELECTED ){
		rc = pDis->rcItem;
		rc.left += (rc.bottom - rc.top);
		::FillRect( gr, &rc, ::GetSysColorBrush( COLOR_HIGHLIGHT ) );
	}

	// �A�C�e���Ƀt�H�[�J�X������
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( gr, &pDis->rcItem );
	}

	// �`�F�b�N�{�b�N�X�\��
	rc = pDis->rcItem;
	rc.top += 2;
	rc.bottom -= 2;
	rc.left += 2;
	rc.right = rc.left + (rc.bottom - rc.top);
	UINT uState =  DFCS_BUTTONCHECK | DFCS_FLAT;
	if( TRUE == (BOOL)pDis->itemData )
		uState |= DFCS_CHECKED;		// �`�F�b�N���
	::DrawFrameControl( gr, &rc, DFC_BUTTON, uState );

	// �F���{��`
	rc = pDis->rcItem;
	rc.left += rc.bottom - rc.top + 2;
	rc.top += 2;
	rc.bottom -= 2;
	rc.right -= 2;
	gr.SetBrushColor( cr );
	gr.SetPen( ::GetSysColor(COLOR_3DSHADOW) );
	::RoundRect( gr, rc.left, rc.top, rc.right, rc.bottom , 5, 5 );

	return TRUE;
}

/*! �F�I�����X�g�� LBN_SELCHANGE ����
	@date 2006.05.01 ryoji �V�K�쐬
*/
BOOL CDlgSameColor::OnSelChangeListColors( HWND hwndCtl )
{
	// �F�I�����X�g�Ō��݃t�H�[�J�X�̂���F�ɂ���
	// �^�C�v�ʐݒ肩�瓯�F�̍��ڂ����o���č��ڃ��X�g�ɕ\������
	HWND hwndListInfo;
	COLORREF cr;
	WCHAR szText[30];
	LPWSTR pszStop;
	int i;
	int j;

	hwndListInfo = ::GetDlgItem( GetHwnd(), IDC_LIST_ITEMINFO );
	List_ResetContent( hwndListInfo );

	i = List_GetCaretIndex( hwndCtl );
	if( LB_ERR != i ){
		List_GetText( hwndCtl, i, szText );
		cr = wcstoul( szText, &pszStop, 10 );

		switch( m_wID )
		{
		case IDC_BUTTON_SAMETEXTCOLOR:
			for( j = 0; j < COLORIDX_LAST; ++j ){
				if( cr == m_pTypes->m_ColorInfoArr[j].m_colTEXT ){
					::List_AddString( hwndListInfo, m_pTypes->m_ColorInfoArr[j].m_szName);
				}
			}
			break;

		case IDC_BUTTON_SAMEBKCOLOR:
			for( j = 0; j < COLORIDX_LAST; ++j ){
			if( 0 != (g_ColorAttributeArr[j].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji �t���O���p�Ŋȑf��
					continue;
				if( cr == m_pTypes->m_ColorInfoArr[j].m_colBACK ){
					::List_AddString( hwndListInfo, m_pTypes->m_ColorInfoArr[j].m_szName);
				}
			}
			break;

		default:
			break;
		}
	}

	return TRUE;
}

/*! �T�u�N���X�����ꂽ�w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W��
	@date 2006.04.26 ryoji �V�K�쐬
*/
LRESULT CALLBACK CDlgSameColor::ColorStatic_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HDC			hDC;
	RECT		rc;

	CDlgSameColor* pCDlgSameColor;
	pCDlgSameColor = (CDlgSameColor*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch( uMsg ){
	case WM_PAINT:
		// �E�B���h�E�`��
		PAINTSTRUCT ps;

		hDC = ::BeginPaint( hwnd, &ps );

		// �F���{��`
		::GetClientRect( hwnd, &rc );
		rc.left += 2;
		rc.top += 2;
		rc.right -=2;
		rc.bottom -= 2;
		{
			CGraphics gr(hDC);
			gr.SetBrushColor( pCDlgSameColor->m_cr );
			gr.SetPen( ::GetSysColor(COLOR_3DSHADOW) );
			::RoundRect( gr, rc.left, rc.top, rc.right, rc.bottom, 5, 5 );
		}
		::EndPaint( hwnd, &ps );
		return (LRESULT)0;

	case WM_ERASEBKGND:
		// �w�i�`��
		hDC = (HDC)wParam;
		::GetClientRect( hwnd, &rc );

		// �e��WM_CTLCOLORSTATIC�𑗂��Ĕw�i�u���V���擾���A�w�i�`�悷��
		{
			HBRUSH	hBrush = (HBRUSH)::SendMessageAny( GetParent( hwnd ), WM_CTLCOLORSTATIC, wParam, (LPARAM)hwnd );
			HBRUSH	hBrushOld = (HBRUSH)::SelectObject( hDC, hBrush );
			::FillRect( hDC, &rc, hBrush );
			::SelectObject( hDC, hBrushOld );
		}
		return (LRESULT)1;

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)pCDlgSameColor->m_wpColorStaticProc );
		pCDlgSameColor->m_wpColorStaticProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return CallWindowProc( pCDlgSameColor->m_wpColorStaticProc, hwnd, uMsg, wParam, lParam );
}

/*! �T�u�N���X�����ꂽ�F�I�����X�g�̃E�B���h�E�v���V�[�W��
	@date 2006.04.26 ryoji �V�K�쐬
*/
LRESULT CALLBACK CDlgSameColor::ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	POINT po;
	RECT rcItem;
	RECT rc;
	int nItemNum;
	int i;

	CDlgSameColor* pCDlgSameColor;
	pCDlgSameColor = (CDlgSameColor*)::GetWindowLongPtr( hwnd, GWLP_USERDATA );

	switch( uMsg ){
	case WM_LBUTTONUP:
		// �}�E�X�{�^�����ɂ��鍀�ڂ̑I���^�I���������g�O������
		po.x = LOWORD(lParam);	// horizontal position of cursor
		po.y = HIWORD(lParam);	// vertical position of cursor
		nItemNum = List_GetCount( hwnd );
		for( i = 0; i < nItemNum; ++i ){
			List_GetItemRect( hwnd, i, &rcItem );
			rc = rcItem;
			rc.top += 2;
			rc.bottom -= 2;
			rc.left += 2;
			rc.right = rc.left + (rc.bottom - rc.top);
			if( ::PtInRect( &rc, po ) ){
				BOOL bCheck;
				bCheck = !(BOOL)List_GetItemData( hwnd, i );
				List_SetItemData( hwnd, i, bCheck );
				::InvalidateRect( hwnd, &rcItem, TRUE );
				break;
			}
		}
		break;

	case WM_KEYUP:
		// �t�H�[�J�X���ڂ̑I���^�I���������g�O������
		if( VK_SPACE == wParam ){
			BOOL bCheck;
			i = List_GetCaretIndex( hwnd );
			if( LB_ERR != i ){
				bCheck = !(BOOL)List_GetItemData( hwnd, i );
				List_SetItemData( hwnd, i, bCheck );
				::InvalidateRect( hwnd, NULL, TRUE );
			}
		}
		break;

	case WM_DESTROY:
		// �T�u�N���X������
		::SetWindowLongPtr( hwnd, GWLP_WNDPROC, (LONG_PTR)pCDlgSameColor->m_wpColorListProc );
		pCDlgSameColor->m_wpColorListProc = NULL;
		return (LRESULT)0;

	default:
		break;
	}

	return ::CallWindowProc( pCDlgSameColor->m_wpColorListProc, hwnd, uMsg, wParam, lParam );
}


