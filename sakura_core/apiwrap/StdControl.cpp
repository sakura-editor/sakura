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

}
