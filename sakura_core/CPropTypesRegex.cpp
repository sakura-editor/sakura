//	$Id$
/*!	@file
	タイプ別設定正規表現キーワードダイアログボックス

	@author MIK
	@date 2001/11/17  新規作成
	$Revision$
*/
/*
	Copyright (C) 2001, MIK
	Copyright (C) 2002, MIK
	Copyright (C) 2003, MIK

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

//@@@ 2001.11.17 add start MIK

#include "stdafx.h"
#include "sakura_rc.h"
#include "CPropTypes.h"
#include "debug.h"
#include <windows.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "etc_uty.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "funccode.h"	//Stonee, 2001/05/18
#include <stdio.h>	//@@@ 2001.11.17 add MIK
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK


#include "sakura.hh"
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


/* 正規表現キーワード ダイアログプロシージャ */
INT_PTR CALLBACK CPropTypes::PropTypesRegex(
	HWND	hwndDlg,	// handle to dialog box
	UINT	uMsg,		// message
	WPARAM	wParam,		// first message parameter
	LPARAM	lParam 		// second message parameter
)
{
	PROPSHEETPAGE*	pPsp;
	CPropTypes* pCPropTypes;
	switch( uMsg ){
	case WM_INITDIALOG:
		pPsp = (PROPSHEETPAGE*)lParam;
		pCPropTypes = ( CPropTypes* )(pPsp->lParam);
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_Regex( hwndDlg, uMsg, wParam, pPsp->lParam );
		}
		break;
	default:
		// Modified by KEITA for WIN64 2003.9.6
		pCPropTypes = ( CPropTypes* )::GetWindowLongPtr( hwndDlg, DWLP_USER );
		if( NULL != pCPropTypes ){
			return pCPropTypes->DispatchEvent_Regex( hwndDlg, uMsg, wParam, lParam );
		}
		break;
	}
	return FALSE;
}

BOOL CPropTypes::Import_Regex(HWND hwndDlg)
{
	CDlgOpenFile	cDlgOpenFile;
	char		szPath[_MAX_PATH + 1];
	int		i, j, k;
	char		szInitDir[_MAX_PATH + 1];
//	CProfile	cProfile;
	FILE		*fp;
	struct RegexKeywordInfo	pRegexKey[MAX_REGEX_KEYWORD];
	char	buff[1024];
	HWND	hwndWork;
	LV_ITEM	lvi;
	char	*p;

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.rkw",	/* [R]egex [K]ey[W]ord */
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return FALSE;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	if( (fp = fopen(szPath, "r")) == NULL )
	{
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "ファイルを開けませんでした。\n\n%s", szPath );
		return FALSE;
	}

	j = 0;
	while(fgets(buff, sizeof(buff), fp))
	{
		if(j >= MAX_REGEX_KEYWORD) break;
		for(i = strlen(buff) - 1; i >= 0; i--){
			if( buff[i] == '\r' || buff[i] == '\n' ) buff[i] = '\0';
		}
		//RxKey[999]=ColorName,RegexKeyword
		if( strlen(buff) < 12 ) continue;
		if( memcmp(buff, "RxKey[", 6) != 0 ) continue;
		if( memcmp(&buff[9], "]=", 2) != 0 ) continue;
		p = strstr(&buff[11], ",");
		if( p )
		{
			*p = '\0';
			p++;
			if( p[0]	//キーワードがある
			 && RegexKakomiCheck(p) == TRUE )	//囲みがある
			{
				//色指定名に対応する番号を探す
				k = GetColorIndexByName( &buff[11] );	//@@@ 2002.04.30
				if( k != -1 )	/* 3文字カラー名からインデックス番号に変換 */
				{
					pRegexKey[j].m_nColorIndex = k;
					strcpy(pRegexKey[j].m_szKeyword, p);
					j++;
				}
				else
				{	/* 日本語名からインデックス番号に変換する */
					for(k = 0; k < COLORIDX_LAST; k++)
					{
						if( strcmp(m_Types.m_ColorInfoArr[k].m_szName, &buff[11]) == 0 )
						{
							pRegexKey[j].m_nColorIndex = k;
							strcpy(pRegexKey[j].m_szKeyword, p);
							j++;
							break;
						}
					}
				}
			}
		}
	}

	fclose(fp);

	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* リストを空にする */
	for(i = 0; i < j; i++)
	{
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = pRegexKey[i].m_szKeyword;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0;
		ListView_InsertItem( hwndWork, &lvi );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_ColorInfoArr[pRegexKey[i].m_nColorIndex].m_szName;
		ListView_SetItem( hwndWork, &lvi );
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return TRUE;
}

