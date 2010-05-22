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

#include "stdafx.h"
#include "dlg/CDlgCompare.h"
#include "global.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "util/shell.h"
#include "debug/Debug.h"
#include "sakura_rc.h"

// �t�@�C�����e��r CDlgCompare.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12300
//	IDC_BUTTON1,					HIDC_CMP_BUTTON1,			//�㉺�ɕ\��
//	IDOK2,							HIDOK2_CMP,					//���E�ɕ\��
	IDOK,							HIDOK_CMP,					//OK
	IDCANCEL,						HIDCANCEL_CMP,				//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_CMP_BUTTON_HELP,		//�w���v
	IDC_CHECK_TILE_H,				HIDC_CMP_CHECK_TILE_H,		//���E�ɕ\��
	IDC_LIST_FILES,					HIDC_CMP_LIST_FILES,		//�t�@�C���ꗗ
	IDC_STATIC_COMPARESRC,			HIDC_CMP_STATIC_COMPARESRC,	//�\�[�X�t�@�C��
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgCompare::CDlgCompare()
{
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
	TCHAR*			pszComparePath,
	HWND*			phwndCompareWnd
)
{
	m_pszPath = pszPath;
	m_bIsModified = bIsModified;
	m_pszComparePath = pszComparePath;
	m_phwndCompareWnd = phwndCompareWnd;
	return CDialog::DoModal( hInstance, hwndParent, IDD_COMPARE, lParam );
}

BOOL CDlgCompare::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u���e��r�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_COMPARE) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
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
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
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
	nRowNum = CAppNodeManager::Instance()->GetOpenedWindowArr( &pEditNodeArr, TRUE );
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
//			if( 0 == stricmp( pfi->m_szPath, m_pszPath ) ){
			if (pEditNodeArr[i].GetHwnd() == CEditWnd::Instance()->GetHwnd()){
				continue;
			}
			auto_sprintf(
				szMenu,
				_T("%ts %ts"),
				(0 < _tcslen(pfi->m_szPath))?pfi->m_szPath:_T("(����)"),
				pfi->m_bIsModified ? _T("*"):_T(" ")
			);
			// gm_pszCodeNameArr_Bracket ����R�s�[����悤�ɕύX
			if(IsValidCodeTypeExceptSJIS(pfi->m_nCharCode)){
				_tcscat( szMenu, CCodeTypeName(pfi->m_nCharCode).Bracket() );
			}
			nItem = ::List_AddString( hwndList, szMenu );
			::SendMessageAny( hwndList, LB_SETITEMDATA, nItem, (LPARAM)pEditNodeArr[i].GetHwnd() );

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
		::SendMessageAny( hwndList, LB_SETHORIZONTALEXTENT, (WPARAM)(nExtent + 8), 0 );
	}
	::SendMessageAny( hwndList, LB_SETCURSEL, (WPARAM)0, 0 );
	TCHAR	szWork[512];
	auto_sprintf( szWork, _T("%ts %ts"),
		(0 < _tcslen( m_pszPath )?m_pszPath:_T("(����)") ),
		m_bIsModified?_T("*"):_T("")
	);
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_COMPARESRC, szWork );
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
	nItem = ::SendMessageAny( hwndList, LB_GETCURSEL, 0, 0 );
	*m_phwndCompareWnd = (HWND)::SendMessageAny( hwndList, LB_GETITEMDATA, nItem, 0 );
	/* �g���C����G�f�B�^�ւ̕ҏW�t�@�C�����v���ʒm */
	::SendMessageAny( *m_phwndCompareWnd, MYWM_GETFILEINFO, 0, 0 );
	pfi = (EditInfo*)&m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

	_tcscpy( m_pszComparePath, pfi->m_szPath );

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


