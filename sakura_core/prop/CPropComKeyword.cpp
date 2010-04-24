/*!	@file
	@brief ���ʐݒ�_�C�A���O�{�b�N�X�A�u�����L�[���[�h�v�y�[�W

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, KEITA
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_rc.h"
#include "prop/CPropCommon.h"
#include "debug/Debug.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDlgInput1.h"
#include "global.h"
#include "io/CTextStream.h"
#include "util/shell.h"
#include "util/file.h"
using namespace std;

//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				HIDC_BUTTON_ADDSET,			//�L�[���[�h�Z�b�g�ǉ�
	IDC_BUTTON_DELSET,				HIDC_BUTTON_DELSET,			//�L�[���[�h�Z�b�g�폜
	IDC_BUTTON_ADDKEYWORD,			HIDC_BUTTON_ADDKEYWORD,		//�L�[���[�h�ǉ�
	IDC_BUTTON_EDITKEYWORD,			HIDC_BUTTON_EDITKEYWORD,	//�L�[���[�h�ҏW
	IDC_BUTTON_DELKEYWORD,			HIDC_BUTTON_DELKEYWORD,		//�L�[���[�h�폜
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYWORD,	//�C���|�[�g
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYWORD,	//�G�N�X�|�[�g
	IDC_CHECK_KEYWORDCASE,			HIDC_CHECK_KEYWORDCASE,		//�L�[���[�h�̉p�啶�����������
	IDC_COMBO_SET,					HIDC_COMBO_SET,				//�����L�[���[�h�Z�b�g��
	IDC_LIST_KEYWORD,				HIDC_LIST_KEYWORD,			//�L�[���[�h�ꗗ
	IDC_BUTTON_KEYCLEAN		,		HIDC_BUTTON_KEYCLEAN,		//�L�[���[�h����	// 2006.08.06 ryoji
	IDC_BUTTON_KEYSETRENAME,		HIDC_BUTTON_KEYSETRENAME,	//�Z�b�g�̖��̕ύX	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				10800,	//�L�[���[�h�Z�b�g�ǉ�
	IDC_BUTTON_DELSET,				10801,	//�L�[���[�h�Z�b�g�폜
	IDC_BUTTON_ADDKEYWORD,			10802,	//�L�[���[�h�ǉ�
	IDC_BUTTON_EDITKEYWORD,			10803,	//�L�[���[�h�ҏW
	IDC_BUTTON_DELKEYWORD,			10804,	//�L�[���[�h�폜
	IDC_BUTTON_IMPORT,				10805,	//�C���|�[�g
	IDC_BUTTON_EXPORT,				10806,	//�G�N�X�|�[�g
	IDC_CHECK_KEYWORDCASE,			10810,	//�L�[���[�h�̉p�啶�����������
	IDC_COMBO_SET,					10830,	//�����L�[���[�h�Z�b�g��
	IDC_LIST_KEYWORD,				10840,	//�L�[���[�h�ꗗ
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
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_KEYWORD(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p7, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p7 ���b�Z�[�W���� */
INT_PTR CPropCommon::DispatchEvent_p7(
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
	int					nIndex1;
	int					i;
	static int			nListItemHeight;
	LV_COLUMN			lvc;
	LV_ITEM*			plvi;
	static HWND			hwndCOMBO_SET;
	static HWND			hwndLIST_KEYWORD;
	RECT				rc;
	CDlgInput1			cDlgInput1;
	wchar_t				szKeyWord[MAX_KEYWORDLEN + 1];
	DWORD				dwStyle;
	LV_DISPINFO*		plvdi;
	LV_KEYDOWN*			pnkd;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* �_�C�A���O�f�[�^�̐ݒ� p7 */
		SetData_p7( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* �R���g���[���̃n���h�����擾 */
		hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
		hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
		::GetWindowRect( hwndLIST_KEYWORD, &rc );
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = rc.right - rc.left;
		lvc.pszText = _T("");
		lvc.iSubItem = 0;
		ListView_InsertColumn( hwndLIST_KEYWORD, 0, &lvc );

		dwStyle = (DWORD)::GetWindowLong( hwndLIST_KEYWORD, GWL_STYLE );
		::SetWindowLong( hwndLIST_KEYWORD, GWL_STYLE, dwStyle | LVS_SHOWSELALWAYS );
//				(dwStyle & ~LVS_TYPEMASK) | dwView);


		/* �R���g���[���X�V�̃^�C�~���O�p�̃^�C�}�[���N�� */
		::SetTimer( hwndDlg, 1, 300, NULL );

		return TRUE;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		pnkd = (LV_KEYDOWN *)lParam;
		pMNUD  = (NM_UPDOWN*)lParam;
		plvdi = (LV_DISPINFO*)lParam;
		plvi = &plvdi->item;

		if( hwndLIST_KEYWORD == pNMHDR->hwndFrom ){
			switch( pNMHDR->code ){
			case NM_DBLCLK:
//				MYTRACE_A( "NM_DBLCLK     \n" );
				/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
				p7_Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
				return TRUE;
			case LVN_BEGINLABELEDIT:
#ifdef _DEBUG
				MYTRACE_A( "LVN_BEGINLABELEDIT\n" );
												MYTRACE_A( "	plvi->mask =[%xh]\n", plvi->mask );
												MYTRACE_A( "	plvi->iItem =[%d]\n", plvi->iItem );
												MYTRACE_A( "	plvi->iSubItem =[%d]\n", plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE_A( "	plvi->state =[%xf]\n", plvi->state );
												MYTRACE_A( "	plvi->stateMask =[%xh]\n", plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE_A( "	plvi->pszText =[%ts]\n", plvi->pszText );
												MYTRACE_A( "	plvi->cchTextMax=[%d]\n", plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE_A( "	plvi->iImage=[%d]\n", plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE_A( "	plvi->lParam=[%xh(%d)]\n", plvi->lParam, plvi->lParam );
#endif
				return TRUE;
			case LVN_ENDLABELEDIT:
#ifdef _DEBUG
				MYTRACE_A( "LVN_ENDLABELEDIT\n" );
												MYTRACE_A( "	plvi->mask =[%xh]\n", plvi->mask );
												MYTRACE_A( "	plvi->iItem =[%d]\n", plvi->iItem );
												MYTRACE_A( "	plvi->iSubItem =[%d]\n", plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE_A( "	plvi->state =[%xf]\n", plvi->state );
												MYTRACE_A( "	plvi->stateMask =[%xh]\n", plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE_A( "	plvi->pszText =[%ts]\n", plvi->pszText  );
												MYTRACE_A( "	plvi->cchTextMax=[%d]\n", plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE_A( "	plvi->iImage=[%d]\n", plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE_A( "	plvi->lParam=[%xh(%d)]\n", plvi->lParam, plvi->lParam );
#endif
				if( NULL == plvi->pszText ){
					return TRUE;
				}
				if( 0 < _tcslen( plvi->pszText ) ){
					if( MAX_KEYWORDLEN < _tcslen( plvi->pszText ) ){
						InfoMessage( hwndDlg, _T("�L�[���[�h�̒�����%d�o�C�g�܂łł��B"), MAX_KEYWORDLEN );
						return TRUE;
					}
					/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
						plvi->lParam,
						to_wchar(plvi->pszText)
					);
				}else{
					/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam );
				}
				/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
				SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

				ListView_SetItemState( hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

				return TRUE;
			case LVN_KEYDOWN:
//				MYTRACE_A( "LVN_KEYDOWN\n" );
				switch( pnkd->wVKey ){
				case VK_DELETE:
					/* p7:���X�g���őI������Ă���L�[���[�h���폜���� */
					p7_Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					break;
				case VK_SPACE:
					/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
					p7_Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					break;
				}
				return TRUE;
			}
		}else{
			switch( pNMHDR->code ){
			case PSN_HELP:
				OnHelp( hwndDlg, IDD_PROP_KEYWORD );
				return TRUE;
			case PSN_KILLACTIVE:
#ifdef _DEBUG
				MYTRACE_A( "p7 PSN_KILLACTIVE\n" );
#endif
				/* �_�C�A���O�f�[�^�̎擾 p7 */
				GetData_p7( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_KEYWORD;
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* �ʒm�R�[�h */
		wID = LOWORD(wParam);			/* ����ID� �R���g���[��ID� �܂��̓A�N�Z�����[�^ID */
		hwndCtl = (HWND) lParam;		/* �R���g���[���̃n���h�� */
		if( hwndCOMBO_SET == hwndCtl){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex1 = ::SendMessageAny( hwndCOMBO_SET, CB_GETCURSEL, 0, 0 );
				/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
				SetData_p7_KeyWordSet( hwndDlg, nIndex1 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* �{�^���^�`�F�b�N�{�b�N�X���N���b�N���ꂽ */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_ADDSET:	/* �Z�b�g�ǉ� */
					if( MAX_SETNUM <= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
						InfoMessage( hwndDlg, _T("�Z�b�g��%d�܂ł����o�^�ł��܂���B\n"), MAX_SETNUM );
						return TRUE;
					}
					/* ���[�h���X�_�C�A���O�̕\�� */
					wcscpy( szKeyWord, L"" );
					//	Oct. 5, 2002 genta ���������̐ݒ���C���D�o�b�t�@�I�[�o�[�������Ă����D
					if( !cDlgInput1.DoModal(
						G_AppInstance(),
						hwndDlg,
						_T("�L�[���[�h�̃Z�b�g�ǉ�"),
						_T("�Z�b�g������͂��Ă��������B"),
						MAX_SETNAMELEN,
						szKeyWord
						)
					){
						return TRUE;
					}
					if( 0 < wcslen( szKeyWord ) ){
						/* �Z�b�g�̒ǉ� */
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( szKeyWord, false );

						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum - 1;

						/* �_�C�A���O�f�[�^�̐ݒ� p7 */
						SetData_p7( hwndDlg );
					}
					return TRUE;
				case IDC_BUTTON_DELSET:	/* �Z�b�g�폜 */
					nIndex1 = ::SendMessageAny( hwndCOMBO_SET, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIndex1 ){
						return TRUE;
					}
					/* �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v��� */
					static TCHAR		pszLabel[1024];
					_tcscpy( pszLabel, _T("") );
					for( i = 0; i < MAX_TYPES; ++i ){
						CTypeConfig type(i);
						// 2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ����Am_pShareData�𒼐ڌ��Ă����Ȃ��B
						if( nIndex1 == m_Types_nKeyWordSetIdx[i][0]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][1]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][2]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][3]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][4]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][5]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][6]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][7]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][8]
						||  nIndex1 == m_Types_nKeyWordSetIdx[i][9] ){
							_tcscat( pszLabel, _T("�E") );
							_tcscat( pszLabel, type->m_szTypeName );
							_tcscat( pszLabel, _T("�i") );
							_tcscat( pszLabel, type->m_szTypeExts );
							_tcscat( pszLabel, _T("�j") );
							_tcscat( pszLabel, _T("\n") );
						}
					}
					if( IDCANCEL == ::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						_T("�u%ls�v�̃Z�b�g���폜���܂��B\n��낵���ł����H\n�폜���悤�Ƃ���Z�b�g�́A�ȉ��̃t�@�C���^�C�v�Ɋ��蓖�Ă��Ă��܂��B\n�폜�����Z�b�g�͖����ɂȂ�܂��B\n\n%ts"),
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIndex1 ),
						pszLabel
					) ){
						return TRUE;
					}
					/* �폜�Ώۂ̃Z�b�g���g�p���Ă���t�@�C���^�C�v�̃Z�b�g���N���A */
					for( i = 0; i < MAX_TYPES; ++i ){
						// 2002/04/25 YAZAKI STypeConfig�S�̂�ێ�����K�v�͂Ȃ��B
						for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
							if( nIndex1 == m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j] = -1;
							}
							else if( nIndex1 < m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j]--;
							}
						}
					}
					/* ���Ԗڂ̃Z�b�g���폜 */
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
					/* �_�C�A���O�f�[�^�̐ݒ� p7 */
					SetData_p7( hwndDlg );
					return TRUE;
				case IDC_BUTTON_KEYSETRENAME: // �L�[���[�h�Z�b�g�̖��̕ύX
					// ���[�h���X�_�C�A���O�̕\��
					wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
					{
						BOOL bDlgInputResult = cDlgInput1.DoModal(
							G_AppInstance(),
							hwndDlg,
							_T("�Z�b�g�̖��̕ύX"),
							_T("�Z�b�g������͂��Ă��������B"),
							MAX_SETNAMELEN,
							szKeyWord
						);
						if( !bDlgInputResult ){
							return TRUE;
						}
					}
					if( 0 < wcslen( szKeyWord ) ){
						m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord );

						// �_�C�A���O�f�[�^�̐ݒ� p7
						SetData_p7( hwndDlg );
					}
					return TRUE;
				case IDC_CHECK_KEYWORDCASE:	/* �L�[���[�h�̉p�啶����������� */
//					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_bKEYWORDCASEArr[ m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ] = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );	//MIK 2000.12.01 case sense
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE ));			//MIK 2000.12.01 case sense
					return TRUE;
				case IDC_BUTTON_ADDKEYWORD:	/* �L�[���[�h�ǉ� */
					/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
					if( !m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CanAddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
						InfoMessage( hwndDlg, _T("�o�^�ł���L�[���[�h��������ɒB���Ă��܂��B\n") );
						return TRUE;
					}
					/* ���[�h���X�_�C�A���O�̕\�� */
					wcscpy( szKeyWord, L"" );
					if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, _T("�L�[���[�h�ǉ�"), _T("�L�[���[�h����͂��Ă��������B"), MAX_KEYWORDLEN, szKeyWord ) ){
						return TRUE;
					}
					if( 0 < wcslen( szKeyWord ) ){
						/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
						if( 0 == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord ) ){
							// �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ�
							SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
						}
					}
					return TRUE;
				case IDC_BUTTON_EDITKEYWORD:	/* �L�[���[�h�ҏW */
					/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
					p7_Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_DELKEYWORD:	/* �L�[���[�h�폜 */
					/* p7:���X�g���őI������Ă���L�[���[�h���폜���� */
					p7_Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// From Here 2005.01.26 Moca
				case IDC_BUTTON_KEYCLEAN:
					p7_Clean_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// To Here 2005.01.26 Moca
				case IDC_BUTTON_IMPORT:	/* �C���|�[�g */
					/* p7:���X�g���̃L�[���[�h���C���|�[�g���� */
					p7_Import_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_EXPORT:	/* �G�N�X�|�[�g */
					/* p7:���X�g���̃L�[���[�h���G�N�X�|�[�g���� */
					p7_Export_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				}
				break;	/* BN_CLICKED */
			}
		}
		break;	/* WM_COMMAND */

	case WM_TIMER:
		nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
		if( -1 == nIndex1 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), TRUE );
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), TRUE );
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

/* p7:���X�g���őI������Ă���L�[���[�h��ҏW���� */
void CPropCommon::p7_Edit_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	int			nIndex1;
	LV_ITEM	lvi;
	wchar_t		szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1	cDlgInput1;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );

	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
	wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam ) );

	/* ���[�h���X�_�C�A���O�̕\�� */
	if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, _T("�L�[���[�h�ҏW"), _T("�L�[���[�h��ҏW���Ă��������B"), MAX_KEYWORDLEN, szKeyWord ) ){
		return;
	}
	if( 0 < wcslen( szKeyWord ) ){
		/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ҏW */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
			lvi.lParam,
			szKeyWord
		);
	}else{
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}



/* p7:���X�g���őI������Ă���L�[���[�h���폜���� */
void CPropCommon::p7_Delete_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	int			nIndex1;
	LV_ITEM		lvi;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );
	/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h���폜 */
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	//�L�[���[�h����\������B
	DispKeywordCount( hwndDlg );

	return;
}


/* p7:���X�g���̃L�[���[�h���C���|�[�g���� */
/* p7:���X�g���̃L�[���[�h���C���|�[�g���� */
void CPropCommon::p7_Import_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	bool			bAddError = false;

	// 2007.05.19 ryoji ����ʂƓ����悤�ɃC���|�[�g�p�t�H���_�ݒ���g���悤�ɂ���
	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.kwd"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, _T("\\") );

	CTextInputStream in(szPath);
	if(!in){
		ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
		return;
	}
	while( in ){
		wstring szLine = in.ReadLineW();

		//�R�����g����
		if( 2 <= szLine.length() && 0 == auto_memcmp( szLine.c_str(), L"//", 2 )  )continue;
		
		//���
		if( 0 < szLine.length() ){
			/* ���Ԗڂ̃Z�b�g�ɃL�[���[�h��ǉ� */
			int nRetValue = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szLine.c_str() );
			if( 2 == nRetValue ){
				bAddError = true;
				break;
			}
		}
	}
	in.Close();

	if( bAddError ){
		ErrorMessage( hwndDlg, _T("�L�[���[�h�̐�������ɒB�������߁A�������̃L�[���[�h��ǉ��ł��܂���ł����B") );
	}
	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	return;
}


/* p7:���X�g���̃L�[���[�h���G�N�X�|�[�g���� */
void CPropCommon::p7_Export_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1];
	TCHAR			szInitDir[_MAX_PATH + 1];
	int				i;
	int				nKeyWordNum;

	// 2007.05.19 ryoji ����ʂƓ����悤�ɃC���|�[�g�p�t�H���_�ݒ���g���悤�ɂ���
	_tcscpy( szPath, _T("") );
	_tcscpy( szInitDir, m_pShareData->m_sHistory.m_szIMPORTFOLDER );	/* �C���|�[�g�p�t�H���_ */
	/* �t�@�C���I�[�v���_�C�A���O�̏����� */
	cDlgOpenFile.Create(
		G_AppInstance(),
		hwndDlg,
		_T("*.kwd"),
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
//	MYTRACE_A( "%ls\n", szPath );
	/* �t�@�C���̃t���p�X���A�t�H���_�ƃt�@�C�����ɕ��� */
	/* [c:\work\test\aaa.txt] �� [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_sHistory.m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_sHistory.m_szIMPORTFOLDER, _T("\\") );

	CTextOutputStream out(szPath);
	if(!out){
		ErrorMessage( hwndDlg, _T("�t�@�C�����J���܂���ł����B\n\n%ts"), szPath );
		return;
	}
	out.WriteF( L"// " );
	out.WriteF( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
	out.WriteF( L"  �L�[���[�h��`�t�@�C��" );
	out.WriteF( L"\n" );
	out.WriteF( L"\n" );

	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SortKeyWord(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx);	//MIK 2000.12.01 sort keyword

	/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
	nKeyWordNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	for( i = 0; i < nKeyWordNum; ++i ){
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, i );
		out.WriteF( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, i ) );
		out.WriteF( L"\n" );
	}
	out.Close();

	/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
	SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	InfoMessage( hwndDlg, _T("�t�@�C���փG�N�X�|�[�g���܂����B\n\n%ts"), szPath );

	return;
}


//! �L�[���[�h�𐮓ڂ���
void CPropCommon::p7_Clean_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	if( IDYES == ::MessageBox( hwndDlg, _T("���݂̐ݒ�ł͋����L�[���[�h�Ƃ��ĕ\���ł��Ȃ��L�[���[�h���폜���܂����H"),
			GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){	// 2009.03.26 ryoji MB_ICONSTOP->MB_ICONQUESTION
		if( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CleanKeyWords( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
		}
		SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}
}

/* �_�C�A���O�f�[�^�̐ݒ� p7 */
void CPropCommon::SetData_p7( HWND hwndDlg )
{
	int		i;
//	LV_ITEM	lvi;
	HWND	hwndWork;
//	int		nIdx;
//	char*	pszWork;


	/* �Z�b�g���R���{�{�b�N�X�̒l�Z�b�g */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessageAny( hwndWork, CB_RESETCONTENT, 0, 0 );  /* �R���{�{�b�N�X����ɂ��� */
	if( 0 < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		/* �Z�b�g���R���{�{�b�N�X�̃f�t�H���g�I�� */
		::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0 );

		/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
		SetData_p7_KeyWordSet( hwndDlg, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}else{
		/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
		SetData_p7_KeyWordSet( hwndDlg, -1 );
	}

	return;
}


/* �_�C�A���O�f�[�^�̐ݒ� p7 �w��L�[���[�h�Z�b�g�̐ݒ� */
void CPropCommon::SetData_p7_KeyWordSet( HWND hwndDlg, int nIdx )
{
	int		i;
	int		nNum;
	HWND	hwndList;
	LV_ITEM	lvi;

	ListView_DeleteAllItems( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ) );
	if( 0 <= nIdx ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), TRUE );
		//	Jan. 29, 2005 genta �L�[���[�h�Z�b�g�؂�ւ�����̓L�[���[�h�͖��I��
		//	���̂��ߗL���ɂ��Ă����Ƀ^�C�}�[�Ŗ����ɂȂ�D
		//	�Ȃ̂ł����Ŗ����ɂ��Ă����D
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), FALSE );
		return;
	}

	/* �L�[���[�h�̉p�啶����������� */
	if( TRUE == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase(nIdx) ){		//MIK 2000.12.01 case sense
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );
	}

	/* ���Ԗڂ̃Z�b�g�̃L�[���[�h�̐���Ԃ� */
	nNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( nIdx );
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	// 2005.01.25 Moca/genta ���X�g�ǉ����͍ĕ`���}�����Ă��΂₭�\��
	::SendMessageAny( hwndList, WM_SETREDRAW, FALSE, 0 );

	for( i = 0; i < nNum; ++i ){
		/* ���Ԗڂ̃Z�b�g�̂��Ԗڂ̃L�[���[�h��Ԃ� */
		const TCHAR* pszKeyWord = to_tchar(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( nIdx, i ));

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = const_cast<TCHAR*>(pszKeyWord);
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem( hwndList, &lvi );

	}
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nIdx;

	// 2005.01.25 Moca/genta ���X�g�ǉ������̂��ߍĕ`�拖��
	::SendMessageAny( hwndList, WM_SETREDRAW, TRUE, 0 );

	//�L�[���[�h����\������B
	DispKeywordCount( hwndDlg );

	return;
}



/* �_�C�A���O�f�[�^�̎擾 p7 */
int CPropCommon::GetData_p7( HWND hwndDlg )
{
//	HWND	hwndResList;
//	int		i;
//	int		j;
//	int		k;

//@@@ 2002.01.03 YAZAKI �Ō�ɕ\�����Ă����V�[�g�𐳂����o���Ă��Ȃ��o�O�C��
//	m_nPageNum = ID_PAGENUM_KEYWORD;


	return TRUE;
}

/* �_�C�A���O�f�[�^�̎擾 p7 �w��L�[���[�h�Z�b�g�̎擾 */
void CPropCommon::GetData_p7_KeyWordSet( HWND hwndDlg, int nIdx )
{
}

//�L�[���[�h����\������B
void CPropCommon::DispKeywordCount( HWND hwndDlg )
{
	HWND	hwndList;
	int		n;
	TCHAR szCount[ 256 ];

	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	n = ListView_GetItemCount( hwndList );
	if( n < 0 ) n = 0;

	int		nAlloc;
	nAlloc = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetAllocSize( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc -= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc += m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetFreeSize();
	
	auto_sprintf( szCount, _T("(�ő� %d ����, �o�^�� %d, �� %d ��)"), MAX_KEYWORDLEN, n, nAlloc );
	::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_KEYWORD_COUNT ), szCount );
}


