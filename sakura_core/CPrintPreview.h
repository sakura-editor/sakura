/*!	@file
	@brief 印刷プレビュー管理クラス

	@author YAZAKI
	@date 2002/1/11 新規作成
	$Revision$
*/
/*
	Copyright (C) 2002, YAZAKI

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CPrintPreview;

#ifndef _CPRINTPREVIEW_H_
#define _CPRINTPREVIEW_H_
#include "CShareData.h"
#include "CPrint.h" // 2002/2/10 aroka
#include <windows.h> // 2002/2/10 aroka

class CPrintPreview {
/* メンバ関数宣言 */
public:
	/*
	||  コンストラクタ
	*/
	CPrintPreview( class CEditWnd* pParentWnd );
	~CPrintPreview();
	
	/*
	||	イベント
	*/
	//	Window Messages
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* 描画処理 */
	LRESULT OnSize( WPARAM, LPARAM );				/* WM_SIZE 処理 */
	LRESULT OnVScroll( WPARAM wParam, LPARAM lParam );
	LRESULT OnHScroll( WPARAM wParam, LPARAM lParam );
	LRESULT OnMouseMove( WPARAM wParam, LPARAM lParam );
	LRESULT OnMouseWheel( WPARAM wParam, LPARAM lParam );

	//	User Messages
	void OnChangePrintSetting( void );
	void OnPreviewGoPage( int nPage );	/* プレビュー ページ指定 */
	void OnPreviewGoPreviousPage(){ OnPreviewGoPage( m_nCurPageNum - 1 ); }		//	前のページへ
	void OnPreviewGoNextPage(){ OnPreviewGoPage( m_nCurPageNum + 1 ); }		//	前のページへ
	void OnPreviewZoom( BOOL bZoomUp );
	void OnPrint( void );	/* 印刷実行 */
	BOOL OnPrintPageSetting( void );

	/*
	||	コントロール
	*/
	//	スクロールバー
	void InitPreviewScrollBar( void );
	
	//	PrintPreviewバー（画面上部のコントロール）
	void CreatePrintPreviewControls( void );
	void DestroyPrintPreviewControls( void );

	void SetFocusToPrintPreviewBar( void );
	HWND GetPrintPreviewBarHANDLE( void ){ return m_hwndPrintPreviewBar;	}
	
	//	PrintPreviewバーのメッセージ処理。
	//	まずPrintPreviewBar_DlgProcにメッセージが届き、DispatchEvent_PPBに転送する仕組み
	static BOOL CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	BOOL DispatchEvent_PPB(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam 		// second message parameter
	);

protected:
	/*
	||	描画。
	||	DrawXXXXX()は、現在のフォントを半角フォントに設定してから呼び出すこと。
	||	また、DrawXXXXX()から抜けてきたときは、半角フォントに設定されていることを期待してよい。
	||	フォントは、半角フォントと全角フォントしかないことも期待してよい。
	*/
	void DrawHeader( HDC hdc, RECT& rect, HFONT hFontZen );
	void DrawPageText( HDC, int, int, int, HFONT hFontZen, class CDlgCancel* );
	void DrawFooter( HDC hdc, RECT& rect, HFONT hFontZen );

	/* 印刷／プレビュー 行描画 */
	void Print_DrawLine(
		HDC			hdc,
		int			x,
		int			y,
		const char*	pLine,
		int			nLineLen,
		HFONT		hFontZen
	);

public:
	//	フォント列挙
	static int CALLBACK MyEnumFontFamProc(
		ENUMLOGFONT*	pelf,		// pointer to logical-font data
		NEWTEXTMETRIC*	pntm,		// pointer to physical-font data
		int				nFontType,	// type of font
		LPARAM			lParam 		// address of application-defined data
	);
	
	/*
	||	アクセサ
	*/
	void SetPrintSetting( PRINTSETTING* pPrintSetting ){ m_pPrintSetting = pPrintSetting; }
	BOOL GetDefaultPrinterInfo(){ return m_cPrint.GetDefaultPrinter( &m_pPrintSetting->m_mdmDevMode ); };
	int  GetCurPageNum(){ return m_nCurPageNum; }	/* 現在のページ */
	int  GetAllPageNum(){ return m_nAllPageNum; }	/* 現在のページ */

	
	/*
	||	ヘッダ・フッタ
	*/
	void SetHeader(char* pszWork[]);	//	&fなどを登録
	void SetFooter(char* pszWork[]);	//	&p/&Pなどを登録

protected:
	void SetPreviewFontHan( LOGFONT* lf );
	void SetPreviewFontZen( LOGFONT* lf );

/* メンバ変数宣言 */
public:
	/* none */

protected:
	CEditWnd*		m_pParentWnd;	//	親のCEditDoc*。
	
	/*	コントロール制御用	*/
	//	操作バー
	HWND			m_hwndPrintPreviewBar;	/* 印刷プレビュー 操作バー */
	//	スクロールバー
	int				m_nPreviewVScrollPos;	/* 印刷プレビュー：スクロール位置縦 */
	int				m_nPreviewHScrollPos;	/* 印刷プレビュー：スクロール位置横 */
	BOOL			m_SCROLLBAR_HORZ;
	BOOL			m_SCROLLBAR_VERT;
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	//	サイズボックス
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	BOOL			m_SizeBoxCanMove;	/* サイズボックスウィンドウハンドルを動かせるかどうか */
	
	//	表示
	int				m_nPreview_Zoom;	/* 印刷プレビュー：倍率 */
	
	//	印刷位置を決定するための変数
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
//	int				m_nPreview_PaperOffsetRight;	/* 用紙余白右端(1/10mm単位) */
//	int				m_nPreview_PaperOffsetBottom;	/* 用紙余白下端(1/10mm単位) */
	int				m_bPreview_EnableColms;	/* 印字可能桁数/ページ */
	int				m_bPreview_EnableLines;	/* 印字可能行数/ページ */
	int				m_nPreview_LineNumberColmns;	/* 行番号エリアの幅（文字数） */
	int				m_nAllPageNum;	/* 全ページ数 */
	int				m_nCurPageNum;	/* 現在のページ */

	PRINTSETTING*	m_pPrintSetting;	/* 現在の印刷設定 */
	LOGFONT			m_lfPreviewHan;	/* プレビュー用フォント */
	LOGFONT			m_lfPreviewZen;	/* プレビュー用フォント */

	class CLayoutMgr*	m_pLayoutMgr_Print;	/* 印刷用のレイアウト管理情報 */

	int				m_pnDx[10240 + 10];	/* 文字列描画用文字幅配列 */
	// プレビューから出ても現在のプリンタ情報を記憶しておけるようにstaticにする 2003.05.02 かろと 
	static CPrint	m_cPrint;		//!< 現在のプリンタ情報
};

#endif