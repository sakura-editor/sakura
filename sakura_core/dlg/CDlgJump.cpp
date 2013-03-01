/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, jepro, Stonee
	Copyright (C) 2002, aroka, MIK, YAZAKI
	Copyright (C) 2004, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgJump.h"
#include "doc/CEditDoc.h"
#include "func/Funccode.h"		// Stonee, 2001/03/12
#include "outline/CFuncInfo.h"
#include "outline/CFuncInfoArr.h"// 2002/2/10 aroka �w�b�_����
#include "util/shell.h"
#include "window/CEditWnd.h"
#include "sakura_rc.h"
#include "sakura.hh"

// �W�����v CDlgJump.cpp	//@@@ 2002.01.07 add start MIK
const DWORD p_helpids[] = {	//12800
	IDC_BUTTON_JUMP,				HIDC_JUMP_BUTTON_JUMP,			//�W�����v
	IDCANCEL,						HIDCANCEL_JUMP,					//�L�����Z��
	IDC_BUTTON_HELP,				HIDC_JUMP_BUTTON_HELP,			//�w���v
	IDC_CHECK_PLSQL,				HIDC_JUMP_CHECK_PLSQL,			//PL/SQL
	IDC_COMBO_PLSQLBLOCKS,			HIDC_JUMP_COMBO_PLSQLBLOCKS,	//
	IDC_EDIT_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			//�s�ԍ�
	IDC_EDIT_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		//
	IDC_RADIO_LINENUM_LAYOUT,		HIDC_JUMP_RADIO_LINENUM_LAYOUT,	//�܂�Ԃ��P��
	IDC_RADIO_LINENUM_CRLF,			HIDC_JUMP_RADIO_LINENUM_CRLF,	//���s�P��
	IDC_SPIN_LINENUM,				HIDC_JUMP_EDIT_LINENUM,			//12870,	//
	IDC_SPIN_PLSQL_E1,				HIDC_JUMP_EDIT_PLSQL_E1,		//12871,	//
//	IDC_STATIC,						-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

CDlgJump::CDlgJump()
{
	m_nLineNum = 0;			/* �s�ԍ� */
	m_bPLSQL = FALSE;		/* PL/SQL�\�[�X�̗L���s�� */
	m_nPLSQL_E1 = 1;
	m_nPLSQL_E2 = 1;


	return;
}

/* ���[�_���_�C�A���O�̕\�� */
int CDlgJump::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//	BOOL		bLineNumIsCRLF	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
)
{
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
//	m_bLineNumIsCRLF = bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
	return CDialog::DoModal( hInstance, hwndParent, IDD_JUMP, lParam );
}


// From Here Oct. 6, 2000 JEPRO added �s�ԍ����̓{�b�N�X�ɃX�s���R���g���[����t���邽��
// CDlgPrintSetting.cpp��OnNotify��OnSpin�y��CpropComFile.cpp��DispatchEvent_p2����case WM_NOTIFY���Q�l�ɂ���
BOOL CDlgJump::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR*			pNMHDR;
	NM_UPDOWN*		pMNUD;
	int				idCtrl;
	int				nData;
	idCtrl = (int)wParam;
	pNMHDR = (NMHDR*)lParam;
	pMNUD  = (NM_UPDOWN*)lParam;
/* �X�s���R���g���[���̏��� */
	switch( idCtrl ){
	case IDC_SPIN_LINENUM:
	/* �W�����v�������s�ԍ��̎w�� */
		nData = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nData;
		}else
		if( pMNUD->iDelta > 0 ){
			nData--;
		}
		if( nData < 1 ){
			nData = 1;
		}
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, nData, FALSE );
		break;
	case IDC_SPIN_PLSQL_E1:
		nData = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nData;
		}else
		if( pMNUD->iDelta > 0 ){
			nData--;
		}
		if( nData < 1 ){
			nData = 1;
		}
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, nData, FALSE );
		break;
	default:
		break;
	}
	return TRUE;
}
// To Here Oct. 6, 2000


BOOL CDlgJump::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int	nIndex;
	int	nWorkLine;
	switch( wID ){
	case IDC_COMBO_PLSQLBLOCKS:
		nIndex = Combo_GetCurSel( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ) );
		nWorkLine = (int)Combo_GetItemData( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ), nIndex );
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, nWorkLine, FALSE );
		return TRUE;
	}
	return FALSE;
}

