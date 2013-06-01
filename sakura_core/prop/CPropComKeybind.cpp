/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�L�[�o�C���h�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, MIK, jepro
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/


#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "env/CShareData.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"

#define STR_SHIFT_PLUS        _T("Shift+")  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         _T("Ctrl+")  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          _T("Alt+")  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYBIND,		//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYBIND,		//�G�N�X�|�[�g
	IDC_BUTTON_ASSIGN,				HIDC_BUTTON_ASSIGN,				//�L�[���蓖��
	IDC_BUTTON_RELEASE,				HIDC_BUTTON_RELEASE,			//�L�[����
	IDC_CHECK_SHIFT,				HIDC_CHECK_SHIFT,				//Shift�L�[
	IDC_CHECK_CTRL,					HIDC_CHECK_CTRL,				//Ctrl�L�[
	IDC_CHECK_ALT,					HIDC_CHECK_ALT,					//Alt�L�[
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_KEYBIND,	//�@�\�̎��
	IDC_EDIT_KEYSFUNC,				HIDC_EDIT_KEYSFUNC,				//�L�[�Ɋ��蓖�Ă��Ă���@�\
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_KEYBIND,			//�@�\�ꗗ
	IDC_LIST_KEY,					HIDC_LIST_KEY,					//�L�[�ꗗ
	IDC_LIST_ASSIGNEDKEYS,			HIDC_LIST_ASSIGNEDKEYS,			//�@�\�Ɋ��蓖�Ă��Ă���L�[
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_KEYKIND,				-1,
	IDC_LABEL_FUNCtoKEY,			-1,
	IDC_LABEL_KEYtoFUNC,			-1,
	IDC_CHECK_ACCELTBL_EACHWIN,		HIDC_CHECK_ACCELTBL_EACHWIN,	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg �_�C�A���O�{�b�N�X��Window Handle
	@param uMsg ���b�Z�[�W
	@param wParam �p�����[�^1
	@param lParam �p�����[�^2
*/
INT_PTR CALLBACK CPropKeybind::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropKeybind::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* From Here Oct. 13, 2000 Studio C��Mr.�R�[�q�[���ɋ�����������ł������܂������Ă܂��� */
// �E�B���h�E�v���V�[�W���̒��ŁE�E�E
LRESULT CALLBACK CPropComKeybindWndProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch( uMsg ) {
	// WM_CTLCOLORSTATIC ���b�Z�[�W�ɑ΂���
	case WM_CTLCOLORSTATIC:
	// ���F�̃u���V�n���h����Ԃ�
		return (LRESULT)GetStockObject(WHITE_BRUSH);
//	default:
//		break;
	}
	return 0;
}
/* To Here Oct. 13, 2000 */







/* Keybind ���b�Z�[�W���� */
INT_PTR CPropKeybind::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,	// message
	WPARAM	wParam,	// first message parameter
	LPARAM	lParam 	// second message parameter
)
{
	WORD		wNotifyCode;
	WORD		wID;
	HWND		hwndCtl;
	NMHDR*		pNMHDR;
	NM_UPDOWN*	pMNUD;
	int			idCtrl;
	static HWND	hwndCombo;
	static HWND	hwndFuncList;
	static HWND	hwndKeyList;
	static HWND	hwndCheckShift;
	static HWND	hwndCheckCtrl;
	static HWND	hwndCheckAlt;
	static HWND	hwndAssignedkeyList;
//	static HWND hwndLIST_KEYSFUNC;
	static HWND hwndEDIT_KEYSFUNC;
//	int			nLength;
	int			nAssignedKeyNum;

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	int			j;
	EFunctionCode	nFuncCode;
	static WCHAR pszLabel[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Keybind */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndAssignedkeyList = ::GetDlgItem( hwndDlg, IDC_LIST_ASSIGNEDKEYS );
		hwndCheckShift = ::GetDlgItem( hwndDlg, IDC_CHECK_SHIFT );
		hwndCheckCtrl = ::GetDlgItem( hwndDlg, IDC_CHECK_CTRL );
		hwndCheckAlt = ::GetDlgItem( hwndDlg, IDC_CHECK_ALT );
		hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
//		hwndLIST_KEYSFUNC = ::GetDlgItem( hwndDlg, IDC_LIST_KEYSFUNC );
		hwndEDIT_KEYSFUNC = ::GetDlgItem( hwndDlg, IDC_EDIT_KEYSFUNC );

		/* �L�[�I�����̏��� */
//	From Here Oct. 14, 2000 JEPRO �킩��ɂ����̂őI�����Ȃ��悤�ɕύX	//Oct. 17, 2000 JEPRO �����I
//	/* �L�[���X�g�̐擪�̍��ڂ�I���i���X�g�{�b�N�X�j*/
		List_SetCurSel( hwndKeyList, 0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );	//Oct. 14, 2000 JEPRO �����͂ǂ����ł������H(�킩���)
//	To Here Oct. 14, 2000
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

		::SetTimer( hwndDlg, 1, 300, NULL );	// 2007.11.02 ryoji

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
//			OnHelp( hwndDlg, IDD_PROP1P5 );		// Sept. 9, 2000 JEPRO ���ۂ�ID���ɕύX
			OnHelp( hwndDlg, IDD_PROP_KEYBIND );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE( _T("Keybind PSN_KILLACTIVE\n") );
			/* �_�C�A���O�f�[�^�̎擾 Keybind */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_KEYBOARD;

			// �\�����X�V����i�}�N���ݒ��ʂł̃}�N�����ύX�𔽉f�j	// 2007.11.02 ryoji
			nIndex = List_GetCurSel( hwndKeyList );
			nIndex2 = Combo_GetCurSel( hwndCombo );
			nIndex3 = List_GetCurSel( hwndFuncList );
			if( nIndex != LB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
			}
			if( nIndex2 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
				if( nIndex3 != LB_ERR ){
					List_SetCurSel( hwndFuncList, nIndex3 );
				}
			}
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);	/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;	/* �R���g���[���̃n���h�� */

		switch( wNotifyCode ){
		/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
		case BN_CLICKED:
			switch( wID ){
			case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
				/* Keybind:�L�[���蓖�Đݒ���C���|�[�g���� */
				Import( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
				/* Keybind:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
				Export( hwndDlg );
				return TRUE;
			case IDC_BUTTON_ASSIGN:	/* ���t */
				nIndex = List_GetCurSel( hwndKeyList );
				nIndex2 = Combo_GetCurSel( hwndCombo );
				nIndex3 = List_GetCurSel( hwndFuncList );
				if( nIndex == LB_ERR || nIndex2 == CB_ERR || nIndex3 == LB_ERR ){
					return TRUE;
				}
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			case IDC_BUTTON_RELEASE:	/* ���� */
				nIndex = List_GetCurSel( hwndKeyList );
				if( nIndex == LB_ERR ){
					return TRUE;
				}
				nFuncCode = F_DEFAULT;
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			}
			break;	/* BN_CLICKED */
		}
		if( hwndCheckShift == hwndCtl
		 || hwndCheckCtrl == hwndCtl
		 || hwndCheckAlt == hwndCtl
		){
			switch( wNotifyCode ){
			case BN_CLICKED:
				ChangeKeyList( hwndDlg );

				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndKeyList );
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
				}
				nFuncCode = m_Common.m_sKeyBind.m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
				// Oct. 2, 2001 genta
				// 2007.11.02 ryoji F_DISABLE�Ȃ疢���t
				if( nFuncCode == F_DISABLE ){
					auto_strcpy( pszLabel, LTEXT("�����t") );
				}else{
					m_cLookup.Funccode2Name( nFuncCode, pszLabel, 255 );
				}
				Wnd_SetText( hwndEDIT_KEYSFUNC, pszLabel );
				return TRUE;
			}
		}else
		if( hwndFuncList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = List_GetCurSel( hwndKeyList );
				nIndex2 = Combo_GetCurSel( hwndCombo );
				nIndex3 = List_GetCurSel( hwndFuncList );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				/* �@�\�ɑΉ�����L�[���̎擾(����) */
				CNativeT**	ppcAssignedKeyList;
				nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* �@�\�ɑΉ�����L�[���̎擾(����) */
					G_AppInstance(), m_Common.m_sKeyBind.m_nKeyNameArrNum, (KEYDATA*)m_Common.m_sKeyBind.m_pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// 2007.02.22 ryoji �f�t�H���g�@�\�͎擾���Ȃ�
				);	
				/* ���蓖�ăL�[���X�g���N���A���Ēl�̐ݒ� */
				List_ResetContent( hwndAssignedkeyList );
				if( 0 < nAssignedKeyNum){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						/* �f�o�b�O���j�^�ɏo�� */
						const TCHAR* cpszString = ppcAssignedKeyList[j]->GetStringPtr();
						::List_AddString( hwndAssignedkeyList, cpszString );
						delete ppcAssignedKeyList[j];
					}
					delete [] ppcAssignedKeyList;
				}
				return TRUE;
			}
		}else
		if( hwndCombo == hwndCtl){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel( hwndCombo );
				/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
				m_cLookup.SetListItem( hwndFuncList, nIndex2 );	//	Oct. 2, 2001 genta
				return TRUE;
			}

//@@@ 2001.11.08 add start MIK
		}else
		if( hwndAssignedkeyList == hwndCtl){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
			//case LBN_DBLCLK:
				{
					TCHAR	buff[1024], *p;
					int	ret;

					nIndex = List_GetCurSel( hwndAssignedkeyList );
					auto_memset(buff, 0, _countof(buff));
					ret = List_GetText( hwndAssignedkeyList, nIndex, buff);
					if( ret != LB_ERR )
					{
						i = 0;
						p = buff;
						//SHIFT
						if( auto_memcmp(p, STR_SHIFT_PLUS, _tcslen(STR_SHIFT_PLUS)) == 0 ){
							p += _tcslen(STR_SHIFT_PLUS);
							i |= _SHIFT;
						}
						//CTRL
						if( auto_memcmp(p, STR_CTRL_PLUS, _tcslen(STR_CTRL_PLUS)) == 0 ){
							p += _tcslen(STR_CTRL_PLUS);
							i |= _CTRL;
						}
						//ALT
						if( auto_memcmp(p, STR_ALT_PLUS, _tcslen(STR_ALT_PLUS)) == 0 ){
							p += _tcslen(STR_ALT_PLUS);
							i |= _ALT;
						}
						for(j = 0; j < m_Common.m_sKeyBind.m_nKeyNameArrNum; j++)
						{
							if( _tcscmp(m_Common.m_sKeyBind.m_pKeyNameArr[j].m_szKeyName, p) == 0 )
							{
								List_SetCurSel( hwndKeyList, j);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //�`�F�b�N���͂���
								::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );

								// �L�[�ꗗ�̕�������ύX
								ChangeKeyList( hwndDlg );
								break;
							}
						}
					}
					return TRUE;
				}
			}
//@@@ 2001.11.08 add end MIK

		}
		break;

	case WM_TIMER:
		// �{�^���̗L���^������؂�ւ���	// 2007.11.02 ryoji
		nIndex = List_GetCurSel( hwndKeyList );
		nIndex2 = Combo_GetCurSel( hwndCombo );
		nIndex3 = List_GetCurSel( hwndFuncList );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ASSIGN ), !( LB_ERR == nIndex || nIndex2 == CB_ERR || nIndex3 == LB_ERR ) );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_RELEASE ), !( LB_ERR == nIndex ) );
		break;

	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );	// 2007.11.02 ryoji
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.07 End

	}
	return FALSE;
}







