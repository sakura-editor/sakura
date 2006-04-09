/*!	@file
	@brief 共通設定ダイアログボックス、「強調キーワード」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, KEITA
	Copyright (C) 2005, genta, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "stdafx.h"
#include "sakura_rc.h"
#include "CPropCommon.h"
#include "debug.h"
#include <windows.h>
#include <stdio.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "CDlgInput1.h"
#include "global.h"


//@@@ 2001.02.04 Start by MIK: Popup Help
#if 1	//@@@ 2002.01.03 add MIK
#include "sakura.hh"
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				HIDC_BUTTON_ADDSET,			//キーワードセット追加
	IDC_BUTTON_DELSET,				HIDC_BUTTON_DELSET,			//キーワードセット削除
	IDC_BUTTON_ADDKEYWORD,			HIDC_BUTTON_ADDKEYWORD,		//キーワード追加
	IDC_BUTTON_EDITKEYWORD,			HIDC_BUTTON_EDITKEYWORD,	//キーワード編集
	IDC_BUTTON_DELKEYWORD,			HIDC_BUTTON_DELKEYWORD,		//キーワード削除
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYWORD,	//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYWORD,	//エクスポート
	IDC_CHECK_KEYWORDCASE,			HIDC_CHECK_KEYWORDCASE,		//キーワードの英大文字小文字区別
	IDC_COMBO_SET,					HIDC_COMBO_SET,				//強調キーワードセット名
	IDC_LIST_KEYWORD,				HIDC_LIST_KEYWORD,			//キーワード一覧
//	IDC_STATIC,						-1,
	0, 0
};
#else
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				10800,	//キーワードセット追加
	IDC_BUTTON_DELSET,				10801,	//キーワードセット削除
	IDC_BUTTON_ADDKEYWORD,			10802,	//キーワード追加
	IDC_BUTTON_EDITKEYWORD,			10803,	//キーワード編集
	IDC_BUTTON_DELKEYWORD,			10804,	//キーワード削除
	IDC_BUTTON_IMPORT,				10805,	//インポート
	IDC_BUTTON_EXPORT,				10806,	//エクスポート
	IDC_CHECK_KEYWORDCASE,			10810,	//キーワードの英大文字小文字区別
	IDC_COMBO_SET,					10830,	//強調キーワードセット名
	IDC_LIST_KEYWORD,				10840,	//キーワード一覧
//	IDC_STATIC,						-1,
	0, 0
};
#endif
//@@@ 2001.02.04 End

//	From Here Jun. 2, 2001 genta
/*!
	@param hwndDlg ダイアログボックスのWindow Handle
	@param uMsg メッセージ
	@param wParam パラメータ1
	@param lParam パラメータ2
*/
INT_PTR CALLBACK CPropCommon::DlgProc_PROP_KEYWORD(
	HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return DlgProc( &CPropCommon::DispatchEvent_p7, hwndDlg, uMsg, wParam, lParam );
}
//	To Here Jun. 2, 2001 genta

/* p7 メッセージ処理 */
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
//	LPDRAWITEMSTRUCT	pDis;
	int					nIndex1;
//	int					nIndex2;
//	int					nIndex3;
//	int					nNum;
	int					i;
//	int					j;
	static char			pszLabel[1024];
//	HDC					hdc;
//	TEXTMETRIC			tm;
	static int			nListItemHeight;
//	LRESULT				lResult;
	LV_COLUMN			lvc;