BOOL CPropTypes::Export_Regex(HWND hwndDlg)
{
	CDlgOpenFile	cDlgOpenFile;
	char		szPath[_MAX_PATH + 1];
	int		i, j, k;
	char		szInitDir[_MAX_PATH + 1];
//	CProfile	cProfile;
	FILE		*fp;
	char	szKeyWord[256], szColorIndex[256];
	HWND	hwndList;

	strcpy( szPath, "" );
	strcpy( szInitDir, m_pShareData->m_szIMPORTFOLDER );	/* インポート用フォルダ */
	/* ファイルオープンダイアログの初期化 */
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		"*.rkw",	/* [R]egex [K]ey[W]ord */
		szInitDir
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return FALSE;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	strcat( m_pShareData->m_szIMPORTFOLDER, "\\" );

	if( (fp = fopen(szPath, "w")) == NULL )
	{
		::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "ファイルを開けませんでした。\n\n%s", szPath );
		return FALSE;
	}

	fprintf(fp, "// 正規表現キーワード Ver1\n");

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	j = ListView_GetItemCount(hwndList);
	for(i = 0; i < j; i++)
	{
		memset(szKeyWord, 0, sizeof(szKeyWord));
		ListView_GetItemText(hwndList, i, 0, szKeyWord, sizeof(szKeyWord));
		memset(szColorIndex, 0, sizeof(szColorIndex));
		ListView_GetItemText(hwndList, i, 1, szColorIndex, sizeof(szColorIndex));

		const char* p = szColorIndex;
		for(k = 0; k < COLORIDX_LAST; k++)
		{
			if( strcmp( m_Types.m_ColorInfoArr[k].m_szName, szColorIndex ) == 0 )
			{
				p = GetColorNameByIndex(k);
				break;
			}
		}
		//fprintf(fp, "RxKey[%03d]=%s,%s\n", i, szColorIndex, szKeyWord);
		fprintf(fp, "RxKey[%03d]=%s,%s\n", i, p, szKeyWord);
	}

	fclose(fp);

	::MYMESSAGEBOX(	hwndDlg, MB_OK | MB_ICONINFORMATION, GSTR_APPNAME,
		"ファイルへエクスポートしました。\n\n%s", szPath
	);

	return TRUE;
}

