/*!	@file
	@brief �t�@�C����r�_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, JEPRO
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgCompare.h"
#include "window/CEditWnd.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "util/string_ex2.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �t�@�C�����e��r CDlgCompare.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12300
//	IDC_STATIC,						-1,
	IDOK,							HIDOK_CMP,					//OK
	IDCANCEL,						HIDCANCEL_CMP,				//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		//�w���v
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		//���E�ɕ\��
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		//�t�@�C���ꗗ
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	//�\�[�X�t�@�C��
	0, 0
};	//@@@ 2002.01.07 add end MIK

static const SAnchorList anchorList[] = {
	{IDOK,					ANCHOR_BOTTOM},
	{IDCANCEL,				ANCHOR_BOTTOM},
	{IDC_BUTTON_HELP,		ANCHOR_BOTTOM},
	{IDC_CHECK_TILE_H,		ANCHOR_LEFT},
	{IDC_LIST_FILES,        ANCHOR_ALL},
	{IDC_STATIC_COMPARESRC, ANCHOR_LEFT_RIGHT},
};

CDlgCompare::CDlgCompare()
{
	/* �T�C�Y�ύX���Ɉʒu�𐧌䂷��R���g���[���� */
	assert( _countof(anchorList) == _countof(m_rcItems) );

	m_bCompareAndTileHorz = TRUE;	/* ���E�ɕ��ׂĕ\�� */
	return;
}


/* ���[�_���_�C�A���O�̕\�� */
int CDlgCompare::DoModal(
	HINSTANCE		hInstance,
	HWND			hwndParent,
	LPARAM			lParam,
	const TCHAR*	pszPath,
	bool			bIsModified,
	TCHAR*			pszCompareLabel,
	HWND*			phwndCompareWnd
)
{
	m_pszPath = pszPath;
	m_bIsModified = bIsModified;
	m_pszCompareLabel = pszCompareLabel;
	m_phwndCompareWnd = phwndCompareWnd;
	return CDialog::DoModal( hInstance, hwndParent, IDD_COMPARE, lParam );
}

