/*!	@file
	タイプ別設定 - 正規表現キーワード ダイアログボックス

	@author MIK
	@date 2001/11/17  新規作成
*/
/*
	Copyright (C) 2001, MIK, Stonee
	Copyright (C) 2002, MIK
	Copyright (C) 2003, MIK, KEITA
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "StdAfx.h"
#include "CPropTypes.h"
#include "env/CShareData.h"
#include "CRegexKeyword.h"
#include "typeprop/CImpExpManager.h"	// 2010/4/23 Uchi
#include "util/shell.h"
#include "util/other_util.h"
#include "view/colors/EColorIndexType.h"
#include "sakura_rc.h"
#include "sakura.hh"

using namespace std;


static const DWORD p_helpids[] = {	//11600
	IDC_BUTTON_REGEX_IMPORT,	HIDC_BUTTON_REGEX_IMPORT,	//インポート
	IDC_BUTTON_REGEX_EXPORT,	HIDC_BUTTON_REGEX_EXPORT,	//エクスポート
	IDC_BUTTON_REGEX_INS,		HIDC_BUTTON_REGEX_INS,		//挿入
	IDC_BUTTON_REGEX_ADD,		HIDC_BUTTON_REGEX_ADD,		//追加
	IDC_BUTTON_REGEX_UPD,		HIDC_BUTTON_REGEX_UPD,		//更新
	IDC_BUTTON_REGEX_DEL,		HIDC_BUTTON_REGEX_DEL,		//削除
	IDC_BUTTON_REGEX_TOP,		HIDC_BUTTON_REGEX_TOP,		//先頭
	IDC_BUTTON_REGEX_LAST,		HIDC_BUTTON_REGEX_LAST,		//最終
	IDC_BUTTON_REGEX_UP,		HIDC_BUTTON_REGEX_UP,		//上へ
	IDC_BUTTON_REGEX_DOWN,		HIDC_BUTTON_REGEX_DOWN,		//下へ
	IDC_CHECK_REGEX,			HIDC_CHECK_REGEX,			//正規表現キーワードを使用する
	IDC_COMBO_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		//色
	IDC_EDIT_REGEX,				HIDC_EDIT_REGEX,			//正規表現キーワード
	IDC_LIST_REGEX,				HIDC_LIST_REGEX,			//リスト
	IDC_LABEL_REGEX_KEYWORD,	HIDC_EDIT_REGEX,			
	IDC_LABEL_REGEX_COLOR,		HIDC_COMBO_REGEX_COLOR,		
	IDC_FRAME_REGEX,			HIDC_LIST_REGEX,			
	IDC_LABEL_REGEX_VERSION,	HIDC_LABEL_REGEX_VERSION,	//バージョン
//	IDC_STATIC,						-1,
	0, 0
};



// Import
// 2010/4/23 Uchi Importの外出し
bool CPropTypesRegex::Import(HWND hwndDlg)
{
	CImpExpRegex	cImpExpRegex(m_Types);

	// インポート
	bool bImport = cImpExpRegex.ImportUI(m_hInstance, hwndDlg);
	if( bImport ){
		SetDataKeywordList(hwndDlg);
	}
	return bImport;
}

// Export
// 2010/4/23 Uchi Exportの外出し
bool CPropTypesRegex::Export(HWND hwndDlg)
{
	GetData(hwndDlg);
	CImpExpRegex	cImpExpRegex(m_Types);

	// エクスポート
	return cImpExpRegex.ExportUI(m_hInstance, hwndDlg);
}

/* 正規表現キーワード メッセージ処理 */
INT_PTR CPropTypesRegex::DispatchEvent(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndList;
	NMHDR*	pNMHDR;
	int	nIndex, nIndex2, i, j, nRet;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	static int nPrevIndex = -1;	//更新時におかしくなるバグ修正 @@@ 2003.03.26 MIK


	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );

	// ANSIビルドではCP932だと2倍程度必要
	const int nKeyWordSize = MAX_REGEX_KEYWORDLEN;
	TCHAR	szColorIndex[256];

	switch( uMsg ){
	case WM_INITDIALOG:
		// Modified by KEITA for WIN64 2003.9.6
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );

		/* カラム追加 */
		//ListView_DeleteColumn( hwndList, 1 );
		//ListView_DeleteColumn( hwndList, 0 );
		::GetWindowRect( hwndList, &rc );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 54 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPEREGEX_LIST1));
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 38 / 100;
		col.pszText  = const_cast<TCHAR*>(LS(STR_PROPTYPEREGEX_LIST2));
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		nPrevIndex = -1;	//@@@ 2003.05.12 MIK
		SetData( hwndDlg );	/* ダイアログデータの設定 正規表現キーワード */
		if( CheckRegexpVersion( hwndDlg, IDC_LABEL_REGEX_VERSION, false ) == false )	//@@@ 2001.11.17 add MIK
		{
			::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEX_VERSION, LS(STR_PROPTYPEREGEX_NOUSE) );
			//ライブラリがなくて、使用しないになっている場合は、無効にする。
			if( ! IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
			{
				//Disableにする。
				EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
			}
			else
			{
				//使用するになってるんだけどDisableにする。もうユーザは変更できない。
				EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
			}
		}
		return TRUE;

	case WM_COMMAND:
		wNotifyCode = HIWORD(wParam);	/* 通知コード */
		wID	= LOWORD(wParam);	/* 項目ID､ コントロールID､ またはアクセラレータID */
		switch( wNotifyCode ){
		/* ボタン／チェックボックスがクリックされた */
		case BN_CLICKED:
			switch( wID ){
			case IDC_CHECK_REGEX:	/* 正規表現キーワードを使う */
				if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
				{
					if( CheckRegexpVersion( NULL, 0, false ) == false )
					{
						nRet = ::MYMESSAGEBOX(
								hwndDlg,
								MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
								GSTR_APPNAME,
								LS(STR_PROPTYPEREGEX_NOTFOUND) );
						if( nRet != IDYES )
						{
							CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );
							//Disableにする。
							EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
							return TRUE;
						}
					}
				}
				else
				{
					if( CheckRegexpVersion( NULL, 0, false ) == false )
					{
						//Disableにする。
						EnableWindow( GetDlgItem( hwndDlg, IDC_CHECK_REGEX ), FALSE );
					}
				}
				m_Types.m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();	//Need Compile
				return TRUE;

			case IDC_BUTTON_REGEX_INS:	/* 挿入 */
			{
				//挿入するキー情報を取得する。
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				szKeyWord[0] = _T('\0');
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, &szKeyWord[0], nKeyWordSize );
				if( szKeyWord[0] == _T('\0') ) return FALSE;
				//同じキーがないか調べる。
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, LS(STR_PROPTYPEREGEX_NOREG));
					return FALSE;
				}
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					//選択中でなければ最後にする。
					nIndex = nIndex2;
				}
				if( !CheckKeywordList(hwndDlg, &szKeyWord[0], -1) ){
					return FALSE;
				}
				
				//挿入するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//キー情報を挿入する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//挿入したキーを選択する。
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_ADD:	/* 追加 */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				//最後のキー番号を取得する。
				nIndex = ListView_GetItemCount( hwndList );
				//追加するキー情報を取得する。
				szKeyWord[0] = _T('\0');
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, &szKeyWord[0], nKeyWordSize );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, LS(STR_PROPTYPEREGEX_NOREG));
					return FALSE;
				}
				if( !CheckKeywordList(hwndDlg, &szKeyWord[0], -1) ){
					return FALSE;
				}
				//追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//追加したキーを選択する。
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_UPD:	/* 更新 */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					ErrorMessage( hwndDlg, LS(STR_PROPTYPEREGEX_NOSEL));
					return FALSE;
				}
				//更新するキー情報を取得する。
				szKeyWord[0] = _T('\0');
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, &szKeyWord[0], nKeyWordSize );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				if( !CheckKeywordList(hwndDlg, &szKeyWord[0], nIndex) ){
					return FALSE;
				}
				//追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
				//キーを更新する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_SetItem( hwndList, &lvi );

				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );

				//更新したキーを選択する。
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_DEL:	/* 削除 */
				//選択中のキー番号を探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				//削除する。
				ListView_DeleteItem( hwndList, nIndex );
				//同じ位置のキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_TOP:	/* 先頭 */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				szKeyWord[0] = _T('\0');
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//すでに先頭にある。
				nIndex2 = 0;
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyWord[0], nKeyWordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//移動したキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_LAST:	/* 最終 */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				szKeyWord[0] = _T('\0');
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//すでに最終にある。
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyWord[0], nKeyWordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//移動したキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_UP:	/* 上へ */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				szKeyWord[0] = _T('\0');
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//すでに先頭にある。
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex - 1;
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyWord[0], nKeyWordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//移動したキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_DOWN:	/* 下へ */
			{
				auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
				szKeyWord[0] = _T('\0');
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//すでに最終にある。
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex + 2;
				ListView_GetItemText(hwndList, nIndex, 0, &szKeyWord[0], nKeyWordSize);
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = &szKeyWord[0];
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 0;
				lvi.lParam   = 0;
				ListView_InsertItem( hwndList, &lvi );
				lvi.mask     = LVIF_TEXT;
				lvi.iItem    = nIndex2;
				lvi.iSubItem = 1;
				lvi.pszText  = szColorIndex;
				ListView_SetItem( hwndList, &lvi );
				//移動したキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex2, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				GetData( hwndDlg );
				return TRUE;
			}

			case IDC_BUTTON_REGEX_IMPORT:	/* インポート */
				Import(hwndDlg);
				m_Types.m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;

			case IDC_BUTTON_REGEX_EXPORT:	/* エクスポート */
				Export(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_REGEX );
			return TRUE;
		case PSN_KILLACTIVE:
			/* ダイアログデータの取得 正規表現キーワード */
			GetData( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = ID_PROPTYPE_PAGENUM_REGEX;
			return TRUE;
		case LVN_ITEMCHANGED:
			if( pNMHDR->hwndFrom == hwndList )
			{
				HWND	hwndCombo;
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )	//削除、範囲外でクリック時反映されないバグ修正	//@@@ 2003.06.17 MIK
				{
					nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_FOCUSED );
				}
				if( -1 == nIndex )
				{
					/* 初期値を設定する */
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* 正規表現 */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for( i = 0, j = 0; i < COLORIDX_LAST; i++ )
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) &&
							0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
						{
							if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
							{
								Combo_SetCurSel( hwndCombo, j );	/* コンボボックスのデフォルト選択 */
								break;
							}
							j++;
						}
					}
					return FALSE;
				}
				if( nPrevIndex != nIndex )	//@@@ 2003.03.26 MIK
				{	//更新時にListViewのSubItemを正しく取得できないので、その対策
					auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
					szKeyWord[0] = _T('\0');
					ListView_GetItemText(hwndList, nIndex, 0, &szKeyWord[0], nKeyWordSize);
					ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, &szKeyWord[0] );	/* 正規表現 */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for(i = 0, j = 0; i < COLORIDX_LAST; i++)
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) &&
							0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
						{
							if(_tcscmp(m_Types.m_ColorInfoArr[i].m_szName, szColorIndex) == 0)
							{
								Combo_SetCurSel(hwndCombo, j);
								break;
							}
							j++;
						}
					}
				}
				nPrevIndex = nIndex;
			}
			break;
		}
		break;

	case WM_HELP:
		{
			HELPINFO *p = (HELPINFO *)lParam;
			MyWinHelp( (HWND)p->hItemHandle, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	}
	return FALSE;
}

