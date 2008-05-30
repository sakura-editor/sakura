/*!	@file
	@brief �����̊Ǘ��_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
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



#include "stdafx.h"
#include <stdio.h>
#include "sakura_rc.h"
#include "global.h"
#include "funccode.h"
#include "mymessage.h"
#include "dlg/CDialog.h"
#include "dlg/CDlgFavorite.h"
#include "debug/Debug.h"
#include "util/shell.h"

#include "sakura.hh"
const DWORD p_helpids[] = {
	IDC_TAB_FAVORITE,				HIDC_TAB_FAVORITE,				//�^�u
	IDC_LIST_FAVORITE_FILE,			HIDC_LIST_FAVORITE_FILE,		//�t�@�C��
	IDC_LIST_FAVORITE_FOLDER,		HIDC_LIST_FAVORITE_FOLDER,		//�t�H���_
	IDC_LIST_FAVORITE_SEARCH,		HIDC_LIST_FAVORITE_SEARCH,		//����
	IDC_LIST_FAVORITE_REPLACE,		HIDC_LIST_FAVORITE_REPLACE,		//�u��
	IDC_LIST_FAVORITE_GREP_FILE,	HIDC_LIST_FAVORITE_GREPFILE,	//GREP�t�@�C��
	IDC_LIST_FAVORITE_GREP_FOLDER,	HIDC_LIST_FAVORITE_GREPFOLDER,	//GREP�t�H���_
	IDC_LIST_FAVORITE_CMD,			HIDC_LIST_FAVORITE_CMD,			//�R�}���h
	IDC_BUTTON_CLEAR,				HIDC_BUTTON_FAVORITE_CLEAR,		//�����̃N���A	// 2006.10.10 ryoji
	IDOK,							HIDC_FAVORITE_IDOK,				//OK
	IDCANCEL,						HIDC_FAVORITE_IDCANCEL,			//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_BUTTON_FAVORITE_HELP,		//�w���v
//	IDC_STATIC,						-1,
	0, 0
};



//SDK�ɂ�����`����Ă��Ȃ��B
#ifndef	ListView_SetCheckState
//#if (_WIN32_IE >= 0x0300)
#define ListView_SetCheckState(hwndLV, i, fCheck) \
  ListView_SetItemState(hwndLV, i, INDEXTOSTATEIMAGEMASK((fCheck)?2:1), LVIS_STATEIMAGEMASK)
//#endif
#endif



CDlgFavorite::CDlgFavorite()
{
	int	i;

	m_nCurrentTab = 0;
	_tcscpy( m_szMsg, _T("") );

	{
		memset_raw( m_aFavoriteInfo, 0, sizeof( m_aFavoriteInfo ) );

		i = 0;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("�t�@�C��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("�t�H���_");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = true;
		m_aFavoriteInfo[i].m_bHaveView  = true;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentSearch;
		m_aFavoriteInfo[i].m_pszCaption = _T("����");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_SEARCH;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentReplace;
		m_aFavoriteInfo[i].m_pszCaption = _T("�u��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_REPLACE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFile;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREP�t�@�C��");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FILE;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentGrepFolder;
		m_aFavoriteInfo[i].m_pszCaption = _T("GREP�t�H���_");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_GREP_FOLDER;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = &m_cRecentCmd;
		m_aFavoriteInfo[i].m_pszCaption = _T("�R�}���h");
		m_aFavoriteInfo[i].m_nId        = IDC_LIST_FAVORITE_CMD;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		i++;
		m_aFavoriteInfo[i].m_pRecent    = NULL;
		m_aFavoriteInfo[i].m_pszCaption = NULL;
		m_aFavoriteInfo[i].m_nId        = -1;
		m_aFavoriteInfo[i].m_bHaveFavorite = false;
		m_aFavoriteInfo[i].m_bHaveView  = false;

		/* ����ȏ㑝�₷�Ƃ��̓e�[�u���T�C�Y�����������Ă� */
	}
}

CDlgFavorite::~CDlgFavorite()
{
	for( int nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		m_aFavoriteInfo[nTab].m_pRecent->Terminate();
	}
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgFavorite::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_FAVORITE, lParam );
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFavorite::SetData( void )
{
	int		nTab;

	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		SetDataOne( nTab, 0 );
	}

	::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );

	return;
}

