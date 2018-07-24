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
	Copyright (C) 2013, Uchi

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

#ifndef SAKURA_CPROPCOMMON_8B67EE84_54E5_4541_A820_EE4FC61CCF0D_H_
#define SAKURA_CPROPCOMMON_8B67EE84_54E5_4541_A820_EE4FC61CCF0D_H_

#include "func/CFuncLookup.h"
#include "env/CommonSetting.h"

struct DLLSHAREDATA;
class CImageListMgr;
class CSMacroMgr;
class CMenuDrawer;// 2002/2/10 aroka to here

/*! プロパティシート番号
	@date 2008.6.22 Uchi #define -> enum に変更	
	@date 2008.6.22 Uchi順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に
*/
enum PropComSheetOrder {
	ID_PROPCOM_PAGENUM_GENERAL = 0,		//!< 全般
	ID_PROPCOM_PAGENUM_WIN,				//!< ウィンドウ
	ID_PROPCOM_PAGENUM_MAINMENU,		//!< メインメニュー
	ID_PROPCOM_PAGENUM_TOOLBAR,			//!< ツールバー
	ID_PROPCOM_PAGENUM_TAB,				//!< タブバー
	ID_PROPCOM_PAGENUM_STATUSBAR,		//!< ステータスバー
	ID_PROPCOM_PAGENUM_EDIT,			//!< 編集
	ID_PROPCOM_PAGENUM_FILE,			//!< ファイル
	ID_PROPCOM_PAGENUM_FILENAME,		//!< ファイル名表示
	ID_PROPCOM_PAGENUM_BACKUP,			//!< バックアップ
	ID_PROPCOM_PAGENUM_FORMAT,			//!< 書式
	ID_PROPCOM_PAGENUM_GREP,			//!< 検索
	ID_PROPCOM_PAGENUM_KEYBOARD,		//!< キー割り当て
	ID_PROPCOM_PAGENUM_CUSTMENU,		//!< カスタムメニュー
	ID_PROPCOM_PAGENUM_KEYWORD,			//!< 強調キーワード
	ID_PROPCOM_PAGENUM_HELPER,			//!< 支援
	ID_PROPCOM_PAGENUM_MACRO,			//!< マクロ
	ID_PROPCOM_PAGENUM_PLUGIN,			//!< プラグイン
	ID_PROPCOM_PAGENUM_MAX,
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
	void Create( HWND, CImageListMgr*, CMenuDrawer* );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	INT_PTR DoPropertySheet( int, bool );	/* プロパティシートの作成 */

	// 2002.12.11 Moca 追加
	void InitData( const int* = NULL, const TCHAR* = NULL, const TCHAR* = NULL );	//!< DLLSHAREDATAから一時データ領域に設定を複製する
	void ApplyData( int* = NULL );	//!< 一時データ領域からにDLLSHAREDATA設定をコピーする
	int GetPageNum(){ return m_nPageNum; }

	//
	static INT_PTR CALLBACK DlgProc_page(
		HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

	//	Jun. 2, 2001 genta
	//	ここにあったEvent Handlerはprotectedエリアに移動した．

	HWND				m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND				m_hwndThis;		/* このダイアログのハンドル */
	PropComSheetOrder	m_nPageNum;
	DLLSHAREDATA*		m_pShareData;
	int					m_nKeywordSet1;
	//	Oct. 16, 2000 genta
	CImageListMgr*	m_pcIcons;	//	Image List
	
	//	Oct. 2, 2001 genta 外部マクロ追加に伴う，対応部分の別クラス化
	//	Oct. 15, 2001 genta Lookupはダイアログボックス内で別インスタンスを作るように
	//	(検索対象として，設定用common領域を指すようにするため．)
	CFuncLookup			m_cLookup;

	CMenuDrawer*		m_pcMenuDrawer;
	/*
	|| ダイアログデータ
	*/
	CommonSetting	m_Common;

	// 2005.01.13 MIK セット数増加
	struct SKeywordSetIndex{
		int typeId;
		int index[MAX_KEYWORDSET_PER_TYPE];
	};
	std::vector<SKeywordSetIndex>	m_Types_nKeyWordSetIdx;
	TCHAR			m_tempTypeName[MAX_TYPES_NAME];	//!< タイプ属性：名称
	TCHAR			m_tempTypeExts[MAX_TYPES_EXTS];	//!< タイプ属性：拡張子リスト
	bool			m_bTrayProc;
	HFONT			m_hKeywordHelpFont;		//!< キーワードヘルプ フォント ハンドル
	HFONT			m_hTabFont;				//!< タブ フォント ハンドル

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

	//! Message Handler
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得
	void Import( HWND );	//!< インポートする
	void Export( HWND );	//!< エクスポートする

	HFONT SetCtrlFont( HWND hwndDlg, int idc_static, const LOGFONT& lf );			//!< コントロールにフォント設定する		// 2013/4/24 Uchi
	HFONT SetFontLabel( HWND hwndDlg, int idc_static, const LOGFONT& lf, int nps );	//!< フォントラベルにフォントとフォント名設定する	// 2013/4/24 Uchi
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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得
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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

	void Import( HWND );	//!< インポートする
	void Export( HWND );	//!< エクスポートする

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

private:
	void SetKeyWordSet( HWND , int );	/* 指定キーワードセットの設定 */
	void GetKeyWordSet( HWND , int );	/* 指定キーワードセットの取得 */
	void DispKeywordCount( HWND hwndDlg );

	void Edit_List_KeyWord( HWND, HWND );		//!< リスト中で選択されているキーワードを編集する
	void Delete_List_KeyWord( HWND , HWND );	//!< リスト中で選択されているキーワードを削除する
	void Import_List_KeyWord( HWND , HWND );	//!< リスト中のキーワードをインポートする
	void Export_List_KeyWord( HWND , HWND );	//!< リスト中のキーワードをエクスポートする
	void Clean_List_KeyWord( HWND , HWND );		//!< リスト中のキーワードを整理する 2005.01.26 Moca
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
	void SetData( HWND );	//!< ダイアログデータの設定
	void SetDataMenuList( HWND, int );
	int  GetData( HWND );	//!< ダイアログデータの取得
	void Import( HWND );	//!< カスタムメニュー設定をインポートする
	void Export( HWND );	//!< カスタムメニュー設定をエクスポートする
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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得
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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得
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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得

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
	void SetData( HWND );	//!< ダイアログデータの設定
	int  GetData( HWND );	//!< ダイアログデータの取得
	void Import( HWND );	//!< メニュー設定をインポートする
	void Export( HWND );	//!< メニュー設定をエクスポートする

private:
	bool GetDataTree( HWND, HTREEITEM, int );

	bool Check_MainMenu( HWND, std::wstring& );						// メニューの検査
	bool Check_MainMenu_Sub( HWND, HTREEITEM, int, std::wstring& );	// メニューの検査
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CPROPCOMMON_8B67EE84_54E5_4541_A820_EE4FC61CCF0D_H_ */
