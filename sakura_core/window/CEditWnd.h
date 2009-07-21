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
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include <shellapi.h>// HDROP
#include "doc/CEditDoc.h"
#include "env/CShareData.h"
#include "func/CFuncKeyWnd.h"
#include "CTabWnd.h"	//@@@ 2003.05.31 MIK
#include "CMenuDrawer.h"
#include "CMainToolBar.h"
#include "CMainStatusBar.h"
#include "view/CEditView.h"
#include "window/CSplitterWnd.h"
#include "dlg/CDlgFind.h"
#include "dlg/CDlgReplace.h"
#include "dlg/CDlgJump.h"
#include "dlg/CDlgGrep.h"
#include "outline/CDlgFuncList.h"
#include "dlg/CDlgOpenFile.h"
#include "CHokanMgr.h"
#include "util/design_template.h"
#include "doc/CDocListener.h"

//by 鬼
#include"CDropTarget.h"

const static int MENUBAR_MESSAGE_MAX_LEN = 30;

//@@@ 2002.01.14 YAZAKI 印刷プレビューをCPrintPreviewに独立させたことによる変更
class CPrintPreview;// 2002/2/10 aroka
class CDropTarget;


//メインウィンドウ内コントロールID
#define IDT_EDIT		455  // 20060128 aroka
#define IDT_TOOLBAR		456
#define IDT_CAPTION		457
#define IDT_FIRST_IDLE	458
#define IDT_SYSMENU		1357
#define ID_TOOLBAR		100

struct STabGroupInfo{
	HWND			hwndTop;
	WINDOWPLACEMENT	wpTop;

	STabGroupInfo() : hwndTop(NULL) { }
	bool IsValid() const{ return hwndTop!=NULL; }
};