/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgFavorite::SetDataOne( int nIndex, int nCurrentIndex )
{
	HWND	hwndList;
	int		i;
	LV_ITEM	lvi;
	int		nNewFocus = -1;

	CRecent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

	/* ���X�g */
	hwndList = ::GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	ListView_DeleteAllItems( hwndList );  /* ���X�g����ɂ��� */

	int		nViewCount = pRecent->GetViewCount();
	m_aFavoriteInfo[nIndex].m_nViewCount = nViewCount;

	TCHAR	tmp[1024];
	for( i = 0; i < pRecent->GetItemCount(); i++ )
	{
		auto_sprintf( tmp, _T("%ts"), (i < nViewCount) ? _T(" ") : _T("(��\��)") );
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = tmp;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndList, &lvi );

		const TCHAR	*p;
		p = pRecent->GetItemText( i );
		auto_sprintf( tmp, _T("%ts"), p ? p : _T("") );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = tmp;
		ListView_SetItem( hwndList, &lvi );

		if( m_aFavoriteInfo[nIndex].m_bHaveFavorite )
		{
			ListView_SetCheckState( hwndList, i, (BOOL)pRecent->IsFavorite( i ) );
		}
		ListView_SetItemState( hwndList, i, 0, LVIS_SELECTED | LVIS_FOCUSED );

		if( i == nCurrentIndex ) nNewFocus = i;
	}

	//�A�C�e���������Ăǂ����I���Ȃ�ŏ���I������B
	if( i > 0 && -1 != nCurrentIndex && nNewFocus == -1 )
	{
		nNewFocus = 0;
	}

	if( -1 != nNewFocus )
	{
		ListView_SetItemState( hwndList, nNewFocus, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
		ListView_EnsureVisible( hwndList, nNewFocus, FALSE );
	}

	return;
}

/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[ */
int CDlgFavorite::GetData( void )
{
	HWND	hwndList;
	int		nCount;
	int		i;
	BOOL	bret;
	int		nTab;

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		CRecent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;

		if( m_aFavoriteInfo[nTab].m_bHaveFavorite )
		{
			hwndList = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nTab].m_nId );

			nCount = ListView_GetItemCount( hwndList );

			//���C�ɓ���t���O���X�V����B
			for( i = 0; i < nCount; i++ )
			{
				bret = ListView_GetCheckState( hwndList, i );
				pRecent->SetFavorite( i, bret ? true : false );
			}

			//���X�g���X�V����B
			pRecent->UpdateView();
		}
	}

	return TRUE;
}

BOOL CDlgFavorite::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	HWND		hwndList;
	HWND		hwndBaseList;
	HWND		hwndTab;
	TCITEM		tcitem;
	LV_COLUMN	col;
	RECT		rc;
	int			nTab;
	POINT		po;
	long		lngStyle;

	_SetHwnd( hwndDlg );

	hwndTab = ::GetDlgItem( hwndDlg, IDC_TAB_FAVORITE );
	TabCtrl_DeleteAllItems( hwndTab );

	//���X�g�r���[�̕\���ʒu���擾����B
	m_nCurrentTab = 0;
	hwndBaseList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[0].m_nId );
	{
		rc.left = rc.top = rc.right = rc.bottom = 0;
		::GetWindowRect( hwndBaseList, &rc );
		po.x = rc.left;
		po.y = rc.top;
		::ScreenToClient( hwndDlg, &po );
		rc.left = po.x;
		rc.top  = po.y;
		po.x = rc.right;
		po.y = rc.bottom;
		::ScreenToClient( hwndDlg, &po );
		rc.right  = po.x;
		rc.bottom = po.y;
	}

	for( nTab = 0; m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		CRecent* pRecent = m_aFavoriteInfo[nTab].m_pRecent;

		hwndList = GetDlgItem( hwndDlg, m_aFavoriteInfo[nTab].m_nId );
		::MoveWindow( hwndList, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, FALSE );
		::ShowWindow( hwndList, SW_HIDE );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 16 / 100;
		col.pszText  = _T("���C�ɓ���");
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );

		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 79 / 100;
		col.pszText  = m_aFavoriteInfo[nTab].m_pszCaption;
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		/* �s�I�� */
		lngStyle = ::SendMessageAny( hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
		lngStyle |= LVS_EX_FULLROWSELECT;
		if( m_aFavoriteInfo[nTab].m_bHaveFavorite ) lngStyle |= LVS_EX_CHECKBOXES;
		::SendMessageAny( hwndList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lngStyle );

		/* �^�u���ڒǉ� */
		tcitem.mask = TCIF_TEXT;
		tcitem.pszText = m_aFavoriteInfo[nTab].m_pszCaption;
		TabCtrl_InsertItem( hwndTab, nTab, &tcitem );
	}

	hwndList = ::GetDlgItem( hwndDlg, m_aFavoriteInfo[m_nCurrentTab].m_nId );
	::ShowWindow( hwndList, SW_SHOW );
	TabCtrl_SetCurSel( hwndTab, m_nCurrentTab );
	//ChangeSlider( m_nCurrentTab );

	/* ���N���X�����o */
	return CDialog::OnInitDialog( GetHwnd(), wParam, lParam );
}

