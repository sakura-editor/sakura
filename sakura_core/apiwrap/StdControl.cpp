/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "StdControl.h"
#include "StdApi.h"

#include <windowsx.h>

namespace ApiWrap{

	/*!
		@brief Window テキストを取得する
		@param[in]  hWnd	ウィンドウハンドル
		@param[out] strText	ウィンドウテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool Wnd_GetText( HWND hWnd, std::wstring& strText )
	{
		// バッファをクリアしておく
		strText.clear();

		// GetWindowTextLength() はウィンドウテキスト取得に必要なバッファサイズを返す。
		// 条件によっては必要なサイズより大きな値を返すことがある模様
		// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getwindowtextlengthw
		const int cchRequired = ::GetWindowTextLength( hWnd );
		if( cchRequired < 0 ){
			// ドキュメントには失敗した場合、あるいはテキストが空の場合には 0 を返すとある。
			// 0 の場合はエラーかどうか判断できないのでテキストの取得処理を続行する。
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}else if( cchRequired == 0 ){
			// GetWindowTextLength はエラーの場合、またはテキストが空の場合は 0 を返す
			if( GetLastError() != 0 ){
				return false;
			}
			return true;
		}

		// ウィンドウテキストを取得するのに必要なバッファを確保する
		strText.resize( cchRequired + 1 );

		// GetWindowText() はコピーした文字数を返す。
		// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getwindowtextw
		const int actualCopied = ::GetWindowText( hWnd, strText.data(), (int)strText.capacity() );
		if( actualCopied < 0 ){
			// 仕様上は負の場合はありえないが、念の為エラーチェックしておく。
			return false;
		}
		else if( actualCopied == 0 ){
			// GetWindowText はエラーの場合、またはテキストが空の場合は 0 を返す
			if( GetLastError() != 0 ){
				return false;
			}
		}
		else if( (int)strText.capacity() <= actualCopied ){
			// GetWindowText() の仕様上はありえないはず
			return false;
		}

		// データサイズを反映する
		strText.assign( strText.data(), actualCopied );

		return true;
	}

	/*!
		@brief リストアイテムのテキストを取得する
		@param[in]  hList		リストコントロールのウインドウハンドル
		@param[in]  nIndex		リストアイテムのインデックス
		@param[out] strText		アイテムテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool List_GetText( HWND hList, int nIndex, std::wstring& strText )
	{
		// バッファをクリアしておく
		strText.clear();

		const int cchRequired = ListBox_GetTextLen( hList, nIndex );
		if( cchRequired < 0 ){
			// LB_ERR(-1)とその他のエラーは区別しない
			return false;
		}else if( cchRequired == 0 ){
			return true;
		}

		// アイテムテキストを設定するのに必要なバッファを確保する
		strText.resize(cchRequired + 1);

		// ListBox_GetText() はコピーした文字数を返す。
		const int actualCopied = ListBox_GetText( hList, nIndex, strText.data() );
		if( actualCopied < 0 ){
			// LB_ERR(-1)とその他のエラーは区別しない
			return false;
		}
		else if( (int)strText.capacity() <= actualCopied ){
			// ListBox_GetText() の仕様上はありえないはず
			return false;
		}

		// データサイズを反映する
		strText.assign( strText.data(), actualCopied );

		return true;
	}

	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* pszText, size_t cchText)
	{
		LRESULT nCount = SendMessage( hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if( nCount == LB_ERR )
			return LB_ERR;
		if( cchText <= (size_t) nCount )
			return LB_ERRSPACE;
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, LPARAM(pszText) );
	}

	/*!
		@brief ダイアログアイテムのテキストを取得する
		@param[in]  hDlg		ウィンドウハンドル
		@param[in]  nIDDlgItem	ダイアログアイテムのID
		@param[out] strText		アイテムテキストを受け取る変数
		@return		成功した場合 true
		@return		失敗した場合 false
	*/
	bool DlgItem_GetText( HWND hDlg, int nIDDlgItem, std::wstring& strText )
	{
		// バッファをクリアしておく
		strText.clear();

		// アイテムのハンドルを取得する
		HWND hWnd = ::GetDlgItem( hDlg, nIDDlgItem );
		if( hWnd == NULL ){
			return false;
		}

		return Wnd_GetText( hWnd, strText );
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* pszText, int nMaxCount)
	{
		return GetDlgItemText(hwndDlg, nIDDlgItem, pszText, nMaxCount);
	}

	bool TreeView_GetItemTextVector(HWND hwndTree, TVITEM& item, std::vector<WCHAR>& vecStr)
	{
		BOOL ret = FALSE;
		int nBufferSize = 64;
		while( FALSE == ret ){
			nBufferSize *= 2;
			if( 0x10000 < nBufferSize ){
				break;
			}
			vecStr.resize(nBufferSize);
			item.pszText = &vecStr[0];
			item.cchTextMax = (int)vecStr.size();
			ret = TreeView_GetItem(hwndTree, &item);
		}
		return FALSE != ret;
	}

	// TreeView 全開･全閉
	void TreeView_ExpandAll(HWND hwndTree, bool bExpand, int nMaxDepth)
	{
		HTREEITEM	htiCur;
		HTREEITEM	htiItem;
		HTREEITEM	htiNext;

		::SendMessageAny(hwndTree, WM_SETREDRAW, (WPARAM)FALSE, 0);

		htiCur = htiItem = TreeView_GetSelection( hwndTree );
		if (!bExpand && htiCur != NULL) {
			// 閉じる時はトップに変更
			for (htiNext = htiCur; htiNext !=  NULL; ) {
				htiItem = htiNext;
				htiNext = TreeView_GetParent( hwndTree, htiItem );
			}
			if (htiCur != htiItem) {
				htiCur = htiItem;
				TreeView_SelectItem( hwndTree, htiCur );
			}
		}

		std::vector<HTREEITEM> tree;
		HTREEITEM item = TreeView_GetRoot(hwndTree);
		while( 0 < tree.size() || item != NULL ){
			while(item != NULL && (int)tree.size() < nMaxDepth ){
				// 先に展開してからGetChildしないと、ファイルツリーのサブアイテムが展開されない
				TreeView_Expand(hwndTree, item, bExpand ? TVE_EXPAND : TVE_COLLAPSE);
				tree.push_back(item);
				item = TreeView_GetChild(hwndTree, item);
			}
			item = tree.back();
			tree.pop_back();
			item = TreeView_GetNextSibling(hwndTree, item);
		}

		// 選択位置を戻す
		if (htiCur == NULL) {
			if (bExpand ) {
				htiItem = TreeView_GetRoot( hwndTree );
				TreeView_SelectSetFirstVisible( hwndTree, htiItem );
			}
			TreeView_SelectItem( hwndTree, NULL );
		}
		else {
			TreeView_SelectSetFirstVisible( hwndTree, htiCur );
		}

		::SendMessageAny(hwndTree, WM_SETREDRAW, (WPARAM)TRUE, 0);
	}
}