//	LV_ITEM				lvi;
	LV_ITEM*			plvi;
	static HWND			hwndCOMBO_SET;
	static HWND			hwndLIST_KEYWORD;
	RECT				rc;
	CDlgInput1			cDlgInput1;
	char				szKeyWord[MAX_KEYWORDLEN + 1];
	DWORD				dwStyle;
	LV_DISPINFO*		plvdi;
	LV_KEYDOWN*			pnkd;

	switch( uMsg ){
	case WM_INITDIALOG:
		/* ダイアログデータの設定 p7 */
		SetData_p7( hwndDlg );
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* コントロールのハンドルを取得 */
		hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
		hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
		::GetWindowRect( hwndLIST_KEYWORD, &rc );
		lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lvc.fmt = LVCFMT_LEFT;
		lvc.cx = rc.right - rc.left;
		lvc.pszText = "";
		lvc.iSubItem = 0;
		ListView_InsertColumn( hwndLIST_KEYWORD, 0, &lvc );

		dwStyle = (DWORD)::GetWindowLong( hwndLIST_KEYWORD, GWL_STYLE );
		::SetWindowLong( hwndLIST_KEYWORD, GWL_STYLE, dwStyle | LVS_SHOWSELALWAYS );
//				(dwStyle & ~LVS_TYPEMASK) | dwView);


		/* コントロール更新のタイミング用のタイマーを起動 */
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
//				MYTRACE( "NM_DBLCLK     \n" );
				/* p7:リスト中で選択されているキーワードを編集する */
				p7_Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
				return TRUE;
			case LVN_BEGINLABELEDIT:
#ifdef _DEBUG
				MYTRACE( "LVN_BEGINLABELEDIT\n" );
												MYTRACE( "	plvi->mask =[%xh]\n", plvi->mask );
												MYTRACE( "	plvi->iItem =[%d]\n", plvi->iItem );
												MYTRACE( "	plvi->iSubItem =[%d]\n", plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE( "	plvi->state =[%xf]\n", plvi->state );
												MYTRACE( "	plvi->stateMask =[%xh]\n", plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE( "	plvi->pszText =[%s]\n", plvi->pszText );
												MYTRACE( "	plvi->cchTextMax=[%d]\n", plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE( "	plvi->iImage=[%d]\n", plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE( "	plvi->lParam=[%xh(%d)]\n", plvi->lParam, plvi->lParam );
#endif
				return TRUE;
			case LVN_ENDLABELEDIT:
#ifdef _DEBUG
				MYTRACE( "LVN_ENDLABELEDIT\n" );
												MYTRACE( "	plvi->mask =[%xh]\n", plvi->mask );
												MYTRACE( "	plvi->iItem =[%d]\n", plvi->iItem );
												MYTRACE( "	plvi->iSubItem =[%d]\n", plvi->iSubItem );
				if (plvi->mask & LVIF_STATE)	MYTRACE( "	plvi->state =[%xf]\n", plvi->state );
												MYTRACE( "	plvi->stateMask =[%xh]\n", plvi->stateMask );
				if (plvi->mask & LVIF_TEXT)		MYTRACE( "	plvi->pszText =[%s]\n", plvi->pszText  );
												MYTRACE( "	plvi->cchTextMax=[%d]\n", plvi->cchTextMax );
				if (plvi->mask & LVIF_IMAGE)	MYTRACE( "	plvi->iImage=[%d]\n", plvi->iImage );
				if (plvi->mask & LVIF_PARAM)	MYTRACE( "	plvi->lParam=[%xh(%d)]\n", plvi->lParam, plvi->lParam );
#endif
				if( NULL == plvi->pszText ){
					return TRUE;
				}
				if( 0 < strlen( plvi->pszText ) ){
					if( MAX_KEYWORDLEN < strlen( plvi->pszText ) ){
						::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
							"キーワードの長さは%dバイトまでです。", MAX_KEYWORDLEN
						);
						return TRUE;
					}
					/* ｎ番目のセットにキーワードを編集 */
					m_CKeyWordSetMgr.UpdateKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam, plvi->pszText );
				}else{
					/* ｎ番目のセットのｍ番目のキーワードを削除 */
					m_CKeyWordSetMgr.DelKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam );
				}
				/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
				SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

				ListView_SetItemState( hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

				return TRUE;
			case LVN_KEYDOWN:
//				MYTRACE( "LVN_KEYDOWN\n" );
				switch( pnkd->wVKey ){
				case VK_DELETE:
					/* p7:リスト中で選択されているキーワードを削除する */
					p7_Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					break;
				case VK_SPACE:
					/* p7:リスト中で選択されているキーワードを編集する */
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
				MYTRACE( "p7 PSN_KILLACTIVE\n" );
#endif
				/* ダイアログデータの取得 p7 */
				GetData_p7( hwndDlg );
				return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
			case PSN_SETACTIVE:
				m_nPageNum = ID_PAGENUM_KEYWORD;
				return TRUE;
			}
		}
		break;
	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID = LOWORD(wParam);			/* 項目ID､ コントロールID､ またはアクセラレータID */
		hwndCtl = (HWND) lParam;		/* コントロールのハンドル */
		if( hwndCOMBO_SET == hwndCtl){
			switch( wNotifyCode ){
			case CBN_SELCHANGE:
				nIndex1 = ::SendMessage( hwndCOMBO_SET, CB_GETCURSEL, 0, 0 );
				/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
				SetData_p7_KeyWordSet( hwndDlg, nIndex1 );
				return TRUE;
			}
		}else{
			switch( wNotifyCode ){
			/* ボタン／チェックボックスがクリックされた */
			case BN_CLICKED:
				switch( wID ){
				case IDC_BUTTON_ADDSET:	/* セット追加 */
					if( MAX_SETNUM <= m_CKeyWordSetMgr.m_nKeyWordSetNum ){
						::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
							"セットは%d個までしか登録できません。\n", MAX_SETNUM
						);
						return TRUE;
					}
					/* モードレスダイアログの表示 */
					strcpy( szKeyWord, "" );
					//	Oct. 5, 2002 genta 長さ制限の設定を修正．バッファオーバーランしていた．
					if( FALSE == cDlgInput1.DoModal( m_hInstance, hwndDlg, "キーワードのセット追加", "セット名を入力してください。", MAX_SETNAMELEN, szKeyWord ) ){
						return TRUE;
					}
					if( 0 < strlen( szKeyWord ) ){
						/* セットの追加 */
						m_CKeyWordSetMgr.AddKeyWordSet( szKeyWord, FALSE );

						m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = m_CKeyWordSetMgr.m_nKeyWordSetNum - 1;

						/* ダイアログデータの設定 p7 */
						SetData_p7( hwndDlg );
					}
					return TRUE;
				case IDC_BUTTON_DELSET:	/* セット削除 */
					nIndex1 = ::SendMessage( hwndCOMBO_SET, CB_GETCURSEL, 0, 0 );
					if( CB_ERR == nIndex1 ){
						return TRUE;
					}
					/* 削除対象のセットを使用しているファイルタイプを列挙 */
					strcpy( pszLabel, "" );
					for( i = 0; i < MAX_TYPES; ++i ){
						// 2002/04/25 YAZAKI Types全体を保持する必要はないし、m_pShareDataを直接見ても問題ない。
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
							strcat( pszLabel, "・" );
							strcat( pszLabel, m_pShareData->m_Types[i].m_szTypeName );
							strcat( pszLabel, "（" );
							strcat( pszLabel, m_pShareData->m_Types[i].m_szTypeExts );
							strcat( pszLabel, "）" );
							strcat( pszLabel, "\n" );
						}
					}
					if( IDCANCEL == ::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
						"「%s」のセットを削除します。\nよろしいですか？\n削除しようとするセットは、以下のファイルタイプに割り当てられています。\n削除したセットは無効になります。\n\n%s",
						m_CKeyWordSetMgr.GetTypeName( nIndex1 ),
						pszLabel
					) ){
						return TRUE;
					}
					/* 削除対象のセットを使用しているファイルタイプのセットをクリア */
					for( i = 0; i < MAX_TYPES; ++i ){
						// 2002/04/25 YAZAKI Types全体を保持する必要はない。
						for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
							if( nIndex1 == m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j] = -1;
							}
							else if( nIndex1 < m_Types_nKeyWordSetIdx[i][j] ){
								m_Types_nKeyWordSetIdx[i][j]--;
							}
						}
					}
					/* ｎ番目のセットを削除 */
					m_CKeyWordSetMgr.DelKeyWordSet( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
					/* ダイアログデータの設定 p7 */
					SetData_p7( hwndDlg );
					return TRUE;
				case IDC_BUTTON_KEYSETRENAME: // キーワードセットの名称変更
					// モードレスダイアログの表示
					strcpy( szKeyWord, m_CKeyWordSetMgr.GetTypeName( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
					if( FALSE == cDlgInput1.DoModal( m_hInstance, hwndDlg, "セットの名称変更",
							"セット名を入力してください。", MAX_SETNAMELEN, szKeyWord ) ){
						return TRUE;
					}
					if( 0 < strlen( szKeyWord ) ){
						m_CKeyWordSetMgr.SetTypeName( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord );

						// ダイアログデータの設定 p7
						SetData_p7( hwndDlg );
					}
					return TRUE;
				case IDC_CHECK_KEYWORDCASE:	/* キーワードの英大文字小文字区別 */
//					m_CKeyWordSetMgr.m_nKEYWORDCASEArr[ m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ] = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );	//MIK 2000.12.01 case sense
					m_CKeyWordSetMgr.SetKeyWordCase(m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE ));			//MIK 2000.12.01 case sense
					return TRUE;
				case IDC_BUTTON_ADDKEYWORD:	/* キーワード追加 */
					/* ｎ番目のセットのキーワードの数を返す */
					if( !m_CKeyWordSetMgr.CanAddKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
						::MYMESSAGEBOX(	hwndDlg,	MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
							"登録できるキーワード数が上限に達しています。\n"
						);
						return TRUE;
					}
					/* モードレスダイアログの表示 */
					strcpy( szKeyWord, "" );
					if( FALSE == cDlgInput1.DoModal( m_hInstance, hwndDlg, "キーワード追加", "キーワードを入力してください。", MAX_KEYWORDLEN, szKeyWord ) ){
						return TRUE;
					}
					if( 0 < strlen( szKeyWord ) ){
						/* ｎ番目のセットにキーワードを追加 */
						if( 0 == m_CKeyWordSetMgr.AddKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord ) ){
							// ダイアログデータの設定 p7 指定キーワードセットの設定
							SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
						}
					}
					return TRUE;
				case IDC_BUTTON_EDITKEYWORD:	/* キーワード編集 */
					/* p7:リスト中で選択されているキーワードを編集する */
					p7_Edit_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_DELKEYWORD:	/* キーワード削除 */
					/* p7:リスト中で選択されているキーワードを削除する */
					p7_Delete_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// From Here 2005.01.26 Moca
				case IDC_BUTTON_KEYCLEAN:
					p7_Clean_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				// To Here 2005.01.26 Moca
				case IDC_BUTTON_IMPORT:	/* インポート */
					/* p7:リスト中のキーワードをインポートする */
					p7_Import_List_KeyWord( hwndDlg, hwndLIST_KEYWORD );
					return TRUE;
				case IDC_BUTTON_EXPORT:	/* エクスポート */
					/* p7:リスト中のキーワードをエクスポートする */
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
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		//break;
//@@@ 2001.02.04 End

//@@@ 2001.12.22 Start by MIK: Context Menu Help
	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );
		return TRUE;
