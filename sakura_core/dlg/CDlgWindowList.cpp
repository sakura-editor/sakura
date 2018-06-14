/*!	@file
	@brief ウィンドウ一覧ダイアログボックス

	@author Moca
	@date 2015.03.07 Moca CDlgWindowList.cppを元に作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, Stonee, genta, JEPRO, YAZAKI
	Copyright (C) 2002, aroka, MIK, Moca
	Copyright (C) 2003, MIK, genta
	Copyright (C) 2004, MIK, genta, じゅうじ
	Copyright (C) 2006, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2015, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "dlg/CDlgWindowList.h"
#include "Funccode_enum.h"
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_LIST_WINDOW,			HIDC_WINLIST_LIST_WINDOW,
	IDC_BUTTON_SAVE,			HIDC_WINLIST_BUTTTN_SAVE,
	IDC_BUTTON_CLOSE,			HIDC_WINLIST_BUTTTN_CLOSE,
	IDOK,						HIDC_WINLIST_IDOK,
	0, 0
};

static const SAnchorList anchorList[] = {
	{IDC_LIST_WINDOW,			ANCHOR_ALL},
	{IDC_BUTTON_SAVE,			ANCHOR_BOTTOM},
	{IDC_BUTTON_CLOSE,			ANCHOR_BOTTOM},
	{IDOK,                      ANCHOR_BOTTOM},
	{IDC_BUTTON_HELP,           ANCHOR_BOTTOM},
};


CDlgWindowList::CDlgWindowList()
	: CDialog(true)
{
	/* サイズ変更時に位置を制御するコントロール数 */
	assert(_countof(anchorList) == _countof(m_rcItems));
	m_ptDefaultSize.x = -1;
	m_ptDefaultSize.y = -1;
	return;
}

/* モーダルダイアログの表示 */
int CDlgWindowList::DoModal(
	HINSTANCE			hInstance,
	HWND				hwndParent,
	LPARAM				lParam
)
{
	return (int)CDialog::DoModal(hInstance, hwndParent, IDD_WINLIST, lParam);
}

BOOL CDlgWindowList::OnBnClicked(int wID)
{
	switch(wID){
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DLGWINLIST));
		return TRUE;
	case IDC_BUTTON_SAVE:
		CommandSave();
		return TRUE;
	case IDC_BUTTON_CLOSE:
		CommandClose();
		return TRUE;
	case IDOK:
		::EndDialog(GetHwnd(), TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	}
	return CDialog::OnBnClicked(wID);
}


void CDlgWindowList::GetDataListView(std::vector<HWND>& aHwndList)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	aHwndList.clear();
	const int nCount = ListView_GetItemCount(hwndList);
	for (int i = 0; i < nCount; i++) {
		const BOOL bCheck = ListView_GetCheckState(hwndList, i);
		if (bCheck) {
			LV_ITEM lvitem;
			memset_raw(&lvitem, 0, sizeof(lvitem));
			lvitem.mask = LVIF_PARAM;
			lvitem.iItem = i;
			lvitem.iSubItem = 0;
			if (ListView_GetItem(hwndList, &lvitem )) {
				aHwndList.push_back((HWND)lvitem.lParam);
			}
		}
	}
}


void CDlgWindowList::CommandSave()
{
	std::vector<HWND> aHwndList;
	GetDataListView(aHwndList);
	for (int i = 0; i < (int)aHwndList.size(); i++) {
		DWORD dwPid;
		::GetWindowThreadProcessId(aHwndList[i], &dwPid);
		::AllowSetForegroundWindow(dwPid);
		::SendMessage(aHwndList[i], WM_COMMAND, MAKELONG(F_FILESAVE, 0), 0);
	}
	SetData();
}


void CDlgWindowList::CommandClose()
{
	std::vector<HWND> aHwndList;
	GetDataListView(aHwndList);
	for (int i = 0; i < (int)aHwndList.size(); i++) {
		DWORD dwPid;
		::GetWindowThreadProcessId(aHwndList[i], &dwPid);
		::AllowSetForegroundWindow(dwPid);
		::SendMessage(aHwndList[i], MYWM_CLOSE, 0, 0);
	}
	SetData();
}


