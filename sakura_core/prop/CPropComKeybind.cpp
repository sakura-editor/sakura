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


#include "stdafx.h"
#include "prop/CPropCommon.h"
#include "dlg/CDlgOpenFile.h"
#include "macro/CSMacroMgr.h" // 2002/2/10 aroka
#include "debug/Debug.h" ///
#include <stdio.h>	/// 2002/2/3 aroka from here
#include "io/CTextStream.h"
#include "CDataProfile.h"
#include "charset/charcode.h"
#include "util/shell.h"
#include "util/file.h"
#include "util/string_ex2.h"
#include "env/CShareData_IO.h"
using namespace std;

const wchar_t STR_KEYDATA_HEAD2[] = L"// �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2";	// (���o�[�W�����j �ǂݍ��݂̂ݑΉ� 2008/5/3 by Uchi
const wchar_t WSTR_KEYDATA_HEAD[] = L"SakuraKeyBind_Ver3";	//2007.10.05 kobake �t�@�C���`����ini�`���ɕύX


#define STR_SHIFT_PLUS        _T("Shift+")  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         _T("Ctrl+")  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          _T("Alt+")  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
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
#else
static const DWORD p_helpids[] = {	//10700
	IDC_BUTTON_IMPORT,				10700,	//�C���|�[�g
	IDC_BUTTON_EXPORT,				10701,	//�G�N�X�|�[�g
	IDC_BUTTON_ASSIGN,				10702,	//�L�[���蓖��
	IDC_BUTTON_RELEASE,				10703,	//�L�[����
	IDC_CHECK_SHIFT,				10710,	//Shift�L�[
	IDC_CHECK_CTRL,					10711,	//Ctrl�L�[
	IDC_CHECK_ALT,					10712,	//Alt�L�[
	IDC_COMBO_FUNCKIND,				10730,	//�@�\�̎��
	IDC_EDIT_KEYSFUNC,				10740,	//�L�[�Ɋ��蓖�Ă��Ă���@�\
	IDC_LIST_FUNC,					10750,	//�@�\�ꗗ
	IDC_LIST_KEY,					10751,	//�L�[�ꗗ
	IDC_LIST_ASSIGNEDKEYS,			10752,	//�@�\�Ɋ��蓖�Ă��Ă���L�[
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_KEYKIND,				-1,
	IDC_LABEL_FUNCtoKEY,			-1,
	IDC_LABEL_KEYtoFUNC,			-1,
	IDC_CHECK_ACCELTBL_EACHWIN,		10760,	// �E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
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
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_KEYBIND(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p5, hwndDlg, uMsg, wParam, lParam );
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







/* p5 ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_p5(
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
		/* �_�C�A���O�f�[�^�̐ݒ� p5 */
		SetData_p5( hwndDlg );
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
		::SendMessageAny( hwndKeyList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
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
//			MYTRACE_A( "p5 PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 p5 */
			GetData_p5( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_KEYBOARD;

			// �\�����X�V����i�}�N���ݒ��ʂł̃}�N�����ύX�𔽉f�j	// 2007.11.02 ryoji
			nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
			nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
			nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
			if( nIndex != LB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
			}
			if( nIndex2 != CB_ERR ){
				::SendMessageAny( hwndDlg, WM_COMMAND, MAKEWPARAM( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
				if( nIndex3 != LB_ERR ){
					::SendMessageAny( hwndFuncList, LB_SETCURSEL, nIndex3, 0 );
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
				/* p5:�L�[���蓖�Đݒ���C���|�[�g���� */
				p5_Import_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
				/* p5:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
				p5_Export_KeySetting( hwndDlg );
				return TRUE;
			case IDC_BUTTON_ASSIGN:	/* ���t */
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				p5_ChangeKeyList( hwndDlg );

				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
				/* �@�\�ɑΉ�����L�[���̎擾(����) */
				CNativeT**	ppcAssignedKeyList;
				nAssignedKeyNum = CKeyBind::GetKeyStrList(	/* �@�\�ɑΉ�����L�[���̎擾(����) */
					G_AppInstance(), m_Common.m_sKeyBind.m_nKeyNameArrNum, (KEYDATA*)m_Common.m_sKeyBind.m_pKeyNameArr,
					&ppcAssignedKeyList, nFuncCode,
					FALSE	// 2007.02.22 ryoji �f�t�H���g�@�\�͎擾���Ȃ�
				);	
				/* ���蓖�ăL�[���X�g���N���A���Ēl�̐ݒ� */
				::SendMessageAny( hwndAssignedkeyList, LB_RESETCONTENT, 0, 0 );
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
				nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
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

					nIndex = ::SendMessageAny( hwndAssignedkeyList, LB_GETCURSEL, 0, 0 );
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
								::SendMessageAny( hwndKeyList, LB_SETCURSEL, (WPARAM)j, (LPARAM)0);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //�`�F�b�N���͂���
								::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );

								// �L�[�ꗗ�̕�������ύX
								p5_ChangeKeyList( hwndDlg );
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
		nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
		nIndex2 = ::SendMessageAny( hwndCombo, CB_GETCURSEL, 0, 0 );
		nIndex3 = ::SendMessageAny( hwndFuncList, LB_GETCURSEL, 0, 0 );
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.11.07 End

	}
	return FALSE;
}







/* �_�C�A���O�f�[�^�̐ݒ� p5 */
void CPropCommon::SetData_p5( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndKeyList;
	int			i;

	/* �@�\��ʈꗗ�ɕ�������Z�b�g�i�R���{�{�b�N�X�j*/
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 2, 2001 genta

	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���

	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
			::List_AddString( hwndKeyList, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
	}

	// 2009.08.15 nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	::CheckDlgButton( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN, m_Common.m_sKeyBind.m_bCreateAccelTblEachWin );

	return;
}



/* �_�C�A���O�f�[�^�̎擾 p5 */
int CPropCommon::GetData_p5( HWND hwndDlg )
{
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_KEYBOARD;
	// 2009.08.15 nasukoji	�E�B���h�E���ɃA�N�Z�����[�^�e�[�u�����쐬����(Wine�p)
	m_Common.m_sKeyBind.m_bCreateAccelTblEachWin = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ACCELTBL_EACHWIN );

	return TRUE;
}
/*! p5: �L�[���X�g���`�F�b�N�{�b�N�X�̏�Ԃɍ��킹�čX�V���� */
void CPropCommon::p5_ChangeKeyList( HWND hwndDlg){
	HWND	hwndKeyList;
	int 	nIndex;
	int 	nIndexTop;
	int 	i;
	wchar_t	szKeyState[64];
	
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	nIndex = ::SendMessageAny( hwndKeyList, LB_GETCURSEL, 0, 0 );
	nIndexTop = ::SendMessageAny( hwndKeyList, LB_GETTOPINDEX, 0, 0 );
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
	::SendMessageAny( hwndKeyList, LB_RESETCONTENT, 0, 0 );
	for( i = 0; i < m_Common.m_sKeyBind.m_nKeyNameArrNum; ++i ){
		TCHAR	pszLabel[256];
		auto_sprintf( pszLabel, _T("%ls%ts"), szKeyState, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName );
		::List_AddString( hwndKeyList, pszLabel );
	}
	::SendMessageAny( hwndKeyList, LB_SETCURSEL, nIndex, 0 );
	::SendMessageAny( hwndKeyList, LB_SETTOPINDEX, nIndexTop, 0 );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
}

/* p5:�L�[���蓖�Đݒ���C���|�[�g���� */
void CPropCommon::p5_Import_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];

	KEYDATA			pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */
	TCHAR			szInitDir[_MAX_PATH + 1];

	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, _T("\\") );



	//�I�[�v��
	CDataProfile in;
	in.SetReadingMode();
	if(!in.ReadProfile(szPath)){
		ErrorMessage_A( hwndDlg, "�t�@�C�����J���܂���ł����B\n\n%ts", szPath );
		return;
	}
	static const wchar_t* szSection=L"Keybind";
	static const wchar_t* szSecInfo=L"Info";

	//�o�[�W�����m�F
	bool	bVer3;			// �V�o�[�W�����̃t�@�C��
	bool	bVer2;
	WCHAR szHeader[256];
	bVer3 = true;
	bVer2 = false;
	in.IOProfileData(szSecInfo, L"KEYBIND_VERSION", MakeStringBufferW(szHeader));
	if(wcscmp(szHeader,WSTR_KEYDATA_HEAD)!=0)	bVer3=false;

	int	nKeyNameArrNum;			// �L�[���蓖�ĕ\�̗L���f�[�^��
	if ( bVer3 ) {
		//Count�擾 -> nKeyNameArrNum
		in.IOProfileData(szSecInfo, L"KEYBIND_COUNT", nKeyNameArrNum);
		if (nKeyNameArrNum<0 || nKeyNameArrNum>100)	bVer3=false; //�͈̓`�F�b�N

		CShareData_IO::ShareData_IO_KeyBind(in, nKeyNameArrNum, pKeyNameArr, true);	// 2008/5/25 Uchi
	}

	if (!bVer3) {
		// �V�o�[�W�����łȂ�
		CTextInputStream in(szPath);
		if (!in) {
			ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
			return;
		}
		// �w�b�_�`�F�b�N
		wstring	szLine = in.ReadLineW();
		bVer2 = true;
		if ( wcscmp(szLine.c_str(), STR_KEYDATA_HEAD2) != 0)	bVer2 = false;
		// �J�E���g�`�F�b�N
		int	i, cnt;
		if ( bVer2 ) {
			int	an;
			szLine = in.ReadLineW();
			cnt = swscanf(szLine.c_str(), L"Count=%d", &an);
			if ( cnt != 1 || an < 0 || an > 100 ) {
				bVer2 = false;
			}
			else {
				nKeyNameArrNum = an;
			}
		}
		if ( bVer2 ) {
			//�e�v�f�擾
			for(i = 0; i < 100; i++) {
				int n, kc, nc;
				//�l -> szData
				wchar_t szData[1024];
				auto_strcpy(szData, in.ReadLineW().c_str());

				//��͊J�n
				cnt = swscanf(szData, L"KeyBind[%03d]=%04x,%n",
												&n,   &kc, &nc);
				if( cnt !=2 && cnt !=3 )	{ bVer2= false; break;}
				if( i != n ) break;
				pKeyNameArr[i].m_nKeyCode = kc;
				wchar_t* p = szData + nc;

				//��ɑ����g�[�N��
				for(int j=0;j<8;j++)
				{
					wchar_t* q=auto_strchr(p,L',');
					if(!q)	{ bVer2= false; break;}
					*q=L'\0';

					//�@�\���𐔒l�ɒu��������B(���l�̋@�\�������邩��)
					//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
					WCHAR	szFuncNameJapanese[256];
					EFunctionCode n = CSMacroMgr::GetFuncInfoByName(G_AppInstance(), p, szFuncNameJapanese);
					if( n == F_INVALID )
					{
						if( WCODE::Is09(*p) )
						{
							n = (EFunctionCode)auto_atol(p);
						}
						else
						{
							n = F_DEFAULT;
						}
					}
					pKeyNameArr[i].m_nFuncCodeArr[j] = n;
					p = q + 1;
				}

				auto_strcpy(pKeyNameArr[i].m_szKeyName, to_tchar(p));
			}
		}
	}
	if (!bVer3  && !bVer2) {
		ErrorMessage( hwndDlg, _T("�L�[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%ts"), szPath );
	}

	// �f�[�^�̃R�s�[
	m_Common.m_sKeyBind.m_nKeyNameArrNum = nKeyNameArrNum;
	memcpy_raw( m_Common.m_sKeyBind.m_pKeyNameArr, pKeyNameArr, sizeof_raw( pKeyNameArr ) );

	// �_�C�A���O�f�[�^�̐ݒ� p5
	//@@@ 2001.11.07 modify start MIK: �@�\�Ɋ��蓖�Ă��Ă���L�[�ꗗ���X�V����B
	HWND			hwndCtrl;
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	//@@@ 2001.11.07 modify end MIK
}