//@@@ 2001.12.22 End

	}
	return FALSE;
}

/* p7:リスト中で選択されているキーワードを編集する */
void CPropCommon::p7_Edit_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	int			nIndex1;
	LV_ITEM		lvi;
	char		szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1	cDlgInput1;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );

	/* ｎ番目のセットのｍ番目のキーワードを返す */
	strcpy( szKeyWord, m_CKeyWordSetMgr.GetKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam ) );

	/* モードレスダイアログの表示 */
	if( FALSE == cDlgInput1.DoModal( m_hInstance, hwndDlg, "キーワード編集", "キーワードを編集してください。", MAX_KEYWORDLEN, szKeyWord ) ){
		return;
	}
	if( 0 < strlen( szKeyWord ) ){
		/* ｎ番目のセットにキーワードを編集 */
		m_CKeyWordSetMgr.UpdateKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam, szKeyWord );
	}else{
		/* ｎ番目のセットのｍ番目のキーワードを削除 */
		m_CKeyWordSetMgr.DelKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	}
	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}



/* p7:リスト中で選択されているキーワードを削除する */
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
	/* ｎ番目のセットのｍ番目のキーワードを削除 */
	m_CKeyWordSetMgr.DelKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	//キーワード数を表示する。
	DispKeywordCount( hwndDlg );

	return;
}


