/*!	@file
	タイプ別設定正規表現キーワードダイアログボックス

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

#include "stdafx.h"
#include "sakura_rc.h"
#include "CPropTypes.h"
#include "debug.h"
#include <windows.h>
#include <commctrl.h>
#include "CDlgOpenFile.h"
#include "global.h"
#include "CProfile.h"
#include "CShareData.h"
#include "funccode.h"	//Stonee, 2001/05/18
#include <stdio.h>	//@@@ 2001.11.17 add MIK
#include "CRegexKeyword.h"	//@@@ 2001.11.17 add MIK
#include "io/CTextStream.h"
#include "util/shell.h"
#include "util/file.h"
using namespace std;


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
	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1]=_T("");
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.rkw"),					// [R]egex [K]ey[W]ord
		m_pShareData->m_szIMPORTFOLDER	// インポート用フォルダ
	);
	if( !cDlgOpenFile.DoModal_GetOpenFileName( szPath ) ){
		return FALSE;
	}

	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );


	CTextInputStream in(szPath);
	if(!in){
	/*
	FILE		*fp;
	if( (fp = _tfopen(szPath, _T("r"))) == NULL )
	{
	*/
		ErrorMessage( hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
		return FALSE;
	}

	RegexKeywordInfo	pRegexKey[MAX_REGEX_KEYWORD];
	TCHAR				buff[1024];
	int					i, j, k;
	j = 0;
	while(in)
	{
		//1行読み込み
		wstring line=in.ReadLineW();
		_wcstotcs(buff,line.c_str(),_countof(buff));

		if(j >= MAX_REGEX_KEYWORD) break;

		//RxKey[999]=ColorName,RegexKeyword
		if( auto_strlen(buff) < 12 ) continue;
		if( auto_memcmp(buff, _T("RxKey["), 6) != 0 ) continue;
		if( auto_memcmp(&buff[9], _T("]="), 2) != 0 ) continue;
		TCHAR	*p;
		p = auto_strstr(&buff[11], _T(","));
		if( p )
		{
			*p = _T('\0');
			p++;
			if( p[0] && RegexKakomiCheck(to_wchar(p)) )	//囲みがある
			{
				//色指定名に対応する番号を探す
				k = GetColorIndexByName( &buff[11] );	//@@@ 2002.04.30
				if( k != -1 )	/* 3文字カラー名からインデックス番号に変換 */
				{
					pRegexKey[j].m_nColorIndex = k;
					_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
					j++;
				}
				else
				{	/* 日本語名からインデックス番号に変換する */
					for(k = 0; k < COLORIDX_LAST; k++)
					{
						if( auto_strcmp(m_Types.m_ColorInfoArr[k].m_szName, &buff[11]) == 0 )
						{
							pRegexKey[j].m_nColorIndex = k;
							_tcstowcs(pRegexKey[j].m_szKeyword, p, _countof(pRegexKey[j].m_szKeyword));
							j++;
							break;
						}
					}
				}
			}
		}
	}

	in.Close();

	HWND	hwndWork;
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* リストを空にする */
	for(i = 0; i < j; i++)
	{
		LV_ITEM	lvi;
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(pRegexKey[i].m_szKeyword));
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

	
	/* ファイルオープンダイアログの初期化 */
	CDlgOpenFile	cDlgOpenFile;
	TCHAR			szPath[_MAX_PATH + 1]=_T("");
	cDlgOpenFile.Create(
		m_hInstance,
		hwndDlg,
		_T("*.rkw"),					// [R]egex [K]ey[W]ord
		m_pShareData->m_szIMPORTFOLDER	// インポート用フォルダ
	);
	if( !cDlgOpenFile.DoModal_GetSaveFileName( szPath ) ){
		return FALSE;
	}
	/* ファイルのフルパスを、フォルダとファイル名に分割 */
	/* [c:\work\test\aaa.txt] → [c:\work\test] + [aaa.txt] */
	::SplitPath_FolderAndFile( szPath, m_pShareData->m_szIMPORTFOLDER, NULL );
	_tcscat( m_pShareData->m_szIMPORTFOLDER, _T("\\") );

	CTextOutputStream out(szPath);
	if(!out){
		ErrorMessage( hwndDlg, _T("ファイルを開けませんでした。\n\n%ts"), szPath );
		return FALSE;
	}

	out.WriteF(L"// 正規表現キーワード Ver1\n");

	HWND	hwndList;
	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	int j = ListView_GetItemCount(hwndList);
	for(int i = 0; i < j; i++)
	{
		TCHAR	szKeyWord[256];
		auto_memset(szKeyWord, 0, _countof(szKeyWord));
		ListView_GetItemText(hwndList, i, 0, szKeyWord, _countof(szKeyWord));

		TCHAR	szColorIndex[256];
		auto_memset(szColorIndex, 0, _countof(szColorIndex));
		ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));

		const TCHAR* p = szColorIndex;
		for(int k = 0; k < COLORIDX_LAST; k++)
		{
			if( _tcscmp( m_Types.m_ColorInfoArr[k].m_szName, szColorIndex ) == 0 )
			{
				p = GetColorNameByIndex(k);
				break;
			}
		}
		out.WriteF( L"RxKey[%03d]=%ts,%ts\n", i, p, szKeyWord);
	}

	out.Close();

	InfoMessage( hwndDlg, _T("ファイルへエクスポートしました。\n\n%ls"), szPath );

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
	int	nIndex, nIndex2, i, j, nRet;
	LV_ITEM	lvi;
	LV_COLUMN	col;
	RECT		rc;
	static int nPrevIndex = -1;	//更新時におかしくなるバグ修正 @@@ 2003.03.26 MIK


	hwndList = GetDlgItem( hwndDlg, IDC_LIST_REGEX );

	TCHAR	szKeyWord[256];
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
		col.pszText  = _T("キーワード");
		col.iSubItem = 0;
		ListView_InsertColumn( hwndList, 0, &col );
		col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		col.fmt      = LVCFMT_LEFT;
		col.cx       = (rc.right - rc.left) * 38 / 100;
		col.pszText  = _T("色指定");
		col.iSubItem = 1;
		ListView_InsertColumn( hwndList, 1, &col );

		nPrevIndex = -1;	//@@@ 2003.05.12 MIK
		SetData_Regex( hwndDlg );	/* ダイアログデータの設定 正規表現キーワード */
		if( CheckRegexpVersion( hwndDlg, IDC_LABEL_REGEX_VERSION, false ) == false )	//@@@ 2001.11.17 add MIK
		{
			::DlgItem_SetText( hwndDlg, IDC_LABEL_REGEX_VERSION, _T("正規表現キーワードは使えません。") );
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
								_T("正規表現ライブラリが見つかりません。\n\n正規表現キーワードは機能しませんが、それでも有効にしますか？"),
								_T("正規表現キーワードを使用する") );
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
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				//同じキーがないか調べる。
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, _T("これ以上登録できません。"));
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					auto_memset(szColorIndex, 0, _countof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
					if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
					{
						ErrorMessage( hwndDlg, _T("同じキーワードで登録済みです。"));
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
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//囲みをチェックする。
				{
					TopErrorMessage(
							hwndDlg,
							_T("正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。"),
							_T("正規表現キーワード") );
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？"),
							_T("正規表現キーワード") );
					if( nRet != IDYES ) return FALSE;
				}
				//挿入するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
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
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				if( nIndex2 >= MAX_REGEX_KEYWORD )
				{
					ErrorMessage( hwndDlg, _T("これ以上登録できません。"));
					return FALSE;
				}
				for(i = 0; i < nIndex2; i++)
				{
					auto_memset(szColorIndex, 0, _countof(szColorIndex));
					ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
					if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
					{
						ErrorMessage( hwndDlg, _T("同じキーワードで登録済みです。"));
						return FALSE;
					}
				}
				//書式をチェックする。
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//囲みをチェックする。
				{
					TopErrorMessage(
						hwndDlg,
						_T("正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。"),
						_T("正規表現キーワード")
					);
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？"),
							_T("正規表現キーワード") );
					if( nRet != IDYES ) return FALSE;
				}
				//追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
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
					ErrorMessage( hwndDlg, _T("キーワードが選択されていません。"));
					return FALSE;
				}
				//更新するキー情報を取得する。
				auto_memset(szKeyWord, 0, _countof(szKeyWord));
				::DlgItem_GetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord, _countof(szKeyWord) );
				if( szKeyWord[0] == L'\0' ) return FALSE;
				nIndex2 = ListView_GetItemCount(hwndList);
				for(i = 0; i < nIndex2; i++)
				{
					if( i != nIndex )
					{
						auto_memset(szColorIndex, 0, _countof(szColorIndex));
						ListView_GetItemText(hwndList, i, 0, szColorIndex, _countof(szColorIndex));
						if( _tcscmp(szKeyWord, szColorIndex) == 0 ) 
						{
							ErrorMessage( hwndDlg, _T("同じキーワードで登録済みです。"));
							return FALSE;
						}
					}
				}
				//書式をチェックする。
				if( !RegexKakomiCheck(to_wchar(szKeyWord)) )	//囲みをチェックする。
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_OK | MB_ICONSTOP | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("正規表現キーワードを / と /k で囲ってください。\nキーワードに / がある場合は m# と #k で囲ってください。"),
							_T("正規表現キーワード") );
					return FALSE;
				}
				if( !CheckRegexpSyntax( to_wchar(szKeyWord), hwndDlg, false ) )
				{
					nRet = ::MYMESSAGEBOX(
							hwndDlg,
							MB_YESNO | MB_ICONQUESTION | MB_TOPMOST | MB_DEFBUTTON2,
							GSTR_APPNAME,
							_T("書式が正しくないか、正規表現ライブラリが見つかりません。\n\n登録しますか？"),
							_T("正規表現キーワード") );
					if( nRet != IDYES ) return FALSE;
				}
				//追加するキー情報を取得する。
				auto_memset(szColorIndex, 0, _countof(szColorIndex));
				::DlgItem_GetText( hwndDlg, IDC_COMBO_REGEX_COLOR, szColorIndex, _countof(szColorIndex) );
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
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
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
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
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
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
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
				ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
				ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
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
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* 正規表現 */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for( i = 0, j = 0; i < COLORIDX_LAST; i++ )
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
						{
							if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
							{
								::SendMessageAny( hwndCombo, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* コンボボックスのデフォルト選択 */
								break;
							}
							j++;
						}
					}
					return FALSE;
				}
				if( nPrevIndex != nIndex )	//@@@ 2003.03.26 MIK
				{	//更新時にListViewのSubItemを正しく取得できないので、その対策
					ListView_GetItemText(hwndList, nIndex, 0, szKeyWord, _countof(szKeyWord));
					ListView_GetItemText(hwndList, nIndex, 1, szColorIndex, _countof(szColorIndex));
					::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, szKeyWord );	/* 正規表現 */
					hwndCombo = GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
					for(i = 0, j = 0; i < COLORIDX_LAST; i++)
					{
						if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
						{
							if(_tcscmp(m_Types.m_ColorInfoArr[i].m_szName, szColorIndex) == 0)
							{
								::SendMessageAny(hwndCombo, CB_SETCURSEL, j, 0);
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
			MyWinHelp( (HWND)p->hItemHandle, m_szHelpFile, HELP_WM_HELP, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		}
		return TRUE;
		/*NOTREACHED*/
		//break;

	//Context Menu
	case WM_CONTEXTMENU:
		MyWinHelp( hwndDlg, m_szHelpFile, HELP_CONTEXTMENU, (ULONG_PTR)(LPVOID)p_helpids );	// 2006.10.10 ryoji MyWinHelpに変更に変更
		return TRUE;

	}
	return FALSE;
}

/* ダイアログデータの設定 正規表現キーワード */
void CPropTypes::SetData_Regex( HWND hwndDlg )
{
	HWND		hwndWork;
	int			i, j;
	LV_ITEM		lvi;
	DWORD		dwStyle;

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
	::SendMessage( ::GetDlgItem( hwndDlg, IDC_EDIT_REGEX ), EM_LIMITTEXT, _countof( m_Types.m_RegexKeywordArr[0].m_szKeyword ) - 1, (LPARAM)0 );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_REGEX, _T("//k") );	/* 正規表現 */

	/* 色種類のリスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_REGEX_COLOR );
	::SendMessageAny( hwndWork, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0 );  /* コンボボックスを空にする */
	for( i = 0; i < COLORIDX_LAST; i++ )
	{
		if ( 0 == (g_ColorAttributeArr[i].fAttribute & COLOR_ATTRIB_NO_BACK) )	// 2006.12.18 ryoji フラグ利用で簡素化
		{
			j = ::SendMessage( hwndWork, CB_ADDSTRING, (WPARAM)0, (LPARAM)m_Types.m_ColorInfoArr[i].m_szName );
			if( m_Types.m_ColorInfoArr[i].m_nColorIdx == COLORIDX_REGEX1 )
				::SendMessageAny( hwndWork, CB_SETCURSEL, (WPARAM)j, (LPARAM)0 );	/* コンボボックスのデフォルト選択 */
		}
	}

	if( m_Types.m_bUseRegexKeyword )
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_CHECKED );
	else
		CheckDlgButton( hwndDlg, IDC_CHECK_REGEX, BST_UNCHECKED );

	/* リスト */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_LIST_REGEX );
	ListView_DeleteAllItems(hwndWork);  /* リストを空にする */

	/* 行選択 */
	dwStyle = (DWORD)::SendMessageAny( hwndWork, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	dwStyle |= LVS_EX_FULLROWSELECT;
	::SendMessageAny( hwndWork, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle );

	/* データ表示 */
	for(i = 0; i < MAX_REGEX_KEYWORD; i++)
	{
		if( m_Types.m_RegexKeywordArr[i].m_szKeyword[0] == L'\0' ) break;
		
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = const_cast<TCHAR*>(to_tchar(m_Types.m_RegexKeywordArr[i].m_szKeyword));
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
	TCHAR	szKeyWord[256], szColorIndex[256];

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
			szKeyWord[0]    = _T('\0');
			szColorIndex[0] = _T('\0');
			ListView_GetItemText(hwndList, i, 0, szKeyWord,    _countof(szKeyWord)   );
			ListView_GetItemText(hwndList, i, 1, szColorIndex, _countof(szColorIndex));
			_tcstowcs(m_Types.m_RegexKeywordArr[i].m_szKeyword, szKeyWord, _countof(m_Types.m_RegexKeywordArr[i].m_szKeyword));
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
		}
		else	//未登録部分はクリアする
		{
			m_Types.m_RegexKeywordArr[i].m_szKeyword[0] = L'\0';
			m_Types.m_RegexKeywordArr[i].m_nColorIndex = COLORIDX_REGEX1;
		}
	}

	//タイプ設定の変更があった
	m_Types.m_nRegexKeyMagicNumber++;
//	m_Types.m_nRegexKeyMagicNumber = 0;	//Not Compiled.

	return TRUE;
}

BOOL CPropTypes::RegexKakomiCheck(const wchar_t *s)
{
	const wchar_t	*p;
	int	length, i;
	static const wchar_t *kakomi[7 * 2] = {
		L"/",  L"/k",
		L"m/", L"/k",
		L"m#", L"#k",
		L"/",  L"/ki",
		L"m/", L"/ki",
		L"m#", L"#ki",
		NULL, NULL,
	};

	length = wcslen(s);

	for(i = 0; kakomi[i] != NULL; i += 2)
	{
		//文字長を確かめる
		if( length > (int)wcslen(kakomi[i]) + (int)wcslen(kakomi[i+1]) )
		{
			//始まりを確かめる
			if( wcsncmp(kakomi[i], s, wcslen(kakomi[i])) == 0 )
			{
				//終わりを確かめる
				p = &s[length - wcslen(kakomi[i+1])];
				if( wcscmp(p, kakomi[i+1]) == 0 )
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
