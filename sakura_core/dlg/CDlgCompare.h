/*!	@file
	@brief ファイル比較ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CDLGCOMPARE_58A610D8_1E87_4096_96E2_724BEAC310F1_H_
#define SAKURA_CDLGCOMPARE_58A610D8_1E87_4096_96E2_724BEAC310F1_H_
#pragma once

class CDlgCompare;

#include "dlg/CDialog.h"
/*!
	@brief ファイル比較ダイアログボックス
*/
class CDlgCompare final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCompare();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, LPARAM lParam,
				 const WCHAR* pszPath, HWND* phwndCompareWnd );	/* モーダルダイアログの表示 */

	const WCHAR*	m_pszPath;
	HWND*			m_phwndCompareWnd;
	BOOL			m_bCompareAndTileHorz;/* 左右に並べて表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked(int wID) override;
	LPVOID GetHelpIdTable(void) override;	//@@@ 2002.01.18 add

	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;	// 標準以外のメッセージを捕捉する
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnSize( WPARAM wParam, LPARAM lParam ) override;
	BOOL OnMove( WPARAM wParam, LPARAM lParam ) override;
	BOOL OnMinMaxInfo( LPARAM lParam );

	void SetData( void ) override;	/* ダイアログデータの設定 */
	int GetData( void ) override;	/* ダイアログデータの取得 */

private:
	POINT			m_ptDefaultSize;
	RECT			m_rcItems[6];
};
#endif /* SAKURA_CDLGCOMPARE_58A610D8_1E87_4096_96E2_724BEAC310F1_H_ */
