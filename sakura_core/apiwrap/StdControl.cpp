#include "StdAfx.h"
#include "StdControl.h"
#include "util/tchar_receive.h"



namespace ApiWrap{

	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str)
	{
		LRESULT nCount = SendMessage( hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if( nCount == LB_ERR )
			return LB_ERR;
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<ACHAR>(str,nCount+1) );	// +1: NULL •¶Žš•ª
	}

	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str)
	{
		LRESULT nCount = SendMessage( hwndList, LB_GETTEXTLEN, (WPARAM)nIndex, (LPARAM)0);
		if( nCount == LB_ERR )
			return LB_ERR;
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<WCHAR>(str,nCount+1) );	// +1: NULL •¶Žš•ª
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
}
