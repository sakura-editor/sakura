/*!	@file
	@brief タグファイル作成ダイアログボックス

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGTAGSMAKE_65A013DF_4315_4254_8C64_4E7489E8E5FC_H_
#define SAKURA_CDLGTAGSMAKE_65A013DF_4315_4254_8C64_4E7489E8E5FC_H_
#pragma once

class CDlgTagsMake;

#include "dlg/CDialog.h"
/*!
	@brief タグファイル作成ダイアログボックス
*/
class CDlgTagsMake final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagsMake();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, const WCHAR* pszPath );	/* モーダルダイアログの表示 */

	WCHAR	m_szPath[_MAX_PATH+1];	/* フォルダー */
	WCHAR	m_szTagsCmdLine[_MAX_PATH];	/* コマンドラインオプション(個別) */
	int		m_nTagsOpt;				/* CTAGSオプション(チェック) */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked(int wID) override;
	LPVOID	GetHelpIdTable(void) override;

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */

private:
	void SelectFolder( HWND hwndDlg );
};
#endif /* SAKURA_CDLGTAGSMAKE_65A013DF_4315_4254_8C64_4E7489E8E5FC_H_ */
