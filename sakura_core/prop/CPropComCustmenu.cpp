/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�J�X�^�����j���[�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "prop/CPropCommon.h"
#include "dlg/CDlgInput1.h"
#include "dlg/CDlgOpenFile.h"
#include "debug/Debug.h" // 2002/2/10 aroka
#include "io/CTextStream.h"
#include "util/shell.h"
#include "util/file.h"
using namespace std;

// Sept. 5, 2000 JEPRO ���p�J�^�J�i�̑S�p���ɔ�����������ύX(27��46)
// 2007.10.02 kobake UNICODE���ɍۂ��āA�J�X�^�����j���[�t�@�C���̎d�l��ύX
#define WSTR_CUSTMENU_HEAD_V2		L"SakuraEditorMenu_Ver2"


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE,				//���j���[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT,				//���j���[�֋@�\�}��
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD,				//���j���[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_UP,					//���j���[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN,				//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT,				//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT,				//�G�N�X�|�[�g
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND,			//�@�\�̎��
	IDC_COMBO_MENU,					HIDC_COMBO_MENU,				//���j���[�̎��
	IDC_LIST_FUNC,					HIDC_LIST_FUNC,					//�@�\�ꗗ
	IDC_LIST_RES,					HIDC_LIST_RES,					//���j���[�ꗗ
//	IDC_LABEL_MENUFUNCKIND,			-1,
//	IDC_LABEL_MENUCHOICE,			-1,
//	IDC_LABEL_MENUFUNC,				-1,
//	IDC_LABEL_MENU,					-1,
//	IDC_LABEL_MENUKEYCHANGE,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10100
	IDC_BUTTON_DELETE,				10100,	//���j���[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		10101,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				10102,	//���j���[�֋@�\�}��
	IDC_BUTTON_ADD,					10103,	//���j���[�֋@�\�ǉ�
	IDC_BUTTON_UP,					10104,	//���j���[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				10105,	//���j���[�̋@�\�����ֈړ�
	IDC_BUTTON_IMPORT,				10106,	//�C���|�[�g
	IDC_BUTTON_EXPORT,				10107,	//�G�N�X�|�[�g
	IDC_COMBO_FUNCKIND,				10130,	//�@�\�̎��
	IDC_COMBO_MENU,					10131,	//���j���[�̎��
	IDC_LIST_FUNC,					10150,	//�@�\�ꗗ
	IDC_LIST_RES,					10151,	//���j���[�ꗗ
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUCHOICE,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_MENU,					-1,
	IDC_LABEL_MENUKEYCHANGE,		-1,
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_CUSTMENU(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p8, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p8 ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_p8(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	static HWND	hwndCOMBO_FUNCKIND;
	static HWND	hwndLIST_FUNC;
	static HWND	hwndCOMBO_MENU;
	static HWND	hwndLIST_RES;

	int			i;

	int			nIdx1;
	int			nIdx2;
	int			nNum2;
	int			nIdx3;
	int			nIdx4;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];

	CDlgInput1	cDlgInput1;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p8 */
		SetData_p8( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndCOMBO_FUNCKIND = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndLIST_FUNC = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
		hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

		/* �L�[�I�����̏��� */
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );

		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_CUSTMENU );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE_A( "p8 PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 p8 */
			GetData_p8( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_CUSTMENU;

			// �\�����X�V����i�}�N���ݒ��ʂł̃}�N�����ύX�𔽉f�j	// 2007.11.02 ryoji
			nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
			nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
			nIdx3 = ::SendMessageAny( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
			nIdx4 = ::SendMessageAny( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
			if( nIdx1 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_MENU );
				if( nIdx2 != LB_ERR ){
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );
				}
			}
			if( nIdx3 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCOMBO_FUNCKIND );
				if( nIdx4 != LB_ERR ){
					::SendMessageAny( hwndLIST_FUNC, LB_SETCURSEL, nIdx4, 0 );
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
				/* p8:�J�X�^�����j���[�ݒ���C���|�[�g���� */
				p8_Import_CustMenuSetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
				/* p8:�J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
				p8_Export_CustMenuSetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_MENUNAME:
				//	���j���[������̐ݒ�
				nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx1 ){
					break;
				}
				::DlgItem_GetText( hwndDlg, IDC_EDIT_MENUNAME,
					m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1], MAX_CUSTOM_MENU_NAME_LEN );
				//	Combo Box���ύX �폜���ēo�^
				::SendMessageAny( hwndCOMBO_MENU, CB_DELETESTRING, nIdx1, 0 );
				::SendMessage( hwndCOMBO_MENU, CB_INSERTSTRING, nIdx1,
					(LPARAM)m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				// �폜����ƑI�������������̂ŁC���ɖ߂�
				::SendMessageAny( hwndCOMBO_MENU, CB_SETCURSEL, nIdx1, 0 );
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}

		if( hwndCOMBO_MENU == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( CB_ERR == nIdx1 ){
					break;
				}
				/* ���j���[���ڈꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
				::SendMessageAny( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] ){
						auto_strcpy( szLabel2, LTEXT(" ��������������������������") );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
					}else{
						//	Oct. 3, 2001 genta
						m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i], szLabel, 256 );
						/* �L�[ */
						if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] ){
							auto_strcpy( szLabel2, szLabel );
						}else{
							auto_sprintf( szLabel2, LTEXT("%ls(%hc)"), szLabel, m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] );
						}
					}
					::List_AddString( hwndLIST_RES, szLabel2 );
				}
				//	Oct. 15, 2001 genta ���j���[����ݒ�
				::DlgItem_SetText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[nIdx1] );
				
				break;	/* CBN_SELCHANGE */
			}
		}else
		if( hwndLIST_RES == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_DBLCLK:
				nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx1 ){
					break;
				}
				nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
					break;
				}

				if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] ){
					break;
				}

