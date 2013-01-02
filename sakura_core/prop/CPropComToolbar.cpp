/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�c�[���o�[�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, MIK
	Copyright (C) 2002, genta, MIK, YAZAKI, aroka
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2005, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CPropCommon.h"
#include "CMenuDrawer.h" // 2002/2/10 aroka
#include "CImageListMgr.h" // 2005/8/9 aroka
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE_TOOLBAR,				//�c�[���o�[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR_TOOLBAR,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT_TOOLBAR,				//�c�[���o�[�֋@�\�}��
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD_TOOLBAR,				//�c�[���o�[�֋@�\�ǉ�
	IDC_BUTTON_UP,					HIDC_BUTTON_UP_TOOLBAR,					//�c�[���o�[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN_TOOLBAR,				//�c�[���o�[�̋@�\�����ֈړ�
	IDC_CHECK_TOOLBARISFLAT,		HIDC_CHECK_TOOLBARISFLAT,				//�t���b�g�ȃ{�^��
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_TOOLBAR,			//�@�\�̎��
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_TOOLBAR,					//�@�\�ꗗ
	IDC_LIST_RES,					HIDC_LIST_RES_TOOLBAR,					//�c�[���o�[�ꗗ
	IDC_BUTTON_INSERTWRAP,			HIDC_BUTTON_INSERTWRAP,					//�c�[���o�[�ܕ�	// 2006.08.06 ryoji
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
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
INT_PTR CALLBACK CPropToolbar::DlgProc_page(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( reinterpret_cast<pDispatchPage>(&CPropToolbar::DispatchEvent), hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Box�Ɏw��̒l��}������ (Windows XP�̖����p)
	
	Windows XP + manifest�̎���LB_INSERTSTRING���l0���󂯕t���Ȃ��̂�
	�Ƃ肠����0�ȊO�̒l�����Ă���0�ɐݒ肵�����ĉ������B
	1��ڂ̑}����0�łȂ���Ή��ł������͂��B
	
	@param hWnd [in] ���X�g�{�b�N�X�̃E�B���h�E�n���h��
	@param index [in] �}���ʒu
	@param value [in] �}������l
	@return �}���ʒu�B�G���[�̎���LB_ERR�܂���LB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_INSERTDATA(
	HWND hWnd,              //!< handle to destination window 
	int index,          //!< item index
	int value
)
{
	int nIndex1 = List_InsertItemData( hWnd, index, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: �v�f�̑}���Ɏ��s���܂����B(%d:%d)"), index, nIndex1 );
		return nIndex1;
	}
	else if( List_SetItemData( hWnd, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: INS: �l�̐ݒ�Ɏ��s���܂����B:%d"), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Box�Ɏw��̒l��ǉ����� (Windows XP�̖����p)
	
	Windows XP + manifest�̎���LB_ADDSTRING���l0���󂯕t���Ȃ��̂�
	�Ƃ肠����0�ȊO�̒l�����Ă���0�ɐݒ肵�����ĉ������B
	1��ڂ̑}����0�łȂ���Ή��ł������͂��B
	
	@param hWnd [in] ���X�g�{�b�N�X�̃E�B���h�E�n���h��
	@param index [in] �}���ʒu
	@param value [in] �}������l
	@return �}���ʒu�B�G���[�̎���LB_ERR�܂���LB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_ADDDATA(
	HWND hWnd,              //!< handle to destination window 
	int value
)
{
	int nIndex1 = List_AddItemData( hWnd, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: �v�f�̒ǉ��Ɏ��s���܂����B(%d)"), nIndex1 );
		return nIndex1;
	}
	else if( List_SetItemData( hWnd, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, _T("Toolbar Dialog: ADD: �l�̐ݒ�Ɏ��s���܂����B:%d"), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}

/* Toolbar ���b�Z�[�W���� */
INT_PTR CPropToolbar::DispatchEvent(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	WORD				wNotifyCode;
	WORD				wID;
	HWND				hwndCtl;
	NMHDR*				pNMHDR;
	NM_UPDOWN*			pMNUD;
	int					idCtrl;
	static HWND			hwndCombo;
	static HWND			hwndFuncList;
	static HWND			hwndResList;
	LPDRAWITEMSTRUCT	pDis;
	int					nIndex1;
	int					nIndex2;
//	int					nIndex3;
	int					nNum;
	int					i;
	int					j;
	static char			pszLabel[256];
	HDC					hdc;
	TEXTMETRIC			tm;
	static int			nListItemHeight;
	LRESULT				lResult;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� Toolbar */
		SetData( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
		hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
		hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

		hdc = ::GetDC( hwndDlg );
		::GetTextMetrics( hdc, &tm );
		::ReleaseDC( hwndDlg, hdc );
		nListItemHeight = 18/*18*/; //Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�ł̋@�\�A�C�e���̍s�Ԃ������������ĕ\���s���𑝂₵��(20��18 ����ȏ㏬�������Ă����ʂ͂Ȃ��悤��)
		if( nListItemHeight < tm.tmHeight ){
			nListItemHeight = tm.tmHeight;
		}
//		nListItemHeight+=2;

//	From Here Oct.14, 2000 JEPRO added	(Ref. CPropComCustmenu.cpp ����WM_INITDIALOG���Q�l�ɂ���)
		/* �L�[�I�����̏��� */
		::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
//	To Here Oct. 14, 2000

		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_DRAWITEM:
		idCtrl = (UINT) wParam;	/* �R���g���[����ID */
		pDis = (LPDRAWITEMSTRUCT) lParam;	/* ���ڕ`���� */
		switch( idCtrl ){
		case IDC_LIST_RES:	/* �c�[���o�[�{�^�����ʃ��X�g */
		case IDC_LIST_FUNC:	/* �{�^���ꗗ���X�g */
			DrawToolBarItemList( pDis );	/* �c�[���o�[�{�^�����X�g�̃A�C�e���`�� */
			return TRUE;
		}
		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_TOOLBAR );
			return TRUE;
		case PSN_KILLACTIVE:
//			MYTRACE_A( "PROP_TOOLBAR PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 Toolbar */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
		case PSN_SETACTIVE:
			m_nPageNum = ID_PAGENUM_TOOLBAR;
			return TRUE;
		}
		break;

	case WM_COMMAND:
		wNotifyCode = HIWORD( wParam );	/* �ʒm�R�[�h */
		wID = LOWORD( wParam );			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */

		if( hwndResList == hwndCtl ){
			switch( wNotifyCode ){
			case LBN_SELCHANGE:
				return TRUE;
			}
		}else
		if( hwndCombo == hwndCtl ){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex2 = Combo_GetCurSel( hwndCombo );

				List_ResetContent( hwndFuncList );

				/* �@�\�ꗗ�ɕ�������Z�b�g (���X�g�{�b�N�X) */
				//	From Here Oct. 15, 2001 genta Lookup���g���悤�ɕύX
				nNum = m_cLookup.GetItemCount( nIndex2 );
				for( i = 0; i < nNum; ++i ){
					nIndex1 = m_cLookup.Pos2FuncCode( nIndex2, i );
					int nbarNo = m_pcMenuDrawer->FindToolbarNoFromCommandId( nIndex1 );

					if( nbarNo >= 0 ){
						/* �c�[���o�[�{�^���̏����Z�b�g (���X�g�{�b�N�X) */
						lResult = ::Listbox_ADDDATA( hwndFuncList, (LPARAM)nbarNo );
						if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
							break;
						}
						lResult = List_SetItemHeight( hwndFuncList, lResult, nListItemHeight );
					}

				}
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, 0 );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					List_SetCurSel( hwndResList, nIndex1 );
					break;

// 2005/8/9 aroka �ܕԃ{�^���������ꂽ��A�E�̃��X�g�Ɂu�c�[���o�[�ܕԁv��ǉ�����B
				case IDC_BUTTON_INSERTWRAP:
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					//	From Here Apr. 13, 2002 genta
					//	2010.06.25 Moca �܂�Ԃ��̃c�[���o�[�̃{�^���ԍ��萔����ύX�B�Ō�ł͂Ȃ��Œ�l�ɂ���
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, CMenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					List_SetCurSel( hwndResList, nIndex1 );
					break;

				case IDC_BUTTON_DELETE:
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 ){
						break;
					}
					i = List_DeleteString( hwndResList, nIndex1 );
					if( i == LB_ERR ){
						break;
					}
					if( nIndex1 >= i ){
						if( i == 0 ){
							i = List_SetCurSel( hwndResList, 0 );
						}else{
							i = List_SetCurSel( hwndResList, i - 1 );
						}
					}else{
						i = List_SetCurSel( hwndResList, nIndex1 );
					}
					break;

				case IDC_BUTTON_INSERT:
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					nIndex2 = List_GetCurSel( hwndFuncList );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = List_GetItemData( hwndFuncList, nIndex2 );
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					//	To Here Apr. 13, 2002 genta
					List_SetCurSel( hwndResList, nIndex1 + 1 );
					break;


				case IDC_BUTTON_ADD:
					nIndex1 = List_GetCount( hwndResList );
					nIndex2 = List_GetCurSel( hwndFuncList );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = List_GetItemData( hwndFuncList, nIndex2 );
					//	From Here Apr. 13, 2002 genta
					//	�����ł� i != 0 ���Ƃ͎v�����ǁA�ꉞ�ی��ł��B
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: �v�f�̒ǉ��Ɏ��s���܂����B:%d"), nIndex1 );
						break;
					}
					//	To Here Apr. 13, 2002 genta
					List_SetCurSel( hwndResList, nIndex1 );
					break;

				case IDC_BUTTON_UP:
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 || 0 >= nIndex1 ){
						break;
					}
					i = List_GetItemData( hwndResList, nIndex1 );

					j = List_DeleteString( hwndResList, nIndex1 );
					if( j == LB_ERR ){
						break;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1 - 1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: �v�f�̒ǉ��Ɏ��s���܂����B:%d"), nIndex1 );
						break;
					}
					//	To Here Apr. 13, 2002 genta
					List_SetCurSel( hwndResList, nIndex1 );
					break;

				case IDC_BUTTON_DOWN:
					i = List_GetCount( hwndResList );
					nIndex1 = List_GetCurSel( hwndResList );
					if( LB_ERR == nIndex1 || nIndex1 + 1 >= i ){
						break;
					}
					i = List_GetItemData( hwndResList, nIndex1 );

					j = List_DeleteString( hwndResList, nIndex1 );
					if( j == LB_ERR ){
						break;
					}
					//	From Here Apr. 13, 2002 genta
					nIndex1 = ::Listbox_INSERTDATA( hwndResList, nIndex1 + 1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						TopErrorMessage( NULL, _T("Toolbar Dialog: �v�f�̒ǉ��Ɏ��s���܂����B:%d"), nIndex1 );
						break;
					}
					List_SetCurSel( hwndResList, nIndex1 );
					//	To Here Apr. 13, 2002 genta
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIndex1 = List_GetCurSel( hwndResList );
		nIndex2 = List_GetCurSel( hwndFuncList );
		i = List_GetCount( hwndResList );
		if( LB_ERR == nIndex1 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
			if( nIndex1 <= 0 ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
			}
			if( nIndex1 + 1 >= i ){
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
			}else{
				::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
			}
		}
		if( LB_ERR == nIndex1 || LB_ERR == nIndex2 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
		}
		if( LB_ERR == nIndex2 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
		}
		break;

	case WM_DESTROY:
		::KillTimer( hwndDlg, 1 );
		break;

