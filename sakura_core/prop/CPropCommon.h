/*!	@file
	@brief 共通設定ダイアログボックスの処理

	@author	Norio Nakatani
	@date 1998/12/24 新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta, jepro
	Copyright (C) 2001, genta
	Copyright (C) 2002, YAZAKI, aroka, Moca
	Copyright (C) 2005, MIK, Moca, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

class CPropCommon;

#ifndef _CPROP1_H_
#define _CPROP1_H_
#include <windows.h>
#include "CShareData.h"
#include "sakura_rc.h"
#include "CFuncLookup.h"
class CImageListMgr;
class CSMacroMgr;
class CMenuDrawer;// 2002/2/10 aroka to here

#define ID_PAGENUM_ZENPAN	0	//Oct. 25, 2000 JEPRO ZENPAN1→ZENPAN に変更
#define ID_PAGENUM_WIN		1	//Oct. 25, 2000 JEPRO  5→ 1 に変更
#define ID_PAGENUM_TAB		2	//Feb. 11, 2007 genta URLと入れ換え	// 2007.02.13 順序変更（TABをWINの次に）
#define ID_PAGENUM_EDIT		3
#define ID_PAGENUM_FILE		4
#define ID_PAGENUM_BACKUP	5
#define ID_PAGENUM_FORMAT	6	//Oct. 25, 2000 JEPRO  1→ 5 に変更
#define ID_PAGENUM_GREP		7
#define ID_PAGENUM_KEYBOARD	8
#define ID_PAGENUM_CUSTMENU	9	//Oct. 25, 2000 JEPRO 11→ 9 に変更
#define ID_PAGENUM_TOOLBAR	10	//Oct. 25, 2000 JEPRO  9→10 に変更
#define ID_PAGENUM_KEYWORD	11	//Oct. 25, 2000 JEPRO 10→11 に変更
#define ID_PAGENUM_HELPER	12
#define ID_PAGENUM_MACRO	13	//Oct. 25, 2000 JEPRO 10→11 に変更
#define ID_PAGENUM_FILENAME	14	// Moca 追加

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 共通設定ダイアログボックスクラス

	1つのダイアログボックスに複数のプロパティページが入った構造に
	なっており、Dialog procedureとEvent Dispatcherがページごとにある．

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class SAKURA_CORE_API CPropCommon
{
public:
	/*
	||  Constructors
	*/
	CPropCommon();
	~CPropCommon();
	//	Sep. 29, 2001 genta マクロクラスを渡すように;
//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
	void Create( HWND, CImageListMgr*, CSMacroMgr* , CMenuDrawer* );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	int DoPropertySheet( int/*, int*/ );	/* プロパティシートの作成 */

	// 2002.12.11 Moca 追加
	void InitData( void );		//!< DLLSHAREDATAから一時データ領域に設定を複製する
	void ApplyData( void );		//!< 一時データ領域からにDLLSHAREDATA設定をコピーする

	//	Jun. 2, 2001 genta
	//	ここにあったEvent Handlerはprotectedエリアに移動した．

	HWND			m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND			m_hwndThis;		/* このダイアログのハンドル */
	int				m_nPageNum;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;
//	int				m_nActiveItem;

	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List
	
	//	Oct. 2, 2001 genta 外部マクロ追加に伴う，対応部分の別クラス化
	//	Oct. 15, 2001 genta Lookupはダイアログボックス内で別インスタンスを作るように
	//	(検索対象として，設定用common領域を指すようにするため．)
	CFuncLookup			m_cLookup;
	CSMacroMgr*			m_pcSMacro;

	CMenuDrawer*		m_pcMenuDrawer;
	/*
	|| ダイアログデータ
	*/
	SFilePath		m_szHelpFile;
	CommonSetting	m_Common;
	short			m_nKeyNameArrNum;				/* キー割り当て表の有効データ数 */
	KEYDATA			m_pKeyNameArr[100];				/* キー割り当て表 */

	CKeyWordSetMgr	m_CKeyWordSetMgr;
	//2002/04/25 YAZAKI Types全体を保持する必要はない。
	//Types			m_Types[MAX_TYPES];
	// 2005.01.13 MIK セット数増加
	int				m_Types_nKeyWordSetIdx[MAX_TYPES][MAX_KEYWORDSET_PER_TYPE];
	//@@@ 2002.01.03 YAZAKI 共通設定『マクロ』がタブを切り替えるだけで設定が保存されないように。
	MacroRec		m_MacroTable[MAX_CUSTMACRO];	//!< キー割り当て用マクロテーブル
	TCHAR			m_szMACROFOLDER[_MAX_PATH];		/* マクロ用フォルダ */

	//! ファイル名簡易表示
	int		m_nTransformFileNameArrNum;
	TCHAR	m_szTransformFileNameFrom[MAX_TRANSFORM_FILENAME][_MAX_PATH];
	TCHAR	m_szTransformFileNameTo[MAX_TRANSFORM_FILENAME][_MAX_PATH];

protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp( HWND, int );	/* ヘルプ */
	int	SearchIntArr( int , int* , int );
//	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
//	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* 色ボタンの描画 */ // 2002.11.09 Moca 未使用
	BOOL SelectColor( HWND , COLORREF* );	/* 色選択ダイアログ */

	//	Jun. 2, 2001 genta
	//	Event Handler, Dialog Procedureの見直し
	//	Global関数だったDialog procedureをclassのstatic methodとして
	//	組み込んだ．
	//	ここから以下 Macroまで配置の見直しとstatic methodの追加

	//! 汎用ダイアログプロシージャ
	static INT_PTR DlgProc(
		INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//==============================================================
	//!	全般ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_GENERAL(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR DispatchEvent_p1( HWND, UINT, WPARAM, LPARAM );
	void SetData_p1( HWND );	/* ダイアログデータの設定 p1 */
	int  GetData_p1( HWND );	/* ダイアログデータの取得 p1 */

	//==============================================================
	//!	ファイルページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FILE(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for File page
	INT_PTR DispatchEvent_p2( HWND, UINT, WPARAM, LPARAM );
	void SetData_p2( HWND );	/* ダイアログデータの設定 p2 */
	int  GetData_p2( HWND );	/* ダイアログデータの取得 p2 */

	//==============================================================
	//!	キー割り当てページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_KEYBIND(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Key Bind page
	INT_PTR DispatchEvent_p5( HWND, UINT, WPARAM, LPARAM );
	void SetData_p5( HWND );	/* ダイアログデータの設定 p5 */
	int  GetData_p5( HWND );	/* ダイアログデータの取得 p5 */
	void p5_ChangeKeyList( HWND ); /* p5: キーリストをチェックボックスの状態に合わせて更新する*/
	void p5_Import_KeySetting( HWND );	/* p5:キー割り当て設定をインポートする */
	void p5_Export_KeySetting( HWND );	/* p5:キー割り当て設定をエクスポートする */

	//==============================================================
	//!	ツールバーページのDialog Procedure
	// 20050809 aroka _p6から_PROP_TOOLBARに名前変更
	static INT_PTR CALLBACK DlgProc_PROP_TOOLBAR(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Toolbar page
	INT_PTR DispatchEvent_PROP_TOOLBAR( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_TOOLBAR( HWND );	/* ツールバー ダイアログデータの設定 */
	int  GetData_PROP_TOOLBAR( HWND );	/* ツールバー ダイアログデータの取得 */
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */

	//==============================================================
	//!	キーワードページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_KEYWORD(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Keyword page
	INT_PTR DispatchEvent_p7( HWND, UINT, WPARAM, LPARAM );
	void SetData_p7( HWND );	/* ダイアログデータの設定 p7 */
	void SetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの設定 p7 指定キーワードセットの設定 */
	int  GetData_p7( HWND );	/* ダイアログデータの取得 p7 */
	void GetData_p7_KeyWordSet( HWND , int );	/* ダイアログデータの取得 p7 指定キーワードセットの取得 */
	void DispKeywordCount( HWND hwndDlg );

	//==============================================================
	//!	カスタムメニューページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_CUSTMENU(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	INT_PTR DispatchEvent_p8( HWND, UINT, WPARAM, LPARAM );
	void SetData_p8( HWND );	/* ダイアログデータの設定 p8 */
	int  GetData_p8( HWND );	/* ダイアログデータの取得 p8 */

	//==============================================================
	//!	書式ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FORMAT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Format page
	INT_PTR DispatchEvent_p9( HWND, UINT, WPARAM, LPARAM );
	void SetData_p9( HWND );	/* ダイアログデータの設定 p9 */
	int  GetData_p9( HWND );	/* ダイアログデータの取得 p9 */
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	//==============================================================
	//!	支援ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_HELPER(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Custom Menu page
	INT_PTR DispatchEvent_p10( HWND, UINT, WPARAM, LPARAM );
	void SetData_p10( HWND );
	int  GetData_p10( HWND );

	//==============================================================
	//!	バックアップページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_BACKUP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Backup page
	INT_PTR DispatchEvent_PROP_BACKUP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_BACKUP( HWND );
	int GetData_PROP_BACKUP( HWND );

	//==============================================================
	//!	ウィンドウページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_WIN(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Window page
	INT_PTR DispatchEvent_PROP_WIN( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_WIN( HWND );
	int GetData_PROP_WIN( HWND );

	//==============================================================
	//!	タブ動作ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_TAB(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for TAB page
	INT_PTR DispatchEvent_PROP_TAB( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_TAB( HWND );
	int GetData_PROP_TAB( HWND );
	void EnableTabPropInput(HWND hwndDlg);

	//==============================================================
	//!	編集ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_EDIT(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for edit page
	INT_PTR DispatchEvent_PROP_EDIT( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_EDIT( HWND );
	int GetData_PROP_EDIT( HWND );

	//==============================================================
	//!	GREPページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_GREP(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Grep page
	INT_PTR DispatchEvent_PROP_GREP( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_GREP( HWND );
	int GetData_PROP_GREP( HWND );
	void SetRegexpVersion( HWND ); // 2007.08.12 genta バージョン表示

	//	From Here Jun. 2, 2001 genta
	//==============================================================
	//!	マクロページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_MACRO(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for Macro page
	INT_PTR DispatchEvent_PROP_Macro( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_Macro( HWND );//!<ダイアログデータの設定 Macro
	int GetData_PROP_Macro( HWND );//!<ダイアログデータの取得 Macro
	void InitDialog_PROP_Macro( HWND hwndDlg );//!< Macroページの初期化
	//	To Here Jun. 2, 2001 genta
	void SetMacro2List_Macro( HWND hwndDlg );//!< Macroデータの設定
	void SelectBaseDir_Macro( HWND hwndDlg );//!< Macroディレクトリの選択
	void OnFileDropdown_Macro( HWND hwndDlg );//!< ファイルドロップダウンが開かれるとき
	void CheckListPosition_Macro( HWND hwndDlg );//!< リストビューのFocus位置確認
	static int CALLBACK DirCallback_Macro( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );

	//==============================================================
	//!	ファイル名表示ページのDialog Procedure
	static INT_PTR CALLBACK DlgProc_PROP_FILENAME(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	//! Message Handler for FileName page
	INT_PTR DispatchEvent_PROP_FILENAME( HWND, UINT, WPARAM, LPARAM );
	void SetData_PROP_FILENAME( HWND );
	int  GetData_PROP_FILENAME( HWND );
	static int SetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR, bool );//!<ListViewのアイテムを設定
	static void GetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR );//!<ListViewのアイテムを取得
	static int MoveListViewItem_FILENAME( HWND hListView, int, int );//!<ListViewのアイテムを移動する



	int nLastPos_Macro; //!< 前回フォーカスのあった場所
	int m_nLastPos_FILENAME; //!< 前回フォーカスのあった場所 ファイル名タブ用


	void p7_Edit_List_KeyWord( HWND, HWND );	/* p7:リスト中で選択されているキーワードを編集する */
	void p7_Delete_List_KeyWord( HWND , HWND );	/* p7:リスト中で選択されているキーワードを削除する */
	void p7_Import_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをインポートする */
	void p7_Export_List_KeyWord( HWND , HWND );	/* p7:リスト中のキーワードをエクスポートする */
	void p7_Clean_List_KeyWord( HWND , HWND );	//! p7:リスト中のキーワードを整理する 2005.01.26 Moca

	void p8_Import_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をインポートする */
	void p8_Export_CustMenuSetting( HWND );	/* p8:カスタムメニュー設定をエクスポートする */

	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	バックアップ設定のON/OFF
	//	20051107 aroka
	void UpdateBackupFile(HWND hwndDlg);	//	バックアップファイルの詳細設定

	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	ファイル設定のON/OFF
	//	Sept. 9, 2000 JEPRO		次行を追加
	void EnableWinPropInput( HWND hwndDlg) ;	//	ウィンドウ設定のON/OFF
	//	Sept. 10, 2000 JEPRO	次行を追加
	void CPropCommon::EnableFormatPropInput( HWND hwndDlg );	//	書式設定のON/OFF
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPROP1_H_ */



