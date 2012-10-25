/*!	@file
	@brief 常駐部

	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_

#include <windows.h>
#include "CShareData.h"
#include "CMenuDrawer.h"
#include "CDlgGrep.h" // 2002/2/10 aroka
#include "CImageListMgr.h" // 2002/2/10 aroka

//!	常駐部の管理
/*!
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
	@date 2007.10.30 kobake OnHelp_MenuItemをCEditWndから持ってきた
	@date 2007.10.30 kobake 関数名変更: OnHelp_MenuItem→ShowFuncHelp
*/
class SAKURA_CORE_API CEditApp
{
public:
	/*
	||  Constructors
	*/
	CEditApp();
	~CEditApp();

	/*
	|| メンバ関数
	*/
	HWND Create( HINSTANCE );	/* 作成 */
	bool CreateTrayIcon( HWND );	// 20010412 by aroka
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */
	void MessageLoop( void );	/* メッセージループ */
	void OnDestroy( void );		/* WM_DESTROY 処理 */	// 2006.07.09 ryoji
	int	CreatePopUpMenu_L( void );	/* ポップアップメニュー(トレイ左ボタン) */
	int	CreatePopUpMenu_R( void );	/* ポップアップメニュー(トレイ右ボタン) */

	//ウィンドウ管理
	static bool OpenNewEditor(							//!< 新規編集ウィンドウの追加 ver 0
		HINSTANCE			hInstance,					//!< [in] インスタンスID (実は未使用)
		HWND				hWndParent,					//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
		const TCHAR*		pszPath,					//!< [in] 新規エディタで開くファイル名とオプション．NULLで新規エディタ作成．
		int					nCharCode,					//!< [in] 新規エディタの文字コード
		BOOL				bReadOnly,					//!< [in] FALSEでなければ読み取り専用で開く
		bool				sync			= false,	//!< [in] trueなら新規エディタの起動まで待機する
		const TCHAR*		szCurDir		= NULL,		//!< [in] 新規エディタのカレントディレクトリ
		bool				bNewWindow		= false		//!< [in] 新規エディタをウインドウで開く
	);
	static bool OpenNewEditor2(						//!< 新規編集ウィンドウの追加 ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo*	pfi,
		BOOL			bReadOnly,
		bool			sync		= false,
		bool			bNewWindow	= false
	);

	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom, BOOL bExit, int nGroup );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更	// 2006.12.25, 2007.02.13 ryoji 引数追加
	static void TerminateApplication( HWND hWndFrom );	/* サクラエディタの全終了 */	// 2006.12.25 ryoji 引数追加

	static LPCTSTR GetHelpFilePath();		//!< ヘルプファイルのフルパスを返す
	static void ShowFuncHelp( HWND, int );	//!< メニューアイテムに対応するヘルプを表示

	/*
	|| 実装ヘルパ系
	*/
protected:
	void	DoGrep();	//Stonee, 2001/03/21
	BOOL TrayMessage(HWND , DWORD , UINT , HICON , const TCHAR* );	/*!< タスクトレイのアイコンに関する処理 */
	void OnCommand( WORD , WORD  , HWND );	/*!< WM_COMMANDメッセージ処理 */
	void OnNewEditor( bool ); //!< 2003.05.30 genta 新規ウィンドウ作成処理を切り出し

	static INT_PTR CALLBACK ExitingDlgProc(	/*!< 終了ダイアログ用プロシージャ */	// 2006.07.02 ryoji CControlProcess から移動
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);


	/*
	|| メンバ変数
	*/
private:
	CMenuDrawer		m_CMenuDrawer;
	bool			m_bUseTrayMenu;			//トレイメニュー表示中
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	BOOL			m_bCreatedTrayIcon;		//!< トレイにアイコンを作った

	DLLSHAREDATA*	m_pShareData;
	CDlgGrep		m_cDlgGrep;				// Jul. 2, 2001 genta

	CImageListMgr	m_hIcons;

	UINT			m_uCreateTaskBarMsg;	//!< RegisterMessageで得られるMessage IDの保管場所。Apr. 24, 2001 genta
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */


/*[EOF]*/
