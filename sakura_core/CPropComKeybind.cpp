//	$Id$
/*!	@file
	���ʐݒ�_�C�A���O�{�b�N�X�A�u�L�[�o�C���h�v�y�[�W

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


#include "CPropCommon.h"
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "CMacro.h"	//@@@ 2001.11.08 add MIK

//	From Here Sept. 5, 2000 JEPRO ���p�J�^�J�i�̑S�p���ɔ�����������ύX(21��34)
#define STR_KEYDATA_HEAD_LEN  34
//	To Here Sept. 5, 2000
#define STR_KEYDATA_HEAD      "�e�L�X�g�G�f�B�^ �L�[�ݒ�t�@�C��\x1a"

// From Here 2001.12.03 hor
//		Borland C++ Compiler 5.5 �ŃG���[�ɂȂ����̂Ł�������܂���
//#define STR_KEYDATA_HEAD2     "// �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2"	//@@@ 2001.11.07 add MIK
#define STR_KEYDATA_HEAD2     "\/\/ �e�L�X�g�G�f�B�^�L�[�ݒ� Ver2"	//@@@ 2001.11.07 add MIK
// To Here 2001.12.03 hor

#define STR_SHIFT_PLUS        "Shift+"  //@@@ 2001.11.08 add MIK
#define STR_CTRL_PLUS         "Ctrl+"  //@@@ 2001.11.08 add MIK
#define STR_ALT_PLUS          "Alt+"  //@@@ 2001.11.08 add MIK

//@@@ 2001.02.04 Start by MIK: Popup Help
const DWORD p_helpids[] = {	//10700
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_KEYBIND(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p5, hwndDlg, uMsg, wParam, lParam );
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
BOOL CPropCommon::DispatchEvent_p5(
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
	CMemory**	ppcAssignedKeyList;
	int			nLength;
	int			nAssignedKeyNum;
	const char*	cpszString;

	int			nIndex;
	int			nIndex2;
	int			nIndex3;
	int			i;
	int			j;
//	int			nNum;
	int			nFuncCode;
	static char pszLabel[256];
	static char	szKeyState[64];
	int			nIndexTop;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p5 */
		SetData_p5( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

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
		::SendMessage( hwndKeyList, LB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );	//Oct. 14, 2000 JEPRO �����͂ǂ����ł������H(�킩���)
//	To Here Oct. 14, 2000
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

//		/* �R���{�{�b�N�X�̃��[�U�[ �C���^�[�t�F�C�X���g���C���^�[�t�F�[�X�ɂ��� */
//		::SendMessage( hwndCombo, CB_SETEXTENDEDUI, (WPARAM) (BOOL) TRUE, 0 );

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
//			MYTRACE( "p5 PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 p5 */
			GetData_p5( hwndDlg );
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
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
//				nFuncCode = (nsFuncCode::ppnFuncListArr[nIndex2])[nIndex3];
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
				m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			case IDC_BUTTON_RELEASE:	/* ���� */
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = 0;
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
				m_pKeyNameArr[nIndex].m_nFuncCodeArr[i] = nFuncCode;
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndFuncList );
				return TRUE;
			}
		}
		if( hwndCheckShift == hwndCtl
		 || hwndCheckCtrl == hwndCtl
		 || hwndCheckAlt == hwndCtl
		){
			switch( wNotifyCode ){
			case BN_CLICKED:
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndexTop = ::SendMessage( hwndKeyList, LB_GETTOPINDEX, 0, 0 );
				strcpy( szKeyState, "" );
				i = 0;
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_SHIFT ) ){
					i |= _SHIFT;
					strcat( szKeyState, "Shift+" );
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_CTRL ) ){
					i |= _CTRL;
					strcat( szKeyState, "Ctrl+" );
				}
				if( ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_ALT ) ){
					i |= _ALT;
					strcat( szKeyState, "Alt+" );
				}
				/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
				::SendMessage( hwndKeyList, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < m_nKeyNameArrNum; ++i ){
					wsprintf( pszLabel, "%s%s", szKeyState, m_pKeyNameArr[i].m_szKeyName );
					::SendMessage( hwndKeyList, LB_ADDSTRING, 0, (LPARAM)pszLabel );
				}
				::SendMessage( hwndKeyList, LB_SETCURSEL, nIndex, 0 );
				::SendMessage( hwndKeyList, LB_SETTOPINDEX, nIndexTop, 0 );
				::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
				return TRUE;
			}
		}else
		if( hwndKeyList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
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
				nFuncCode = m_pKeyNameArr[nIndex].m_nFuncCodeArr[i];
				// Oct. 2, 2001 genta
				if( m_cLookup.Funccode2Name( nFuncCode, pszLabel, 255 )){
//				if( 0 < ::LoadString( m_hInstance, nFuncCode, pszLabel, 255 )  ){	//}
					::SetWindowText( hwndEDIT_KEYSFUNC, pszLabel );
				}else{
					::SetWindowText( hwndEDIT_KEYSFUNC, "--�s��--" );
				}
				return TRUE;
			}
		}else
		if( hwndFuncList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				nIndex3 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
				nFuncCode = m_cLookup.Pos2FuncCode( nIndex2, nIndex3 );	// Oct. 2, 2001 genta
//				nFuncCode = (nsFuncCode::ppnFuncListArr[nIndex2])[nIndex3];
				/* �@�\�ɑΉ�����L�[���̎擾(����) */
				nAssignedKeyNum =  CKeyBind::GetKeyStrList( m_hInstance, m_nKeyNameArrNum, (KEYDATA*)m_pKeyNameArr, &ppcAssignedKeyList, nFuncCode );	/* �@�\�ɑΉ�����L�[���̎擾(����) */
				/* ���蓖�ăL�[���X�g���N���A���Ēl�̐ݒ� */
				::SendMessage( hwndAssignedkeyList, LB_RESETCONTENT, 0, 0 );
				if( 0 < nAssignedKeyNum){
					for( j = 0; j < nAssignedKeyNum; ++j ){
						/* �f�o�b�O���j�^�ɏo�� */
						cpszString = ppcAssignedKeyList[j]->GetPtr( &nLength );
						::SendMessage( hwndAssignedkeyList, LB_ADDSTRING, 0, (LPARAM)cpszString );
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
//				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );
				/* �@�\�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
				m_cLookup.SetListItem( hwndFuncList, nIndex2 );	//	Oct. 2, 2001 genta
#if 0
				::SendMessage( hwndFuncList, LB_RESETCONTENT, 0, 0 );
				for( i = 0; i < nsFuncCode::pnFuncListNumArr[nIndex2]; ++i ){
					if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[nIndex2])[i], pszLabel, 255 ) ){
						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)pszLabel );
					}else{
						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)"--����`--" );
					}
				}
#endif
//	From Here Sept. 7, 2000 JEPRO �킩��ɂ����̂őI�����Ȃ��悤�ɕύX
//				::SendMessage( hwndFuncList, LB_SETCURSEL, (WPARAM)0, 0 );
//	To Here Sept. 7, 2000
				return TRUE;
			}

//@@@ 2001.11.08 add start MIK
		}else
		if( hwndAssignedkeyList == hwndCtl){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
			//case LBN_DBLCLK:
				{
					char	buff[1024], *p;
					int	ret;

					nIndex = ::SendMessage( hwndAssignedkeyList, LB_GETCURSEL, 0, 0 );
					memset(buff, 0, sizeof(buff));
					ret = ::SendMessage( hwndAssignedkeyList, LB_GETTEXT, nIndex, (LPARAM)buff);
					if( ret != LB_ERR )
					{
						i = 0;
						p = buff;
						//SHIFT
						if( memcmp(p, STR_SHIFT_PLUS, strlen(STR_SHIFT_PLUS)) == 0 ){
							p += strlen(STR_SHIFT_PLUS);
							i |= _SHIFT;
						}
						//CTRL
						if( memcmp(p, STR_CTRL_PLUS, strlen(STR_CTRL_PLUS)) == 0 ){
							p += strlen(STR_CTRL_PLUS);
							i |= _CTRL;
						}
						//ALT
						if( memcmp(p, STR_ALT_PLUS, strlen(STR_ALT_PLUS)) == 0 ){
							p += strlen(STR_ALT_PLUS);
							i |= _ALT;
						}
						for(j = 0; j < m_nKeyNameArrNum; j++)
						{
							if( strcmp(m_pKeyNameArr[j].m_szKeyName, p) == 0 )
							{
								::SendMessage( hwndKeyList, LB_SETCURSEL, (WPARAM)j, (LPARAM)0);
								if( i & _SHIFT ) ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_SHIFT, BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _CTRL )  ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_CTRL,  BST_UNCHECKED );  //�`�F�b�N���͂���
								if( i & _ALT )   ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_CHECKED );  //�`�F�b�N
								else             ::CheckDlgButton( hwndDlg, IDC_CHECK_ALT,   BST_UNCHECKED );  //�`�F�b�N���͂���
								::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndKeyList );
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

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		break;
//@@@ 2001.02.04 End

//@@@ 2001.11.07 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids );
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
#if 0
	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
	}
#endif
	/* ��ʂ̐擪�̍��ڂ�I���i�R���{�{�b�N�X�j*/
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���
	/* �L�[�ꗗ�ɕ�������Z�b�g�i���X�g�{�b�N�X�j*/
	hwndKeyList = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	for( i = 0; i < m_nKeyNameArrNum; ++i ){
			::SendMessage( hwndKeyList, LB_ADDSTRING, 0, (LPARAM)m_pKeyNameArr[i].m_szKeyName );
	}
	return;
}



/* �_�C�A���O�f�[�^�̎擾 p5 */
int CPropCommon::GetData_p5( HWND hwndDlg )
{
	m_nPageNum = ID_PAGENUM_KEYBOARD;
	return TRUE;
}



/* p5:�L�[���蓖�Đݒ���C���|�[�g���� */
void CPropCommon::p5_Import_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
	HFILE			hFile;
//	char			szLine[1024];
//	int				i;

	char			pHeader[STR_KEYDATA_HEAD_LEN + 1];
	short			nKeyNameArrNum;				/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
	KEYDATA			pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */
	HWND			hwndCtrl;
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.key",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	hFile = _lopen( szPath, OF_READ );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C�����J���܂���ł����B\n\n%s", szPath
		);
		return;
	}
	if( STR_KEYDATA_HEAD_LEN		!= _lread( hFile, pHeader, STR_KEYDATA_HEAD_LEN ) ||
		sizeof( nKeyNameArrNum )	!= _lread( hFile, &nKeyNameArrNum, sizeof( nKeyNameArrNum ) ) ||
		sizeof( pKeyNameArr )		!= _lread( hFile,  pKeyNameArr,    sizeof( pKeyNameArr ) ) ||
		0 != memcmp( pHeader, STR_KEYDATA_HEAD, STR_KEYDATA_HEAD_LEN )
	){
		_lclose( hFile );  //@@@ 2001.11.07 add MIK

//@@@ 2001.11.07 add start MIK
		{
			FILE	*fp;
			int	i, j, cnt, kc, n, an;
			char	buff[1024], name[1024], szFuncNameJapanese[256], s[1024], *p, *q;

			if( (fp = fopen( szPath, "r" )) == NULL )
			{
				::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
					"�t�@�C�����J���܂���ł����B\n\n%s", szPath
				);
				return;
			}

			if( fgets(buff, sizeof(buff), fp) != NULL
			 && memcmp(buff, STR_KEYDATA_HEAD2, strlen(STR_KEYDATA_HEAD2)) == 0
			 && fgets(buff, sizeof(buff), fp) != NULL )
			{
				cnt = sscanf(buff, "Count=%d", &an);
				nKeyNameArrNum = an;
				if( cnt == 1 && an >= 0 && an <= 100 )
				{
					for(i = 0; i < 100; i++)
					{
						name[0] = '\0';
						if( fgets(buff, sizeof(buff), fp) == NULL ) break;
						for(j = strlen(buff) - 1; j >= 0; j--){
							if( buff[j] == '\n' || buff[j] == '\r' ) buff[j] = '\0';
						}
						cnt = sscanf(buff, "KeyBind[%03d]=%04x,%s",
							&n,
							&kc,
							s,
							name);
						if( cnt != 3 ) break;
						if( i != n ) break;
						pKeyNameArr[i].m_nKeyCode = kc;

						p = s;
						for(j = 0; j < 8; j++)
						{
							q = strstr(p, ",");
							if( !q )
								break;
							*q = '\0';
							//�@�\���𐔒l�ɒu��������B(���l�̋@�\�������邩��)
							n = CMacro::GetFuncInfoByName(m_hInstance, p, szFuncNameJapanese);
							if( n == -1 )
							{
								if( *p >= '0' && *p <= '9' )
								{
									n = atol(p);
								}
								else
								{
									n = 0;
								}
							}
							pKeyNameArr[i].m_nFuncCodeArr[j] = n;
							p = q + 1;
						}

						if( j != 8 ) break;

						strcpy(pKeyNameArr[i].m_szKeyName, p);
					}
					if( i == 100 )
					{
						fclose(fp);
						goto ToMaster;
					}
				}
			}
			fclose(fp);
		}
