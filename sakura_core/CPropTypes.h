//	$Id$
/*!	@file
	タイプ別設定ダイアログボックス

	@author Norio Nakatani
	@date 1998/05/08  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

class CPropTypes;

#ifndef _CPROPTYPES_H_
#define _CPROPTYPES_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"
#include "CKeyBind.h"
//#include "_global_fio.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CPropTypes
{
public:
	/*
	||  Constructors
	*/
	CPropTypes();
	~CPropTypes();
	void Create( HINSTANCE, HWND );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int );	/* プロパティシートの作成 */
	BOOL DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );	/* p1 メッセージ処理 */
	BOOL DispatchEvent_p3( HWND, UINT, WPARAM, LPARAM );	/* p3 メッセージ処理 */
	BOOL DispatchEvent_p3_new( HWND, UINT, WPARAM, LPARAM );	/* p3 メッセージ処理 */
	static BOOL SelectColor( HWND , COLORREF* );	/* 色選択ダイアログ */


	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND		m_hwndThis;		/* このダイアログのハンドル */
	int			m_nPageNum;
	/*
	|| ダイアログデータ
	*/
	char			m_szHelpFile[_MAX_PATH + 1];
	int				m_nMaxLineSize_org;					/* 折り返し文字数 */
	Types			m_Types;
	CKeyWordSetMgr	m_CKeyWordSetMgr;
	int				m_nCurrentColorType;		/* 現在選択されている色タイプ */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp( HWND , int );	/* ヘルプ */
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* 色ボタンの描画 */
	void SetData_p1( HWND );	/* ダイアログデータの設定 p1 */
	int  GetData_p1( HWND );	/* ダイアログデータの取得 p1 */
	void SetData_p3( HWND );	/* ダイアログデータの設定 p3 */
	int  GetData_p3( HWND );	/* ダイアログデータの取得 p3 */
	void SetData_p3_new( HWND );	/* ダイアログデータの設定 p3 */
	int  GetData_p3_new( HWND );	/* ダイアログデータの取得 p3 */
	void p3_Import_Colors( HWND );	/* 色の設定をインポート */
	void p3_Export_Colors( HWND );	/* 色の設定をエクスポート */
	void DrawColorListItem( DRAWITEMSTRUCT*);	/* 色種別リスト オーナー描画 */

	//	Sept. 10, 2000 JEPRO 次行を追加
	void EnableTypesPropInput( HWND hwndDlg );	//	タイプ別設定のカラー設定のON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROPTYPES_H_ */

/*[EOF]*/
