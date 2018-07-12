#include "StdAfx.h"
#include "StdControl.h"
#include "util/tchar_receive.h"



namespace ApiWrap{

	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str)
	{
		LRESULT nCount = SendMessage( hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if( nCount == LB_ERR )
			return LB_ERR;
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<ACHAR>(str,nCount+1) );	// +1: NULL 文字分
	}

	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str)
	{
		LRESULT nCount = SendMessage( hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if( nCount == LB_ERR )
			return LB_ERR;
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<WCHAR>(str,nCount+1) );	// +1: NULL 文字分
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, ACHAR* str, int nMaxCount)
	{
		return GetDlgItemText(hwndDlg, nIDDlgItem, TcharReceiver<ACHAR>(str,nMaxCount), nMaxCount);
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* str, int nMaxCount)
	{
		return GetDlgItemText(hwndDlg, nIDDlgItem, TcharReceiver<WCHAR>(str,nMaxCount), nMaxCount);
	}

	bool TreeView_GetItemTextVector(HWND hwndTree, TVITEM& item, std::vector<TCHAR>& vecStr)
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
