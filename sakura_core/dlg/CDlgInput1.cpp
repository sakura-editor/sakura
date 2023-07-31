/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, MIK
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2023, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "dlg/CDlgInput1.h"

#include "sakura_rc.h"
#include "sakura.hh"

// 入力 CDlgInput1.cpp	//@@@ 2002.01.07 add start MIK
static const DWORD p_helpids[] = {	//13000
	IDOK,					HIDOK_DLG1,
	IDCANCEL,				HIDCANCEL_DLG1,
	IDC_EDIT_INPUT1,		HIDC_DLG1_EDIT1,	//入力フィールド	IDC_EDIT1->IDC_EDIT_INPUT1	2008/7/3 Uchi
	IDC_STATIC_MSG,			HIDC_DLG1_EDIT1,	//メッセージ
//	IDC_STATIC,				-1,
	0, 0
};	//@@@ 2002.01.07 add end MIK

using namespace apiwrap;

/*!
 * コンストラクター
 */
CDlgInput1::CDlgInput1()
	: CDialog(IDD_INPUT1)
{
}

/* モードレスダイアログの表示 */
INT_PTR CDlgInput1::DoModal(
	HINSTANCE		  hInstance,
	HWND			  hwndParent,
	std::wstring_view title,
	std::wstring_view message,
	int				  nMaxTextLen,
	WCHAR*			  pszText
)
{
	_Title         = title;			/* ダイアログタイトル */
	_Message       = message;		/* メッセージ */
	_MaxTextLength = nMaxTextLen;	/* 入力サイズ上限 */
	_Text          = pszText;

	if (_Text.length() > _MaxTextLength)
	{
		_Text.resize(_MaxTextLength);
	}

	const auto bRet = CDialog::DoModal(hInstance, hwndParent, IDD_INPUT1, 0);

	wcscpy_s(pszText, _MaxTextLength + 1, _Text.data());

	return bRet;
}

/*!
 * ダイアログにデータを反映する
 */
void CDlgInput1::SetDlgData(HWND hDlg) const
{
	SetDlgItemTextW(hDlg, IDC_EDIT_INPUT1, _Text);
}

/*!
 * ダイアログからデータを取り込む
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @retval >  0 取り込み正常
 * @retval == 0 取り込みデータなし
 * @retval <  0 取り込み異常
 */
INT_PTR CDlgInput1::GetDlgData(HWND hDlg)
{
	_Text = GetDlgItemTextW(hDlg, IDC_EDIT_INPUT1, std::move(_Text));

	return TRUE;
}

/*!
 * WM_INITDIALOGハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE フォーカスが設定されます。
 * @retval FALSE フォーカスは設定されません。
 */
BOOL CDlgInput1::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	const auto ret = CDialog::OnDlgInitDialog(hDlg, hWndFocus, lParam);

	SetWindowTextW(hDlg, _Title);

	SendEmLimitTextW(hDlg, IDC_EDIT_INPUT1, _MaxTextLength);

	SetDlgItemTextW(hDlg, IDC_STATIC_MSG, _Text);

	return ret;
}

/*!
 * WM_COMMANDハンドラ。
 *
 * @retval TRUE メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CDlgInput1::OnDlgCommand(HWND hDlg, int id, HWND hWndCtl, UINT codeNotify)
{
	// OKボタンを押下されたとき、データを取り込む
	if (id == IDOK)
	{
		GetDlgData(hDlg);
	}

	return __super::OnDlgCommand(hDlg, id, hWndCtl, codeNotify);
}

/*!
 * ヘルプIDテーブルを取得する
 */
INT_PTR CDlgInput1::GetHelpIds(void) const noexcept
{
	return (INT_PTR)p_helpids;
}
