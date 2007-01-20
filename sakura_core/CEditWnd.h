/*!	@file
	@brief 編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
	@date 1998/05/13 新規作成
	@date 2002/01/14 YAZAKI PrintPreviewの分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta
	Copyright (C) 2001-2002, YAZAKI
	Copyright (C) 2002, aroka, genta, MIK
	Copyright (C) 2003, MIK, genta, wmlhq
	Copyright (C) 2004, Moca
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji, aroka, fon, yukihane

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include "CEditDoc.h"
#include "CShareData.h"
#include "CFuncKeyWnd.h"
#include "CTabWnd.h"	//@@@ 2003.05.31 MIK
#include "CMenuDrawer.h"
#include "CImageListMgr.h"

//by 鬼
#include"CDropTarget.h"

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
class CPrintPreview;// 2002/2/10 aroka




//! 編集ウィンドウ（外枠）管理クラス
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class SAKURA_CORE_API CEditWnd
{
public:
	/*
	||  Constructors
	*/
	CEditWnd();
	~CEditWnd();

	/*
	|| メンバ関数
	*/
	//	Mar. 7, 2002 genta 文書タイプ用引数追加
	HWND Create( HINSTANCE, HWND, const char*, int, BOOL, int = -1 );	/* 作成 */


	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
//	BOOL DispatchEvent_PPB( HWND, UINT, WPARAM, LPARAM );	/* 印刷プレビュー 操作バー ダイアログのメッセージ処理 */

	void PrintPreviewModeONOFF( void );	/* 印刷プレビューモードのオン/オフ */

	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* 描画処理 */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE 処理 */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );

	// ツールバー更新用と自動保存用のタイマーを分離した 20060128 aroka
	void OnTimer( HWND, UINT, UINT, DWORD );	/* タイマーの処理 */
	void OnToolbarTimer( HWND, UINT, UINT, DWORD );	/* タイマーの処理 20060128 aroka */
	void OnCommand( WORD, WORD , HWND );

	void CreateToolBar( void );			/* ツールバー作成 */
	void DestroyToolBar( void );		/* ツールバー破棄 */
	void CreateStatusBar( void );		/* ステータスバー作成 */
	void DestroyStatusBar( void );		/* ステータスバー破棄 */

	void LayoutToolBar( void );			/* ツールバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutFuncKey( void );			/* ファンクションキーの配置処理 */	// 2006.12.19 ryoji
	void LayoutTabBar( void );			/* タブバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutStatusBar( void );		/* ステータスバーの配置処理 */		// 2006.12.19 ryoji
	void EndLayoutBars( void );			/* バーの配置終了処理 */			// 2006.12.19 ryoji

	//@@@ 2002.01.14 YAZAKI 印刷プレビューのバーはCPrintPreviewに移動

	void InitMenu( HMENU, UINT, BOOL );
//複数プロセス版
	void MessageLoop( void );	/* メッセージループ */

	int	OnClose( void );	/* 終了時の処理 */

//@@@ 2002.01.14 YAZAKI 不使用のため
//void CEditWnd::ExecCmd(LPCSTR lpszCmd/*, HANDLE hFile*/);

	//	Sep. 10, 2002 genta
	void SetWindowIcon( HICON, int);
	//	Sep. 10, 2002 genta
	void GetDefaultIcon( HICON& hIconBig, HICON& hIconSmall ) const;
	bool GetRelatedIcon(const char* szFile, HICON& hIconBig, HICON& hIconSmall) const;

	void ChangeFileNameNotify( const char *pszTabCaption, const char *pszFilePath, BOOL m_bIsGrep );	//ファイル名変更通知	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加

	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能をCEditWndより移管
	void InitMenubarMessageFont(void);
	void PrintMenubarMessage( const char* msg );
	//	Dec. 4, 2002 genta 実体をCEditViewから移動
	void SendStatusMessage( const char* msg );
	//	Jul. 9, 2005 genta メニューバー右端には出したくない長めのメッセージを出す
	void SendStatusMessage2( const char* msg );
	/*!	SendStatusMessage2()が効き目があるかを予めチェック
		@date 2005.07.09 genta
		@note もしSendStatusMessage2()でステータスバー表示以外の処理を追加
		する場合にはここを変更しないと新しい場所への出力が行われない．
		
		@sa SendStatusMessage2
	*/
	bool SendStatusMessage2IsEffective(void) const {
		return NULL != m_hwndStatusBar;
	}
	void WindowTopMost( int ); // 2004.09.21 Moca

	void SetFocusSearchBox( void ) const;			/* ツールバー検索ボックスへフォーカスを移動 */	// 2006.06.04 yukihane

