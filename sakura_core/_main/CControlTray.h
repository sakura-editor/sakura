/*!	@file
	@brief 常駐部

	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2001/06/03 N.Nakatani grep単語単位で検索を実装するときのためにコマンドラインオプションの処理追加
	@date 2007/10/23 kobake     クラス名、ファイル名変更: CEditApp→CControlTray
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_
#define SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_
#pragma once

#include "_main/CMainWindow.hpp"

#include "dlg/CDlgGrep.h" // 2002/2/10 aroka

struct SLoadInfo;
struct EditInfo;
struct DLLSHAREDATA;

//!	常駐部の管理
/*!
	タスクトレイアイコンの管理，タスクトレイメニューのアクション，
	MRU、キー割り当て、共通設定、編集ウィンドウの管理など
 */
class CControlTray : public CMainWindow {
	using Me = CControlTray;

public:
	/*
	||  Constructors
	*/
	explicit CControlTray() noexcept;
	~CControlTray() override = default;

	/*
	|| メンバ関数
	*/
	HWND    CreateMainWnd(int nCmdShow) override;	/* 作成 */
	bool CreateTrayIcon(HWND hWnd);	// 20010412 by aroka

	LRESULT DispatchEvent(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) override;

	bool    OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;

	void    MessageLoop(void) override;
	void OnDestroy( void );		/* WM_DESTROY 処理 */	// 2006.07.09 ryoji
	int	CreatePopUpMenu_L( void );	/* ポップアップメニュー(トレイ左ボタン) */
	int	CreatePopUpMenu_R( void );	/* ポップアップメニュー(トレイ右ボタン) */
	void CreateAccelTbl( void ); // アクセラレータテーブル作成
	void DeleteAccelTbl( void ); // アクセラレータテーブル破棄

	//ウィンドウ管理
	static bool OpenNewEditor(							//!< 新規編集ウィンドウの追加 ver 0
		HINSTANCE			hInstance,					//!< [in] インスタンスID (実は未使用)
		HWND				hWndParent,					//!< [in] 親ウィンドウハンドル．エラーメッセージ表示用
		const SLoadInfo&	sLoadInfo,					//!< [in]
		const WCHAR*		szCmdLineOption	= NULL,		//!< [in] 追加のコマンドラインオプション
		bool				sync			= false,	//!< [in] trueなら新規エディタの起動まで待機する
		const WCHAR*		pszCurDir		= NULL,		//!< [in] 新規エディタのカレントディレクトリ
		bool				bNewWindow		= false		//!< [in] 新規エディタをウインドウで開く
	);
	static bool OpenNewEditor2(						//!< 新規編集ウィンドウの追加 ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo*	pfi,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void ActiveNextWindow(HWND hwndParent);
	static void ActivePrevWindow(HWND hwndParent);

	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom, BOOL bExit, int nGroup );	/* すべてのウィンドウを閉じる */	//Oct. 7, 2000 jepro 「編集ウィンドウの全終了」という説明を左記のように変更	// 2006.12.25, 2007.02.13 ryoji 引数追加
	static void TerminateApplication( HWND hWndFrom );	/* サクラエディタの全終了 */	// 2006.12.25 ryoji 引数追加

public:
	HWND GetTrayHwnd() const{ return m_hWnd; }

	/*
	|| 実装ヘルパ系
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, CDlgGrep& cDlgGrep);
protected:
	void	DoGrep();	//Stonee, 2001/03/21
	BOOL TrayMessage(HWND hDlg, DWORD dwMessage, UINT uID, HICON hIcon, const WCHAR* pszTip);	/*!< タスクトレイのアイコンに関する処理 */
	void OnNewEditor(bool bNewWindow); //!< 2003.05.30 genta 新規ウィンドウ作成処理を切り出し

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
	bool			m_bUseTrayMenu;			//トレイメニュー表示中
	BOOL			m_bCreatedTrayIcon;		//!< トレイにアイコンを作った

	HWND            hwndHtmlHelp    = nullptr;
	WORD            wHotKeyMods     = 0;
	WORD            wHotKeyCode     = 0;

	bool			bLDClick        = false;	/* 左ダブルクリックをしたか 03/02/20 ai */

	CDlgGrep		m_cDlgGrep;				// Jul. 2, 2001 genta
	int				m_nCurSearchKeySequence;

	UINT			m_uCreateTaskBarMsg;	//!< RegisterMessageで得られるMessage IDの保管場所。Apr. 24, 2001 genta

	SFilePath       m_szLanguageDll;
};

WORD convertHotKeyMods(WORD wHotKeyMods);

#endif /* SAKURA_CCONTROLTRAY_E9E24D69_3511_4EC1_A29A_1D119F68004A_H_ */