/* ダイアログデータの設定 正規表現キーワード */
void CPropTypesRegex::SetData( HWND hwndDlg )
{
	HWND		hwndWork;
	int			i, j;

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEX ), MAX_REGEX_KEYWORDLEN - 1 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* 正規表現 */

	/* 色種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
	Combo_ResetContent( hwndWork );  /* コンボボックスを空にする */
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		GetDefaultColorInfoName( &m_Types.m_ColorInfoArr[i], i );
		if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_TEXT) &&
			0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
		{
			j = Combo_AddString( hwndWork, m_Types.m_ColorInfoArr[i].m_szName );
			if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
				Combo_SetCurSel( hwndWork, j );	/* コンボボックスのデフォルト選択 */
		}
	}

	if( m_Types.m_bUseRegexKeyword )
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );

	/* 行選択 */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	DWORD		dwStyle;
	dwStyle = ListView_GetExtendedListViewStyle( hwndWork );
	dwStyle |= LVS_EX_FULLROWSELECT;
	ListView_SetExtendedListViewStyle( hwndWork, dwStyle );

	SetDataKeywordList( hwndDlg );
}

/* ダイアログデータの設定 正規表現キーワードの一覧部分 */
void CPropTypesRegex::SetDataKeywordList( HWND hwndDlg )
{
	LV_ITEM		lvi;

	/* リスト */
	HWND hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* リストを空にする */

	/* データ表示 */
	wchar_t *pKeyword = &m_Types.m_RegexKeywordList[0];
	for(int i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( *pKeyword == L'\0' ) break;
		
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(pKeyword));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0; //m_Types.m_RegexKeywordArr[i].m_nColorIndex;
		ListView_InsertItem( hwndWork, &lvi );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_ColorInfoArr[m_Types.m_RegexKeywordArr[i].m_nColorIndex].m_szName;
		ListView_SetItem( hwndWork, &lvi );
		for(; *pKeyword != '\0'; pKeyword++){}
		pKeyword++;
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* ダイアログデータの取得 正規表現キーワード */
int CPropTypesRegex::GetData( HWND hwndDlg )
{
	HWND	hwndList;
	int	nIndex, i, j;
	const int szKeyWordSize = _countof(m_Types.m_RegexKeywordList) * 2 + 1;
	auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ szKeyWordSize ]);
	TCHAR	szColorIndex[256];

	//使用する・使用しない
	if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
		m_Types.m_bUseRegexKeyword = true;
	else
		m_Types.m_bUseRegexKeyword = false;

	//リストに登録されている情報を配列に取り込む
	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	nIndex = ListView_GetItemCount(hwndList);
	wchar_t* pKeyword = &m_Types.m_RegexKeywordList[0];
	wchar_t* pKeywordLast = pKeyword + _countof(m_Types.m_RegexKeywordList) - 1;
	// key1\0key2\0\0 の形式
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( i < nIndex )
		{
			szKeyWord[0]    = _T('\0');
			szColorIndex[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, &szKeyWord[0], szKeyWordSize );
			ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));
			if( pKeyword < pKeywordLast - 1 ){
				_tcstowcs(pKeyword, &szKeyWord[0], pKeywordLast - pKeyword);
			}
			//色指定文字列を番号に変換する
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
			for(j = 0; j < COLORIDX_LAST; j++)
			{
				if(_tcscmp(m_Types.m_ColorInfoArr[j].m_szName, szColorIndex) == 0)
				{
					m_Types.m_RegexKeywordArr[i].m_nColorIndex = j;
					break;
				}
			}
			if( *pKeyword ){
				for(; *pKeyword != L'\0'; pKeyword++){}
				pKeyword++;
			}
		}
		else	//未登録部分はクリアする
		{
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
		}
	}
	*pKeyword = L'\0'; // 番兵

	//タイプ設定の変更があった
	m_Types.m_nRegexKeyMagicNumber = CRegexKeyword::GetNewMagicNumber();
