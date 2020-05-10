/*! @file */
/*
	Copyright (C) 2007, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
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

#include <windows.h>
#include <Commctrl.h>
#include "mem/CNativeW.h"
#include <vector>

namespace ApiWrap{

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ共通                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL Wnd_SetText(HWND hwnd, const WCHAR* str)
	{
		return SetWindowText(hwnd, str);
	}

	/*!
		@brief Window テキストを設定する
		@param[in]  hwnd	ウィンドウハンドル
		@param[in]  str		ウィンドウテキスト
	*/
	inline BOOL Wnd_SetText(HWND hwnd, const CNativeW& str)
	{
		return SetWindowText(hwnd, str.GetStringPtr());
	}

	/*!
		@brief Window テキストを取得する
		@param[in]  hwnd	ウィンドウハンドル
		@param[out] str		ウィンドウテキスト
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	inline bool Wnd_GetText(HWND hwnd, CNativeW& str)
	{
		// バッファをクリアしておく
		str.Clear();

		// GetWindowTextLength() はウィンドウテキスト取得に必要なバッファサイズを返す。
		// 条件によっては必要なサイズより大きな値を返すことがある模様
		// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getwindowtextlengtha
		const int length = ::GetWindowTextLength(hwnd);
		if (length < 0)
		{
			// ドキュメントには失敗した場合、あるいはテキストが空の場合には 0 を返すとある。
			// 0 の場合はエラーかどうか判断できないのでテキストの取得処理を続行する。
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}

		// ドキュメントには NULL 文字に関して言及がないので念の為 +1 しておく
		const int bufsize = length + 1;
		
		// ウィンドウタイトルを設定するのに必要なバッファを確保する
		if (str.capacity() < bufsize)
		{
			str.AllocStringBuffer(bufsize);
		}

		int actualCount = ::GetWindowText(hwnd, str.GetStringPtr(), str.capacity());
		if (actualCount < 0)
		{
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}
		else if (actualCount == 0)
		{
			// GetWindowText はエラーの場合、またはテキストが空の場合は 0 を返す
			if (GetLastError() != 0)
			{
				return false;
			}
		}
		else if(actualCount >= str.capacity())
		{
			// GetWindowText() の仕様上はありえないはず
			return false;
		}

		// Win32 API の GetWindowText() を呼んだだけでは CNativeW 内部の
		// データサイズが更新されないのでデータサイズを反映する
		str._SetStringLength(actualCount);

		// 正しく設定されているはず
		assert(str.GetStringLength() == actualCount);
		return true;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      コンボボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int Combo_AddString(HWND hwndCombo, const WCHAR* str)
	{
		// CB_ADDSTRING は失敗の時、負の値を返す。
		// 成功した場合 0 ベースのインデックスを返す。
		// 64bit 対応時に int で十分
		return (int)::SendMessage( hwndCombo, CB_ADDSTRING, 0, LPARAM(str) );
	}

	inline LRESULT Combo_GetLBText(HWND hwndCombo, int nIndex, WCHAR* str)
	{
		return ::SendMessage( hwndCombo, CB_GETLBTEXT, nIndex, LPARAM(str) );
	}

	inline LRESULT Combo_GetText(HWND hwndCombo, WCHAR* str, int cchMax)
	{
		return ::GetWindowText( hwndCombo, str, cchMax );
	}

	inline int Combo_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_DELETESTRING, (WPARAM)index, 0L); }
	inline int Combo_FindStringExact(HWND hwndCtl, int indexStart, const WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_FINDSTRINGEXACT, (WPARAM)indexStart, LPARAM(lpszFind) ); }
	
	inline int Combo_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCOUNT, 0L, 0L); }
	inline int Combo_GetCurSel(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETCURSEL, 0L, 0L); }
	inline int Combo_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT Combo_GetItemData(HWND hwndCtl, int index)			{ return ((LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, CB_GETITEMDATA, (WPARAM)index, 0L)); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int Combo_GetLBTextLen(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_GETLBTEXTLEN, (WPARAM)index, 0L); }
	inline int Combo_InsertString(HWND hwndCtl, int index, const WCHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_INSERTSTRING, (WPARAM)index, LPARAM(lpsz) ); }
	inline int Combo_LimitText(HWND hwndCtl, int cchLimit)				{ return (int)(DWORD)::SendMessage(hwndCtl, CB_LIMITTEXT, (WPARAM)cchLimit, 0L); }
	inline int Combo_ResetContent(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, CB_RESETCONTENT, 0L, 0L); }
	inline int Combo_SetEditSel(HWND hwndCtl, int ichStart, int ichEnd)	{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEDITSEL, 0L, MAKELPARAM(ichStart, ichEnd)); }
	inline int Combo_SetExtendedUI(HWND hwndCtl, UINT flags)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETEXTENDEDUI, (WPARAM)flags, 0L); }
	inline BOOL Combo_ShowDropdown(HWND hwndCtl, BOOL fShow)			{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_SHOWDROPDOWN, (WPARAM)fShow, 0L); }
	inline int Combo_SetDroppedWidth(HWND hwndCtl, int width)			{ return (int)(DWORD)::SendMessage(hwndCtl, CB_SETDROPPEDWIDTH, (WPARAM)width, 0L); }
	inline BOOL Combo_GetDroppedState(HWND hwndCtl)						{ return (BOOL)(DWORD)::SendMessage(hwndCtl, CB_GETDROPPEDSTATE, 0L, 0L ); }

	inline bool Combo_GetLBText(HWND hwndCombo, int nIndex, CNativeW& str)
	{
		// バッファをクリアしておく
		str.Clear();

		// 範囲外は失敗にする
		if (nIndex < 0)
		{
			return false;
		}

		// 文字列長を取得する、取得できなければエラー
		const int length = Combo_GetLBTextLen( hwndCombo, nIndex );
		if ( length == CB_ERR || length < 0)
		{
			return false;
		}

		// 必要なメモリを確保する
		const int bufsize = length + 1;
		str.AllocStringBuffer( bufsize );

		// アイテムテキストを取得する
		const int actualCount = (int)Combo_GetLBText( hwndCombo, nIndex, str.GetStringPtr() );
		if (actualCount == CB_ERR || actualCount < 0)
		{
			return false;
		}
		else if(str.capacity() <= actualCount)
		{
			return false;
		}

		// CNativeW 内部のデータサイズを更新する
		str._SetStringLength(actualCount);

		// 正しく設定されているはず
		assert(str.GetStringLength() == actualCount);
		return true;
	}
	inline void Combo_GetEditSel( HWND hwndCombo, int &nSelStart, int &nSelEnd )
	{
		DWORD dwSelStart = 0;
		DWORD dwSelEnd = 0;
		::SendMessage( hwndCombo, CB_GETEDITSEL, WPARAM( &dwSelStart ), LPARAM( &dwSelEnd ) );
		assert_warning( 0x7FFFFFFF < dwSelStart );
		assert_warning( 0x7FFFFFFF < dwSelEnd );
		nSelStart = static_cast<int>(dwSelStart);
		nSelEnd = static_cast<int>(dwSelEnd);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      リストボックス                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* pszText, size_t cchText);
	template <size_t cchText>
	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR(&pszText)[cchText]) {
		return List_GetText(hwndList, nIndex, pszText, cchText);
	}
	inline LRESULT List_AddString(HWND hwndList, const WCHAR* str)
	{
		return ::SendMessage( hwndList, LB_ADDSTRING, 0, LPARAM(str) );
	}
	inline int List_AddItemData(HWND hwndCtl, int data)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_AddItemData(HWND hwndCtl, void* data)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_ADDSTRING, 0L, (LPARAM)data); }
	inline int List_DeleteString(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_DELETESTRING, (WPARAM)index, 0L); }
	inline int List_FindStringExact(HWND hwndCtl, int indexStart, WCHAR* lpszFind)	{ return (int)(DWORD)::SendMessage( hwndCtl, LB_FINDSTRINGEXACT, WPARAM(indexStart), LPARAM(lpszFind) ); }
	inline int List_GetCaretIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCARETINDEX, 0L, 0L); }
	inline int List_GetCount(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCOUNT, 0L, 0L); }
	inline int List_GetCurSel(HWND hwndCtl)								{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETCURSEL, 0L, 0L); }
	inline int List_GetTextLen(HWND hwndCtl, int nIndex)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTEXTLEN, nIndex, 0L); }
	inline int List_SetCurSel(HWND hwndCtl, int index)					{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETCURSEL, (WPARAM)index, 0L); }
	inline LRESULT List_GetItemData(HWND hwndCtl, int index)			{ return (LRESULT)(ULONG_PTR)::SendMessage(hwndCtl, LB_GETITEMDATA, (WPARAM)index, 0L); }
	inline int List_SetItemData(HWND hwndCtl, int index, int data)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_SetItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMDATA, (WPARAM)index, (LPARAM)data); }
	inline int List_GetItemRect(HWND hwndCtl, int index, RECT* lprc)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMRECT, (WPARAM)index, (LPARAM)lprc); }
	inline int List_GetTopIndex(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETTOPINDEX, 0L, 0L); }
	inline int List_InsertItemData(HWND hwndCtl, int index, int data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertItemData(HWND hwndCtl, int index, void* data)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, (LPARAM)data); }
	inline int List_InsertString(HWND hwndCtl, int index, const WCHAR* lpsz)	{ return (int)(DWORD)::SendMessage(hwndCtl, LB_INSERTSTRING, (WPARAM)index, LPARAM(lpsz)); }
	inline BOOL List_ResetContent(HWND hwndCtl)							{ return (BOOL)(DWORD)::SendMessage(hwndCtl, LB_RESETCONTENT, 0L, 0L); }
	inline void List_SetHorizontalExtent(HWND hwndCtl, int cxExtent)	{ ::SendMessage(hwndCtl, LB_SETHORIZONTALEXTENT, (WPARAM)cxExtent, 0L); }
	inline int List_GetItemHeight(HWND hwndCtl, int index)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_GETITEMHEIGHT, (WPARAM)index, 0L); }
	inline int List_SetItemHeight(HWND hwndCtl, int index, int cy)		{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETITEMHEIGHT, (WPARAM)index, MAKELPARAM(cy, 0)); }
	inline int List_SetTopIndex(HWND hwndCtl, int indexTop)				{ return (int)(DWORD)::SendMessage(hwndCtl, LB_SETTOPINDEX, (WPARAM)indexTop, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      エディット コントロール                //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void EditCtl_LimitText(HWND hwndCtl, int cchLimit)			{ ::SendMessage(hwndCtl, EM_LIMITTEXT, (WPARAM)(cchLimit), 0L); }
	inline void EditCtl_SetSel(HWND hwndCtl, int ichStart, int ichEnd)	{ ::SendMessage(hwndCtl, EM_SETSEL, ichStart, ichEnd); }

	inline void EditCtl_ReplaceSel(HWND hwndCtl, const WCHAR* lpsz)		{ ::SendMessage(hwndCtl, EM_REPLACESEL, 0, LPARAM(lpsz)); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ボタン コントロール                    //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline int BtnCtl_GetCheck(HWND hwndCtl)							{ return (int)(DWORD)::SendMessage(hwndCtl, BM_GETCHECK, 0L, 0L); }
	inline void BtnCtl_SetCheck(HWND hwndCtl, int check)				{ ::SendMessage(hwndCtl, BM_SETCHECK, (WPARAM)check, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      スタティック コントロール              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline HICON StCtl_SetIcon(HWND hwndCtl, HICON hIcon)				{ return (HICON)(UINT_PTR)::SendMessage(hwndCtl, STM_SETICON, (WPARAM)hIcon, 0L); }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      スタティック コントロール              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline void TrackBarCtl_SetRange(HWND hwndCtl, BOOL bRedraw, WORD minimum, WORD maximum)
	{
		::SendMessage(hwndCtl, TBM_SETRANGE, (WPARAM)bRedraw, (LPARAM)MAKELONG(minimum, maximum));
	}
	inline void TrackBarCtl_SetPos(HWND hwndCtl, BOOL bRedraw, LPARAM pos)
	{
		::SendMessage(hwndCtl, TBM_SETPOS, (WPARAM)bRedraw, pos);
	}
	inline LRESULT TrackBarCtl_GetPos(HWND hwndCtl)
	{
		return ::SendMessage(hwndCtl, TBM_GETPOS, (WPARAM)0, (LPARAM)0);
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       ダイアログ内                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	inline BOOL DlgItem_SetText(HWND hwndDlg, int nIDDlgItem, const WCHAR* str)
	{
		return SetDlgItemText(hwndDlg, nIDDlgItem, str);
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* pszText, int nMaxCount);

	bool TreeView_GetItemTextVector(HWND hwndTree, TVITEM& item, std::vector<WCHAR>& vecStr);
	void TreeView_ExpandAll( HWND, bool, int nMaxDepth = 100 );
}
using namespace ApiWrap;
