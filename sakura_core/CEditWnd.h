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
	Copyright (C) 2006, ryoji, aroka, fon, yukihane. Moca
	Copyright (C) 2007, ryoji
	Copyright (C) 2008, nasukoji, ryoji
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include "CEditDoc.h"
#include "CShareData.h"
#include "CTabWnd.h"	//@@@ 2003.05.31 MIK
#include "CFuncKeyWnd.h"
#include "CMenuDrawer.h"
#include "CImageListMgr.h"

//by 鬼
#include "CDropTarget.h"
#include "CViewFont.h"

static const int MENUBAR_MESSAGE_MAX_LEN = 30;

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
//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CEditWnd
{
public:
	static CEditWnd* getInstance()
	{
		static CEditWnd instance;

		return &instance;
	}

private:
	CEditWnd();
	CEditWnd(CEditWnd const&);
	void operator=(CEditWnd const&);

public:
	~CEditWnd();
	/*
	|| メンバ関数
	*/
	//	Mar. 7, 2002 genta 文書タイプ用引数追加
	// 2007.06.26 ryoji グループ指定引数追加
	//! 作成
	HWND Create(
		HINSTANCE,
		HWND,
		CEditDoc*		pcEditDoc,
		CImageListMgr*	pcIcons,
		int nGroup
	);
	void _GetTabGroupInfo(STabGroupInfo* pTabGroupInfo, int& nGroup);
	void _GetWindowRectForInit(int& nWinOX, int& nWinOY, int& nWinCX, int& nWinCY, int nGroup, const STabGroupInfo& sTabGroupInfo);
	HWND _CreateMainWindow(int nGroup, const STabGroupInfo& sTabGroupInfo);
	void _AdjustInMonitor(const STabGroupInfo& sTabGroupInfo);

	void OpenDocumentWhenStart(const char*, ECodeType, bool);	//!< [in] 最初に開くファイルのパス．NULLのとき開くファイル無し．
	void SetDocumentTypeWhenCreate(ECodeType, bool, int = -1);	//!< [in] 文書タイプ．-1のとき強制指定無し．

	void UpdateCaption();

	void MessageLoop( void );								/* メッセージループ */
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */


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
	void OnToolbarTimer( void );	/* タイマーの処理 20060128 aroka */
	void UpdateToolbar( void );		// ツールバーの表示を更新する		// 2008.09.23 nasukoji
	void OnCaptionTimer( void );
	void OnSysMenuTimer( void );
	void OnCommand( WORD, WORD , HWND );
	LRESULT OnNcLButtonDown(WPARAM, LPARAM);
	LRESULT OnNcLButtonUp(WPARAM, LPARAM);
	LRESULT OnLButtonDblClk(WPARAM, LPARAM);

	void CreateToolBar( void );			/* ツールバー作成 */
	void DestroyToolBar( void );		/* ツールバー破棄 */
	void CreateStatusBar( void );		/* ステータスバー作成 */
	void DestroyStatusBar( void );		/* ステータスバー破棄 */
	void AcceptSharedSearchKey();		/* 検索ボックスを更新 */

	//ファイル名変更通知
	void ChangeFileNameNotify( const TCHAR* pszTabCaption, const TCHAR* pszFilePath, bool bIsGrep );	//@@@ 2003.05.31 MIK, 2006.01.28 ryoji ファイル名、Grepモードパラメータを追加
	void InitMenu( HMENU, UINT, BOOL );
	void InitMenubarMessageFont(void);
	LRESULT WinListMenu( HMENU hMenu, EditNode* pEditNodeArr, int nRowNum, BOOL bFull );	/*!< ウィンドウ一覧メニュー作成処理 */	// 2006.03.23 fon
	LRESULT PopupWinList( bool bMousePos );	/*!< ウィンドウ一覧ポップアップ表示処理 */	// 2006.03.23 fon	// 2007.02.28 ryoji フルパス指定のパラメータを削除
	void SetMenuFuncSel( HMENU hMenu, int nFunc, const TCHAR* sKey, bool flag );				// 表示の動的選択	2010/5/19 Uchi
	void LayoutToolBar( void );			/* ツールバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutFuncKey( void );			/* ファンクションキーの配置処理 */	// 2006.12.19 ryoji
	void LayoutTabBar( void );			/* タブバーの配置処理 */			// 2006.12.19 ryoji
	void LayoutStatusBar( void );		/* ステータスバーの配置処理 */		// 2006.12.19 ryoji
	void EndLayoutBars( BOOL bAdjust = TRUE );	/* バーの配置終了処理 */	// 2006.12.19 ryoji


	void PrintPreviewModeONOFF( void );	/* 印刷プレビューモードのオン/オフ */
	void SetWindowIcon( HICON, int);	//	Sep. 10, 2002 genta
	void GetDefaultIcon( HICON* hIconBig, HICON* hIconSmall ) const;	//	Sep. 10, 2002 genta
	bool GetRelatedIcon(const TCHAR* szFile, HICON* hIconBig, HICON* hIconSmall) const;	//	Sep. 10, 2002 genta
	void SetPageScrollByWheel( BOOL bState ) { m_bPageScrollByWheel = bState; }	// ホイール操作によるページスクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.12 nasukoji
	void SetHScrollByWheel( BOOL bState ) { m_bHorizontalScrollByWheel = bState; }	// ホイール操作による横スクロール有無を設定する（TRUE=あり, FALSE=なし）	// 2009.01.12 nasukoji
	void ClearMouseState( void );		// 2009.01.12 nasukoji	マウスの状態をクリアする（ホイールスクロール有無状態をクリア）
	//! 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	bool IsActiveApp() const { return m_bIsActiveApp; }



	BOOL IsPageScrollByWheel() const { return m_bPageScrollByWheel; }		// ホイール操作によるページスクロール有無	// 2009.01.12 nasukoji
	BOOL IsHScrollByWheel() const { return m_bHorizontalScrollByWheel; }	// ホイール操作による横スクロール有無		// 2009.01.12 nasukoji

	void PrintMenubarMessage( const TCHAR* msg );
	void SendStatusMessage( const TCHAR* msg );		//	Dec. 4, 2002 genta 実体をCEditViewから移動
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

	LRESULT Views_DispatchEvent(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	bool CreateEditViewBySplit(int);
	void InitAllViews();
	void Views_Redraw();
	void SetActivePane( int );	/* アクティブなペインを設定 */
	int GetActivePane( void ) const { return m_nActivePaneIndex; }	/* アクティブなペインを取得 */ //2007.08.26 kobake const追加
	void SetDrawSwitchOfAllViews( bool bDraw );					/* すべてのペインの描画スイッチを設定する */	// 2008.06.08 ryoji
	void RedrawAllViews( CEditView* pcViewExclude );				/* すべてのペインをRedrawする */
	BOOL DetectWidthOfLineNumberAreaAllPane( bool bRedraw );	/* すべてのペインで、行番号表示に必要な幅を再設定する（必要なら再描画する） */
	BOOL WrapWindowWidth( int nPane );	/* 右端で折り返す */	// 2008.06.08 ryoji
	BOOL UpdateTextWrap( void );		/* 折り返し方法関連の更新 */	// 2008.06.10 ryoji
	//	Aug. 14, 2005 genta TAB幅と折り返し位置の更新
	void ChangeLayoutParam( bool bShowProgress, int nTabSize, int nMaxLineKetas );
	//	Aug. 14, 2005 genta
	CLogicPoint* SavePhysPosOfAllView();
	void RestorePhysPosOfAllView( CLogicPoint* posary );
	// 互換BMPによる画面バッファ 2007.09.09 Moca
	void Views_DeleteCompatibleBitmap(); //!< CEditViewの画面バッファを削除

	void SetFocusSearchBox( void ) const;			/* ツールバー検索ボックスへフォーカスを移動 */	// 2006.06.04 yukihane
	void SetDebugModeON( void );	/* デバッグモニタモードに設定 */
	void SetDebugModeOFF( void );

	const CEditView& GetActiveView() const { return *m_pcEditView; }
	CEditView& GetActiveView() { return *m_pcEditView; }
	bool IsEnablePane(int n) const { return 0 <= n && n < m_nEditViewCount; }
	int	GetAllViewCount() const { return m_nEditViewCount; }

	CEditView* GetDragSourceView() const { return m_pcDragSourceView; }
	void SetDragSourceView( CEditView* pcDragSourceView ) { m_pcDragSourceView = pcDragSourceView; }

	enum EIconClickStatus{
		icNone,
		icDown,
		icClicked,
		icDoubleClicked
	};
	int	CreateFileDropDownMenu( HWND );	//開く(ドロップダウン)	//@@@ 2002.06.15 MIK
	void Timer_ONOFF( bool ); /* 更新の開始／停止 20060128 aroka */

	//! 周期内でm_nTimerCountをインクリメント
	void IncrementTimerCount(int nInterval)
	{
		m_nTimerCount++;
		if( nInterval <= m_nTimerCount ){ // 2012.11.29 aroka 呼び出し間隔のバグ修正
			m_nTimerCount = 0;
		}
	}

	void CreateAccelTbl( void ); // ウィンドウ毎のアクセラレータテーブル作成(Wine用)
	void DeleteAccelTbl( void ); // ウィンドウ毎のアクセラレータテーブル破棄(Wine用)

	/* IDropTarget実装 */	// 2008.06.20 ryoji
	STDMETHODIMP DragEnter( LPDATAOBJECT, DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragOver( DWORD, POINTL, LPDWORD );
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop( LPDATAOBJECT, DWORD, POINTL, LPDWORD );

	void	ProcSearchBox( MSG* );	//検索(ボックス)

	//	Jul. 21, 2003 genta ToolBarのOwner Draw
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	/*
	|| メンバ変数
	*/
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	CEditDoc*		m_pcEditDoc;
	HWND			m_hwndParent;
    HWND			m_hwndReBar;	// Rebar ウィンドウ	//@@@ 2006.06.17 ryoji
    HWND			m_hwndToolBar;
	HWND			m_hwndStatusBar;
	HWND			m_hwndProgressBar;
	DLLSHAREDATA*	m_pShareData;

	CFuncKeyWnd		m_CFuncKeyWnd;
	CTabWnd			m_cTabWnd;		//タブウインドウ	//@@@ 2003.05.31 MIK
	CMenuDrawer		m_CMenuDrawer;
	bool			m_bDragMode;
	POINT			m_ptDragPosOrg;
	CDropTarget*	m_pcDropTarget;
	CPrintPreview*	m_pPrintPreview;	//!< 印刷プレビュー表示情報。必要になったときのみインスタンスを生成する。

	CSplitterWnd	m_cSplitterWnd;		//!< 分割フレーム
	CEditView*		m_pcDragSourceView;	//!< ドラッグ元のビュー
	CViewFont*		m_pcViewFont;		//!< フォント

	CImageListMgr*	m_pcIcons;

	CEditView*		m_pcEditViewArr[4];			//!< ビュー 
	CEditView*		m_pcEditView;				//!< 有効なビュー
	int				m_nActivePaneIndex;			//!< 有効なビューのindex
	int				m_nEditViewCount;			//!< 有効なビューの数
	const int		m_nEditViewMaxCount;		//!< ビューの最大数=4

private:
	UINT			m_uMSIMEReconvertMsg;
	UINT			m_uATOKReconvertMsg;

	HWND	m_hwndSearchBox;
	HFONT	m_hFontSearchBox;
	bool	m_bIsActiveApp;	//!< 自アプリがアクティブかどうか	// 2007.03.08 ryoji
	LPTSTR	m_pszLastCaption;
	LPTSTR	m_pszMenubarMessage; //!< メニューバー右端に表示するメッセージ
	int 	m_nTimerCount; //!< OnTimer用 2003.08.29 wmlhq
	int		m_nCurrentFocus;	//!< 現在のフォーカス情報
	int		m_nWinSizeType;	//!< サイズ変更のタイプ
	BOOL	m_bPageScrollByWheel;		//!< ホイール操作によるページスクロールあり	// 2009.01.12 nasukoji
	BOOL	m_bHorizontalScrollByWheel;	//!< ホイール操作による横スクロールあり		// 2009.01.12 nasukoji
	HACCEL	m_hAccelWine;		//!< ウィンドウ毎のアクセラレータテーブルのハンドル(Wine用)	// 2009.08.15 nasukoji
	HACCEL	m_hAccel;			//!< アクセラレータテーブル(共有 or ウィンドウ毎)

	HFONT		m_hFontCaretPosInfo;	//!< キャレットの行桁位置表示用フォント
	int			m_nCaretPosInfoCharWidth;	//!< キャレットの行桁位置表示用フォントの幅
	int			m_nCaretPosInfoCharHeight;	//!< キャレットの行桁位置表示用フォントの高さ
	int			m_pnCaretPosInfoDx[64];	// 文字列描画用文字幅配列

	BOOL				m_bUIPI;		// エディタ－トレイ間でのUI特権分離確認用フラグ	// 2007.06.07 ryoji
	EIconClickStatus	m_IconClicked;

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */


/*[EOF]*/