/* �_�C�A���O�f�[�^�̐ݒ� Keybind */
void CPropKeybind::SetData( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndKeyList;
	int			i;

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 2, 2001 genta

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		::List_AddString( hwndKeyList, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
	}

	// 2009.08.15 nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	::CheckDlgButton( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN, m_Common.m_sKeyBind.m_bCreateAccelTblEachWin );

	return;
}



/* �_�C�A���O�f�[�^�̎擾 Keybind */
int CPropKeybind::GetData( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_KEYBOARD;
	// 2009.08.15 nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	m_Common.m_sKeyBind.m_bCreateAccelTblEachWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN );

	return TRUE;
}
	
/*! Keybind: �L�[���X�g���`�F�b�N�{�b�N�X�̏�Ԃɍ��킹�čX�V���� */
void CPropKeybind::ChangeKeyList( HWND hwndDlg){
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	nIndex = List_GetCurSel( hwndKeyList );
	nIndexTop = List_GetTopIndex( hwndKeyList );
	wcscpy( szKeyState, L"" );
	i = 0;
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
		i |= _SHIFT;
		wcscat( szKeyState, L"Shift+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
		i |= _CTRL;
		wcscat( szKeyState, L"Ctrl+" );
	}
	if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
		i |= _ALT;
		wcscat( szKeyState, L"Alt+" );
	}
	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	List_ResetContent( hwndKeyList );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		auto_sprintf( pszLabel, _T("%ls%ts"), szKeyState, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
		::List_AddString( hwndKeyList, pszLabel );
	}
	List_SetCurSel( hwndKeyList, nIndex );
	List_SetTopIndex( hwndKeyList, nIndexTop );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
}

