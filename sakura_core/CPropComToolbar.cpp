//	$Id$
/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�o�b�N�A�b�v�v�y�[�W

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "CPropCommon.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
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
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				11000,	//�c�[���o�[����@�\�폜
	IDC_BUTTON_INSERTSEPARATOR,		11001,	//�Z�p���[�^�}��
	IDC_BUTTON_INSERT,				11002,	//�c�[���o�[�֋@�\�}��
	IDC_BUTTON_ADD,					11003,	//�c�[���o�[�֋@�\�ǉ�
	IDC_BUTTON_UP,					11004,	//�c�[���o�[�̋@�\����ֈړ�
	IDC_BUTTON_DOWN,				11005,	//�c�[���o�[�̋@�\�����ֈړ�
	IDC_CHECK_TOOLBARISFLAT,		11010,	//�t���b�g�ȃ{�^��
	IDC_COMBO_FUNCKIND,				11030,	//�@�\�̎��
	IDC_LIST_FUNC,					11040,	//�@�\�ꗗ
	IDC_LIST_RES,					11041,	//�c�[���o�[�ꗗ
	IDC_LABEL_MENUFUNCKIND,			-1,
	IDC_LABEL_MENUFUNC,				-1,
	IDC_LABEL_TOOLBAR,				-1,
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
BOOL CALLBACK CPropCommon::DlgProc_PROP_TOOLBAR(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( DispatchEvent_p6, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p6 ���b�Z�[�W���� */
BOOL CPropCommon::DispatchEvent_p6(
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
		/* �_�C�A���O�f�[�^�̐ݒ� p6 */
		SetData_p6( hwndDlg );
		::SetWindowLong( hwndDlg, DWL_USER, (LONG)lParam );

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
		::SendMessage( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );
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
//			MYTRACE( "p6 PSN_KILLACTIVE\n" );
			/* �_�C�A���O�f�[�^�̎擾 p6 */
			GetData_p6( hwndDlg );
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
//				nIndex = ::SendMessage( hwndKeyList, LB_GETCURSEL, 0, 0 );
				nIndex2 = ::SendMessage( hwndCombo, CB_GETCURSEL, 0, 0 );

				::SendMessage( hwndFuncList, LB_RESETCONTENT, 0, 0 );
//				nNum = ::SendMessage( hwndFuncList, LB_GETCOUNT, 0, 0 );
//				for( i = 0; i < nNum; ++i ){
//					::SendMessage( hwndFuncList, LB_DELETESTRING, 0, 0 );
//				}
				/* �@�\�ꗗ�ɕ�������Z�b�g (���X�g�{�b�N�X) */
				//	From Here Oct. 15, 2001 genta Lookup���g���悤�ɕύX
				nNum = m_cLookup.GetItemCount( nIndex2 );
				for( i = 0; i < nNum; ++i ){
					nIndex1 = m_cLookup.Pos2FuncCode( nIndex2, i );
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
//					for( j = 0; j < m_cShareData.m_nMyButtonNum; ++j ){
//						if( m_cShareData.m_tbMyButton[j].idCommand == nIndex1 ){
					for( j = 0; j < m_pcMenuDrawer->m_nMyButtonNum; ++j ){
						if( m_pcMenuDrawer->m_tbMyButton[j].idCommand == nIndex1 ){	//	j�́AnIndex1�Ŏw�肳�ꂽ�@�\�R�[�h������
				//	To Here Oct. 15, 2001 genta Lookup���g���悤�ɕύX
							break;
						}
					}
//jepro note: ���s�s�v???
//@@@ 2002.01.03 YAZAKI m_tbMyButton�Ȃǂ�CShareData����CMenuDrawer�ֈړ��������Ƃɂ��C���B
//					if( j < m_cShareData.m_nMyButtonNum ){
					if( j < m_pcMenuDrawer->m_nMyButtonNum ){

//						/* �c�[���o�[�{�^���̏����Z�b�g (���X�g�{�b�N�X) */
//						for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
							lResult = ::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)j );
							lResult = ::SendMessage( hwndFuncList, LB_SETITEMHEIGHT , lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
//						}
					}

//jeprotest �R�����g�A�E�g����Ă����ȉ��̂T�s�̃R�����g�A�E�g����������ƃG���[���o��
//					if( 0 < ::LoadString( m_hInstance, (nsFuncCode::ppnFuncListArr[nIndex2])[i], pszLabel, sizeof(pszLabel) ) ){
//						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)pszLabel );
//					}else{
//						::SendMessage( hwndFuncList, LB_ADDSTRING, 0, (LPARAM)"--����`--" );
//					}
				}
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_INSERTSEPARATOR:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, 0 );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_DELETE:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( i == LB_ERR ){
						break;
					}
					if( nIndex1 >= i ){
						if( i == 0 ){
							i = ::SendMessage( hwndResList, LB_SETCURSEL, 0, 0 );
						}else{
							i = ::SendMessage( hwndResList, LB_SETCURSEL, i - 1, 0 );
						}
					}else{
						i = ::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					}
					break;

				case IDC_BUTTON_INSERT:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 ){
//						break;
						nIndex1 = 0;
					}
					nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = ::SendMessage( hwndFuncList, LB_GETITEMDATA, nIndex2, 0 );
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1 + 1, 0 );
					break;


				case IDC_BUTTON_ADD:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
					nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex2 ){
						break;
					}
					i = ::SendMessage( hwndFuncList, LB_GETITEMDATA, nIndex2, 0 );
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1, i );
					if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
						break;
					}
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_UP:
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 || 0 >= nIndex1 ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_GETITEMDATA, nIndex1, 0 );

					j = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( j == LB_ERR ){
						break;
					}
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1 - 1, i );
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;

				case IDC_BUTTON_DOWN:
					i = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
					nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
					if( LB_ERR == nIndex1 || nIndex1 + 1 >= i ){
						break;
					}
					i = ::SendMessage( hwndResList, LB_GETITEMDATA, nIndex1, 0 );

					j = ::SendMessage( hwndResList, LB_DELETESTRING, nIndex1, 0 );
					if( j == LB_ERR ){
						break;
					}
					nIndex1 = ::SendMessage( hwndResList, LB_INSERTSTRING, nIndex1 + 1, i );
					::SendMessage( hwndResList, LB_SETCURSEL, nIndex1, 0 );
					break;
				}

				break;
			}
		}
		break;

	case WM_TIMER:
		nIndex1 = ::SendMessage( hwndResList, LB_GETCURSEL, 0, 0 );
		nIndex2 = ::SendMessage( hwndFuncList, LB_GETCURSEL, 0, 0 );
		i = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );
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
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (DWORD)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}