//			idListBox = (int) LOWORD(wParam);	// identifier of list box
//			hwndListBox = (HWND) lParam;		// handle of list box
				TCHAR		szKey[2];
				auto_sprintf( szKey, _T("%hc"), m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] );
				{
					BOOL bDlgInputResult = cDlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						_T("���j���[�A�C�e���̃A�N�Z�X�L�[�ݒ�"),
						_T("�L�[����͂��Ă��������B"),
						1,
						szKey
					);
					if( !bDlgInputResult ){
						return TRUE;
					}
				}
				//	Oct. 3, 2001 genta
				m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );
				//::LoadString( G_AppInstance(), m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2], szLabel, 255 );

				{
					KEYCODE keycode[3]={0}; _tctomb(szKey, keycode);
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = keycode[0];
				}
//@@@ 2002.01.08 YAZAKI �J�X�^�����j���[�ŃA�N�Z�X�L�[�����������A���J�b�R ( �����j���[���ڂɈ��c��o�O�C��
				if (m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]){
					auto_sprintf( szLabel2, LTEXT("%ts(%hc)"),
						szLabel,
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]
					);
				}
				else {
					auto_sprintf( szLabel2, LTEXT("%ls"), szLabel );
				}

				::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel2 );
				::SendMessageAny( hwndLIST_RES, LB_DELETESTRING, nIdx2 + 1, 0 );

				break;
			case LBN_SELCHANGE:
				nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx1 ){
					break;
				}

				if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
					break;
				}

				nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
				if( LB_ERR == nIdx2 ){
					break;
				}

				/* �L�[ */
				if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ||
					' '  == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] ){
				}else{
				}
				break;	/* LBN_SELCHANGE */
			}
		}
		else if( hwndCOMBO_FUNCKIND == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIdx3 = ::SendMessageAny( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );

				/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
//	Oct. 14, 2000 jepro note: ������for�u���b�N�Ŏ��ۂɃ��X�g�������Ă���悤�ł���
				// Oct. 3, 2001 genta
				// ��p���[�`���ɒu������
				m_cLookup.SetListItem( hwndLIST_FUNC, nIdx3 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)L" ��������������������������" );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
					if( nIdx2 == LB_ERR || nIdx2 == LB_ERRSPACE ){
						break;
					}
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );


					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i--){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = F_0;
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

//					::SetWindowText( hwndEDIT_KEY, L"" );
					break;
				case IDC_BUTTON_DELETE:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessageAny( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					if( nNum2 == LB_ERR ){
						break;
					}

					for( i = nIdx2; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; ++i ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i + 1];
					}
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]--;

					if( nNum2 > 0 ){
						if( nNum2 <= nIdx2 ){
							nIdx2 = nNum2 - 1;
						}
						nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					}else{
					}
					break;


				case IDC_BUTTON_INSERT:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessageAny( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessageAny( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					List_GetText( hwndLIST_FUNC, nIdx4, szLabel );

					for( i = m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]; i > nIdx2; i-- ){
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][i - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i] = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][i - 1];
					}
					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = ::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2, (LPARAM)szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					break;


				case IDC_BUTTON_ADD:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}

					if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
						break;
					}

					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						nIdx2 = 0;
					}
					nNum2 = ::SendMessageAny( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						nIdx2 = 0;
					}
					nIdx3 = ::SendMessageAny( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx3 ){
						break;
					}
					nIdx4 = ::SendMessageAny( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx4 ){
						break;
					}
					//	Oct. 3, 2001 genta
					if( m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0 )
						break;

					List_GetText( hwndLIST_FUNC, nIdx4, szLabel );

					//	Oct. 3, 2001 genta
					m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nNum2] = m_cLookup.Pos2FuncCode( nIdx3, nIdx4 );
					m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nNum2] = '\0';
					m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1]++;

					nIdx2 = List_AddString( hwndLIST_RES, szLabel );
					if( LB_ERR == nIdx2 || LB_ERRSPACE == nIdx2 ){
						break;
					}
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2, 0 );

					break;

				case IDC_BUTTON_UP:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					if( 0 == nIdx2 ){
						break;
					}

					{
						EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1];
						KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 - 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 - 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}

					List_GetText( hwndLIST_RES, nIdx2, szLabel );
					::SendMessageAny( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 - 1, (LPARAM)szLabel );
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2 - 1, 0 );
					break;

				case IDC_BUTTON_DOWN:
					nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx1 ){
						break;
					}
					nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIdx2 ){
						break;
					}
					nNum2 = ::SendMessageAny( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
					if( LB_ERR == nNum2 ){
						break;
					}
					if( nNum2 - 1 <= nIdx2 ){
						break;
					}

					{
						EFunctionCode	nFunc = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1];
						KEYCODE key = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2 + 1] = m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2 + 1]  = m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2];
						m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx1][nIdx2] =	nFunc;
						m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx1][nIdx2]  = key;
					}
					List_GetText( hwndLIST_RES, nIdx2, szLabel );
					::SendMessageAny( hwndLIST_RES, LB_DELETESTRING, nIdx2, 0 );
					::SendMessage( hwndLIST_RES, LB_INSERTSTRING, nIdx2 + 1, (LPARAM)szLabel );
					::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, nIdx2 + 1, 0 );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIdx1 = ::SendMessageAny( hwndCOMBO_MENU, CB_GETCURSEL, 0, 0 );
		nIdx2 = ::SendMessageAny( hwndLIST_RES, LB_GETCURSEL, 0, 0 );
		nIdx3 = ::SendMessageAny( hwndCOMBO_FUNCKIND, CB_GETCURSEL, 0, 0 );
		nIdx4 = ::SendMessageAny( hwndLIST_FUNC, LB_GETCURSEL, 0, 0 );
		i = ::SendMessageAny( hwndLIST_RES, LB_GETCOUNT, 0, 0 );
		if( LB_ERR == nIdx2	){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			if( nIdx2 <= 0 ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
			}
			if( nIdx2 + 1 >= i ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
			}
		}
		if( LB_ERR == nIdx2 || LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
		}
		if( LB_ERR == nIdx4 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		if( MAX_CUSTOM_MENU_ITEMS <= m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx1] ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERTSEPARATOR ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		if( LB_ERR != nIdx3 && LB_ERR != nIdx4 &&
		 	m_cLookup.Pos2FuncCode( nIdx3, nIdx4 ) == 0
		){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}
		break;
	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}