/* p7:リスト中のキーワードをインポートする */
void CPropCommon::p7_Import_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
//	::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
//		"ファイルから単語をインポートする機能は、まだ完成していないのです。\n"
//	);
//	return;


	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	FILE*			pFile;
	char			szLine[1024];
	int				i;
	bool			bAddError = false;

	strcpy( szPath, "" );
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.kwd",
		szPath
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return;
	}
	pFile = fopen( szPath, "r" );
	if( NULL == pFile ){
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", szPath
		);
		return;
	}
	while( NULL != fgets( szLine, sizeof(szLine), pFile ) ){
//		MYTRACE( szLine );
		if( 2 < strlen( szLine ) && 0 == memcmp( szLine, "//", 2 )  ){
		}else{
			if( 0 < (int)strlen( szLine ) ){
				for( i = 0; i < (int)strlen( szLine ); ++i ){
					if( szLine[i] == '\r' || szLine[i] == '\n' ){
						szLine[i] = '\0';
					}
				}
			}
			if( 0 < (int)strlen( szLine ) ){
				/* ｎ番目のセットにキーワードを追加 */
				int nRetValue = m_CKeyWordSetMgr.AddKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szLine );
				if( 2 == nRetValue ){
					bAddError = true;
					break;
				}
			}
		}
	}
	fclose( pFile );
	if( bAddError ){
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"キーワードの数が上限に達したため、いくつかのキーワードを追加できませんでした。"
		);
	}
	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	return;
}