/* �_�C�A���O�f�[�^�̐ݒ� p6 */
void CPropCommon::SetData_p6( HWND hwndDlg )
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
//	for( i = 0; i < nsFuncCode::nFuncKindNum; ++i ){
//		::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)nsFuncCode::ppszFuncKind[i] );
//	}
	
	/* ��ʂ̐擪�̍��ڂ�I��(�R���{�{�b�N�X) */
	::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)0, (LPARAM)0 );	//Oct. 14, 2000 JEPRO JEPRO �u--����`--�v��\�������Ȃ��悤�ɑ匳 Funcode.cpp �ŕύX���Ă���
	::PostMessage( hwndCombo, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

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
	for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
		lResult = ::SendMessage( hwndResList, LB_ADDSTRING, 0, (LPARAM)m_Common.m_nToolBarButtonIdxArr[i] );
		lResult = ::SendMessage( hwndResList, LB_SETITEMHEIGHT , lResult, (LPARAM)MAKELPARAM(nListItemHeight, 0) );
	}
//	/* �c�[���o�[�̐擪�̍��ڂ�I��(���X�g�{�b�N�X)*/
	::SendMessage( hwndResList, LB_SETCURSEL, 0, 0 );	//Oct. 14, 2000 JEPRO �������R�����g�A�E�g����Ɛ擪���ڂ��I������Ȃ��Ȃ�

	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ�  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_TOOLBARISFLAT, m_Common.m_bToolBarIsFlat );
	return;
}



/* �_�C�A���O�f�[�^�̎擾 p6 */
int CPropCommon::GetData_p6( HWND hwndDlg )
{
	HWND	hwndResList;
	int		i;
	int		j;
	int		k;

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_TOOLBAR;


	hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	/* �c�[���o�[�{�^���̐� */
	m_Common.m_nToolBarButtonNum = ::SendMessage( hwndResList, LB_GETCOUNT, 0, 0 );

	/* �c�[���o�[�{�^���̏����擾 */
	k = 0;
	for( i = 0; i < m_Common.m_nToolBarButtonNum; ++i ){
		j = ::SendMessage( hwndResList, LB_GETITEMDATA, i, 0 );
		if( LB_ERR != j ){
			m_Common.m_nToolBarButtonIdxArr[k] = j;
			k++;
		}
	}
	m_Common.m_nToolBarButtonNum = k;

	/* �t���b�g�c�[���o�[�ɂ���^���Ȃ�  */
	m_Common.m_bToolBarIsFlat = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TOOLBARISFLAT );

	return TRUE;
}


/*[EOF]*/