//! 編集ウィンドウ（外枠）管理クラス
// 2002.02.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
// 2007.10.30 kobake IsFuncEnable,IsFuncCheckedをFunccode.hに移動
// 2007.10.30 kobake OnHelp_MenuItemをCEditAppに移動
class SAKURA_CORE_API CEditWnd
: public TSingleInstance<CEditWnd> //###
, public CDocListenerEx
{
public:
	/*
	||  Constructors
	*/
	CEditWnd();
	~CEditWnd();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           作成                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//	Mar. 7, 2002 genta 文書タイプ用引数追加
	// 2007.06.26 ryoji グループ指定引数追加
	//! 作成
	HWND Create(
		int				nGroup				//!< [in] グループID
	);
	void _GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(CMyRect* rcResult, int nGroup, const STabGroupInfo& sTabGroupInfo);	//!< ウィンドウ生成用の矩形を取得
	HWND _CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo);
	void _AdjustInMonitor(const STabGroupInfo& sTabGroupInfo);

	void OpenDocumentWhenStart(
		const SLoadInfo& sLoadInfo		//!< [in]
	);

	void SetDocumentTypeWhenCreate(
		ECodeType		nCharCode,							//!< [in] 漢字コード
		bool			bViewMode,							//!< [in] ビューモードで開くかどうか
		CTypeConfig	nDocumentType = CTypeConfig(-1)	//!< [in] 文書タイプ．-1のとき強制指定無し．
	);
	void UpdateCaption();
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//ドキュメントイベント
	void OnAfterSave(const SSaveInfo& sSaveInfo);

	//管理
	void MessageLoop( void );								/* メッセージループ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */

	//各種イベント
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* 描画処理 */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE 処理 */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	BOOL DoMouseWheel( WPARAM wParam, LPARAM lParam );	// マウスホイール処理	// 2007.10.16 ryoji
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );
	int	OnClose();	/* 終了時の処理 */
	void OnDropFiles( HDROP );	/* ファイルがドロップされた */
	BOOL OnPrintPageSetting( void );/* 印刷ページ設定 */
	LRESULT OnTimer( WPARAM, LPARAM );	// WM_TIMER 処理	// 2007.04.03 ryoji
	void OnEditTimer( void );	/* タイマーの処理 */
	void OnCaptionTimer( void );
	void OnSysMenuTimer( void );
	void OnCommand( WORD, WORD , HWND );
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           通知                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//ファイル名変更通知
	void ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep );	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         メニュー                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void InitMenu( HMENU, UINT, BOOL );
	void InitMenubarMessageFont(void);	//	メニューバーへのメッセージ表示機能をCEditWndより移管	//	Dec. 4, 2002 genta
	LRESULT WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull );	/*!< ウィンドウ一覧メニュー作成処理 */	// 2006.03.23 fon
	LRESULT PopupWinList( bool bMousePos );	/*!< ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon	// 2007.02.28 ryoji フルパス指定のパラメータを削除


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           整形                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void LayoutToolBar( void );			/* ツールバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutFuncKey( void );			/* ファンクションキーの配置処理 */	// 2006.12.19 ryoji
	void LayoutTabBar( void );			/* タブバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutStatusBar( void );		/* ステータスバーの配置処理 */		// 2006.12.19 ryoji
	void EndLayoutBars( BOOL bAdjust = TRUE );	/* バーの配置終了処理 */	// 2006.12.19 ryoji


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           設定                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintPreviewModeONOFF( void );	/* 印刷プレビューモードのオン/オフ */
	
	//アイコン
	void SetWindowIcon( HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel( BOOL bState ) { m_bPageScrollByWheel = bState; };	// ホイール操作によるページスクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.17 nasukoji
	void SetHScrollByWheel( BOOL bState ) { m_bHorizontalScrollByWheel = bState; };	// ホイール操作による横スクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.17 nasukoji
	void ClearMouseState( void );		// 2009.01.17 nasukoji	マウスの状態をクリアする（ホイールスクロール有無状態をクリア）


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           情報                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	//! 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	BOOL IsActiveApp() const { return m_bIsActiveApp; };

	//!ツールチップのテキストを取得。2007.09.08 kobake 追加
	void GetTooltipText(TCHAR* wszBuf, size_t nBufCount, int nID) const;

	//!印刷プレビュー中かどうか
	bool IsInPreviewMode()
	{
		return m_pPrintPreview!=NULL;
	}

	BOOL IsPageScrollByWheel() const { return m_bPageScrollByWheel; };		// ホイール操作によるページスクロール有無	// 2009.01.17 nasukoji
	BOOL IsHScrollByWheel() const { return m_bHorizontalScrollByWheel; };	// ホイール操作による横スクロール有無		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           表示                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void PrintMenubarMessage( const TCHAR* msg );
	void SendStatusMessage( const TCHAR* msg );		//	Dec. 4, 2002 genta 実体をCEditViewから移動


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      ウィンドウ操作                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	void WindowTopMost( int ); // 2004.09.21 Moca

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        ビュー管理                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void Views_RedrawAll();
	void Views_Redraw();
	void SetActivePane( int );	/* アクティブなペインを設定 */
	int GetActivePane( void ) const;	/* アクティブなペインを取得 */ //2007.08.26 kobake const追加
	void SetDrawSwitchOfAllViews( bool bDraw );					/* すべてのペインの描画スイッチを設定する */	// 2008.06.08 ryoji
	void RedrawAllViews( CEditView* pcViewExclude );				/* すべてのペインをRedrawする */
	void Views_DisableSelectArea(bool bRedraw);
	BOOL DetectWidthOfLineNumberAreaAllPane( bool bRedraw );	/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
	BOOL WrapWindowWidth( int nPane );	/* 右端で折り返す */	// 2008.06.08 ryoji
	BOOL UpdateTextWrap( void );		/* 折り返し方法関連の更新 */	// 2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB幅と折り返し位置の更新
	void ChangeLayoutParam( bool bShowProgress, CLayoutInt nTabSize, CLayoutInt nMaxLineKetas );
	//	Aug. 14, 2005 genta
	CLogicPoint* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView( CLogicPoint* pptPosArray );
	// 互換BMPによる画面バッファ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); //!< CEditViewの画面バッファを削除


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       各種アクセサ                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	HWND			GetHwnd()		const	{ return this?m_hWnd:NULL; }
	CMenuDrawer&	GetMenuDrawer()			{ return m_CMenuDrawer; }
	CEditDoc&		GetDocument();
	const CEditDoc&	GetDocument() const;

	//ビュー
	const CEditView&	GetActiveView() const { return *m_pcEditViewArr[m_nActivePaneIndex]; }
	CEditView&			GetActiveView()       { return *m_pcEditViewArr[m_nActivePaneIndex]; }
	CEditView*			GetDragSourceView() const					{ return m_pcDragSourceView; }
	void				SetDragSourceView( CEditView* pcDragSourceView )	{ m_pcDragSourceView = pcDragSourceView; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         実装補助                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//by 鬼
protected:
	enum EIconClickStatus{
		icNone,
		icDown,
		icClicked,
		icDoubleClicked
	};

protected:
	//ドロップダウンメニュー
	int	CreateFileDropDownMenu( HWND );	//開く(ドロップダウン)	//@@@ 2002.06.15 MIK

	//タイマー
	void Timer_ONOFF( BOOL ); /* 更新の開始／停止 20060128 aroka */

public:
	//! 周期内でm_nTimerCountをインクリメント
	void IncrementTimerCount(int nInterval)
	{
		m_nTimerCount++;
		if( nInterval < m_nTimerCount ){
			m_nTimerCount = 0;
		}
	}

	//D&Dフラグ管理
	void SetDragPosOrg(CMyPoint ptDragPosOrg){ m_ptDragPosOrg=ptDragPosOrg; }
	void SetDragMode(bool bDragMode){ m_bDragMode = bDragMode; }
	bool GetDragMode() const{ return m_bDragMode; }
	const CMyPoint& GetDragPosOrg() const{ return m_ptDragPosOrg; }

	/* IDropTarget実装 */	// 2008.06.20 ryoji
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );

	//フォーカス管理
	int GetCurrentFocus() const{ return m_nCurrentFocus; }
	void SetCurrentFocus(int n){ m_nCurrentFocus = n; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
private:
	//自ウィンドウ
	HWND			m_hWnd;

	//親ウィンドウ
	HWND			m_hwndParent;

public:
	//子ウィンドウ
	CMainToolBar	m_cToolbar;			//!< ツールバー
	CTabWnd			m_cTabWnd;			//!< タブウインドウ	//@@@ 2003.05.31 MIK
	CFuncKeyWnd		m_CFuncKeyWnd;		//!< ファンクションバー
	CMainStatusBar	m_cStatusBar;		//!< ステータスバー
	CPrintPreview*	m_pPrintPreview;	//!< 印刷プレビュー表示情報。必要になったときのみインスタンスを生成する。

	CSplitterWnd	m_cSplitterWnd;		//!< 分割フレーム
	CEditView*		m_pcEditViewArr[4];	//!< ビュー
	int				m_nActivePaneIndex;	//!< アクティブなビュー
	CEditView*		m_pcDragSourceView;	//!< ドラッグ元のビュー

	//ダイアログ達
	CDlgFind		m_cDlgFind;			// 「検索」ダイアログ
	CDlgReplace		m_cDlgReplace;		// 「置換」ダイアログ
	CDlgJump		m_cDlgJump;			// 「指定行へジャンプ」ダイアログ
	CDlgGrep		m_cDlgGrep;			// Grepダイアログ
	CDlgFuncList	m_cDlgFuncList;		// アウトライン解析結果ダイアログ
	CDlgOpenFile	m_cDlgOpenFile;		// ファイルオープンダイアログ
	CHokanMgr		m_cHokanMgr;		// 入力補完

private:
	//共有データ
	DLLSHAREDATA*	m_pShareData;

	//ヘルパ
	CMenuDrawer		m_CMenuDrawer;

	//メッセージID
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;

	//状態
	bool			m_bIsActiveApp;		//!< 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	LPTSTR			m_pszLastCaption;
	LPTSTR			m_pszMenubarMessage; //!< メニューバー右端に表示するメッセージ
	int				m_nTimerCount;		//!< OnTimer用 2003.08.29 wmlhq
	int				m_nCurrentFocus;	//!< 現在のフォーカス情報
	int				m_nWinSizeType;		//!< サイズ変更のタイプ。SIZE_MAXIMIZED, SIZE_MINIMIZED 等。
	BOOL			m_bPageScrollByWheel;		//!< ホイール操作によるページスクロールあり	// 2009.01.17 nasukoji
	BOOL			m_bHorizontalScrollByWheel;	//!< ホイール操作による横スクロールあり		// 2009.01.17 nasukoji

	//フォント・イメージ
	HFONT			m_hFontCaretPosInfo;		//!< キャレットの行桁位置表示用フォント
	int				m_nCaretPosInfoCharWidth;	//!< キャレットの行桁位置表示用フォントの幅
	int				m_nCaretPosInfoCharHeight;	//!< キャレットの行桁位置表示用フォントの高さ

	//D&Dフラグ
	bool			m_bDragMode;
	CMyPoint		m_ptDragPosOrg;
	CDropTarget*	m_pcDropTarget;

	//その他フラグ
	BOOL				m_bUIPI;		// エディタ－トレイ間でのUI特権分離確認用フラグ	// 2007.06.07 ryoji
	EIconClickStatus	m_IconClicked;

public:
	ESelectCountMode	m_nSelectCountMode; // 選択文字カウント方法

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */





