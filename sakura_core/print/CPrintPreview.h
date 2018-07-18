/*!	@file
	@brief 印刷プレビュー管理クラス

	@author YAZAKI
	@date 2002/1/11 新規作成
*/
/*
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2003, かろと

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

#ifndef SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_
#define SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_

#include <Windows.h> // 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "util/design_template.h"
#include "CPrint.h" // 2002/2/10 aroka

class CColorStrategy;
class CColorStrategyPool;
class CDlgCancel;
class CEditWnd;
class CLayout;
class CLayoutMgr;

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
	void OnChangeSetting();
	void OnChangePrintSetting( void );
	void OnPreviewGoPage( int nPage );	/* プレビュー ページ指定 */
	void OnPreviewGoPreviousPage(){ OnPreviewGoPage( m_nCurPageNum - 1 ); }		//	前のページへ
	void OnPreviewGoNextPage(){ OnPreviewGoPage( m_nCurPageNum + 1 ); }		//	前のページへ
	void OnPreviewGoDirectPage( void );
	void OnPreviewZoom( BOOL bZoomUp );
	void OnPrint( void );	/* 印刷実行 */
	BOOL OnPrintPageSetting( void );
	void OnCheckAntialias( void );

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
	HWND GetPrintPreviewBarHANDLE_Safe() const{ if(!this)return NULL; else return m_hwndPrintPreviewBar; } //!< thisがNULLでも実行できる版。2007.10.29 kobake
	
	//	PrintPreviewバーのメッセージ処理。
	//	まずPrintPreviewBar_DlgProcにメッセージが届き、DispatchEvent_PPBに転送する仕組み
	static INT_PTR CALLBACK PrintPreviewBar_DlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	INT_PTR DispatchEvent_PPB(
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
	void DrawHeaderFooter( HDC hdc, const CMyRect& rect , bool bHeader );
	CColorStrategy* DrawPageTextFirst( int nPageNum );
	CColorStrategy* DrawPageText( HDC, int, int, int nPageNum, CDlgCancel*, CColorStrategy* pStrategyStart );

	// 印刷／プレビュー 行描画
	CColorStrategy* Print_DrawLine(
		HDC				hdc,
		POINT			ptDraw,		//!< 描画座標。HDC内部単位。
		const wchar_t*	pLine,
		int				nDocLineLen,
		int				nLineStart,
		int				nLineLen,
		CLayoutInt		nIndent,	//!< 折り返しインデント桁数 // 2006.08.14 Moca
		const CLayout*	pcLayout = NULL,	//!< 色付用Layout
		CColorStrategy*	pStrategyStart = NULL
	);

	// 印刷／プレビュー ブロック描画
	void Print_DrawBlock(
		HDC				hdc,
		POINT			ptDraw,		//!< 描画座標。HDC内部単位。
		const wchar_t*	pPhysicalLine,
		int				nBlockLen,
		int				nKind,		//< 0:半角, 1:全角
		const CLayout*	pcLayout,	//!< 色設定用Layout
		int				nColorIndex,
		int				nBgnPhysical,
		CLayoutInt		nLayoutX,
		int				nDx,
		const int*		pDxArray
	);

	// 指定ロジック位置のCColorStrategyを取得
	CColorStrategy* GetColorStrategy(
		const CStringRef&	cStringLine,
		int					iLogic,
		CColorStrategy*		pStrategy,
		bool&				bChange
	);

	// 印刷用フォントを作成する
	void CreateFonts(HDC hdc);
	// 印刷用フォントを破棄する
	void DestroyFonts();

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
	void SetPrintSetting( PRINTSETTING* pPrintSetting ){
		m_sPrintSetting = *pPrintSetting;
		m_pPrintSetting = &m_sPrintSetting;
		m_pPrintSettingOrg = pPrintSetting;
	}
	BOOL GetDefaultPrinterInfo(){ return m_cPrint.GetDefaultPrinter( &m_pPrintSetting->m_mdmDevMode ); }
	int  GetCurPageNum(){ return m_nCurPageNum; }	/* 現在のページ */
	int  GetAllPageNum(){ return m_nAllPageNum; }	/* 現在のページ */

	
	/*
	||	ヘッダ・フッタ
	*/
	void SetHeader(char* pszWork[]);	//	&fなどを登録
	void SetFooter(char* pszWork[]);	//	&p/&Pなどを登録

protected:
	void SetPreviewFontHan( const LOGFONT* lf );
	void SetPreviewFontZen( const LOGFONT* lf );

/* メンバ変数宣言 */
public:
	/* none */

protected:
	CEditWnd*		m_pParentWnd;	//	親のCEditDoc*。

	// 2006.08.17 Moca YAZAKIさんのメモの通りDC/BMPをCEditDocからCPrintPreviewへ移動
	HDC				m_hdcCompatDC;	//!< 再描画用コンパチブルDC
	HBITMAP			m_hbmpCompatBMP;	//!< 再描画用メモリBMP
	HBITMAP			m_hbmpCompatBMPOld;	//!< 再描画用メモリBMP(OLD)
	int				m_nbmpCompatScale;	//!< BMPの画面の10(COMPAT_BMP_BASE)ピクセル幅あたりのBMPのピクセル幅

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
	short			m_nPreview_PaperAllWidth;	/* 用紙幅(1/10mm単位) */
	short			m_nPreview_PaperAllHeight;	/* 用紙高さ(1/10mm単位) */
	short			m_nPreview_PaperWidth;		/* 用紙印刷有効幅(1/10mm単位) */
	short			m_nPreview_PaperHeight;		/* 用紙印刷有効高さ(1/10mm単位) */
	short			m_nPreview_PaperOffsetLeft;	/* 用紙余白左端(1/10mm単位) */
	short			m_nPreview_PaperOffsetTop;	/* 用紙余白上端(1/10mm単位) */
	CKetaXInt		m_bPreview_EnableColumns;	/* 印字可能桁数/ページ */
	int				m_bPreview_EnableLines;		/* 印字可能行数/ページ */
	int				m_nPreview_LineNumberColumns;	/* 行番号エリアの幅（文字数） */
	WORD			m_nAllPageNum;				/* 全ページ数 */
	WORD			m_nCurPageNum;				/* 現在のページ */

	PRINTSETTING*	m_pPrintSetting;			/* 現在の印刷設定(キャッシュへのポインタ) */
	PRINTSETTING*	m_pPrintSettingOrg;			/* 現在の印刷設定(共有データ) */
	PRINTSETTING	m_sPrintSetting;			/* 現在の印刷設定(キャッシュ) */
	LOGFONT			m_lfPreviewHan;				/* プレビュー用フォント */
	LOGFONT			m_lfPreviewZen;				/* プレビュー用フォント */

	HFONT			m_hFontHan;					// 印刷用半角フォントハンドル
	HFONT			m_hFontHan_b;				// 印刷用半角フォントハンドル 太字
	HFONT			m_hFontHan_u;				// 印刷用半角フォントハンドル 下線
	HFONT			m_hFontHan_bu;				// 印刷用半角フォントハンドル 太字、下線
	HFONT			m_hFontZen;					// 印刷用全角フォントハンドル
	HFONT			m_hFontZen_b;				// 印刷用全角フォントハンドル 太字
	HFONT			m_hFontZen_u;				// 印刷用全角フォントハンドル 下線
	HFONT			m_hFontZen_bu;				// 印刷用全角フォントハンドル 太字、下線
	int				m_nAscentHan;				// 半角文字のアセント（文字高/基準ラインからの高さ）
	int				m_nAscentZen;				// 全角文字のアセント（文字高/基準ラインからの高さ）

	CColorStrategyPool*	m_pool;					// 色定義管理情報

public:
	class CLayoutMgr*	m_pLayoutMgr_Print;		/* 印刷用のレイアウト管理情報 */
protected:
	STypeConfig m_typePrint;

	// プレビューから出ても現在のプリンタ情報を記憶しておけるようにstaticにする 2003.05.02 かろと 
	static CPrint	m_cPrint;					//!< 現在のプリンタ情報

	bool			m_bLockSetting;				// 設定のロック
	bool			m_bDemandUpdateSetting;		// 設定の更新要求

private:
	DISALLOW_COPY_AND_ASSIGN(CPrintPreview);
};

#endif /* SAKURA_CPRINTPREVIEW_4FBD8BE8_4E93_4714_A3F2_F69081A2EDBDR_H_ */