BOOL CDlgJump::OnBnClicked( int wID )
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		/* �u�w��s�փW�����v�v�̃w���v */
		//Stonee, 2001/03/12 ��l�������A�@�\�ԍ�����w���v�g�s�b�N�ԍ��𒲂ׂ�悤�ɂ���
		MyWinHelp( GetHwnd(), m_pszHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_JUMP_DIALOG) );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
	case IDC_CHECK_PLSQL:		/* PL/SQL�\�[�X�̗L���s�� */
		if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PLSQL ) ){
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL1 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL2 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL3 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_PLSQL_E1 ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_SPIN_PLSQL_E1 ), TRUE );	//Oct. 6, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ), TRUE );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			m_pShareData->m_bLineNumIsCRLF_ForJump = true;
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_CRLF ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL1 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL2 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL3 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_PLSQL_E1 ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_SPIN_PLSQL_E1 ), FALSE );	//Oct. 6, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ), FALSE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ), TRUE );
			::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_CRLF ), TRUE );
		}
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
		if( m_pShareData->m_bLineNumIsCRLF_ForJump ){
			::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, FALSE );
			::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, TRUE );
		}else{
			::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, TRUE );
			::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, FALSE );
		}
		return TRUE;
	case IDC_BUTTON_JUMP:			/* �w��s�փW�����v */	//Feb. 20, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_JUMP]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		//From Here Feb. 20, 2001 JEPRO ���s���R�����g�A�E�g (CEditView_Command.cpp �� Command_JUMP ���֌W���Ă���̂ŎQ�Ƃ̂���)
//		::EndDialog( GetHwnd(), GetData() );
//		���s����ǉ�
		if( 0 < GetData() ){
			CloseDialog( 1 );
		}else{
			OkMessage( GetHwnd(), _T("�������s�ԍ�����͂��Ă��������B") );
		}
//To Here Feb. 20, 2001
		{	//@@@ 2002.2.2 YAZAKI �w��s�փW�����v���A�_�C�A���O��\������R�}���h�ƁA���ۂɃW�����v����R�}���h�ɕ����B
			CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
			pCEditDoc->m_pcEditWnd->GetActiveView().GetCommander().HandleCommand(F_JUMP, TRUE, 0, 0, 0, 0);	//	�W�����v�R�}���h���s
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;
	}
	/* ���N���X�����o */
	return CDialog::OnBnClicked( wID );
}



/* �_�C�A���O�f�[�^�̐ݒ� */
void CDlgJump::SetData( void )
{
	CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
	CFuncInfoArr	cFuncInfoArr;
	int				i;
	HWND			hwndCtrl;
	wchar_t			szText[1024];
	int				nIndexCurSel = 0;	//	Sep. 11, 2004 genta ������
	int				nIndex;
	int				nWorkLine; //$$ �����ɂ��A���C�A�E�g�E���W�b�N�̒P�ʂ����݂��邽�߁A�~�X�̌����ɂȂ�₷��
	int				nPLSQLBlockNum;

//	GetHwnd() = hwndDlg;
//From Here Oct. 7, 2000 JEPRO �O����͂����s�ԍ���ێ�����悤�ɉ��s��ύX
//	::DlgItem_SetText( GetHwnd(), IDC_EDIT_LINENUM, "" );	/* �s�ԍ� */
	if( 0 == m_nLineNum ){
		::DlgItem_SetText( GetHwnd(), IDC_EDIT_LINENUM, _T("") );	/* �s�ԍ� */
	}else{
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, m_nLineNum, FALSE );	/* �O��̍s�ԍ� */
	}
//To Here Oct. 7, 2000
	::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE );

	/* PL/SQL�֐����X�g�쐬 */
	hwndCtrl = ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS );

/* �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@ */
	if( OUTLINE_PLSQL == pCEditDoc->m_cDocType.GetDocumentAttribute().m_eDefaultOutline ){
		pCEditDoc->m_cDocOutline.MakeFuncList_PLSQL( &cFuncInfoArr );
	}
	nWorkLine = -1;
	nIndex = 0;
	nPLSQLBlockNum = 0;
	for( i = 0; i < cFuncInfoArr.GetNum(); ++i ){
		if( 31 == cFuncInfoArr.GetAt( i )->m_nInfo ||
			41 == cFuncInfoArr.GetAt( i )->m_nInfo ){
		}
		if( 31 == cFuncInfoArr.GetAt( i )->m_nInfo ){
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF_ForJump ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				auto_sprintf( szText, L"%d �s  %ts  �p�b�P�[�W�d�l��",
					cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetStringPtr()
				);
			}else{
				auto_sprintf( szText, L"%d �s  %ts  �p�b�P�[�W�d�l��",
					cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetStringPtr()
				);
			}
			nIndex = Combo_AddString( hwndCtrl, szText );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF_ForJump ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				Combo_SetItemData( hwndCtrl, nIndex, (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF );
			}
			else{
				Combo_SetItemData( hwndCtrl, nIndex, (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT );
			}
			nPLSQLBlockNum++;
		}
		if( 41 == cFuncInfoArr.GetAt( i )->m_nInfo ){
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF_ForJump ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				auto_sprintf( szText, L"%d �s  %ts  �p�b�P�[�W�{�̕�",
					cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetStringPtr()
				);
			}else{
				auto_sprintf( szText, L"%d �s  %ts  �p�b�P�[�W�{�̕�",
					cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetStringPtr()
				);
			}
			nIndexCurSel = nIndex = Combo_AddString( hwndCtrl, szText );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF_ForJump ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				nWorkLine = (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF;
				Combo_SetItemData( hwndCtrl, nIndex, (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF );
			}else{
				nWorkLine = (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT;
				Combo_SetItemData( hwndCtrl, nIndex, (Int)cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT );
			}
			++nPLSQLBlockNum;
		}
	}
	Combo_SetCurSel( hwndCtrl, nIndexCurSel );

	/* PL/SQL�̃p�b�P�[�W�{�̂����o���ꂽ�ꍇ */
	if( -1 != nWorkLine ){
		m_nPLSQL_E1 = nWorkLine;
		::SetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE );
	}
	/* PL/SQL�̃p�b�P�[�W�u���b�N�����o���ꂽ�ꍇ */
	if( 0 < nPLSQLBlockNum ){
		m_bPLSQL = TRUE;
	}
	::CheckDlgButton( GetHwnd(), IDC_CHECK_PLSQL, m_bPLSQL );	/* PL/SQL�\�[�X�̗L���s�� */
	if( BST_CHECKED == ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PLSQL ) ){
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL1 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL2 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL3 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_PLSQL_E1 ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_SPIN_PLSQL_E1 ), TRUE );	//Oct. 6, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ), TRUE );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
		m_pShareData->m_bLineNumIsCRLF_ForJump = true;
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_CRLF ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL1 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL2 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_LABEL_PLSQL3 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_EDIT_PLSQL_E1 ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_SPIN_PLSQL_E1 ), FALSE );	//Oct. 6, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_COMBO_PLSQLBLOCKS ), FALSE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ), TRUE );
		::EnableWindow( ::GetDlgItem( GetHwnd(), IDC_RADIO_LINENUM_CRLF ), TRUE );
	}
	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	if( m_pShareData->m_bLineNumIsCRLF_ForJump ){
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, TRUE );
	}else{
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( GetHwnd(), IDC_RADIO_LINENUM_CRLF, FALSE );
	}
	return;
}




/* �_�C�A���O�f�[�^�̎擾 */
/*   TRUE==����   FALSE==���̓G���[  */
int CDlgJump::GetData( void )
{
	BOOL	pTranslated;

	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	if( ::IsDlgButtonChecked( GetHwnd(), IDC_RADIO_LINENUM_LAYOUT ) ){
		m_pShareData->m_bLineNumIsCRLF_ForJump = false;
	}else{
		m_pShareData->m_bLineNumIsCRLF_ForJump = true;
	}

	/* PL/SQL�\�[�X�̗L���s�� */
	m_bPLSQL = ::IsDlgButtonChecked( GetHwnd(), IDC_CHECK_PLSQL );
	m_nPLSQL_E1 = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E1, &pTranslated, FALSE );
	if( m_nPLSQL_E1 == 0 && !pTranslated ){
		m_nPLSQL_E1 = 1;
	}

//	m_nPLSQL_E2 = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_PLSQL_E2, &pTranslated, FALSE );
//	if( m_nPLSQL_E2 == 0 && !pTranslated ){
//		m_nPLSQL_E2 = 1;
//	}

	/* �s�ԍ� */
	m_nLineNum = ::GetDlgItemInt( GetHwnd(), IDC_EDIT_LINENUM, &pTranslated, FALSE );
	if( m_nLineNum == 0 && !pTranslated ){
		return FALSE;
	}
	return TRUE;
}

//@@@ 2002.01.18 add start
LPVOID CDlgJump::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}
//@@@ 2002.01.18 add end


