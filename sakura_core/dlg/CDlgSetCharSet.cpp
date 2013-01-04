/*!	@file
	@brief �����R�[�h�Z�b�g�ݒ�_�C�A���O�{�b�N�X

	@author Uchi
	@date 2010/6/14  �V�K�쐬
*/
/*
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgSetCharSet.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "env/DLLSHAREDATA.h"
#include "sakura_rc.h"
#include "sakura.hh"

//�����R�[�h�Z�b�g�ݒ� CDlgSetCharSet
const DWORD p_helpids[] = {
	IDOK,							HIDOK_GREP,							//����
	IDCANCEL,						HIDCANCEL_GREP,						//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_GREP_BUTTON_HELP,				//�w���v
	IDC_COMBO_CHARSET,				HIDC_OPENDLG_COMBO_CODE,			//�����R�[�h�Z�b�g
	IDC_CHECK_BOM,					HIDC_OPENDLG_CHECK_BOM,				//����
	0, 0
};



CDlgSetCharSet::CDlgSetCharSet()
{
	m_pnCharSet = NULL;			// �����R�[�h�Z�b�g
	m_pbBom = NULL;				// �����R�[�h�Z�b�g
}



/* ���[�_���_�C�A���O�̕\�� */
int CDlgSetCharSet::DoModal( HINSTANCE hInstance, HWND hwndParent, ECodeType* pnCharSet, bool* pbBom)
{
	m_pnCharSet = pnCharSet;	// �����R�[�h�Z�b�g
	m_pbBom = pbBom;			// BOM

	return (int)CDialog::DoModal( hInstance, hwndParent, IDD_SETCHARSET, NULL );
}



BOOL CDlgSetCharSet::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd( hwndDlg );
	
	m_hwndCharSet = ::GetDlgItem( GetHwnd(), IDC_COMBO_CHARSET );	// �����R�[�h�Z�b�g�R���{�{�b�N�X
	m_hwndCheckBOM = ::GetDlgItem( GetHwnd(), IDC_CHECK_BOM );		// BOM�`�F�b�N�{�b�N�X

	// �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ���
	Combo_SetExtendedUI( m_hwndCharSet, TRUE );

	// �����R�[�h�Z�b�g�I���R���{�{�b�N�X������
	int i;
	CCodeTypesForCombobox cCodeTypes;
	Combo_ResetContent( m_hwndCharSet );
	for (i = 1; i < cCodeTypes.GetCount(); ++i) {
		int idx = Combo_AddString( m_hwndCharSet, cCodeTypes.GetName(i) );
		Combo_SetItemData( m_hwndCharSet, idx, cCodeTypes.GetCode(i) );
	}

	/* ���N���X�����o */
	return CDialog::OnInitDialog( hwndDlg, wParam, lParam );
}



BOOL CDlgSetCharSet::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�����R�[�h�Z�b�g�ݒ�v�̃w���v */
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_CHG_CHARSET) );
		return TRUE;
	case IDOK:
		/* �_�C�A���O�f�[�^�̎擾 */
		if( GetData() ){
			CloseDialog( TRUE );
		}
		return TRUE;
	case IDCANCEL:
		CloseDialog( FALSE );
		return TRUE;
	}

	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}



// BOM �̐ݒ�
void CDlgSetCharSet::SetBOM( void )
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	nIdx = Combo_GetCurSel( m_hwndCharSet );
	lRes = Combo_GetItemData( m_hwndCharSet, nIdx );
	CCodeTypeName	cCodeTypeName( lRes );
	if (cCodeTypeName.UseBom()) {
		::EnableWindow( m_hwndCheckBOM, TRUE );
		if (lRes == *m_pnCharSet) {
			fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
		}
		else{
			fCheck = cCodeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
		}
	}
	else {
		::EnableWindow( m_hwndCheckBOM, FALSE );
		fCheck = BST_UNCHECKED;
	}
	BtnCtl_SetCheck( m_hwndCheckBOM, fCheck );
}



// �����R�[�h�I�����̏���
BOOL CDlgSetCharSet::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int 		nIdx;
	LRESULT		lRes;
	WPARAM		fCheck;

	switch (wID) {
	//	�����R�[�h�̕ύX��BOM�`�F�b�N�{�b�N�X�ɔ��f
	case IDC_COMBO_CHARSET:
		SetBOM();
		nIdx = Combo_GetCurSel( hwndCtl );
		lRes = Combo_GetItemData( hwndCtl, nIdx );
		CCodeTypeName	cCodeTypeName( lRes );
		if (cCodeTypeName.UseBom()) {
			::EnableWindow( m_hwndCheckBOM, TRUE );
			if (lRes == *m_pnCharSet) {
				fCheck = *m_pbBom ? BST_CHECKED : BST_UNCHECKED;
			}
			else{
				fCheck = cCodeTypeName.IsBomDefOn() ? BST_CHECKED : BST_UNCHECKED;
			}
		}
		else {
			::EnableWindow( m_hwndCheckBOM, FALSE );
			fCheck = BST_UNCHECKED;
		}
		BtnCtl_SetCheck( m_hwndCheckBOM, fCheck );
		break;
	}
	return TRUE;
}



LPVOID CDlgSetCharSet::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgSetCharSet::SetData( void )
{
	// �����R�[�h�Z�b�g
	int		nIdx, nCurIdx;
	ECodeType nCharSet;
	CCodeTypesForCombobox cCodeTypes;

	nCurIdx = Combo_GetCurSel( m_hwndCharSet );
	for (nIdx = 0; nIdx < Combo_GetCount( m_hwndCharSet ); nIdx++) {
		nCharSet = (ECodeType)Combo_GetItemData( m_hwndCharSet, nIdx );
		if (nCharSet == *m_pnCharSet) {
			nCurIdx = nIdx;
		}
	}
	Combo_SetCurSel( m_hwndCharSet, nCurIdx );

	// BOM��ݒ�
	SetBOM();
}



/* �_�C�A���O�f�[�^�̎擾 */
/* TRUE==����  FALSE==���̓G���[  */
int CDlgSetCharSet::GetData( void )
{
	// �����R�[�h�Z�b�g
	int		nIdx;
	nIdx = Combo_GetCurSel( m_hwndCharSet );
	*m_pnCharSet = (ECodeType)Combo_GetItemData( m_hwndCharSet, nIdx );

	// BOM
	*m_pbBom = ( BtnCtl_GetCheck( m_hwndCheckBOM ) == BST_CHECKED );

	return TRUE;
}