//@@@ 2001.02.04 Start by MIK: Popup Help
	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, m_pszHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_pszHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelp�ɕύX�ɕύX
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}




/* �_�C�A���O�f�[�^�̐ݒ� Toolbar */
void CPropToolbar::SetData( HWND hwndDlg )
{
	HWND		hwndCombo;
	HWND		hwndResList;
	HDC			hdc;
	int			i;
	int			nListItemHeight;
	LRESULT		lResult;
	TEXTMETRIC	tm;

	/* �@�\��ʈꗗ�ɕ�������Z�b�g(�R���{�{�b�N�X) */
	hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 15, 2001 genta
	
	/* ��ʂ̐擪�̍��ڂ�I��(�R���{�{�b�N�X) */
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���
	::PostMessageCmd( hwndCombo, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

	/* �R���g���[���̃n���h�����擾 */
	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	hdc = ::GetDC( hwndDlg );
	::GetTextMetrics( hdc, &tm );
	::ReleaseDC( hwndDlg, hdc );

	nListItemHeight = 18; //Oct. 18, 2000 JEPRO �u�c�[���o�[�v�^�u�ł̃c�[���o�[�A�C�e���̍s�Ԃ������������ĕ\���s���𑝂₵��(20��18 ����ȏ㏬�������Ă����ʂ͂Ȃ��悤��)
	if( nListItemHeight < tm.tmHeight ){
		nListItemHeight = tm.tmHeight;
	}
//	nListItemHeight+=2;

	/* �c�[���o�[�{�^���̏����Z�b�g(���X�g�{�b�N�X)*/
	for( i = 0; i < m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
		//	From Here Apr. 13, 2002 genta
		lResult = ::Listbox_ADDDATA( hwndResList, (LPARAM)m_Common.m_sToolBar.m_nToolBarButtonIdxArr[i] );
		if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		lResult = List_SetItemHeight( hwndResList, lResult, nListItemHeight );
	}
	/* �c�[���o�[�̐擪�̍��ڂ�I��(���X�g�{�b�N�X)*/
	List_SetCurSel( hwndResList, 0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�

	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ�  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_TOOLBARISFLAT, m_Common.m_sToolBar.m_bToolBarIsFlat );
	return;
}



/* �_�C�A���O�f�[�^�̎擾 Toolbar */
int CPropToolbar::GetData( HWND hwndDlg )
{
	HWND	hwndResList;
	int		i;
	int		j;
	int		k;

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_TOOLBAR;


	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	/* �c�[���o�[�{�^���̐� */
	m_Common.m_sToolBar.m_nToolBarButtonNum = List_GetCount( hwndResList );

	/* �c�[���o�[�{�^���̏����擾 */
	k = 0;
	for( i = 0; i < m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
		j = List_GetItemData( hwndResList, i );
		if( LB_ERR != j ){
			m_Common.m_sToolBar.m_nToolBarButtonIdxArr[k] = j;
			k++;
		}
	}
	m_Common.m_sToolBar.m_nToolBarButtonNum = k;

	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ�  */
	m_Common.m_sToolBar.m_bToolBarIsFlat = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TOOLBARISFLAT );

	return TRUE;
}