/* �_�C�A���O�f�[�^�̐ݒ� p8 */
void CPropCommon::SetData_p8( HWND hwndDlg )
{
	HWND		hwndCOMBO_MENU;
	HWND		hwndCombo;
	HWND		hwndLIST_RES;
	int			i;
	int			nIdx;
	WCHAR		szLabel[300];
	WCHAR		szLabel2[300];

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 3, 2001 genta

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* ���j���[�ꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	hwndCOMBO_MENU = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	for( i = 0; i < MAX_CUSTOM_MENU; ++i ){
		Combo_AddString( hwndCOMBO_MENU, m_Common.m_sCustomMenu.m_szCustMenuNameArr[i] );
	}
	/* ���j���[�ꗗ�̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessageAny( hwndCOMBO_MENU, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );

	/* ���j���[���ڈꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	hwndLIST_RES = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
//	hwndEDIT_KEY = ::GetDlgItem( hwndDlg, IDC_EDIT_KEY );
	::SendMessageAny( hwndLIST_RES, LB_RESETCONTENT, 0, 0 );
	nIdx = 0;
	for( i = 0; i < m_Common.m_sCustomMenu.m_nCustMenuItemNumArr[nIdx]; ++i ){
		if( 0 == m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i] ){
			auto_strcpy( szLabel, LTEXT(" ��������������������������") );	//Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�Ŏg���Ă���Z�p���[�^�Ɠ�������ɓ��ꂵ��
		}else{
			//	Oct. 3, 2001 genta
			m_cLookup.Funccode2Name( m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
			//::LoadString( G_AppInstance(), m_Common.m_sCustomMenu.m_nCustMenuItemFuncArr[nIdx][i], szLabel, 256 );
		}
		/* �L�[ */
		if( '\0' == m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i] ){
			auto_strcpy( szLabel2, szLabel );
		}else{
			auto_sprintf( szLabel2, LTEXT("%ts(%hc)"),
				szLabel,
				m_Common.m_sCustomMenu.m_nCustMenuItemKeyArr[nIdx][i]
			);
		}
		::List_AddString( hwndLIST_RES, szLabel2 );
	}
	
	//	Oct. 15, 2001 genta ���j���[����ݒ�
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MENUNAME, m_Common.m_sCustomMenu.m_szCustMenuNameArr[0] );
	