/* 正規表現キーワード メッセージ処理 */
INT_PTR CPropTypes::DispatchEvent_Regex(
	HWND		hwndDlg,	// handle to dialog box
	UINT		uMsg,		// message
	WPARAM		wParam,		// first message parameter
	LPARAM		lParam 		// second message parameter
)
{
	WORD	wNotifyCode;
	WORD	wID;
	HWND	hwndCtl, hwndList;
	int	idCtrl;
	NMHDR*	pNMHDR;
	int	nIndex, nIndex2, i, j, k, nRet;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	char	szKeyWord[256], szColorIndex[256];
	static int nPrevIndex = -1;	//更新時におかしくなるバグ修正 @@@ 2003.03.26 MIK

	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );

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
		col.pszText  = "キーワード";
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 38 / 100;
		col.pszText  = "色指定";
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		nPrevIndex = -1;	//@@@ 2003.05.12 MIK
		SetData_Regex( hwndDlg );	/* ダイアログデータの設定 正規表現キーワード */
		if( CheckRegexpVersion( hwndDlg, IDC_LABEL_REGEX_VERSION, false ) == false )	//@@@ 2001.11.17 add MIK
		{
			::SetDlgItemText( hwndDlg, IDC_LABEL_REGEX_VERSION, "正規表現キーワードは使えません。" );
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
		hwndCtl	= (HWND) lParam;	/* コントロールのハンドル */
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
								"正規表現ライブラリが見つかりません。\n\n正規表現キーワードは機能しませんが、それでも有効にしますか？",
								"正規表現キーワードを使用する" );
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
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile
				return TRUE;

			case IDC_BUTTON_REGEX_INS:	/* 挿入 */
				//挿入するキー情報を取得する。
				memset(szKeyWord, 0, sizeof(szKeyWord));
				::GetDlgItemText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, sizeof(szKeyWord) );
				if( szKeyWord[0] == '\0' ) return FALSE;
				//同じキーがないか調べる。
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "これ以上登録できません。");
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					memset(szColorIndex, 0, sizeof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, sizeof(szColorIndex));
					if( strcmp(szKeyWord, szColorIndex) == 0 ) 
					{
						::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "同じキーワードで登録済みです。");
						return FALSE;
					}
				}
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					//選択中でなければ最後にする。
					nIndex = nIndex2;
				}
				//書式をチェックする。
				if( RegexKakomiCheck(szKeyWord) == FALSE )	//囲みをチェックする。
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							GSTR_APPNAME,
							"正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。",
							"正規表現キーワード" );
					return FALSE;
				}
				if( CheckRegexpSyntax( szKeyWord, hwndDlg, false ) == false )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							"書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？",
							"正規表現キーワード" );
					if( nRet != IDYES ) return FALSE;
				}
				//挿入するキー情報を取得する。
				memset(szColorIndex, 0, sizeof(szColorIndex));
				::GetDlgItemText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, sizeof(szColorIndex) );
				if( strcmp(szColorIndex, "カーソル行アンダーライン") == 0 )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "カーソル行アンダーラインは指定できません。");
					return FALSE;
				}
				//キー情報を挿入する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_ADD:	/* 追加 */
				//最後のキー番号を取得する。
				nIndex = ListView_GetItemCount( hwndList );
				//追加するキー情報を取得する。
				memset(szKeyWord, 0, sizeof(szKeyWord));
				::GetDlgItemText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, sizeof(szKeyWord) );
				if( szKeyWord[0] == '\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "これ以上登録できません。");
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					memset(szColorIndex, 0, sizeof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, sizeof(szColorIndex));
					if( strcmp(szKeyWord, szColorIndex) == 0 ) 
					{
						::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "同じキーワードで登録済みです。");
						return FALSE;
					}
				}
				//書式をチェックする。
				if( RegexKakomiCheck(szKeyWord) == FALSE )	//囲みをチェックする。
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_OK | MB_ICONSTOP | MB_TOPMOST,
							GSTR_APPNAME,
							"正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。",
							"正規表現キーワード" );
					return FALSE;
				}
				if( CheckRegexpSyntax( szKeyWord, hwndDlg, false ) == false )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							"書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？",
							"正規表現キーワード" );
					if( nRet != IDYES ) return FALSE;
				}
				//追加するキー情報を取得する。
				memset(szColorIndex, 0, sizeof(szColorIndex));
				::GetDlgItemText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, sizeof(szColorIndex) );
				if( strcmp(szColorIndex, "カーソル行アンダーライン") == 0 )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "カーソル行アンダーラインは指定できません。");
					return FALSE;
				}
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_UPD:	/* 更新 */
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "キーワードが選択されていません。");
					return FALSE;
				}
				//更新するキー情報を取得する。
				memset(szKeyWord, 0, sizeof(szKeyWord));
				::GetDlgItemText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, sizeof(szKeyWord) );
				if( szKeyWord[0] == '\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				for(i = 0; i < nIndex2; i++)
				{
					if( i != nIndex )
					{
						memset(szColorIndex, 0, sizeof(szColorIndex));
						ListView_GetItemText(hwndList, i, 0, szColorIndex, sizeof(szColorIndex));
						if( strcmp(szKeyWord, szColorIndex) == 0 ) 
						{
							::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "同じキーワードで登録済みです。");
							return FALSE;
						}
					}
				}
				//書式をチェックする。
				if( RegexKakomiCheck(szKeyWord) == FALSE )	//囲みをチェックする。
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_OK | MB_ICONSTOP | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							"正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。",
							"正規表現キーワード" );
					return FALSE;
				}
				if( CheckRegexpSyntax( szKeyWord, hwndDlg, false ) == false )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							"書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？",
							"正規表現キーワード" );
					if( nRet != IDYES ) return FALSE;
				}
				//追加するキー情報を取得する。
				memset(szColorIndex, 0, sizeof(szColorIndex));
				::GetDlgItemText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, sizeof(szColorIndex) );
				if( strcmp(szColorIndex, "カーソル行アンダーライン") == 0 )
				{
					::MYMESSAGEBOX( hwndDlg, MB_OK | MB_ICONSTOP, GSTR_APPNAME, "カーソル行アンダーラインは指定できません。");
					return FALSE;
				}
				//キーを更新する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_DEL:	/* 削除 */
				//選択中のキー番号を探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				//削除する。
				ListView_DeleteItem( hwndList, nIndex );
				//同じ位置のキーを選択状態にする。
				ListView_SetItemState( hwndList, nIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_TOP:	/* 先頭 */
				//選択中のキーを探す。
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//すでに先頭にある。
				nIndex2 = 0;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, sizeof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, sizeof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_LAST:	/* 最終 */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//すでに最終にある。
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, sizeof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, sizeof(szColorIndex));
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_UP:	/* 上へ */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				if( 0 == nIndex ) return TRUE;	//すでに先頭にある。
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex - 1;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, sizeof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, sizeof(szColorIndex));
				ListView_DeleteItem(hwndList, nIndex);	//古いキーを削除
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_DOWN:	/* 下へ */
				nIndex = ListView_GetNextItem( hwndList, -1, LVNI_ALL | LVNI_SELECTED );
				if( -1 == nIndex ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 - 1 == nIndex ) return TRUE;	//すでに最終にある。
				if( nIndex2 <= 1 ) return TRUE;
				nIndex2 = nIndex + 2;
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, sizeof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, sizeof(szColorIndex));
				//キーを追加する。
				lvi.mask     = LVIF_TEXT | LVIF_PARAM;
				lvi.pszText  = szKeyWord;
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
				GetData_Regex( hwndDlg );
				return TRUE;

			case IDC_BUTTON_REGEX_IMPORT:	/* インポート */
				Import_Regex(hwndDlg);
				m_Types.m_nRegexKeyMagicNumber++;	//Need Compile	//@@@ 2001.11.17 add MIK 正規表現キーワードのため
				return TRUE;

			case IDC_BUTTON_REGEX_EXPORT:	/* エクスポート */
				Export_Regex(hwndDlg);
				return TRUE;
			}
			break;
		}
		break;

	case WM_NOTIFY:
		idCtrl = (int)wParam;
		pNMHDR = (NMHDR*)lParam;
		switch( pNMHDR->code ){
		case PSN_HELP:
			OnHelp( hwndDlg, IDD_PROP_REGEX );
			return TRUE;
		case PSN_KILLACTIVE:
			/* ダイアログデータの取得 正規表現キーワード */
			GetData_Regex( hwndDlg );
			return TRUE;
//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
		case PSN_SETACTIVE:
			m_nPageNum = 3;
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
					::SetDlgItemText( hwndDlg, IDC_EDIT_REGEX, "//k" );	/* 正規表現 */
					j = 0;
					k = 0;
					for( i = 0; i < COLORIDX_LAST; i++ )
					{
						if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_UNDERLINE )
						{
							k++;
							continue;
						}
						if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 ) j = i;
					}
					j -= k;	//スキップした分を差し引く
					if( j >= 0 )
					{
						hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
						::SendMessage( hwndCombo, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* コンボボックスのデフォルト選択 */
					}
					return FALSE;
				}
				if( nPrevIndex != nIndex )	//@@@ 2003.03.26 MIK
				{	//更新時にListViewのSubItemを正しく取得できないので、その対策
					ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, sizeof(szKeyWord));
					ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, sizeof(szColorIndex));
					::SetDlgItemText( hwndDlg, IDC_EDIT_REGEX, szKeyWord );	/* 正規表現 */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for(i = 0, j = 0; i < COLORIDX_LAST; i++)
					{
						//if(strcmp(m_Types.m_ColorInfoArr[i].m_szName, "カーソル行アンダーライン") != 0)
						if( m_Types.m_ColorInfoArr[i].m_nColorIdx != COLORIDX_UNDERLINE )
						{
							if(strcmp(m_Types.m_ColorInfoArr[i].m_szName, szColorIndex) == 0)
							{
								::SendMessage(hwndCombo, CB_SETCURSEL, j, 0);
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
			::WinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );
		}
		return TRUE;
		/*NOTREACHED*/
		//break;

	//Context Menu
	case WM_CONTEXTMENU:
		::WinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );
		return TRUE;

	}
	return FALSE;
}

