//	$Id$
/*!	@file
	編集ウィンドウ（外枠）管理クラス

	@author Norio Nakatani
	@date 1998/05/13 新規作成
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

#ifndef _CEDITWND_H_
#define _CEDITWND_H_

class CEditWnd;

#include "CEditDoc.h"
#include "CKeyBind.h"
#include "CShareData.h"
#include "CFuncKeyWnd.h"
#include "CMenuDrawer.h"
#include "CImageListMgr.h"




/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
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
	HWND Create( HINSTANCE, HWND, const char*, int, BOOL );	/* 作成 */


	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* メッセージ処理 */
	BOOL DispatchEvent_PPB( HWND, UINT, WPARAM, LPARAM );	/* 印刷プレビュー 操作バー ダイアログのメッセージ処理 */

	void PrintPreviewModeONOFF( void );	/* 印刷プレビューモードのオン/オフ */

	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* 描画処理 */
	LRESULT OnSize( WPARAM, LPARAM );	/* WM_SIZE 処理 */
	LRESULT OnLButtonUp( WPARAM, LPARAM );
	LRESULT OnLButtonDown( WPARAM, LPARAM );
	LRESULT OnMouseMove( WPARAM, LPARAM );
	LRESULT OnMouseWheel( WPARAM, LPARAM );
	LRESULT OnHScroll( WPARAM, LPARAM );
	LRESULT OnVScroll( WPARAM, LPARAM );

	void OnTimer( HWND, UINT, UINT, DWORD );	/* タイマーの処理 */
	void OnCommand( WORD, WORD , HWND );

	void CreateToolBar( void );			/* ツールバー作成 */
	void CreateStatusBar( void );		/* ステータスバー作成 */
	void DestroyStatusBar( void );		/* ステータスバー破棄 */
	void CreatePrintPreviewBar( void );	/* ツールバー作成 */

	void InitMenu( HMENU, UINT, BOOL );
//複数プロセス版
	void MessageLoop( void );	/* メッセージループ */

	int	OnClose( void );	/* 終了時の処理 */


void CEditWnd::ExecCmd(LPCSTR lpszCmd/*, HANDLE hFile*/);


//	void MyAppendMenu( HMENU, int, int, char* );	/* メニュー項目を追加 */
//#ifdef _DEBUG
	void SetDebugModeON( void );	/* デバッグモニタモードに設定 */
//#endif
	/*
	|| スタティックなメンバ関数
	*/
	static int IsFuncEnable( CEditDoc*, DLLSHAREDATA*, int );	/* 機能が利用可能か調べる */
	static int IsFuncChecked( CEditDoc*, DLLSHAREDATA*, int );	/* 機能がチェック状態か調べる */

	static void OnHelp_MenuItem( HWND, int );	/* メニューアイテムに対応するヘルプを表示 */
	static int FuncID_To_HelpContextID( int );	/* 機能IDに対応するメニューコンテキスト番号を返す */

	/*
	|| メンバ変数
	*/
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	CEditDoc		m_cEditDoc;
	HWND			m_hwndParent;
    HWND			m_hwndToolBar;
	HWND			m_hwndStatusBar;
	HWND			m_hwndProgressBar;
	HWND			m_hwndPrintPreviewBar;	/* 印刷プレビュー 操作バー */
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;
	HBITMAP			m_hbmpOPENED;
	HBITMAP			m_hbmpOPENED_THIS;
	CFuncKeyWnd		m_CFuncKeyWnd;
	CMenuDrawer		m_CMenuDrawer;
	int				m_nWinSizeType;	/* サイズ変更のタイプ */
	BOOL			m_bDragMode;
	int				m_nDragPosOrgX;
	int				m_nDragPosOrgY;
	BOOL			m_SCROLLBAR_HORZ;
	BOOL			m_SCROLLBAR_VERT;

//	HANDLE			m_hThread;

//	int				m_nChildArrNum;
//	HWND			m_hwndChildArr[32];


	/* 印刷プレビュー表示情報 */
	int				m_nPreviewVScrollPos;	/* 印刷プレビュー：スクロール位置縦 */
	int				m_nPreviewHScrollPos;	/* 印刷プレビュー：スクロール位置横 */
	int				m_nPreview_Zoom;	/* 印刷プレビュー：倍率 */
	int				m_nPreview_CurPage;	/* 印刷プレビュー：ページ */
	int				m_nPreview_AllPageNum;	/* 印刷プレビュー：全ページ数 */
	int				m_nPreview_ViewWidth;		/* 印刷プレビュー：ビュー幅(ピクセル) */
	int				m_nPreview_ViewHeight;		/* 印刷プレビュー：ビュー高さ(ピクセル) */
	int				m_nPreview_ViewMarginLeft;	/* 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位) */
	int				m_nPreview_ViewMarginTop;	/* 印刷プレビュー：ビュー左端と用紙の間隔(1/10mm単位) */
	int				m_nPreview_PaperAllWidth;	/* 用紙幅(1/10mm単位) */
	int				m_nPreview_PaperAllHeight;	/* 用紙高さ(1/10mm単位) */
	int				m_nPreview_PaperWidth;	/* 用紙印刷有効幅(1/10mm単位) */
	int				m_nPreview_PaperHeight;	/* 用紙印刷有効高さ(1/10mm単位) */
	int				m_nPreview_PaperOffsetLeft;	/* 用紙余白左端(1/10mm単位) */
	int				m_nPreview_PaperOffsetTop;	/* 用紙余白上端(1/10mm単位) */
	int				m_nPreview_PaperOffsetRight;	/* 用紙余白右端(1/10mm単位) */
	int				m_nPreview_PaperOffsetBottom;	/* 用紙余白下端(1/10mm単位) */
	int				m_bPreview_EnableColms;	/* 印字可能桁数/ページ */
	int				m_bPreview_EnableLines;	/* 印字可能行数/ページ */
	int				m_nPreview_LineNumberColmns;	/* 行番号エリアの幅（文字数） */
	int				m_nAllPageNum;	/* 全ページ数 */
	int				m_nCurPageNum;	/* 現在のページ */

	PRINTSETTING*	m_pPrintSetting;	/* 現在の印刷設定 */
	LOGFONT			m_lfPreviewHan;	/* プレビュー用フォント */
	LOGFONT			m_lfPreviewZen;	/* プレビュー用フォント */

	CLayoutMgr		m_CLayoutMgr_Print;	/* 印刷用のレイアウト管理情報 */
	int				m_pnDx[10240 + 10];	/* 文字列描画用文字幅配列 */
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
	void InitPreviewScrollBar( void );	/* 印刷プレビュー スクロールバー初期化 */
	void OnPreviewZoom( BOOL );	/* プレビュー拡大縮小 */
	void OnPreviewGoPage( int );	/* プレビュー ページ指定 */
	/* 印刷／プレビュー 行描画 */
	void CEditWnd::Print_DrawLine(
		HDC			hdc,
		int			x,
		int			y,
		const char*	pLine,
		int			nLineLen,
		HFONT		hFontZen
	);
	/* 印刷/印刷プレビュー ページテキストの描画 */
	void CEditWnd::DrawPageText( HDC, int, int, int, HFONT, CDlgCancel* );
	void OnPrint( void );	/* 印刷実行 */
	void OnChangePrintSetting( void );	/* 印刷設定の反映 */
public:
	BOOL OnPrintPageSetting( void );/* 印刷ページ設定 */

};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITWND_H_ */


/*[EOF]*/
