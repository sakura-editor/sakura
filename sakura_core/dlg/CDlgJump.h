/*!	@file
	@brief 指定行へのジャンプダイアログボックス

	@author Norio Nakatani
	@date 1998/05/31 作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

class CDlgJump;

#include "dlg/CDialog.h"
//! 指定行へのジャンプダイアログボックス
class CDlgJump final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	int			m_nLineNum;		/*!< 行番号 */
	BOOL		m_bPLSQL;		/*!< PL/SQLソースの有効行か */
	int			m_nPLSQL_E1;
	int			m_nPLSQL_E2;
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnNotify(NMHDR* pNMHDR) override;	//	Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange(HWND hwndCtl, int wID) override;
	BOOL OnBnClicked(int wID) override;
	BOOL OnEnSetFocus(HWND hwndCtl, int wID) override;
	BOOL OnEnKillFocus(HWND hwndCtl, int wID) override;

	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add
	void SetData( void ) override;	/* ダイアログデータの設定 */
	int GetData( void ) override;	/* ダイアログデータの取得 */
};
