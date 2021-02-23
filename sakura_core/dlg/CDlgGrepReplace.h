/*!	@file
	@brief GREP置換ダイアログボックス

	@author Norio Nakatani
	@date 2011.12.15 CDlgFrep.hから作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, Moca
	Copyright (C) 2014, Moca
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#ifndef SAKURA_CDLGGREPREPLACE_D97F4D2D_9963_40FB_91C1_5A6FF0407E99_H_
#define SAKURA_CDLGGREPREPLACE_D97F4D2D_9963_40FB_91C1_5A6FF0407E99_H_
#pragma once

class CDlgGrep;

#include "dlg/CDialog.h"
#include "dlg/CDlgGrep.h"

//! GREP置換ダイアログボックス
class CDlgGrepReplace final : public CDlgGrep
{
public:
	/*
	||  Constructors
	*/
	CDlgGrepReplace();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, const WCHAR* pszCurrentFilePath, LPARAM lParam );	/* モーダルダイアログの表示 */

	bool		m_bPaste;
	bool		m_bBackup;

	std::wstring	m_strText2;				//!< 置換後
	int				m_nReplaceKeySequence;	//!< 置換後シーケンス

protected:
	CRecentReplace			m_cRecentReplace;
	CFontAutoDeleter		m_cFontText2;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnDestroy() override;
	BOOL OnBnClicked(int wID) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add

	void SetData( void ) override;	/* ダイアログデータの設定 */
	int GetData( void ) override;	/* ダイアログデータの取得 */
};
#endif /* SAKURA_CDLGGREPREPLACE_D97F4D2D_9963_40FB_91C1_5A6FF0407E99_H_ */