/* ダイアログデータの設定 正規表現キーワード */
void CPropTypes::SetData_Regex( HWND hwndDlg )
{
	HWND		hwndWork;
	int		i, nItem, j, k;
	LV_ITEM		lvi;
	DWORD		dwStyle;

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEX ), EM_LIMITTEXT, (WPARAM)(sizeof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) - 1 ), (LPARAM)0 );
	::SetDlgItemText( hwndDlg, IDC_EDIT_REGEX, "//k" );	/* 正規表現 */

	/* 色種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
	::SendMessage( hwndWork, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );  /* コンボボックスを空にする */
	j = 0;
	k = 0;
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		//if( strcmp(m_Types.m_ColorInfoArr[i].m_szName, "カーソル行アンダーライン") == 0 )
		if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_UNDERLINE )
		{
			k++;
			continue;
		}
		nItem = ::SendMessage( hwndWork, CB_ADDSTRING, (WPARAM)0, (LPARAM)(char*)m_Types.m_ColorInfoArr[i].m_szName );
		//if( strcmp(m_Types.m_ColorInfoArr[i].m_szName, "正規表現キーワード1") == 0 ) j = i;
		if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 ) j = i;
	}
	j -= k;	//スキップした分を差し引く
	if( j >= 0 )
		::SendMessage( hwndWork, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* コンボボックスのデフォルト選択 */

	if( m_Types.m_bUseRegexKeyword )
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );

	/* リスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* リストを空にする */

	/* 行選択 */
	dwStyle = (DWORD)::SendMessage( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	dwStyle |= LVS_EX_FULLROWSELECT;
	::SendMessage( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle );

	/* データ表示 */
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_Types.m_RegexKeywordArr[i].m_szKeyword[0] == '\0' ) break;
		
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = m_Types.m_RegexKeywordArr[i].m_szKeyword;
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.lParam   = 0; //m_Types.m_RegexKeywordArr[i].m_nColorIndex;
		ListView_InsertItem( hwndWork, &lvi );
		lvi.mask     = LVIF_TEXT;
		lvi.iItem    = i;
		lvi.iSubItem = 1;
		lvi.pszText  = m_Types.m_ColorInfoArr[m_Types.m_RegexKeywordArr[i].m_nColorIndex].m_szName;
		ListView_SetItem( hwndWork, &lvi );
	}
	ListView_SetItemState( hwndWork, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	return;
}

