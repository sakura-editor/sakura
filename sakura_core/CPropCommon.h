//	$Id$
/*!	@file
	共通設定ダイアログボックスの処理

	@author	Norio Nakatani
	@date 1998/12/24 新規作成
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

class CPropCommon;

#ifndef _CPROP1_H_
#define _CPROP1_H_

#include <windows.h>
#include "CShareData.h"
#include "CMemory.h"
#include "CKeyBind.h"
#include "CKeyWordSetMgr.h"
#include "CImageListMgr.h"
#include "sakura_rc.h"

#define ID_PAGENUM_ZENPAN	0	//Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更
#define ID_PAGENUM_WIN		1	//Oct. 25, 2000 JEPRO  5→ 1 に変更
#define ID_PAGENUM_EDIT		2
#define ID_PAGENUM_FILE		3
#define ID_PAGENUM_BACKUP	4
#define ID_PAGENUM_FORMAT	5	//Oct. 25, 2000 JEPRO  1→ 5 に変更
#define ID_PAGENUM_URL		6
#define ID_PAGENUM_GREP		7
#define ID_PAGENUM_KEYBOARD	8
#define ID_PAGENUM_CUSTMENU	9	//Oct. 25, 2000 JEPRO 11→ 9 に変更
#define ID_PAGENUM_TOOLBAR	10	//Oct. 25, 2000 JEPRO  9→10 に変更
#define ID_PAGENUM_KEYWORD	11	//Oct. 25, 2000 JEPRO 10→11 に変更
#define ID_PAGENUM_HELPER	12
#define ID_PAGENUM_MACRO	13	//Oct. 25, 2000 JEPRO 10→11 に変更


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 共通設定ダイアログボックスクラス

	1つのダイアログボックスに複数のプロパティページが入った構造に
	なっており、Dialog procedureとEvent Dispatcherがページごとにある．
*/
class SAKURA_CORE_API CPropCommon
{
public:
	/*
	||  Constructors
	*/
	CPropCommon();
	~CPropCommon();
	void Create( HINSTANCE, HWND, CImageListMgr* );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int/*, int*/ );	/* プロパティシートの作成 */

	//	Jun. 2, 2001 genta
	//	ここにあったEvent Handlerはprotectedエリアに移動した．

	HINSTANCE		m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hwndThis;		/* このダイアログのハンドル */
	int				m_nPageNum;
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;
//	int				m_nActiveItem;

	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List

	/*
	|| ダイアログデータ
	*/
	char			m_szHelpFile[_MAX_PATH + 1];
	Common			m_Common;
	short			m_nKeyNameArrNum;				/* キー割り当て表の有効データ数 */
	KEYDATA			m_pKeyNameArr[100];				/* キー割り当て表 */

	CKeyWordSetMgr	m_CKeyWordSetMgr;
	Types			m_Types[MAX_TYPES];
protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp( HWND, int );	/* ヘルプ */
	int	SearchIntArr( int , int* , int );
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* 色ボタンの描画 */
	BOOL SelectColor( HWND , COLORREF* );	/* 色選択ダイアログ */

	//	Jun. 2, 2001 genta
	//	Event Handler, Dialog Procedureの見直し
	//	Global関数だったDialog procedureをclassのstatic methodとして
	//	組み込んだ．
	//	ここから以下 Macroまで配置の見直しとstatic methodの追加

	//! 汎用ダイアログプロシージャ
	static BOOL DlgProc(
		BOOL (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//==============================================================
	//!	全般ページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_GENERAL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	BOOL DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );
	void SetData_p1( HWND );	/* ダイアログデータの設定 p1 */
	int  GetData_p1( HWND );	/* ダイアログデータの取得 p1 */

	//==============================================================
	//!	ファイルページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_FILE(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for File page
	BOOL DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );
	void SetData_p2( HWND );	/* ダイアログデータの設定 p2 */
	int  GetData_p2( HWND );	/* ダイアログデータの取得 p2 */

	//==============================================================
	//!	キー割り当てページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_KEYBIND(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Key Bind page
	BOOL DispatchEvent_p5( HWND, UINT, WPARAM, LPARAM );
	void SetData_p5( HWND );	/* ダイアログデータの設定 p5 */
	int  GetData_p5( HWND );	/* ダイアログデータの取得 p5 */
	void p5_Import_KeySetting( HWND );	/* p5:キー割り当て設定をインポートする */
	void p5_Export_KeySetting( HWND );	/* p5:キー割り当て設定をエクスポートする */

	//==============================================================
	//!	ツールバーページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_TOOLBAR(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Toolbar page
	BOOL DispatchEvent_p6( HWND, UINT, WPARAM, LPARAM );
	void SetData_p6( HWND );	/* ダイアログデータの設定 p6 */
	int  GetData_p6( HWND );	/* ダイアログデータの取得 p6 */

	//==============================================================
	//!	キーワードページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_KEYWORD(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Keyword page
	BOOL DispatchEvent_p7( HWND, UINT, WPARAM, LPARAM );
	void SetData_p7( HWND );	/* ダイアログデータの設定 p7 */
	void SetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	int  GetData_p7( HWND );	/* ダイアログデータの取得 p7 */
	void GetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの取得 p7 指定キーワードセットの取得 */

	//==============================================================
	//!	カスタムメニューページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_CUSTMENU(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	BOOL DispatchEvent_p8( HWND, UINT, WPARAM, LPARAM );
	void SetData_p8( HWND );	/* ダイアログデータの設定 p8 */
	int  GetData_p8( HWND );	/* ダイアログデータの取得 p8 */

	//==============================================================
	//!	書式ページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_FORMAT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Format page
	BOOL DispatchEvent_p9( HWND, UINT, WPARAM, LPARAM );
	void SetData_p9( HWND );	/* ダイアログデータの設定 p9 */
	int  GetData_p9( HWND );	/* ダイアログデータの取得 p9 */
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	//==============================================================
	//!	支援ページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_HELPER(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	BOOL DispatchEvent_p10( HWND, UINT, WPARAM, LPARAM );
	void SetData_p10( HWND );
	int  GetData_p10( HWND );

	//==============================================================
	//!	バックアップページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_BACKUP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Backup page
	BOOL DispatchEvent_PROP_BACKUP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_BACKUP( HWND );
	int GetData_PROP_BACKUP( HWND );

	//==============================================================
	//!	ウィンドウページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_WIN(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Window page
	BOOL DispatchEvent_PROP_WIN( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_WIN( HWND );
	int GetData_PROP_WIN( HWND );

	//==============================================================
	//!	クリッカブルURLページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_URL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for clickable URL page
	BOOL DispatchEvent_PROP_URL( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_URL( HWND );
	int GetData_PROP_URL( HWND );

	//==============================================================
	//!	編集ページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_EDIT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for edit page
	BOOL DispatchEvent_PROP_EDIT( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_EDIT( HWND );
	int GetData_PROP_EDIT( HWND );

	//==============================================================
	//!	GREPページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_GREP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Grep page
	BOOL DispatchEvent_PROP_GREP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_GREP( HWND );
	int GetData_PROP_GREP( HWND );

	//	From Here Jun. 2, 2001 genta
	//==============================================================
	//!	マクロページのDialog Procedure
	static BOOL CALLBACK DlgProc_PROP_MACRO(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Macro page
	BOOL DispatchEvent_PROP_Macro( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_Macro( HWND );//!<ダイアログデータの設定 Macro
	int GetData_PROP_Macro( HWND );//!<ダイアログデータの取得 Macro
	//	To Here Jun. 2, 2001 genta

	void p7_Edit_List_KeyWord( HWND, HWND );	/* p7:リスト中で選択されているキーワードを編集する */
	void p7_Delete_List_KeyWord( HWND , HWND );	/* p7:リスト中で選択されているキーワードを削除する */
	void p7_Import_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをインポートする */
	void p7_Export_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをエクスポートする */

	void p8_Import_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をインポートする */
	void p8_Export_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をエクスポートする */

	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	バックアップ設定のON/OFF
	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	ファイル設定のON/OFF
	//	Sept. 9, 2000 JEPRO		次行を追加
	void EnableWinPropInput( HWND hwndDlg) ;	//	ウィンドウ設定のON/OFF
	//	Sept. 10, 2000 JEPRO	次行を追加
	void CPropCommon::EnableFormatPropInput( HWND hwndDlg );	//	書式設定のON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROP1_H_ */


/*[EOF]*/
