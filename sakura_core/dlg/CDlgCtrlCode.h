/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002, MIK
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGCTRLCODE_84B2AD70_76BB_4CE3_9B09_4D1AF49BF5EE_H_
#define SAKURA_CDLGCTRLCODE_84B2AD70_76BB_4CE3_9B09_4D1AF49BF5EE_H_
#pragma once

class CDlgCtrlCode;

#include "dlg/CDialog.h"
/*!
	@brief コントロールコード入力ダイアログボックス
*/
//2007.10.18 kobake GetCharCode()を作成。
class CDlgCtrlCode final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCtrlCode();

	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	wchar_t GetCharCode() const{ return m_nCode; } //!< 選択された文字コードを取得

private:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	LPVOID	GetHelpIdTable( void ) override;

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */

private:
	/*
	|| メンバ変数
	*/
	wchar_t		m_nCode;	// コード
};
#endif /* SAKURA_CDLGCTRLCODE_84B2AD70_76BB_4CE3_9B09_4D1AF49BF5EE_H_ */