//	/* �J�X�^�����j���[�̐擪�̍��ڂ�I���i���X�g�{�b�N�X�j*/	//Oct. 8, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
	::SendMessageAny( hwndLIST_RES, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );
	return;
}



/* �_�C�A���O�f�[�^�̎擾 p8 */
int CPropCommon::GetData_p8( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_CUSTMENU;

	return TRUE;
}





/* p8:�J�X�^�����j���[�ݒ���C���|�[�g���� */
void CPropCommon::p8_Import_CustMenuSetting( HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1] = _T("");
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.mnu"),
		m_pShareData->m_szIMPORTFOLDER // �C���|�[�g�p�t�H���_
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}

	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );


	//�w�b�_�m�F
	CTextInputStream in(szPath);
	if(!in){
		ErrorMessage_A( hwndDlg, "�t�@�C�����J���܂���ł����B\n\n%ts", szPath );
		return;
	}
//	wstring szHeader=in.ReadLineW();
//	if(szHeader.length()>2)szHeader=&szHeader.c_str()[2]; //�R�����g����
//	if(wcscmp(szHeader.c_str(),WSTR_CUSTMENU_HEAD_V2)==0){
//		//OK
//	}
//	else{
//		ErrorMessage_A( hwndDlg, "�J�X�^�����j���[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%ts", szPath );
//		return;
//	}
	in.Close();

	//���g
//	CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;
// delet 2008/5/24 Uchi
//	for(int i=0;i<MAX_CUSTOM_MENU;i++){
//		//�Z�N�V������
//		wchar_t szSection[64];
//		auto_sprintf(szSection, L"Menu%d", i);