/* p7:リスト中のキーワードをエクスポートする */
void CPropCommon::p7_Export_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
//	::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
//		"単語をファイルへエクスポートする機能は、まだ完成していないのです。\n"
//	);
	CDlgOpenFile	cDlgOpenFile;
	char			szPath[_MAX_PATH + 1];
	FILE*			pFile;
//	char			szLine[1024];
	int				i;
	int				nKeyWordNum;

	strcpy( szPath, "" );
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.kwd",
		szPath
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return;
	}
//	MYTRACE( "%s\n", szPath );
	pFile = fopen( szPath, "w" );
	if( NULL == pFile ){
		::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME,
			"ファイルを開けませんでした。\n\n%s", szPath
		);
		return;
	}
	fputs( "// ", pFile );
	fputs( m_CKeyWordSetMgr.GetTypeName( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ), pFile );
	fputs( "  キーワード定義ファイル", pFile );
	fputs( "\n", pFile );
	fputs( "\n", pFile );

	m_CKeyWordSetMgr.SortKeyWord(m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx);	//MIK 2000.12.01 sort keyword

	/* ｎ番目のセットのキーワードの数を返す */
	nKeyWordNum = m_CKeyWordSetMgr.GetKeyWordNum( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	for( i = 0; i < nKeyWordNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		m_CKeyWordSetMgr.GetKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, i );
		fputs( m_CKeyWordSetMgr.GetKeyWord( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, i ), pFile );
		fputs( "\n", pFile );
	}
	fclose( pFile );
	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
		"ファイルへエクスポートしました。\n\n%s", szPath
	);

	return;
}