/* �c�[���o�[�{�^�����X�g�̃A�C�e���`��
	@date 2003.08.27 Moca �V�X�e���J���[�̃u���V��CreateSolidBrush�����GetSysColorBrush��
	@date 2005.08.09 aroka CPropCommon.cpp ����ړ�
	@date 2007.11.02 ryoji �{�^���ƃZ�p���[�^�Ƃŏ����𕪂���
*/
void CPropToolbar::DrawToolBarItemList( DRAWITEMSTRUCT* pDis )
{
	TBBUTTON	tbb;
	HBRUSH		hBrush;
	RECT		rc;
	RECT		rc0;
	RECT		rc1;
	RECT		rc2;


//	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	hBrush = ::GetSysColorBrush( COLOR_WINDOW );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
//	::DeleteObject( hBrush );

	rc  = pDis->rcItem;
	rc0 = pDis->rcItem;
	rc0.left += 18;//20 //Oct. 18, 2000 JEPRO �s�擪�̃A�C�R���Ƃ���ɑ����L���v�V�����Ƃ̊Ԃ������l�߂�(20��18)
	rc1 = rc0;
	rc2 = rc0;

	if( (int)pDis->itemID < 0 ){
	}else{

//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
//		tbb = m_cShareData.m_tbMyButton[pDis->itemData];
//		tbb = m_pcMenuDrawer->m_tbMyButton[pDis->itemData];
		tbb = m_pcMenuDrawer->getButton(pDis->itemData);

		// �{�^���ƃZ�p���[�^�Ƃŏ����𕪂���	2007.11.02 ryoji
		WCHAR	szLabel[256];
		if( tbb.fsStyle & TBSTYLE_SEP ){
			// �e�L�X�g�����\������
			if( tbb.idCommand == F_SEPARATOR ){
				auto_strcpy( szLabel, LTEXT("����������������������") );	// nLength ���g�p 2003/01/09 Moca
			}else if( tbb.idCommand == F_MENU_NOT_USED_FIRST ){
				if( ::LoadStringW_AnyBuild( G_AppInstance(), tbb.idCommand, szLabel, _countof( szLabel ) ) <= 0 ){
					auto_strcpy( szLabel, LTEXT("�\�\�c�[���o�[�ܕԁ\�\") );
				}
			}else{
				auto_strcpy( szLabel, LTEXT("���������s�@����������") );
			}
		//	From Here Oct. 15, 2001 genta
		}else{
			// �A�C�R���ƃe�L�X�g��\������
			m_pcIcons->Draw( tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top + 2, ILD_NORMAL );
			m_cLookup.Funccode2Name( tbb.idCommand, szLabel, _countof( szLabel ) );
		}
		//	To Here Oct. 15, 2001 genta

		/* �A�C�e�����I������Ă��� */
		if( pDis->itemState & ODS_SELECTED ){
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
			hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}else{
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
			hBrush = ::GetSysColorBrush( COLOR_WINDOW );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
		}
		rc1.left++;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect( pDis->hDC, &rc1, hBrush );
//		::DeleteObject( hBrush );

		::SetBkMode( pDis->hDC, TRANSPARENT );
		TextOutW_AnyBuild( pDis->hDC, rc1.left + 4, rc1.top + 2, szLabel, wcslen( szLabel ) );

	}

	/* �A�C�e���Ƀt�H�[�J�X������ */
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &rc2 );
	}
	return;
}



