//	$Id$
/*!	@file
	@brief �t�@�C���^�C�v�ꗗ�_�C�A���O

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <windows.h>
#include <commctrl.h>
#include "sakura_rc.h"
#include "CDlgTypeList.h"
#include "etc_uty.h"
#include "debug.h"
#include "funccode.h"	//Stonee, 2001/03/12

// �^�C�v�ʐݒ�ꗗ CDlgTypeList.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12700
	IDC_BUTTON_TEMPCHANGE,	HIDC_TL_BUTTON_TEMPCHANGE,	//�ꎞ�K�p
	IDOK,					HIDOK_TL,					//�ݒ�
	IDCANCEL,				HIDCANCEL_TL,				//�L�����Z��
	IDC_BUTTON_HELP,		HIDC_TL_BUTTON_HELP,		//�w���v
	IDC_LIST_TYPES,			HIDC_TL_LIST_TYPES,			//���X�g
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

/* ���[�_���_�C�A���O�̕\�� */
int CDlgTypeList::DoModal( HINSTANCE hInstance, HWND hwndParent, int* pnSettingType )
{
	int	nRet;
	m_nSettingType = *pnSettingType;
	nRet = (int)CDialog::DoModal( hInstance, hwndParent, IDD_TYPELIST, NULL );
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

//@@@ 2002.01.18 add start
LPVOID CDlgTypeList::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end

/*[EOF]*/
