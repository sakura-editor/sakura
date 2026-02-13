/*!	@file
	@brief 文字コードセット設定ダイアログボックス

	@author Uchi
	@date 2010/6/14  新規作成
*/
/*
	Copyright (C) 2010, Uchi
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDLGSETCHARSET_82E8A81C_64D0_45DE_BECC_4721CCC93FEB_H_
#define SAKURA_CDLGSETCHARSET_82E8A81C_64D0_45DE_BECC_4721CCC93FEB_H_
#pragma once

#include "dlg/CDialog.h"

enum ECodeType : int;

//! 文字コードセット設定ダイアログボックス
class CDlgSetCharSet final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgSetCharSet();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, ECodeType* pnCharSet, bool* pbBom );	/* モーダルダイアログの表示 */

	ECodeType*	m_pnCharSet = nullptr;			// 文字コードセット
	bool*		m_pbBom = nullptr;				// BOM

	HWND		m_hwndCharSet;
	HWND		m_hwndCheckBOM;

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnCbnSelChange(HWND hwndCtl, int wID) override;
	LPVOID	GetHelpIdTable( ) override;

	void	SetData( ) override;	/* ダイアログデータの設定 */
	int 	GetData( ) override;	/* ダイアログデータの取得 */

	void	SetBOM( );		// BOM の設定
};
#endif /* SAKURA_CDLGSETCHARSET_82E8A81C_64D0_45DE_BECC_4721CCC93FEB_H_ */
