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
	Copyright (C) 2010, Uchi

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#ifndef SAKURA_PROP_CPROPCOMMON_H_
#define SAKURA_PROP_CPROPCOMMON_H_

class CPropCommon;

#include "func/CFuncLookup.h"
#include "env/CommonSetting.h"

struct DLLSHAREDATA;
class CImageListMgr;
class CSMacroMgr;
class CMenuDrawer;// 2002/2/10 aroka to here

//	#define -> enum に変更	2008/6/22 Uchi
//	順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に	2008/6/22 Uchi
enum ComPropSheetOrder {
	ID_PAGENUM_GENERAL = 0,		// 全般
	ID_PAGENUM_WIN,				// ウィンドウ
	ID_PAGENUM_MAINMENU,		// メインメニュー
	ID_PAGENUM_TOOLBAR,			// ツールバー
	ID_PAGENUM_TAB,				// タブバー
	ID_PAGENUM_STATUSBAR,		// ステータスバー
	ID_PAGENUM_EDIT,			// 編集
	ID_PAGENUM_FILE,			// ファイル
	ID_PAGENUM_FILENAME,		// ファイル名表示
	ID_PAGENUM_BACKUP,			// バックアップ
	ID_PAGENUM_FORMAT,			// 書式
	ID_PAGENUM_GREP,			// 検索
	ID_PAGENUM_KEYBOARD,		// キー割り当て
	ID_PAGENUM_CUSTMENU,		// カスタムメニュー
	ID_PAGENUM_KEYWORD,			// 強調キーワード
	ID_PAGENUM_HELPER,			// 支援
	ID_PAGENUM_MACRO,			// マクロ
	ID_PAGENUM_PLUGIN,			// プラグイン
};
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 共通設定ダイアログボックスクラス

	1つのダイアログボックスに複数のプロパティページが入った構造に
	なっており、Dialog procedureとEvent Dispatcherがページごとにある．

	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
class CPropCommon
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

	HWND				m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND				m_hwndThis;		/* このダイアログのハンドル */
	ComPropSheetOrder	m_nPageNum;
	DLLSHAREDATA*		m_pShareData;
//	int				m_nSettingType;
//	int				m_nActiveItem;
	int					m_nKeywordSet1;
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
	LPCTSTR			m_pszHelpFile;
	CommonSetting	m_Common;

	//2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
	//STypeConfig			m_Types[MAX_TYPES];
	// 2005.01.13 MIK セット数増加
	int				m_Types_nKeyWordSetIdx[MAX_TYPES][MAX_KEYWORDSET_PER_TYPE];

protected:
	/*
	||  実装ヘルパ関数
	*/
	void OnHelp( HWND, int );	/* ヘルプ */
	int	SearchIntArr( int , int* , int );
//	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
//	void DrawColorButton( DRAWITEMSTRUCT* , COLORREF );	/* 色ボタンの描画 */ // 2002.11.09 Moca 未使用
//	BOOL SelectColor( HWND , COLORREF* );	/* 色選択ダイアログ */

	//	Jun. 2, 2001 genta
	//	Event Handler, Dialog Procedureの見直し
	//	Global関数だったDialog procedureをclassのstatic methodとして
	//	組み込んだ．
	//	ここから以下 Macroまで配置の見直しとstatic methodの追加

	//! 汎用ダイアログプロシージャ
	static INT_PTR DlgProc(
		INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR DlgProc2( //独立ウィンドウ用
		INT_PTR (CPropCommon::*DispatchPage)( HWND, UINT, WPARAM, LPARAM ),
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	typedef	INT_PTR (CPropCommon::*pDispatchPage)( HWND, UINT, WPARAM, LPARAM );

	int nLastPos_Macro; //!< 前回フォーカスのあった場所
	int m_nLastPos_FILENAME; //!< 前回フォーカスのあった場所 ファイル名タブ用

public:
	//
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
	void Import( HWND );	/* インポートする */
	void Export( HWND );	/* エクスポートする */
};


/*!
	@brief 共通設定プロパティページクラス

	1つのプロパティページ毎に定義
	Dialog procedureとEvent Dispatcherがページごとにある．
	変数の定義はCPropCommonで行う
*/
//==============================================================
//!	全般ページ
class CPropGeneral : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
};

//==============================================================
//!	ファイルページ
class CPropFile : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	//	Aug. 21, 2000 genta
	void EnableFilePropInput(HWND hwndDlg);	//	ファイル設定のON/OFF
};

//==============================================================
//!	キー割り当てページ
class CPropKeybind : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

	void Import( HWND );	/* インポートする */
	void Export( HWND );	/* エクスポートする */

private:
	void ChangeKeyList( HWND ); /* キーリストをチェックボックスの状態に合わせて更新する*/
};

//==============================================================
//!	ツールバーページ
class CPropToolbar : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void DrawToolBarItemList( DRAWITEMSTRUCT* );	/* ツールバーボタンリストのアイテム描画 */
};

