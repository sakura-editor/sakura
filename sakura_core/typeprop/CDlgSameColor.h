/*!	@file
	@brief 文字色／背景色統一ダイアログ

	@author ryoji
	@date 2006/04/26 作成
*/
/*
	Copyright (C) 2006, ryoji

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose, 
	including commercial applications, and to alter it and redistribute it 
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such, 
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef SC_CDLGSAMECOLOR_H__
#define SC_CDLGSAMECOLOR_H__

#include "dlg/CDialog.h"

struct STypeConfig;

/*!	@brief 文字色／背景色統一ダイアログ

	タイプ別設定のカラー設定で，文字色／背景色統一の対象色を指定するために補助的に
	使用されるダイアログボックス
*/
class CDlgSameColor : public CDialog
{
public:
	CDlgSameColor();
	~CDlgSameColor();
	int DoModal( HINSTANCE, HWND, WORD, STypeConfig*, COLORREF );		//!< モーダルダイアログの表示

protected:

	virtual LPVOID GetHelpIdTable( void );
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	//! ダイアログのメッセージ処理
	virtual BOOL OnInitDialog( HWND, WPARAM, LPARAM );			//!< WM_INITDIALOG 処理
	virtual BOOL OnBnClicked( int );							//!< BN_CLICKED 処理
	virtual BOOL OnDrawItem( WPARAM wParam, LPARAM lParam );	//!< WM_DRAWITEM 処理
	BOOL OnSelChangeListColors( HWND hwndCtl );					//!< 色選択リストの LBN_SELCHANGE 処理

	static LRESULT CALLBACK ColorStatic_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< サブクラス化された指定色スタティックのウィンドウプロシージャ
	static LRESULT CALLBACK ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< サブクラス化された色選択リストのウィンドウプロシージャ

	WNDPROC m_wpColorStaticProc;	//!< サブクラス化以前の指定色スタティックのウィンドウプロシージャ
	WNDPROC m_wpColorListProc;		//!< サブクラス化以前の色選択リストのウィンドウプロシージャ

	WORD m_wID;			//!< タイプ別設定ダイアログ（親ダイアログ）で押されたボタンID
	STypeConfig* m_pTypes;	//!< タイプ別設定データ
	COLORREF m_cr;		//!< 指定色
};

#endif