//@@@ 2001.11.07 add end MIK

		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�L�[�ݒ�t�@�C���̌`�����Ⴂ�܂��B\n\n%s", szPath
		);
		return;
	}
	_lclose( hFile );

ToMaster:	//@@@ 2001.11.07 add MIK
	/* �f�[�^�̃R�s�[ */
	m_nKeyNameArrNum = nKeyNameArrNum;
	memcpy( m_pKeyNameArr, pKeyNameArr, sizeof( pKeyNameArr ) );

//	CShareData::SetKeyNames( m_pShareData );	/* �L�[���̂̃Z�b�g */


//	/* �_�C�A���O�f�[�^�̐ݒ� p5 */
//	SetData_p5( hwndDlg );
//@@@ 2001.11.07 modify start MIK: �@�\�Ɋ��蓖�Ă��Ă���L�[�ꗗ���X�V����B
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_KEY );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_KEY, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
	hwndCtrl = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_LIST_FUNC, LBN_SELCHANGE ), (LPARAM)hwndCtrl );
//@@@ 2001.11.07 modify end MIK

	return;
}


/* p5:�L�[���蓖�Đݒ���G�N�X�|�[�g���� */
void CPropCommon::p5_Export_KeySetting( HWND hwndDlg )
{
	CDlgOpenFile	cDlgOpenFile;
	char*			pszMRU = NULL;;
	char*			pszOPENFOLDER = NULL;;
	char			szPath[_MAX_PATH + 1];
//	HFILE			hFile;	//@@@ 2001.11.07 del MIK
//	char			szLine[1024];
//	int				i;
//	char			pHeader[STR_KEYDATA_HEAD_LEN + 1];
//	short			nKeyNameArrNum;				/* �L�[���蓖�ĕ\�̗L���f�[�^�� */
//	KEYDATA			pKeyNameArr[100];				/* �L�[���蓖�ĕ\ */
	char			szInitDir[_MAX_PATH + 1];

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.key",
		szInitDir,
		(const char **)&pszMRU,
		(const char **)&pszOPENFOLDER
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

#if 0  //@@@ 2001.11.07 del start MIK
	hFile = _lcreat( szPath, 0 );
	if( HFILE_ERROR == hFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C�����J���܂���ł����B\n\n%s", szPath
		);
		return;
	}
	if( STR_KEYDATA_HEAD_LEN		!= _lwrite( hFile, (LPCSTR)STR_KEYDATA_HEAD, STR_KEYDATA_HEAD_LEN ) ||
		sizeof( m_nKeyNameArrNum )	!= _lwrite( hFile, (LPCSTR)&m_nKeyNameArrNum, sizeof( m_nKeyNameArrNum ) ) ||
		sizeof( m_pKeyNameArr )		!= _lwrite( hFile, (LPCSTR) m_pKeyNameArr,    sizeof( m_pKeyNameArr ) )
	){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"�t�@�C���̏������݂Ɏ��s���܂����B\n\n%s", szPath
		);
		_lclose( hFile );	//@@@ 2001.11.09 add MIK
		return;
	}
	_lclose( hFile );
#endif  //@@@ 2001.11.07 del end MIK
//@@@ 2001.11.07 add start MIK: �e�L�X�g�`���ŕۑ�
	{
		FILE	*fp;
		int	i, j;
		char	szFuncName[256], szFuncNameJapanese[256], *p;
		
		if( (fp = fopen( szPath, "w" )) == NULL )
		{
			::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
				"�t�@�C�����J���܂���ł����B\n\n%s", szPath
			);
			return;
		}
		
		fprintf(fp, "%s\n", STR_KEYDATA_HEAD2);
		fprintf(fp, "Count=%d\n", m_nKeyNameArrNum);
		
		for(i = 0; i < 100; i++)
		{
			fprintf(fp, "KeyBind[%03d]=%04x", i, m_pKeyNameArr[i].m_nKeyCode);

			for(j = 0; j < 8; j++)
			{
				p = CMacro::GetFuncInfoByID(m_hInstance, m_pKeyNameArr[i].m_nFuncCodeArr[j], szFuncName, szFuncNameJapanese);
				if( p )
				{
					fprintf(fp, ",%s", p);
				}
				else
				{
					fprintf(fp, ",%d", m_pKeyNameArr[i].m_nFuncCodeArr[j]);
				}
			}
			
			fprintf(fp, ",%s\n", m_pKeyNameArr[i].m_szKeyName);
		}
		
		fclose(fp);
	}
//@@@ 2001.11.07 add end MIK

	return;
}


/*[EOF]*/
