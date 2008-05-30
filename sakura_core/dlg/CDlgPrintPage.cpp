/*!	@file
	@brief ����_�C�A���O�{�b�N�X

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, aroka, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "stdafx.h"
#include "dlg/CDlgPrintPage.h"
#include "sakura_rc.h"
#include "debug/Debug.h"// 2002/2/10 aroka

// ��� CDlgPrintPage.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
const DWORD p_helpids[] = {	//12400
	IDOK,							HIDOK_PP,				//OK
	IDCANCEL,						HIDCANCEL_PP,			//�L�����Z��
	IDC_EDIT_FROM,					HIDC_PP_EDIT_FROM,		//�y�[�W����
	IDC_EDIT_TO,					HIDC_PP_EDIT_TO,		//�y�[�W�܂�
	IDC_RADIO_ALL,					HIDC_PP_RADIO_ALL,		//���ׂ�
	IDC_RADIO_FROMTO,				HIDC_PP_RADIO_FROMTO,	//�w��
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgPrintPage::CDlgPrintPage()
{
	return;
}




/* ���[�_���_�C�A���O�̕\�� */
int CDlgPrintPage::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_PRINTPAGE, lParam );
}




/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgPrintPage::SetData( void )
{
	TCHAR szText[100];
//	From Here Sept. 12, 2000 JEPRO �X�^�C���������ύX(�w���I�񂾎���������͈͂���͂ł���悤�ɂ���)
//	��x�ݒ肵���y�[�W�ݒ�l��ۑ��ł���悤�ɂ��������܂��ł��ĂȂ�
//	::CheckDlgButton( GetHwnd(), IDC_RADIO_ALL, m_bAllPage?BST_CHECKED:BST_UNCHECKED );
//	auto_sprintf( szText, "%d �` %d ��", m_nPageMin, m_nPageMax );
//	::DlgItem_SetText( GetHwnd(), IDC_STATIC_ALL, szText );
//
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, m_nPageFrom, FALSE );
//	::SetDlgItemInt( GetHwnd(), IDC_EDIT_TO, m_nPageTo, FALSE );
//
	if( TRUE == m_bAllPage ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_ALL, BST_CHECKED );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_FROMTO, BST_CHECKED );
	}
	auto_sprintf( szText, _T("%d �` %d �y�[�W"), m_nPageMin, m_nPageMax );
	::DlgItem_SetText( GetHwnd(), IDC_STATIC_ALL, szText );

	::SetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, m_nPageFrom, FALSE );
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_TO, m_nPageTo, FALSE );

	//	����͈͂��w�肷�邩�ǂ���
	if( !m_bAllPage ){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), FALSE );
	}
//	To Here Sept. 12, 2000

	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgPrintPage::GetData( void )
{
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_ALL ) ){
		m_bAllPage = TRUE;
	}else{
		m_bAllPage = FALSE;
	}
	m_nPageFrom = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_FROM, NULL, FALSE );
	m_nPageTo	= ::GetDlgItemInt( GetHwnd(), IDC_EDIT_TO  , NULL, FALSE );
	/* �Ŕ͈̓`�F�b�N */
	if( !m_bAllPage ){
		if( m_nPageMin <= m_nPageFrom && m_nPageFrom <= m_nPageMax &&
			m_nPageMin <= m_nPageTo   && m_nPageTo <= m_nPageMax &&
			m_nPageFrom <= m_nPageTo
		){
		}else{
			::MYMESSAGEBOX(	GetHwnd(), MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("���̓G���["), _T("�y�[�W�͈͎w�肪����������܂���B") );
			return FALSE;
		}
	}
	return TRUE;
}




BOOL CDlgPrintPage::OnBnClicked( int wID )
{
	switch( wID ){
//	From Here Sept. 12, 2000 JEPRO �X�^�C���������ύX(�w���I�񂾎���������͈͂���͂ł���悤�ɂ���)
//	��x�ݒ肵���y�[�W�ݒ�l��ۑ��ł���悤�ɂ��������܂��ł��ĂȂ�
	case IDC_RADIO_ALL:
	case IDC_RADIO_FROMTO:
	//	����͈͂��w�肷�邩�ǂ���
		if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_FROMTO ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), TRUE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_FROM ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_FROM ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_TO ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_TO ), FALSE );
		}
		return TRUE;
//	To Here Sept. 12, 2000

	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		if( GetData() ){
			CloseDialog( 1 );
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgPrintPage::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


