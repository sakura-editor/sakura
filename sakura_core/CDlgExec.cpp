//	$Id$
/************************************************************************
	CDlgExec.cpp
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
#include "CDlgExec.h"
//#include "funccode.h"
#include "sakura_rc.h"

CDlgExec::CDlgExec()
{
	m_szCommand[0] = '\0';		/* �R�}���h���C�� */
	m_bGetStdout = TRUE;	// �W���o�͂𓾂�

	return;
}




/* ���[�_���_�C�A���O�̕\�� */
int CDlgExec::DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam )
{
	m_szCommand[0] = '\0';	/* �R�}���h���C�� */
	return CDialog::DoModal( hInstance, hwndParent, IDD_EXEC, lParam );
}




/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgExec::SetData( void )
{
//	MYTRACE( "CDlgExec::SetData()" );
	int		i;
	HWND	hwndCombo;
	
	/*****************************
	*  ������                    *
	*****************************/
	/* ���[�U�[���R���{ �{�b�N�X�̃G�f�B�b�g �R���g���[���ɓ��͂ł���e�L�X�g�̒����𐧌����� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_LIMITTEXT, (WPARAM)sizeof( m_szCommand ) - 1, 0 );
	/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
	::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand ), CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );


	// �W���o�͂𓾂�
//	From Here Sept. 12, 2000 jeprotest
	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, m_bGetStdout?BST_CHECKED:BST_UNCHECKED );
//	::CheckDlgButton( m_hWnd, IDC_CHECK_GETSTDOUT, TRUE );
//	To Here Sept. 12, 2000 	���܂������Ȃ��̂Ō��ɖ߂��Ă���

	
	/*****************************
	*  �f�[�^�ݒ�                *
	*****************************/
//	HWND hwndCombo;
	strcpy( m_szCommand, m_pShareData->m_szCmdArr[0] );
	hwndCombo = ::GetDlgItem( m_hWnd, IDC_COMBO_m_szCommand );
	::SendMessage( hwndCombo, CB_RESETCONTENT, 0, 0 );
	::SetDlgItemText( m_hWnd, IDC_COMBO_TEXT, m_szCommand );
	for( i = 0; i < m_pShareData->m_nCmdArrNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)m_pShareData->m_szCmdArr[i] );
	}
	::SendMessage( hwndCombo, CB_SETCURSEL, 0, 0 );
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
int CDlgExec::GetData( void )
{
	::GetDlgItemText( m_hWnd, IDC_COMBO_m_szCommand, m_szCommand, sizeof( m_szCommand ) - 1 );
	
	// �W���o�͂𓾂�
//	From Here Sept. 12, 2000 jeprotest
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
//	To Here Sept. 12, 2000 �@���܂������Ȃ��̂Ō��ɖ߂��Ă���
		 m_bGetStdout = TRUE;
	}else{
		 m_bGetStdout = FALSE;
	}
	return 1;
}



BOOL CDlgExec::OnBnClicked( int wID )
{
//	int	nRet;
//	CEditView*	pcEditView = (CEditView*)m_lParam;
	switch( wID ){
	//	From Here Sept. 12, 2000 jeprotest
	case IDC_CHECK_GETSTDOUT:
//		if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
		if( ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_GETSTDOUT ) ){
			 m_bGetStdout = TRUE;
		}else{
			 m_bGetStdout = FALSE;
		}
		break;
	//	To Here Sept. 12, 2000 ���܂������Ȃ��̂Ō��ɖ߂��Ă���
	case IDC_BUTTON_HELP:
		/* �u�����v�̃w���v */
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, 103 );
		break;
	case IDOK:			/* ������ */
		/* �_�C�A���O�f�[�^�̎擾 */
		GetData();
		CloseDialog( 1 );
		return TRUE;
	case IDCANCEL:
		CloseDialog( 0 );
		return TRUE;
	}
	return FALSE;
}


/*[EOF]*/

