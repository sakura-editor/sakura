#include "stdafx.h"
#include "StdControl.h"
#include "util/tchar_receive.h"



namespace ApiWrap{

	//※バッファサイズを256と仮定

	LRESULT List_GetText(HWND hwndList, int nIndex, ACHAR* str)
	{
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<ACHAR>(str,256) );
	}

	LRESULT List_GetText(HWND hwndList, int nIndex, WCHAR* str)
	{
		return SendMessage( hwndList, LB_GETTEXT, (WPARAM)nIndex, (LPARAM)(TCHAR*)TcharReceiver<WCHAR>(str,256) );
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, ACHAR* str, int nMaxCount)
	{
		return GetDlgItemText(hwndDlg, nIDDlgItem, TcharReceiver<ACHAR>(str,256), nMaxCount);
	}

	UINT DlgItem_GetText(HWND hwndDlg, int nIDDlgItem, WCHAR* str, int nMaxCount)
	{
		return GetDlgItemText(hwndDlg, nIDDlgItem, TcharReceiver<WCHAR>(str,256), nMaxCount);
	}

}