//	void MyAppendMenu( HMENU, int, int, char* );	/* メニュー項目を追加 */
//#ifdef _DEBUG
	void SetDebugModeON( void );	/* デバッグモニタモードに設定 */
	// 2005.06.24 Moca
	void SetDebugModeOFF( void );
//#endif
	LRESULT PopupWinList( BOOL bFull, bool fromMouse );	/*!< ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon
	LRESULT WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull );	/*!< ウィンドウ一覧メニュー作成処理 */	// 2006.03.23 fon

	/*
	|| スタティックなメンバ関数
	*/
	static int IsFuncEnable( CEditDoc*, DLLSHAREDATA*, int );	/* 機能が利用可能か調べる */
	static int IsFuncChecked( CEditDoc*, DLLSHAREDATA*, int );	/* 機能がチェック状態か調べる */

	static void OnHelp_MenuItem( HWND, int );	/* メニューアイテムに対応するヘルプを表示 */
//	static int FuncID_To_HelpContextID( int );	/* 機能IDに対応するメニューコンテキスト番号を返す */

	/*
	|| メンバ変数
	*/
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	CEditDoc		m_cEditDoc;
	HWND			m_hwndParent;
    HWND			m_hwndReBar;	// Rebar ウィンドウ	//@@@ 2006.06.17 ryoji
    HWND			m_hwndToolBar;
	HWND			m_hwndStatusBar;
	HWND			m_hwndProgressBar;
	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより
	//	変数を移動
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;
//@@@ 2002.01.14 YAZAKI 不使用のため
//	HBITMAP			m_hbmpOPENED;
//	HBITMAP			m_hbmpOPENED_THIS;
	CFuncKeyWnd		m_CFuncKeyWnd;
	CTabWnd			m_cTabWnd;		//タブウインドウ	//@@@ 2003.05.31 MIK
	CMenuDrawer		m_CMenuDrawer;
	int				m_nWinSizeType;	/* サイズ変更のタイプ */
	//	うまくやれば、以下はPrintPreviewへ行きそう。
	BOOL			m_bDragMode;
	int				m_nDragPosOrgX;
	int				m_nDragPosOrgY;
	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより
	//	変数を移動
//	HANDLE			m_hThread;

//	int				m_nChildArrNum;
//	HWND			m_hwndChildArr[32];


	/* 印刷プレビュー表示情報 */
	//	必要になったとき（プレビューコマンドを選んだとき）に生成し、必要なくなったら（プレビューコマンドを終了するときに）破棄すること。
	CPrintPreview*	m_pPrintPreview;
	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより
	//	変数を移動
	//	うまくやれば、以下もPrintPreviewへ行きそう
	HDC				m_hdcCompatDC;	/* 再描画用コンパチブルＤＣ */
	HBITMAP			m_hbmpCompatBMP;	/* 再描画用メモリＢＭＰ */
	HBITMAP			m_hbmpCompatBMPOld;	/* 再描画用メモリＢＭＰ(OLD) */

	//	Oct. 12, 2000 genta
	CImageListMgr	m_cIcons;	//	Image List
	
	/*
	|| 実装ヘルパ系
	*/
protected:
	void OnDropFiles( HDROP );	/* ファイルがドロップされた */
	//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことにより
	//	メソッドを移動
//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
public:
	BOOL OnPrintPageSetting( void );/* 印刷ページ設定 */

private:
	UINT	m_uMSIMEReconvertMsg;
	UINT	m_uATOKReconvertMsg;

//by 鬼
private:
	enum {icNone, icDown, icClicked, icDoubleClicked} m_IconClicked;
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);

	int	CreateFileDropDownMenu( HWND );	//開く(ドロップダウン)	//@@@ 2002.06.15 MIK
	HWND	m_hwndSearchBox;
	HFONT	m_fontSearchBox;
	void	ProcSearchBox( MSG* );	//検索(ボックス)
	int		m_nCurrentFocus;
	
	//	Jul. 21, 2003 genta ToolBarのOwner Draw
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	//	Dec. 4, 2002 genta
	//	メニューバーへのメッセージ表示機能をCEditWndより移管
	HFONT		m_hFontCaretPosInfo;	/*!< キャレットの行桁位置表示用フォント */
	int			m_nCaretPosInfoCharWidth;	/*!< キャレットの行桁位置表示用フォントの幅 */
	int			m_nCaretPosInfoCharHeight;	/*!< キャレットの行桁位置表示用フォントの高さ */
	int			m_pnCaretPosInfoDx[64];	/* 文字列描画用文字幅配列 */

	int m_nTimerCount; //!< OnTimer用 2003.08.29 wmlhq

	void Timer_ONOFF( BOOL ); /* 更新の開始／停止 20060128 aroka */
public:
	void OnSysMenuTimer();
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */


/*[EOF]*/
