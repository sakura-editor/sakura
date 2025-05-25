/*!	@file
	@brief 文字色／背景色統一ダイアログ

	@author ryoji
	@date 2006/04/26 作成
*/
/*
	Copyright (C) 2006, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_
#define SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_
#pragma once

#include "dlg/CDialog.h"

struct STypeConfig;

/*!	@brief 文字色／背景色統一ダイアログ

	タイプ別設定のカラー設定で，文字色／背景色統一の対象色を指定するために補助的に
	使用されるダイアログボックス
*/
class CDlgSameColor final : public CDialog
{
public:
	CDlgSameColor();
	~CDlgSameColor();
	int DoModal( HINSTANCE hInstance, HWND hwndParent, WORD wID, STypeConfig* pTypes, COLORREF cr );		//!< モーダルダイアログの表示

protected:

	LPVOID GetHelpIdTable( void ) override;
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;	//! ダイアログのメッセージ処理
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam ) override;			//!< WM_INITDIALOG 処理
	BOOL OnBnClicked( int wID ) override;							//!< BN_CLICKED 処理
	BOOL OnDrawItem( WPARAM wParam, LPARAM lParam ) override;	//!< WM_DRAWITEM 処理
	BOOL OnSelChangeListColors( HWND hwndCtl );					//!< 色選択リストの LBN_SELCHANGE 処理

	static LRESULT CALLBACK ColorStatic_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< サブクラス化された指定色スタティックのウィンドウプロシージャ
	static LRESULT CALLBACK ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< サブクラス化された色選択リストのウィンドウプロシージャ

	WNDPROC m_wpColorStaticProc;	//!< サブクラス化以前の指定色スタティックのウィンドウプロシージャ
	WNDPROC m_wpColorListProc;		//!< サブクラス化以前の色選択リストのウィンドウプロシージャ

	WORD m_wID;			//!< タイプ別設定ダイアログ（親ダイアログ）で押されたボタンID
	STypeConfig* m_pTypes;	//!< タイプ別設定データ
	COLORREF m_cr;		//!< 指定色
};
#endif /* SAKURA_CDLGSAMECOLOR_181C0F46_A420_4A62_A543_FE2B88C20FBE_H_ */