/* p5:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
void CPropCommon::p5_Export_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];

	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.key"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, _T("\\") );

	//@@@ 2001.11.07 add start MIK: �e�L�X�g�`���ŕۑ�
	{
//		int	i, j;
//		WCHAR	szFuncNameJapanese[256];
		
		CTextOutputStream out(szPath);
		if(!out){
			ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
			return;
		}
		
//	delete 2008/5/25 Uchi
//		out.WriteF( L"[SakuraKeybind]\n" );
//		out.WriteF( L"Ver=%ls\n", WSTR_KEYDATA_HEAD);
//		out.WriteF( L"Count=%d\n", m_Common.m_sKeyBind.m_nKeyNameArrNum);
//		
//		for(i = 0; i < 100; i++)
//		{
//			out.WriteF( L"KeyBind[%03d]=%04x", i, m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nKeyCode);
//
//			for(j = 0; j < 8; j++)
//			{
//				//@@@ 2002.2.2 YAZAKI �}�N����CSMacroMgr�ɓ���
//				WCHAR szFuncName[256];
//				WCHAR	*p = CSMacroMgr::GetFuncInfoByID(
//					G_AppInstance(),
//					m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j],
//					szFuncName,
//					szFuncNameJapanese
//				);
//				if( p ) {
//					out.WriteF( L",%ls", p);
//				}
//				else {
//					out.WriteF( L",%d", m_Common.m_sKeyBind.m_pKeyNameArr[i].m_nFuncCodeArr[j]);
//				}
//			}
//			
//			out.WriteF( L",%ls\n", m_Common.m_sKeyBind.m_pKeyNameArr[i].m_szKeyName);
//		}
		
		out.Close();

		/* �L�[���蓖�ď�� */
		// 2008/5/25 Uchi
		static const wchar_t* szSecInfo=L"Info";
		CDataProfile cProfile;

		// �������݃��[�h�ݒ�
		cProfile.SetWritingMode();

		// �w�b�_
		StaticString<wchar_t,256> szKeydataHead = WSTR_KEYDATA_HEAD;
		cProfile.IOProfileData( szSecInfo, L"KEYBIND_VERSION", szKeydataHead );
		cProfile.IOProfileData_WrapInt( szSecInfo, L"KEYBIND_COUNT", m_Common.m_sKeyBind.m_nKeyNameArrNum );

		//���e
		CShareData_IO::ShareData_IO_KeyBind(cProfile, m_Common.m_sKeyBind.m_nKeyNameArrNum, m_Common.m_sKeyBind.m_pKeyNameArr, true);

		// ��������
		cProfile.WriteProfile( szPath, WSTR_KEYDATA_HEAD);
	}
//@@@ 2001.11.07 add end MIK

	return;
}



