//	$Id$
/*!	@file
	@brief �w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/05/31 �쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, jepro

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
//#include <stdio.h>
#include "sakura_rc.h"
#include "CDlgJump.h"
#include "etc_uty.h"
#include "debug.h"
#include "CEditDoc.h"
#include "funccode.h"		// Stonee, 2001/03/12

// �W�����v CDlgJump.cpp	//@@@ 2002.01.07 add start MIK
#include "sakura.hh"
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
		nData = ::GetDlgItemInt( m_hWnd, IDC_EDIT_LINENUM, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nData;
		}else
		if( pMNUD->iDelta > 0 ){
			nData--;
		}
		if( nData < 1 ){
			nData = 1;
		}
		::SetDlgItemInt( m_hWnd, IDC_EDIT_LINENUM, nData, FALSE );
		break;
	case IDC_SPIN_PLSQL_E1:
		nData = ::GetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, NULL, FALSE );
		if( pMNUD->iDelta < 0 ){
			++nData;
		}else
		if( pMNUD->iDelta > 0 ){
			nData--;
		}
		if( nData < 1 ){
			nData = 1;
		}
		::SetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, nData, FALSE );
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
		nIndex = ::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), CB_GETCURSEL, 0, 0 );
		nWorkLine = (int)::SendMessage( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), CB_GETITEMDATA, nIndex, 0 );
		::SetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, nWorkLine, FALSE );
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
		::WinHelp( m_hWnd, m_szHelpFile, HELP_CONTEXT, ::FuncID_To_HelpContextID(F_JUMP_DIALOG) );
		return TRUE;
	case IDC_CHECK_PLSQL:		/* PL/SQL�\�[�X�̗L���s�� */
		if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PLSQL ) ){
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL1 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL2 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL3 ), TRUE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_PLSQL_E1 ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_SPIN_PLSQL_E1 ), TRUE );	//Oct. 6, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), TRUE );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			m_pShareData->m_bLineNumIsCRLF = TRUE;
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_LAYOUT ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_CRLF ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL1 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL2 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL3 ), FALSE );	//Sept. 12, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_PLSQL_E1 ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_SPIN_PLSQL_E1 ), FALSE );	//Oct. 6, 2000 JEPRO
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), FALSE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_LAYOUT ), TRUE );
			::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_CRLF ), TRUE );
		}
		/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
		if( m_pShareData->m_bLineNumIsCRLF ){
			::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_LAYOUT, FALSE );
			::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_CRLF, TRUE );
		}else{
			::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_LAYOUT, TRUE );
			::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_CRLF, FALSE );
		}
		return TRUE;
	case IDC_BUTTON_JUMP:			/* �w��s�փW�����v */	//Feb. 20, 2001 JEPRO �{�^������[IDOK]��[IDC_BUTTON_JUMP]�ɕύX
		/* �_�C�A���O�f�[�^�̎擾 */
		//From Here Feb. 20, 2001 JEPRO ���s���R�����g�A�E�g (CEditView_Command.cpp �� Command_JUMP ���֌W���Ă���̂ŎQ�Ƃ̂���)
//		::EndDialog( m_hWnd, GetData() );
//		���s����ǉ�
		if( 0 < GetData() ){
			CloseDialog( 1 );
		}else{
			::MYMESSAGEBOX( m_hWnd, MB_OK , GSTR_APPNAME,
				"�������s�ԍ�����͂��Ă��������B"
			);
		}
//To Here Feb. 20, 2001
		{	//@@@ 2002.2.2 YAZAKI �w��s�փW�����v���A�_�C�A���O��\������R�}���h�ƁA���ۂɃW�����v����R�}���h�ɕ����B
			CEditDoc*		pCEditDoc = (CEditDoc*)m_lParam;
			pCEditDoc->m_cEditViewArr[pCEditDoc->m_nActivePaneIndex].HandleCommand(F_JUMP, TRUE, 0, 0, 0, 0);	//	�W�����v�R�}���h���s
		}
		return TRUE;
	case IDCANCEL:
		::EndDialog( m_hWnd, FALSE );
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
	char			szText[1024];
	int				nIndexCurSel;
	int				nIndex;
	int				nWorkLine;
	int				nPLSQLBlockNum;

//	m_hWnd = hwndDlg;
//From Here Oct. 7, 2000 JEPRO �O����͂����s�ԍ���ێ�����悤�ɉ��s��ύX
//	::SetDlgItemText( m_hWnd, IDC_EDIT_LINENUM, "" );	/* �s�ԍ� */
	if( 0 == m_nLineNum ){
		::SetDlgItemText( m_hWnd, IDC_EDIT_LINENUM, "" );	/* �s�ԍ� */
	}else{
		::SetDlgItemInt( m_hWnd, IDC_EDIT_LINENUM, m_nLineNum, FALSE );	/* �O��̍s�ԍ� */
	}
//To Here Oct. 7, 2000
	::SetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE );

	/* PL/SQL�֐����X�g�쐬 */
	hwndCtrl = ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS );

/* �^�C�v�ʂɐݒ肳�ꂽ�A�E�g���C����͕��@ */
	if( OUTLINE_PLSQL == pCEditDoc->GetDocumentAttribute().m_nDefaultOutline ){
		pCEditDoc->MakeFuncList_PLSQL( &cFuncInfoArr );
	}
	nWorkLine = -1;
	nIndex = 0;
	nPLSQLBlockNum = 0;
	for( i = 0; i < cFuncInfoArr.GetNum(); ++i ){
		if( 31 == cFuncInfoArr.GetAt( i )->m_nInfo ||
			41 == cFuncInfoArr.GetAt( i )->m_nInfo ){
//			MYTRACE( "cFuncInfoArr.GetAt( i )->m_cmemFuncName=%s(%d)\n",
//				(char*)(cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetPtr( NULL ) ),
//				cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF
//			);
		}
		if( 31 == cFuncInfoArr.GetAt( i )->m_nInfo ){
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				wsprintf( szText, "%d �s  %s  �p�b�P�[�W�d�l��",
					cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetPtr( NULL )
				);
			}else{
				wsprintf( szText, "%d �s  %s  �p�b�P�[�W�d�l��",
					cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetPtr( NULL )
				);
			}
			nIndex = ::SendMessage( hwndCtrl, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)szText );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				::SendMessage( hwndCtrl, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM) (DWORD)cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF );
			}else{
				::SendMessage( hwndCtrl, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM) (DWORD)cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT );
			}
			nPLSQLBlockNum++;
		}
		if( 41 == cFuncInfoArr.GetAt( i )->m_nInfo ){
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				wsprintf( szText, "%d �s  %s  �p�b�P�[�W�{�̕�",
					cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetPtr( NULL )
				);
			}else{
				wsprintf( szText, "%d �s  %s  �p�b�P�[�W�{�̕�",
					cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT,
					cFuncInfoArr.GetAt( i )->m_cmemFuncName.GetPtr( NULL )
				);
			}
			nIndexCurSel = nIndex = ::SendMessage( hwndCtrl, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)szText );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
			if( m_pShareData->m_bLineNumIsCRLF ){	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
				nWorkLine = cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF;
				::SendMessage( hwndCtrl, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM) (DWORD)cFuncInfoArr.GetAt( i )->m_nFuncLineCRLF );
			}else{
				nWorkLine = cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT;
				::SendMessage( hwndCtrl, CB_SETITEMDATA, (WPARAM)nIndex, (LPARAM) (DWORD)cFuncInfoArr.GetAt( i )->m_nFuncLineLAYOUT );
			}
			++nPLSQLBlockNum;
		}
	}
	::SendMessage( hwndCtrl, CB_SETCURSEL, nIndexCurSel, 0 );

	/* PL/SQL�̃p�b�P�[�W�{�̂����o���ꂽ�ꍇ */
	if( -1 != nWorkLine ){
		m_nPLSQL_E1 = nWorkLine;
		::SetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, m_nPLSQL_E1, FALSE );
	}
	/* PL/SQL�̃p�b�P�[�W�u���b�N�����o���ꂽ�ꍇ */
	if( 0 < nPLSQLBlockNum ){
		m_bPLSQL = TRUE;
	}
	::CheckDlgButton( m_hWnd, IDC_CHECK_PLSQL, m_bPLSQL );	/* PL/SQL�\�[�X�̗L���s�� */
	if( BST_CHECKED == ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PLSQL ) ){
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL1 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL2 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL3 ), TRUE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_PLSQL_E1 ), TRUE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_SPIN_PLSQL_E1 ), TRUE );	//Oct. 6, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), TRUE );
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
		m_pShareData->m_bLineNumIsCRLF = TRUE;
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_LAYOUT ), FALSE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_CRLF ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL1 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL2 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_LABEL_PLSQL3 ), FALSE );	//Sept. 12, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_EDIT_PLSQL_E1 ), FALSE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_SPIN_PLSQL_E1 ), FALSE );	//Oct. 6, 2000 JEPRO
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_COMBO_PLSQLBLOCKS ), FALSE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_LAYOUT ), TRUE );
		::EnableWindow( ::GetDlgItem( m_hWnd, IDC_RADIO_LINENUM_CRLF ), TRUE );
	}
	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
//@@@ 2002.01.08 YAZAKI �ݒ��ۑ����邽�߂�ShareData�Ɉړ�
	if( m_pShareData->m_bLineNumIsCRLF ){
		::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_LAYOUT, FALSE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_CRLF, TRUE );
	}else{
		::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_LAYOUT, TRUE );
		::CheckDlgButton( m_hWnd, IDC_RADIO_LINENUM_CRLF, FALSE );
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
	if( ::IsDlgButtonChecked( m_hWnd, IDC_RADIO_LINENUM_LAYOUT ) ){
		m_pShareData->m_bLineNumIsCRLF = FALSE;
	}else{
		m_pShareData->m_bLineNumIsCRLF = TRUE;
	}

	/* PL/SQL�\�[�X�̗L���s�� */
	m_bPLSQL = ::IsDlgButtonChecked( m_hWnd, IDC_CHECK_PLSQL );
	m_nPLSQL_E1 = ::GetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E1, &pTranslated, FALSE );
	if( m_nPLSQL_E1 == 0 && FALSE == pTranslated ){
		m_nPLSQL_E1 = 1;
	}

//	m_nPLSQL_E2 = ::GetDlgItemInt( m_hWnd, IDC_EDIT_PLSQL_E2, &pTranslated, FALSE );
//	if( m_nPLSQL_E2 == 0 && FALSE == pTranslated ){
//		m_nPLSQL_E2 = 1;
//	}

	/* �s�ԍ� */
	m_nLineNum = ::GetDlgItemInt( m_hWnd, IDC_EDIT_LINENUM, &pTranslated, FALSE );
	if( m_nLineNum == 0 && FALSE == pTranslated ){
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

/*[EOF]*/
