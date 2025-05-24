/*!	@file
	@brief ウィンドウの位置と大きさダイアログ

	@author Moca
	@date 2004/05/13 作成
*/
/*
	Copyright (C) 2004, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGWINSIZE_864AE2FE_C5EB_4CF8_9812_197F16568FFE_H_
#define SAKURA_CDLGWINSIZE_864AE2FE_C5EB_4CF8_9812_197F16568FFE_H_
#pragma once

#include "dlg/CDialog.h"
#include "env/CommonSetting.h"

/*!	@brief 位置と大きさの設定ダイアログ

	共通設定のウィンドウ設定で，ウィンドウ位置を指定するために補助的に
	使用されるダイアログボックス
*/
class CDlgWinSize final : public CDialog
{
public:
	CDlgWinSize();
	~CDlgWinSize();
	int DoModal( HINSTANCE hInstance, HWND hwndParent, EWinSizeMode& eSaveWinSize,
				 EWinSizeMode& eSaveWinPos, int& nWinSizeType, RECT& rc );	//!< モーダルダイアログの表示

protected:

	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnBnClicked(int wID) override;
	BOOL OnEnSetFocus(HWND hwndCtl, int wID) override;
	BOOL OnEnKillFocus(HWND hwndCtl, int wID) override;
	int  GetData( void ) override;
	void SetData( void ) override;
	LPVOID GetHelpIdTable( void ) override;

	void RenewItemState( void );

private:
	EWinSizeMode	m_eSaveWinSize;	//!< ウィンドウサイズの保存: 0/デフォルト，1/継承，2/指定
	EWinSizeMode	m_eSaveWinPos;	//!< ウィンドウ位置の保存: 0/デフォルト，1/継承，2/指定
	int				m_nWinSizeType;	//!< ウィンドウ表示方法: 0/標準，1/最大化，2/最小化
	RECT			m_rc;
};
#endif /* SAKURA_CDLGWINSIZE_864AE2FE_C5EB_4CF8_9812_197F16568FFE_H_ */