//! キーワードを整頓する
void CPropCommon::p7_Clean_List_KeyWord( HWND hwndDlg, HWND hwndLIST_KEYWORD )
{
	if( IDYES == ::MessageBox( hwndDlg, "現在の設定では強調キーワードとして表示できないキーワードを削除しますか？",
			GSTR_APPNAME, MB_YESNO | MB_ICONSTOP ) ){
		if( m_CKeyWordSetMgr.CleanKeyWords( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
		}
		SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}
}

/* ダイアログデータの設定 p7 */
void CPropCommon::SetData_p7( HWND hwndDlg )
{
	int		i;
//	LV_ITEM	lvi;
	HWND	hwndWork;
//	int		nIdx;
//	char*	pszWork;


	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	::SendMessage( hwndWork, CB_RESETCONTENT, 0, 0 );  /* コンボボックスを空にする */
	if( 0 < m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			::SendMessage( hwndWork, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR)m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		/* セット名コンボボックスのデフォルト選択 */
		::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, 0 );

		/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
		SetData_p7_KeyWordSet( hwndDlg, m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}else{
		/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
		SetData_p7_KeyWordSet( hwndDlg, -1 );
	}

	return;
}


/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
void CPropCommon::SetData_p7_KeyWordSet( HWND hwndDlg, int nIdx )
{
	int		i;
	int		nNum;
	char*	pszKeyWord;
	HWND	hwndList;
	LV_ITEM	lvi;

	ListView_DeleteAllItems( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ) );
	if( 0 <= nIdx ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), TRUE );
		//	Jan. 29, 2005 genta キーワードセット切り替え直後はキーワードは未選択
		//	そのため有効にしてすぐにタイマーで無効になる．
		//	なのでここで無効にしておく．
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

	/* キーワードの英大文字小文字区別 */
//	if( TRUE == m_CKeyWordSetMgr.m_ppcKeyWordSetArr[nIdx]->m_nKEYWORDCASE ){
//	if( TRUE == m_CKeyWordSetMgr.m_cKeyWordSetArr[nIdx].m_nKEYWORDCASE ){
//	if( TRUE == m_CKeyWordSetMgr.m_nKEYWORDCASEArr[nIdx] ){		//MIK 2000.12.01 case sense
	if( TRUE == m_CKeyWordSetMgr.GetKeyWordCase(nIdx) ){		//MIK 2000.12.01 case sense
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );
	}

	/* ｎ番目のセットのキーワードの数を返す */
	nNum = m_CKeyWordSetMgr.GetKeyWordNum( nIdx );
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	// 2005.01.25 Moca/genta リスト追加中は再描画を抑制してすばやく表示
	::SendMessage( hwndList, WM_SETREDRAW, FALSE, 0 );

	for( i = 0; i < nNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		pszKeyWord =  const_cast<char*>(m_CKeyWordSetMgr.GetKeyWord( nIdx, i ));

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = pszKeyWord;
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem( hwndList, &lvi );

	}
	m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nIdx;

	// 2005.01.25 Moca/genta リスト追加完了のため再描画許可
	::SendMessage( hwndList, WM_SETREDRAW, TRUE, 0 );

	//キーワード数を表示する。
	DispKeywordCount( hwndDlg );

	return;
}



/* ダイアログデータの取得 p7 */
int CPropCommon::GetData_p7( HWND hwndDlg )
{
//	HWND	hwndResList;
//	int		i;
//	int		j;
//	int		k;

//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	m_nPageNum = ID_PAGENUM_KEYWORD;


	return TRUE;
}

/* ダイアログデータの取得 p7 指定キーワードセットの取得 */
void CPropCommon::GetData_p7_KeyWordSet( HWND hwndDlg, int nIdx )
{
}

//キーワード数を表示する。
void CPropCommon::DispKeywordCount( HWND hwndDlg )
{
	HWND	hwndList;
	int		n;
	TCHAR szCount[ 256 ];

	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	n = ListView_GetItemCount( hwndList );
	if( n < 0 ) n = 0;

	int		nAlloc;
	nAlloc = m_CKeyWordSetMgr.GetAllocSize( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc -= m_CKeyWordSetMgr.GetKeyWordNum( m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc += m_CKeyWordSetMgr.GetFreeSize();
	
	wsprintf( szCount, _T("(最大 %d 文字, 登録数 %d, 空き %d 個)"), MAX_KEYWORDLEN, n, nAlloc );
	::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_KEYWORD_COUNT ), szCount );
}

/*[EOF]*/