BOOL CDlgCompare::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u���e��r�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//	From Here Oct. 10, 2000 JEPRO added  Ref. code ��CDlgFind.cpp �� OnBnClicked
//	�`�F�b�N�{�b�N�X���{�^��������CDlgCompare.cpp�ɒ��ڏ�������ł݂������s
//	�_�C�A���O�̃{�^���͉��ɕs�������Ă����Ă���܂��B
//	�ȉ��̒ǉ��R�[�h�͑S�������Č��\�ł�����N������Ă��������B�����X�N���[��������Ă����ƂȂ����ꂵ���ł��B
//	case IDC_BUTTON1:	/* �㉺�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		return TRUE;
//	case IDOK:			/* ���E�ɕ\�� */
//		/* �_�C�A���O�f�[�^�̎擾 */
//		HWND	hwndCompareWnd;
//		HWND*	phwndArr;
//		int		i;
//		phwndArr = new HWND[2];
//		phwndArr[0] = ::GetParent( m_hwndParent );
//		phwndArr[1] = hwndCompareWnd;
//		for( i = 0; i < 2; ++i ){
//			if( ::IsZoomed( phwndArr[i] ) ){
//				::ShowWindow( phwndArr[i], SW_RESTORE );
//			}
//		}
//		::TileWindows( NULL, MDITILE_VERTICAL, NULL, 2, phwndArr );
//		delete [] phwndArr;
//		CloseDialog( 0 );
//		return TRUE;
//	To Here Oct. 10, 2000
	case IDOK:			/* ���E�ɕ\�� */
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgCompare::SetData( void )
{
	HWND			hwndList;
	int				nRowNum;
	EditNode*		pEditNodeArr;
	EditInfo*		pfi;
	int				i;
	TCHAR			szMenu[512];
	int				nItem;

	hwndList = :: GetDlgItem( GetHwnd(), IDC_LIST_FILES );

//	2002/2/10 aroka �t�@�C�����Ŕ�r���Ȃ����ߕs�p (2001.12.26 YAZAKI����)
//	//	Oct. 15, 2001 genta �t�@�C��������� stricmp��bcc�ł����Ғʂ蓮��������
//	setlocale ( LC_ALL, "C" );

	/* ���݊J���Ă���ҏW���̃��X�g�����j���[�ɂ��� */
	nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
	if( nRowNum > 0 ){
		// �����X�N���[�����͎��ۂɕ\�����镶����̕����v�����Č��߂�	// 2009.09.26 ryoji
		HDC hDC = ::GetDC( hwndList );
		HFONT hFont = (HFONT)::SendMessageAny(hwndList, WM_GETFONT, 0, 0);
		HFONT hFontOld = (HFONT)::SelectObject(hDC, hFont);
		int nExtent = 0;	// ������̉���
		for( i = 0; i < nRowNum; ++i ){
			/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
			::SendMessageAny( pEditNodeArr[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0 );
			pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

//@@@ 2001.12.26 YAZAKI �t�@�C�����Ŕ�r�����(����)�������Ƃ��ɖ�蓯�m�̔�r���ł��Ȃ�
			if (pEditNodeArr[i].GetHwnd() == CEditWnd::getInstance()->GetHwnd()){
				// 2010.07.30 �����̖��O����������ݒ肷��
				CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, -1 );
				::DlgItem_SetText( GetHwnd(), IDC_STATIC_COMPARESRC, szMenu );
				continue;
			}
			// �ԍ��� �E�B���h�E���X�g�Ɠ����ɂȂ�悤�ɂ���
			CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( szMenu, _countof(szMenu), pfi, pEditNodeArr[i].m_nId, i );

			nItem = ::List_AddString( hwndList, szMenu );
			List_SetItemData( hwndList, nItem, pEditNodeArr[i].GetHwnd() );

			// �������v�Z����
			SIZE sizeExtent;
			if( ::GetTextExtentPoint32( hDC, szMenu, _tcslen(szMenu), &sizeExtent ) && sizeExtent.cx > nExtent ){
				nExtent = sizeExtent.cx;
			}
		}
		delete [] pEditNodeArr;
		// 2002/11/01 Moca �ǉ� ���X�g�r���[�̉�����ݒ�B��������Ȃ��Ɛ����X�N���[���o�[���g���Ȃ�
		::SelectObject(hDC, hFontOld);
		::ReleaseDC( hwndList, hDC );
		List_SetHorizontalExtent( hwndList, nExtent + 8 );
	}
	List_SetCurSel( hwndList, 0 );

	/* ���E�ɕ��ׂĕ\�� */
	//@@@ 2003.06.12 MIK
	// TAB 1�E�B���h�E�\���̂Ƃ��͕��ׂĔ�r�ł��Ȃ�����
	if( TRUE  == m_pShareData->m_Common.m_sTabBar.m_bDispTabWnd
	 && !m_pShareData->m_Common.m_sTabBar.m_bDispTabWndMultiWin )
	{
		m_bCompareAndTileHorz = FALSE;
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_CHECK_TILE_H ), FALSE );
	}
	::CheckDlgButton( GetHwnd(), IDC_CHECK_TILE_H, m_bCompareAndTileHorz );
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgCompare::GetData( void )
{
	HWND			hwndList;
	int				nItem;
	EditInfo*		pfi;
	hwndList = :: GetDlgItem( GetHwnd(), IDC_LIST_FILES );
	nItem = List_GetCurSel( hwndList );
	*m_phwndCompareWnd = (HWND)List_GetItemData( hwndList, nItem );
	/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
	::SendMessageAny( *m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0 );
	pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

	// 2010.07.30 �p�X���͂�߂ĕ\�����ɕύX
	int nId = CAppNodeManager::getInstance()->GetEditNode( *m_phwndCompareWnd )->m_nId;
	CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape( m_pszCompareLabel, _MAX_PATH/*�����s��*/, pfi, nId, -1 );

	/* ���E�ɕ��ׂĕ\�� */
	m_bCompareAndTileHorz = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_TILE_H );

	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgCompare::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

INT_PTR CDlgCompare::DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
	INT_PTR result;
	result = CDialog::DispatchEvent( hWnd, wMsg, wParam, lParam );

	if( wMsg == WM_GETMINMAXINFO ){
		return OnMinMaxInfo( lParam );
	}
	return result;
}

BOOL CDlgCompare::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd(hwndDlg);

	CreateSizeBox();
	CDialog::OnSize();
	
	RECT rc;
	::GetWindowRect( hwndDlg, &rc );
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for( int i = 0; i < _countof(anchorList); i++ ){
		GetItemClientRect( anchorList[i].id, m_rcItems[i] );
	}

	RECT rcDialog = GetDllShareData().m_Common.m_sOthers.m_rcCompareDialog;
	if( rcDialog.left != 0 ||
		rcDialog.bottom != 0 ){
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}

	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}

BOOL CDlgCompare::OnSize( WPARAM wParam, LPARAM lParam )
{
	/* ���N���X�����o */
	CDialog::OnSize( wParam, lParam );

	::GetWindowRect( GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcCompareDialog );

	RECT  rc;
	POINT ptNew;
	::GetWindowRect( GetHwnd(), &rc );
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for( int i = 0 ; i < _countof(anchorList); i++ ){
		ResizeItem( GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor );
	}
	::InvalidateRect( GetHwnd(), NULL, TRUE );
	return TRUE;
}

BOOL CDlgCompare::OnMove( WPARAM wParam, LPARAM lParam )
{
	::GetWindowRect( GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcCompareDialog );
	
	return CDialog::OnMove( wParam, lParam );
}

BOOL CDlgCompare::OnMinMaxInfo( LPARAM lParam )
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*2;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*3;
	return 0;
}