//	$Id$
/************************************************************************

	CDlgTypeList.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
#include <windows.h>
#include <commctrl.h>
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgTypeList.h"
#include "etc_uty.h"
#include "debug.h"

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, int* pnSettingType )
{
	int	nRet;	
	m_nSettingType = *pnSettingType;
	nRet = CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, NULL );
	if( -1 == nRet ){
		return FALSE;
	}else{
		*pnSettingType = nRet;
		return TRUE;
	}
	return nRet;
}

BOOL CDlgTypeList::OnLbnDblclk( int wID )
{
	switch( wID ){
	case IDC_LIST_TYPES:
		::EndDialog( m_hWnd, ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 ) );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�^�C�v�ʐݒ�ꗗ�v�̃w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 72 );
		return TRUE;
	case IDOK:
		::EndDialog( m_hWnd, ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 ) );
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, -1 );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
	
}


/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgTypeList::SetData( void )
{
	int		nIdx;
	HWND	hwndList;
	char	szText[130];
	hwndList = ::GetDlgItem( m_hWnd, IDC_LIST_TYPES );
	for( nIdx = 0; nIdx < MAX_TYPES; ++nIdx ){
		if( 0 < lstrlen( m_pShareData->m_Types[nIdx].m_szTypeExts ) ){		/* �^�C�v�����F�g���q���X�g */
			wsprintf( szText, "%s ( %s )", 
				m_pShareData->m_Types[nIdx].m_szTypeName,	/* �^�C�v�����F���� */
				m_pShareData->m_Types[nIdx].m_szTypeExts	/* �^�C�v�����F�g���q���X�g */
			);
		}else{
			wsprintf( szText, "%s", 
				m_pShareData->m_Types[nIdx].m_szTypeName	/* �^�C�v�����F�g�� */
			);
		}
		::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)szText );
	}
	::SendMessage( hwndList, LB_SETCURSEL, (WPARAM)m_nSettingType, (LPARAM)0 );
	return;
}

/*[EOF]*/