//==============================================================
//!	キーワードページ
class CPropKeyword : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK DlgProc_dialog(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void SetKeyWordSet( HWND , int );	/* 指定キーワードセットの設定 */
	void GetKeyWordSet( HWND , int );	/* 指定キーワードセットの取得 */
	void DispKeywordCount( HWND hwndDlg );

	void Edit_List_KeyWord( HWND, HWND );		/* リスト中で選択されているキーワードを編集する */
	void Delete_List_KeyWord( HWND , HWND );	/* リスト中で選択されているキーワードを削除する */
	void Import_List_KeyWord( HWND , HWND );	/* リスト中のキーワードをインポートする */
	void Export_List_KeyWord( HWND , HWND );	/* リスト中のキーワードをエクスポートする */
	void Clean_List_KeyWord( HWND , HWND );		//!リスト中のキーワードを整理する 2005.01.26 Moca
};

//==============================================================
//!	カスタムメニューページ
class CPropCustmenu : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
	void Import( HWND );	/* カスタムメニュー設定をインポートする */
	void Export( HWND );	/* カスタムメニュー設定をエクスポートする */
};

//==============================================================
//!	書式ページ
class CPropFormat : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void ChangeDateExample( HWND hwndDlg );
	void ChangeTimeExample( HWND hwndDlg );

	//	Sept. 10, 2000 JEPRO	次行を追加
	void EnableFormatPropInput( HWND hwndDlg );	//	書式設定のON/OFF
};

//==============================================================
//!	支援ページ
class CPropHelper : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
};

//==============================================================
//!	バックアップページ
class CPropBackup : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	//	Aug. 16, 2000 genta
	void EnableBackupInput(HWND hwndDlg);	//	バックアップ設定のON/OFF
	//	20051107 aroka
	void UpdateBackupFile(HWND hwndDlg);	//	バックアップファイルの詳細設定
};

//==============================================================
//!	ウィンドウページ
class CPropWin : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	//	Sept. 9, 2000 JEPRO		次行を追加
	void EnableWinPropInput( HWND hwndDlg) ;	//	ウィンドウ設定のON/OFF
};

//==============================================================
//!	タブ動作ページ
class CPropTab : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void EnableTabPropInput(HWND hwndDlg);
};

//==============================================================
//!	編集ページ
class CPropEdit : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void EnableEditPropInput( HWND hwndDlg );
};

//==============================================================
//!	検索ページ
class CPropGrep : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void SetRegexpVersion( HWND ); // 2007.08.12 genta バージョン表示
};

//==============================================================
//!	マクロページ
class CPropMacro : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void InitDialog( HWND hwndDlg );//!< Macroページの初期化
	//	To Here Jun. 2, 2001 genta
	void SetMacro2List_Macro( HWND hwndDlg );//!< Macroデータの設定
	void SelectBaseDir_Macro( HWND hwndDlg );//!< Macroディレクトリの選択
	void OnFileDropdown_Macro( HWND hwndDlg );//!< ファイルドロップダウンが開かれるとき
	void CheckListPosition_Macro( HWND hwndDlg );//!< リストビューのFocus位置確認
	static int CALLBACK DirCallback_Macro( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData );
};

//==============================================================
//!	ファイル名表示ページ
class CPropFileName : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	static int SetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR, bool );//!<ListViewのアイテムを設定
	static void GetListViewItem_FILENAME( HWND hListView, int, LPTSTR, LPTSTR );//!<ListViewのアイテムを取得
	static int MoveListViewItem_FILENAME( HWND hListView, int, int );//!<ListViewのアイテムを移動する
};

//==============================================================
//!	ステータスバーページ
class CPropStatusbar : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
};

//==============================================================
//!	プラグインページ
class CPropPlugin : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	std::tstring GetReadMeFile(const std::tstring& sName);	//	Readme ファイルの取得
	bool BrowseReadMe(const std::tstring& sReadMeName);		//	Readme ファイルの表示
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */

private:
	void SetData_LIST( HWND );
	void InitDialog( HWND hwndDlg );	//!< Pluginページの初期化
	void EnablePluginPropInput(HWND hwndDlg);
};

//==============================================================
//!	メインメニューページ
class CPropMainMenu : CPropCommon
{
public:
	//!	Dialog Procedure
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
protected:
	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	/* ダイアログデータの設定 */
	int  GetData( HWND );	/* ダイアログデータの取得 */
	void Import( HWND );	/* メニュー設定をインポートする */
	void Export( HWND );	/* メニュー設定をエクスポートする */

private:
	bool GetDataTree( HWND, HTREEITEM, int );

	bool Check_MainMenu( HWND, std::wstring& );						// メニューの検査
	bool Check_MainMenu_Sub( HWND, HTREEITEM, int, std::wstring& );	// メニューの検査
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_PROP_CPROPCOMMON_H_ */