BOOL CDlgFavorite::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_HELP:
		/* �w���v */
		MyWinHelp( GetHwnd(), m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID( F_FAVORITE ) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;

	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		::EndDialog( GetHwnd(), (BOOL)GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	case IDC_BUTTON_CLEAR:	//�����̃N���A
		{
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			HWND	hwndTab;
			int		nIndex;
			hwndTab = ::GetDlgItem( GetHwnd(), IDC_TAB_FAVORITE );
			nIndex = TabCtrl_GetCurSel( hwndTab );
			if( -1 != nIndex )
			{
				int	nRet;
				
				if( m_aFavoriteInfo[nIndex].m_bHaveFavorite )
				{
					nRet = ::MYMESSAGEBOX( GetHwnd(), 
						MB_YESNOCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("�ŋߎg����%ts�̗������폜���܂��B\n\n��낵���ł����H\n\n")
						_T("�u�͂��v\t���ׂč폜���܂��B\n")
						_T("�u�������v\t���C�ɓ���ȊO���폜���܂��B\n"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
				}
				else
				{
					nRet = ::MYMESSAGEBOX( GetHwnd(), 
						MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("�ŋߎg����%ts�̗������폜���܂��B\n\n��낵���ł����H\n"),
						m_aFavoriteInfo[nIndex].m_pszCaption );
				}
				
				CRecent	*pRecent = m_aFavoriteInfo[nIndex].m_pRecent;

				switch( nRet )
				{
				case IDYES:
				case IDOK:
					if( pRecent ) pRecent->DeleteAllItem();
					InfoMessage( GetHwnd(), _T("�ŋߎg����%ts�̗������폜���܂����B"), m_aFavoriteInfo[nIndex].m_pszCaption );
					break;
					
				case IDNO:
					if( pRecent )
					{
						HWND hwndList = ::GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
						for( int i = pRecent->GetItemCount() - 1; i >= 0; i-- )
						{
							if( ! ListView_GetCheckState( hwndList, i ) ){
								pRecent->DeleteItem( i );
							}
							else {
								pRecent->SetFavorite( i );
							}
						}
					}
					InfoMessage( GetHwnd(), _T("�ŋߎg����%ts�̗���(���C�ɓ���ȊO)���폜���܂����B"), m_aFavoriteInfo[nIndex].m_pszCaption );
					break;
					
				case IDCANCEL:
				default:
					break;
				}
			}
		}
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFavorite::OnNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMHDR	lpnmhdr;
	HWND	hwndTab;
	HWND	hwndList;
	int		nIndex;

	hwndTab = ::GetDlgItem( GetHwnd(), IDC_TAB_FAVORITE );
	lpnmhdr = (LPNMHDR) lParam;
	if( lpnmhdr->hwndFrom == hwndTab )
	{
		switch( lpnmhdr->code )
		{
		case TCN_SELCHANGE:
			::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, _T("") );
			nIndex = TabCtrl_GetCurSel( hwndTab );
			if( -1 != nIndex )
			{
				//���ݕ\�����̃��X�g���B���B
				hwndList = GetDlgItem( GetHwnd(), m_aFavoriteInfo[m_nCurrentTab].m_nId );
				::ShowWindow( hwndList, SW_HIDE );

				//�V�����\������B
				hwndList = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
				::ShowWindow( hwndList, SW_SHOW );

				::SetFocus( hwndList );

				m_nCurrentTab = nIndex;

				//ChangeSlider( nIndex );
			}
			return TRUE;
			//break;
		}
	}

	/* ���N���X�����o */
	return CDialog::OnNotify( wParam, lParam );
}

BOOL CDlgFavorite::OnActivate( WPARAM wParam, LPARAM lParam )
{
	bool	bret;

	switch( LOWORD( wParam ) )
	{
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		bret = RefreshList();
		::DlgItem_SetText( GetHwnd(), IDC_STATIC_FAVORITE_MSG, m_szMsg );
		return TRUE;
		//break;

	case WA_INACTIVE:
	default:
		break;
	}

	/* ���N���X�����o */
	return CDialog::OnActivate( wParam, lParam );
}

LPVOID CDlgFavorite::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}

/*
	���X�g���X�V����B
*/
bool CDlgFavorite::RefreshList( void )
{
	int		nTab;
	bool	bret;
	bool	ret_val = false;
	TCHAR	msg[1024];

	_tcscpy( msg, _T("") );
	_tcscpy( m_szMsg, _T("") );

	//�S���X�g�̌��ݑI�𒆂̃A�C�e�����擾����B
	for( nTab = 0; NULL != m_aFavoriteInfo[nTab].m_pRecent; nTab++ )
	{
		bret = RefreshListOne( nTab );
		if( bret == true )
		{
			ret_val = true;
		
			if( _tcslen( msg ) > 0 ) _tcscat( msg, _T("�A") );
			_tcscat( msg, m_aFavoriteInfo[nTab].m_pszCaption );
		}
	}

	if( ret_val )
	{
		auto_sprintf( m_szMsg, 
			_T("����(%ts)���X�V���ꂽ���ߕҏW������j�����ĕ\�����܂����B"),
			msg );
	}

	return ret_val;
}

/*
	���X�g�̂���1���X�V����B
*/
bool CDlgFavorite::RefreshListOne( int nIndex )
{
	HWND	hwndList;
	int		nCount;
	int		nCurrentIndex;
	int		nItemCount;
	int		i;
	BOOL	bret;
	LVITEM	lvitem;

	CRecent*	pRecent = m_aFavoriteInfo[nIndex].m_pRecent;
	nItemCount    = pRecent->GetItemCount();
	hwndList      = GetDlgItem( GetHwnd(), m_aFavoriteInfo[nIndex].m_nId );
	nCount        = ListView_GetItemCount( hwndList );
	nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
	if( -1 == nCurrentIndex ) nCurrentIndex = ListView_GetNextItem( hwndList, -1, LVNI_FOCUSED );

	if( nItemCount != nCount ) goto changed;	//�����ς�����̂ōč\�z

	//���C�ɓ��萔���ς�����̂ōč\�z
	if( m_aFavoriteInfo[nIndex].m_nViewCount != pRecent->GetViewCount() ) goto changed;

	for( i = 0; i < nCount; i++ )
	{
		TCHAR	szText[1024];
		auto_memset( szText, 0, _countof( szText ) );
		memset_raw( &lvitem, 0, sizeof( lvitem ) );
		lvitem.mask       = LVIF_TEXT;
		lvitem.pszText    = szText;
		lvitem.cchTextMax = _countof( szText );
		lvitem.iItem      = i;
		lvitem.iSubItem   = 1;
		bret = ListView_GetItem( hwndList, &lvitem );
		if( !bret ) goto changed;	//�G���[�Ȃ̂ōč\�z

		//�A�C�e�����e���ς�����̂ōč\�z
		if( i != pRecent->FindItemByText( szText ) ) goto changed;
	}

	return false;

changed:
	SetDataOne( nIndex, nCurrentIndex );
	

	return true;
}