void CDlgWindowList::SetData()
{
	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	ListView_DeleteAllItems(hwndList);
	EditNode *pEditNode;
	int nRowNum = CAppNodeManager::getInstance()->GetOpenedWindowArr(&pEditNode, TRUE);
	if (0 < nRowNum) {
		CTextWidthCalc calc(hwndList);
		for (int i = 0; i < nRowNum; i++) {
			::SendMessageAny(pEditNode[i].GetHwnd(), MYWM_GETFILEINFO, 0, 0);
			const EditInfo* pEditInfo = &m_pShareData->m_sWorkBuffer.m_EditInfo_MYWM_GETFILEINFO;

			TCHAR szName[_MAX_PATH];
			CFileNameManager::getInstance()->GetMenuFullLabel_WinListNoEscape(szName, _countof(szName), pEditInfo, pEditNode[i].m_nId, i, calc.GetDC());

			LV_ITEM lvi;
			lvi.mask     = LVIF_TEXT | LVIF_PARAM;
			lvi.pszText  = szName;
			lvi.iItem    = i;
			lvi.iSubItem = 0;
			lvi.lParam   = (LPARAM)pEditNode[i].GetHwnd();
			ListView_InsertItem(hwndList, &lvi);
		}

		delete [] pEditNode;
	}
	return;
}


int CDlgWindowList::GetData()
{
	return TRUE;
}


LPVOID CDlgWindowList::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}


INT_PTR CDlgWindowList::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	INT_PTR result;
	result = CDialog::DispatchEvent(hWnd, wMsg, wParam, lParam);

	if (wMsg == WM_GETMINMAXINFO) {
		return OnMinMaxInfo(lParam);
	}
	return result;
}

BOOL CDlgWindowList::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);

	CreateSizeBox();
	CDialog::OnSize();

	RECT rc;
	::GetWindowRect(hwndDlg, &rc);
	m_ptDefaultSize.x = rc.right - rc.left;
	m_ptDefaultSize.y = rc.bottom - rc.top;

	for (int i = 0; i < _countof(anchorList); i++) {
		GetItemClientRect(anchorList[i].id, m_rcItems[i]);
	}

	RECT rcDialog = GetDllShareData().m_Common.m_sOthers.m_rcWindowListDialog;
	if (rcDialog.left != 0 || rcDialog.bottom != 0) {
		m_xPos = rcDialog.left;
		m_yPos = rcDialog.top;
		m_nWidth = rcDialog.right - rcDialog.left;
		m_nHeight = rcDialog.bottom - rcDialog.top;
	}
	SetDialogPosSize();

	HWND hwndList = GetItemHwnd(IDC_LIST_WINDOW);
	RECT rcListView;
	GetItemClientRect(IDC_LIST_WINDOW, rcListView);

	LV_COLUMN	col;
	col.mask     = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	col.fmt      = LVCFMT_LEFT;
	col.cx       = rcListView.right - rcListView.left - ::GetSystemMetrics(SM_CXVSCROLL) - 10;
	TCHAR szNull[] = _T("");
	col.pszText  = szNull;
	col.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &col);
	LONG lngStyle = ListView_GetExtendedListViewStyle(hwndList);
	lngStyle |= LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES;
	ListView_SetExtendedListViewStyle(hwndList, lngStyle);

	::SetForegroundWindow(hwndDlg);
	::BringWindowToTop(hwndDlg);
	return CDialog::OnInitDialog(hwndDlg, wParam, lParam);
}


BOOL CDlgWindowList::OnSize(WPARAM wParam, LPARAM lParam)
{
	CDialog::OnSize(wParam, lParam);

	::GetWindowRect(GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcWindowListDialog);

	RECT  rc;
	POINT ptNew;
	::GetWindowRect(GetHwnd(), &rc);
	ptNew.x = rc.right - rc.left;
	ptNew.y = rc.bottom - rc.top;

	for (int i = 0; i < _countof(anchorList); i++) {
		ResizeItem(GetItemHwnd(anchorList[i].id), m_ptDefaultSize, ptNew, m_rcItems[i], anchorList[i].anchor);
	}
	::InvalidateRect(GetHwnd(), NULL, TRUE);
	return TRUE;
}


BOOL CDlgWindowList::OnMove(WPARAM wParam, LPARAM lParam)
{
	::GetWindowRect(GetHwnd(), &GetDllShareData().m_Common.m_sOthers.m_rcWindowListDialog);
	
	return CDialog::OnMove(wParam, lParam);
}


BOOL CDlgWindowList::OnMinMaxInfo(LPARAM lParam)
{
	LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam;
	if (m_ptDefaultSize.x < 0) {
		return 0;
	}
	lpmmi->ptMinTrackSize.x = m_ptDefaultSize.x;
	lpmmi->ptMinTrackSize.y = m_ptDefaultSize.y;
	lpmmi->ptMaxTrackSize.x = m_ptDefaultSize.x*3;
	lpmmi->ptMaxTrackSize.y = m_ptDefaultSize.y*3;
	return 0;
}


BOOL CDlgWindowList::OnActivate(WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam)) {
	case WA_ACTIVE:
	case WA_CLICKACTIVE:
		SetData();
		return TRUE;
	case WA_INACTIVE:
	default:
		break;
	}
	return CDialog::OnActivate(wParam, lParam);
}