/* ダイアログデータの取得 正規表現キーワード */
int CPropTypes::GetData_Regex( HWND hwndDlg )
{
	HWND	hwndList;
	int	nIndex, i, j;
	char	szKeyWord[256], szColorIndex[256];

//@@@ 2002.01.03 YAZAKI 最後に表示していたシートを正しく覚えていないバグ修正
//	//自分のページ番号
//	m_nPageNum = 3;

	//使用する・使用しない
	if( IsDlgButtonChecked( hwndDlg, IDC_CHECK_REGEX ) )
		m_Types.m_bUseRegexKeyword = TRUE;
	else
		m_Types.m_bUseRegexKeyword = FALSE;

	//リストに登録されている情報を配列に取り込む
	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	nIndex = ListView_GetItemCount(hwndList);
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( i < nIndex )
		{
			szKeyWord[0]    = '\0';
			szColorIndex[0] = '\0';
			ListView_GetItemText(hwndList, i, 0, szKeyWord,    sizeof(szKeyWord)   );
			ListView_GetItemText(hwndList, i, 1, szColorIndex, sizeof(szColorIndex));
			strcpy(m_Types.m_RegexKeywordArr[i].m_szKeyword, szKeyWord);
			//色指定文字列を番号に変換する
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
			for(j = 0; j < COLORIDX_LAST; j++)
			{
				if(strcmp(m_Types.m_ColorInfoArr[j].m_szName, szColorIndex) == 0)
				{
					m_Types.m_RegexKeywordArr[i].m_nColorIndex = j;
					break;
				}
			}
		}
		else	//未登録部分はクリアする
		{
			m_Types.m_RegexKeywordArr[i].m_szKeyword[0] = '\0';
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
		}
	}

	//タイプ設定の変更があった
	m_Types.m_nRegexKeyMagicNumber++;
//	m_Types.m_nRegexKeyMagicNumber = 0;	//Not Compiled.

	return TRUE;
}

BOOL CPropTypes::RegexKakomiCheck(const char *s)
{
	const char	*p;
	int	length, i;
	static const char *kakomi[7 * 2] = {
		"/",  "/k",
		"m/", "/k",
		"m#", "#k",
		"/",  "/ki",
		"m/", "/ki",
		"m#", "#ki",
		NULL, NULL,
	};

	length = strlen(s);

	for(i = 0; kakomi[i] != NULL; i += 2)
	{
		//文字長を確かめる
		if( length > (int)strlen(kakomi[i]) + (int)strlen(kakomi[i+1]) )
		{
			//始まりを確かめる
			if( strncmp(kakomi[i], s, strlen(kakomi[i])) == 0 )
			{
				//終わりを確かめる
				p = &s[length - strlen(kakomi[i+1])];
				if( strcmp(p, kakomi[i+1]) == 0 )
				{
					//正常
					return TRUE;
				}
			}
		}
	}

	return FALSE;
}

//@@@ 2001.11.17 add end MIK


/*[EOF]*/
