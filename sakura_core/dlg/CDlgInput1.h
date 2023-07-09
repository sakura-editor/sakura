/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2023, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#define SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_
#pragma once

#include "dlg/CDialog.h"

#include "apiwrap/apiwrap.hpp"

class CDlgInput1;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief １行入力ダイアログボックス
*/
class CDlgInput1 : public CDialog
{
private:
	std::wstring _Title;				/* ダイアログタイトル */
	std::wstring _Message;				/* メッセージ */
	int			 _MaxTextLength = 0;	/* 入力サイズ上限 */
	std::wstring _Text;					/* テキスト */

public:
	CDlgInput1();
	~CDlgInput1() override = default;

	INT_PTR DoModal(
		HINSTANCE hInstApp,
		HWND hwndParent,
		std::wstring_view title,
		std::wstring_view message,
		int nMaxTextLen,
		WCHAR* pszText );

	/*
	||  Attributes & Operations
	*/
protected:
	void    SetDlgData(HWND hWndDlg) const override;
	INT_PTR GetDlgData(HWND hWndDlg) override;

	BOOL    OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam) override;
	BOOL    OnDlgCommand(HWND hDlg, int id, HWND hWndCtl, UINT codeNotify) override;

	INT_PTR GetHelpIds(void) const noexcept override;

	/*
	||  実装ヘルパ関数
	*/
};

#endif /* SAKURA_CDLGINPUT1_43CB765B_D257_4DBC_85E9_D2587B7E9D8E_H_ */