/* Keybind:�L�[���蓖�Đݒ���C���|�[�g���� */
void CPropKeybind::Import( HWND hwndDlg )
{
	CImpExpKeybind	cImpExpKeybind( m_Common );

	// �C���|�[�g
	if (!cImpExpKeybind.ImportUI( G_AppInstance(), hwndDlg )) {
		// �C���|�[�g�����Ă��Ȃ�
		return;
	}

	// �_�C�A���O�f�[�^�̐ݒ� Keybind
	// 2012.11.18 aroka �L�[�ꗗ�̍X�V�͑S�A�C�e�����X�V����B
	ChangeKeyList( hwndDlg );
	//@@@ 2001.11.07 modify start MIK: �@�\�Ɋ��蓖�Ă��Ă���L�[���X�V����B// 2012.11.18 aroka �R�����g�C��
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
	// 2012.11.25 aroka	�ݒ�̍X�V�i�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)�j
	::CheckDlgButton( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN, m_Common.m_sKeyBind.m_bCreateAccelTblEachWin );
}


/* Keybind:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
void CPropKeybind::Export( HWND hwndDlg )
{
	CImpExpKeybind	cImpExpKeybind( m_Common );

	// �G�N�X�|�[�g
	if (!cImpExpKeybind.ExportUI( G_AppInstance(), hwndDlg )) {
		// �G�N�X�|�[�g�����Ă��Ȃ�
		return;
	}
}