//	rofile.IOProfileData(szSection,L"Name",MakeStringBufferW(menu->m_szCustMenuNameArr[i]));
//		cProfile.IOProfileData(szSection,L"ItemCount",menu->m_nCustMenuItemNumArr[i]);
//		for(int j=0;j<menu->m_nCustMenuItemNumArr[i];j++){
//			cProfile.IOProfileData_WrapInt(szSection,easy_format(L"FNC[%02d]",j),menu->m_nCustMenuItemFuncArr[i][j]);
//			cProfile.IOProfileData(szSection,easy_format(L"KEY[%02d]",j),menu->m_nCustMenuItemKeyArr[i][j]);
//		}
//	}

	static const wchar_t*	szSecInfo=L"Info";
	CDataProfile cProfile;
	cProfile.SetReadingMode();
	cProfile.ReadProfile(szPath);

	//�o�[�W�����m�F
	WCHAR szHeader[256];
	cProfile.IOProfileData(szSecInfo, L"MENU_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader, WSTR_CUSTMENU_HEAD_V2)!=0) {
		ErrorMessage_A( hwndDlg, "�J�X�^�����j���[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%ts", szPath );
		return;
	}

	CShareData::getInstance()->ShareData_IO_CustMenu(cProfile,m_Common.m_sCustomMenu, true);			// 2008/5/24 Uchi

	HWND	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_COMBO_MENU );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_MENU, CBN_SELCHANGE ), (LPARAM)hwndCtrl );

	return;
}

/* p8:�J�X�^�����j���[�ݒ���G�N�X�|�[�g���� */
void CPropCommon::p8_Export_CustMenuSetting( HWND hwndDlg )
{
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1] = _T("");
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.mnu"),
		m_pShareData->m_szIMPORTFOLDER // �C���|�[�g�p�t�H���_
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}

	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );

	// �I�[�v��
	CTextOutputStream out(szPath);
	if(!out){
		ErrorMessage_A(hwndDlg, "�t�@�C�����J���܂���ł����B\n\n%ts", szPath );
		return;
	}

	/* �J�X�^�����j���[��� */
// delete 2008/5/24 Uchi
//	class WriteError{};
//	try{
//		//�w�b�_
//		out.WriteF(L"; %ls\n\n",WSTR_CUSTMENU_HEAD_V2);
//
//		//���e
//		CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;
//		out.WriteF(L"[Info]\n",MAX_CUSTOM_MENU);
//		out.WriteF(L"MENU_VERSION=%ls\n",WSTR_CUSTMENU_HEAD_V2);
//		out.WriteF(L"MAX_CUSTOM_MENU=%d\n",MAX_CUSTOM_MENU);
//		out.WriteF(L"\n");
//		for(int i=0;i<MAX_CUSTOM_MENU;i++){
//			out.WriteF(L"[Menu%d]\n",i);
//			out.WriteF(L"Name=%ls\n",menu->m_szCustMenuNameArr[i]);
//			out.WriteF(L"ItemCount=%d\n",menu->m_nCustMenuItemNumArr[i]);
//			for(int j=0;j<menu->m_nCustMenuItemNumArr[i];j++){
//				out.WriteF(L"FNC[%02d]=%d\n",j,menu->m_nCustMenuItemFuncArr[i][j]);
//				out.WriteF(L"KEY[%02d]=%hc",j,menu->m_nCustMenuItemKeyArr[i][j]); out.WriteF(L"\n"); //��char��'\0'�̏ꍇ���l�����āA���s��ʂɏo��
//			}
//			out.WriteF(L"\n");
//		}
//	}
//	catch(const WriteError&){
//		ErrorMessage_A( hwndDlg, "�t�@�C���̏������݂Ɏ��s���܂����B\n\n%ts", szPath );
//	}
	out.Close();

	/* �J�X�^�����j���[��� */
	// start 2008/5/24 Uchi
	//�w�b�_
	static const wchar_t*	szSecInfo=L"Info";
	CDataProfile	cProfile;
	int				iWork;
	CommonSetting_CustomMenu* menu=&m_Common.m_sCustomMenu;

	// �������݃��[�h�ݒ�
	cProfile.SetWritingMode();

	//�w�b�_
	cProfile.IOProfileData( szSecInfo, L"MENU_VERSION", MakeStringBufferW(WSTR_CUSTMENU_HEAD_V2) );
	iWork = MAX_CUSTOM_MENU;
	cProfile.IOProfileData_WrapInt( szSecInfo, L"MAX_CUSTOM_MENU", iWork );
	
	//���e
	CShareData::getInstance()->ShareData_IO_CustMenu(cProfile, *menu, true);

	// ��������
	cProfile.WriteProfile( szPath, WSTR_CUSTMENU_HEAD_V2);
	// end 2008/5/25

	return;

}
