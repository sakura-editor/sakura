/*!	@file
	@brief DIFF差分表示ダイアログボックス

	@author MIK
	@date 2002.5.27
*/
/*
	Copyright (C) 2002, MIK
	Copyright (C) 2004, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGDIFF_57739DBD_0797_4F83_9DB0_F4D51A54157F_H_
#define SAKURA_CDLGDIFF_57739DBD_0797_4F83_9DB0_F4D51A54157F_H_
#pragma once

#include "dlg/CDialog.h"
#include "basis/CMyString.h"
#include "charset/charset.h"

class CDlgDiff;

/*!
	@brief DIFF差分表示ダイアログボックス
*/
//	Feb. 28, 2004 genta 最後に選択されていた番号を保存する
class CDlgDiff final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgDiff();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, const WCHAR* pszPath );	/* モーダルダイアログの表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnLbnSelChange( HWND hwndCtl, int wID ) override;
	BOOL	OnLbnDblclk( int wID ) override;
	BOOL	OnEnChange( HWND hwndCtl, int wID ) override;
	LPVOID	GetHelpIdTable() override;
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;	// 標準以外のメッセージを捕捉する
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnDestroy( ) override;
	BOOL OnSize( WPARAM wParam, LPARAM lParam ) override;
	BOOL OnMinMaxInfo( LPARAM lParam );

	void	SetData( ) override;	/* ダイアログデータの設定 */
	int		GetData( ) override;	/* ダイアログデータの取得 */

private:
	int			m_nIndexSave = 0;		// 最後に選択されていた番号
	POINT		m_ptDefaultSize = {-1, -1};
	RECT		m_rcItems[22];

public:
	SFilePath	m_szFile1;			// 自ファイル
	SFilePath	m_szFile2;			// 相手ファイル
	bool		m_bIsModifiedDst = false;	// 相手ファイル更新中
	ECodeType	m_nCodeTypeDst = CODE_ERROR;		// 相手ファイルの文字コード
	bool		m_bBomDst = false;	// 相手ファイルのBOM
	int			m_nDiffFlgOpt = 0;	// DIFFオプション
	HWND		m_hWnd_Dst = nullptr;		// 相手ウインドウハンドル
};
#endif /* SAKURA_CDLGDIFF_57739DBD_0797_4F83_9DB0_F4D51A54157F_H_ */
