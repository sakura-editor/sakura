//	$Id$
/*!	@file
	�t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <windows.h>
#include <commctrl.h>
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgTypeList.h"
#include "etc_uty.h"
#include "debug.h"
#include "funccode.h"	//Stonee, 2001/03/12

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
		//	Nov. 29, 2000	genta
		//	����ύX: �w��^�C�v�̐ݒ�_�C�A���O���ꎞ�I�ɕʂ̐ݒ��K�p
		::EndDialog( m_hWnd, ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 )
			| PROP_TEMPCHANGE_FLAG );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgTypeList::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�^�C�v�ʐݒ�ꗗ�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_TYPE_LIST) );
		return TRUE;
	//	Nov. 29, 2000	From Here	genta
	//	�K�p����^�̈ꎞ�I�ύX
	case IDC_BUTTON_TEMPCHANGE:
		::EndDialog( m_hWnd, ::SendMessage( GetDlgItem( m_hWnd, IDC_LIST_TYPES ), LB_GETCURSEL, (WPARAM)0, (LPARAM)0 )
			| PROP_TEMPCHANGE_FLAG );
		return TRUE;
	//	Nov. 29, 2000	To Here
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
