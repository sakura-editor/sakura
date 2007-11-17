#pragma once

/*
2007.09.17 kobake
内部コードがWCHARなので、検索キーワードなどもWCHARで保持する。
そのため、検索ダイアログのコンボボックスなどに、WCHARを設定する場面が出てくる。
UNICODE版では問題無いが、ANSI版では設定の前にコード変換する必要がある。
呼び出し側で変換しても良いが、頻度が多いので、WCHARを直接受け取るAPIラップ関数を提供する。

また、SendMessageの直接呼び出しは、どうしてもWPARAM,LPARAMへの強制キャストが生じるため、
コンパイラの型チェックが働かず、wchar_t, charの混在するソースコードの中ではバグの温床になりやすい。
そういった意味でも、このファイル内のラップ関数を使うことを推奨する。
*/

#include "../util/tchar_convert.h"

namespace ApiWrap{

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ共通                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Wnd_SetText(HWND hwnd, const ACHAR* str)
	{
		return SetWindowTextA(hwnd, str);
	}
	inline BOOL Wnd_SetText(HWND hwnd, const WCHAR* str)
	{
#ifdef _UNICODE
		return SetWindowTextW(hwnd, str);
#else
		return SetWindowTextA(hwnd, to_achar(str));
#endif
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      コンボボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline LRESULT Combo_AddString(HWND hwndCombo, const ACHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}

	inline LRESULT Combo_AddString(HWND hwndCombo, const WCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}

	inline LRESULT Combo_GetText(HWND hwndCombo, int nIndex, TCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_GETLBTEXT, nIndex, (LPARAM)str );
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      リストボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str);
	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str);

	inline LRESULT List_AddString(HWND hwndList, const ACHAR* str)
	{
		return ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}
	inline LRESULT List_AddString(HWND hwndList, const WCHAR* str)
	{
		return ::SendMessage( hwndList, LB_ADDSTRING, 0, (LPARAM)to_tchar(str) );
	}


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ダイアログ内                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const ACHAR* str)
	{
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const WCHAR* str)
	{
		return SetDlgItemText(hwndDlg, nIDDlgItem, to_tchar(str));
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, ACHAR* str, int nMaxCount);
	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* str, int nMaxCount);
	//GetDlgItemText

}
using namespace ApiWrap;


