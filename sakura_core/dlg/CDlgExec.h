/*!	@file
	@brief 外部コマンド実行ダイアログ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGEXEC_4A4BE162_D6C9_4E28_B1AC_091DFFE7DD72_H_
#define SAKURA_CDLGEXEC_4A4BE162_D6C9_4E28_B1AC_091DFFE7DD72_H_
#pragma once

#include "dlg/CDialog.h"
#include "recent/CRecentCmd.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgExec final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgExec();
	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	WCHAR	m_szCommand[1024 + 1];	/* コマンドライン */
	SFilePath	m_szCurDir;	/* カレントディレクトリ */
	bool	m_bEditable;			/* 編集ウィンドウへの入力可能 */	// 2009.02.21 ryoji

protected:
	CRecentCmd m_cRecentCmd;
	CRecentCurDir m_cRecentCur;

	int GetData( void ) override;	/* ダイアログデータの取得 */
	void SetData( void ) override;	/* ダイアログデータの設定 */
	BOOL OnInitDialog(HWND hwnd, WPARAM wParam, LPARAM lParam) override;
	BOOL OnBnClicked(int wID) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add
};
#endif /* SAKURA_CDLGEXEC_4A4BE162_D6C9_4E28_B1AC_091DFFE7DD72_H_ */