//	m_Types.m_nRegexKeyMagicNumber = 0;	//Not Compiled.

	return TRUE;
}

/*!
	@date 2010.07.11 Moca 今のところCRegexKeyword::RegexKeyCheckSyntaxと同一なので、中身を削除して転送関数に変更
*/
BOOL CPropTypesRegex::RegexKakomiCheck(const wchar_t *s)
{
	return CRegexKeyword::RegexKeyCheckSyntax( s );
}
//@@@ 2001.11.17 add end MIK

bool CPropTypesRegex::CheckKeywordList(HWND hwndDlg, const TCHAR* szNewKeyWord, int nUpdateItem)
{
	int nRet;
	//書式をチェックする。
	if( !RegexKakomiCheck(to_wchar(szNewKeyWord)) )	//囲みをチェックする。
	{
		nRet = ::MYMESSAGEBOX(
				hwndDlg,
				MB_OK | MB_ICONSTOP | MB_TOPMOST | MB_DEFBUTTON2,
				GSTR_APPNAME,
				LS(STR_PROPTYPEREGEX_KAKOMI) );
		return false;
	}
	if( !CheckRegexpSyntax( to_wchar(szNewKeyWord), hwndDlg, false, -1, true ) )
	{
		nRet = ::MYMESSAGEBOX(
				hwndDlg,
				MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
				GSTR_APPNAME,
				LS(STR_PROPTYPEREGEX_INVALID) );
		if( nRet != IDYES ) return false;
	}
	// 重複確認・文字列長制限チェック
	const int nKeyWordSize = MAX_REGEX_KEYWORDLEN;
	HWND hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	int  nIndex  = ListView_GetItemCount(hwndList);
	auto_array_ptr<TCHAR> szKeyWord(new TCHAR [ nKeyWordSize ]);
	int nKeywordLen = auto_strlen(to_wchar(szNewKeyWord)) + 1;
	for(int i = 0; i < nIndex; i++){
		if( i != nUpdateItem ){
			szKeyWord[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, &szKeyWord[0], nKeyWordSize);
			if( _tcscmp(szNewKeyWord, &szKeyWord[0]) == 0 ) 
			{
				ErrorMessage( hwndDlg, LS(STR_PROPTYPEREGEX_ALREADY));
				return false;
			}
			// 長さには\0も含む
			nKeywordLen += auto_strlen(to_wchar(&szKeyWord[0])) + 1;
			if( _countof(m_Types.m_RegexKeywordList) - 1 < nKeywordLen ){
				ErrorMessage( hwndDlg, LS(STR_PROPTYPEREGEX_FULL) );
				return false;
			}
		}
	}
	return true;
}
