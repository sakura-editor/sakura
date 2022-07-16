﻿/*!	@file
	@brief 文書ウィンドウの管理

	@author Norio Nakatani
	@date	1998/03/13 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro
	Copyright (C) 2001, asa-o, MIK, hor, Misaka, Stonee, YAZAKI
	Copyright (C) 2002, genta, hor, YAZAKI, Azumaiya, KK, novice, minfu, ai, aroka, MIK
	Copyright (C) 2003, genta, MIK, Moca
	Copyright (C) 2004, genta, Moca, novice, Kazika, isearch
	Copyright (C) 2005, genta, Moca, MIK, ryoji, maru
	Copyright (C) 2006, genta, aroka, fon, yukihane, ryoji
	Copyright (C) 2007, ryoji, maru
	Copyright (C) 2008, ryoji
	Copyright (C) 2009, nasukoji
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CEDITVIEW_54DE503F_6F97_4A16_8165_27F5F0D232E2_H_
#define SAKURA_CEDITVIEW_54DE503F_6F97_4A16_8165_27F5F0D232E2_H_
#pragma once

#include <Windows.h>
#include <ObjIdl.h>  // LPDATAOBJECT
#include <shellapi.h>  // HDROP

#include <thread>

#include "CTextMetrics.h"
#include "CTextDrawer.h"
#include "CTextArea.h"
#include "CCaret.h"
#include "CViewCalc.h" // parent
#include "CEditView_Paint.h"	// parent
#include "CViewParser.h"
#include "CViewSelect.h"
#include "CSearchAgent.h"
#include "view/colors/EColorIndexType.h"
#include "window/CTipWnd.h"
#include "window/CAutoScrollWnd.h"
#include "CDicMgr.h"
//	Jun. 26, 2001 genta	正規表現ライブラリの差し替え
#include "extmodule/CBregexp.h"
#include "CEol.h"				// EEolType
#include "cmd/CViewCommander.h"
#include "mfclike/CMyWnd.h"		// parent
#include "doc/CDocListener.h"	// parent
#include "basis/SakuraBasis.h"	// CLogicInt, CLayoutInt
#include "util/container.h"		// vector_ex
#include "util/design_template.h"

class CViewFont;
class CRuler;
class CDropTarget; /// 2002/2/3 aroka ヘッダー軽量化
class COpeBlk;///
class CSplitBoxWnd;///
class CRegexKeyword;///
class CAutoMarkMgr; /// 2002/2/3 aroka ヘッダー軽量化 to here
class CEditDoc;	//	2002/5/13 YAZAKI ヘッダー軽量化
class CLayout;	//	2002/5/13 YAZAKI ヘッダー軽量化
class CMigemo;	// 2004.09.14 isearch
struct SColorStrategyInfo;
struct CColor3Setting;
class COutputAdapter;

// struct DispPos; //	誰かがincludeしてます
// class CColorStrategy;	// 誰かがincludeしてます
class CColor_Found;

#ifndef IDM_COPYDICINFO
#define IDM_COPYDICINFO 2000
#endif
#ifndef IDM_JUMPDICT
#define IDM_JUMPDICT 2001	// 2006.04.10 fon
#endif

///	マウスからコマンドが実行された場合の上位ビット
///	@date 2006.05.19 genta
const int CMD_FROM_MOUSE = 2;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 文書ウィンドウの管理
	
	1つの文書ウィンドウにつき1つのCEditDocオブジェクトが割り当てられ、
	1つのCEditDocオブジェクトにつき、4つのCEditViweオブジェクトが割り当てられる。
	ウィンドウメッセージの処理、コマンドメッセージの処理、
	画面表示などを行う。
	
	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
*/
//2007.08.25 kobake 文字間隔配列の機能をCTextMetricsに移動
//2007.10.02 kobake Command_TRIM2をCConvertに移動

class CEditView
: public CViewCalc //$$ これが親クラスである必要は無いが、このクラスのメソッド呼び出しが多いので、暫定的に親クラスとする。
, public CEditView_Paint
, public CMyWnd
, public CDocListenerEx
{
	std::thread m_threadUrlOpen;

public:
	const CEditDoc* GetDocument() const
	{
		return m_pcEditDoc;
	}
	CEditDoc* GetDocument()
	{
		return m_pcEditDoc;
	}
public:
	//! 背景にビットマップを使用するかどうか
	//! 2010.10.03 背景実装
	bool IsBkBitmap() const{
		return NULL != m_pcEditDoc->m_hBackImg
			&& 0 != GetDocument()->m_cDocType.GetDocumentAttribute().m_backImgOpacity;
	}

public:
	CEditView* GetEditView()
	{
		return this;
	}
	const CEditView* GetEditView() const
	{
		return this;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        生成と破棄                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* Constructors */
	CEditView( void );
	~CEditView();
	void Close();
	/* 初期化系メンバ関数 */
	BOOL Create(
		HWND		hwndParent,	//!< 親
		CEditDoc*	pcEditDoc,	//!< 参照するドキュメント
		int			nMyIndex,	//!< ビューのインデックス
		BOOL		bShow,		//!< 作成時に表示するかどうか
		bool		bMiniMap
	);
	void CopyViewStatus( CEditView* ) const;					/* 自分の表示状態を他のビューにコピー */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                      クリップボード                         //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//取得
	bool MyGetClipboardData( CNativeW& cmemBuf, bool* pbColumnSelect, bool* pbLineSelect = NULL );			/* クリップボードからデータを取得 */

	//設定
	bool MySetClipboardData( const WCHAR* pszText, int nTextLen, bool bColumnSelect, bool bLineSelect = false );	/* クリップボードにデータを設定 */

	//利用
	void CopyCurLine( bool bAddCRLFWhenCopy, EEolType neweol, bool bEnableLineModePaste );	/* カーソル行をクリップボードにコピーする */	// 2007.10.08 ryoji
	void CopySelectedAllLines( const wchar_t* pszQuote, BOOL bWithLineNumber );			/* 選択範囲内の全行をクリップボードにコピーする */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         イベント                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//ドキュメントイベント
	void OnAfterLoad(const SLoadInfo& sLoadInfo) override;
	/* メッセージディスパッチャ */
	LRESULT DispatchEvent(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//
	void OnChangeSetting();										/* 設定変更を反映させる */
	void OnPaint(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp);			/* 通常の描画処理 */
	void OnPaint2(HDC _hdc, PAINTSTRUCT *pPs, BOOL bDrawFromComptibleBmp);			/* 通常の描画処理 */
	void DrawBackImage(HDC hdc, RECT& rcPaint, HDC hdcBgImg);
	void OnTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	//ウィンドウ
	void OnSize(int cx, int cy);							/* ウィンドウサイズの変更処理 */
	void OnMove(int x, int y, int nWidth, int nHeight);
	//フォーカス
	void OnSetFocus( void );
	void OnKillFocus( void );
	//スクロール
	CLayoutInt  OnVScroll(int nScrollCode, int nPos);							/* 垂直スクロールバーメッセージ処理 */
	CLayoutInt  OnHScroll(int nScrollCode, int nPos);							/* 水平スクロールバーメッセージ処理 */
	//マウス
	void OnLBUTTONDOWN(WPARAM fwKeys, int _xPos, int _yPos);				/* マウス左ボタン押下 */
	void OnMOUSEMOVE(WPARAM fwKeys, int xPos_, int yPos_);				/* マウス移動のメッセージ処理 */
	void OnLBUTTONUP(WPARAM fwKeys, int xPos, int yPos);				/* マウス左ボタン開放のメッセージ処理 */
	void OnLBUTTONDBLCLK(WPARAM fwKeys, int _xPos, int _yPos);			/* マウス左ボタンダブルクリック */
	void OnRBUTTONDOWN(WPARAM fwKeys, int xPos, int yPos);				/* マウス右ボタン押下 */
	void OnRBUTTONUP(WPARAM fwKeys, int xPos, int yPos);				/* マウス右ボタン開放 */
	void OnMBUTTONDOWN(WPARAM fwKeys, int xPos, int yPos);				/* マウス中ボタン押下 */
	void OnMBUTTONUP(WPARAM fwKeys, int xPos, int yPos);				/* マウス中ボタン開放 */
	void OnXLBUTTONDOWN(WPARAM fwKeys, int xPos, int yPos);			/* マウスサイドボタン1押下 */
	void OnXLBUTTONUP(WPARAM fwKeys, int xPos, int yPos);				/* マウスサイドボタン1開放 */		// 2009.01.17 nasukoji
	void OnXRBUTTONDOWN(WPARAM fwKeys, int xPos, int yPos);			/* マウスサイドボタン2押下 */
	void OnXRBUTTONUP(WPARAM fwKeys, int xPos, int yPos);				/* マウスサイドボタン2開放 */		// 2009.01.17 nasukoji
	LRESULT OnMOUSEWHEEL(WPARAM wParam, LPARAM lParam);				//!< 垂直マウスホイールのメッセージ処理
	LRESULT OnMOUSEHWHEEL(WPARAM wParam, LPARAM lParam);			//!< 水平マウスホイールのメッセージ処理
	LRESULT OnMOUSEWHEEL2(WPARAM wParam, LPARAM lParam, bool bHorizontalMsg, EFunctionCode nCmdFuncID);		//!< マウスホイールのメッセージ処理
	bool IsSpecialScrollMode(int nSelect);					/* キー・マウスボタン状態よりスクロールモードを判定する */		// 2009.01.17 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           描画                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	// 2006.05.14 Moca  互換BMPによる画面バッファ
	// 2007.09.30 genta CompatibleDC操作関数
protected:
	//! ロジック行を1行描画
	bool DrawLogicLine(
		SColorStrategyInfo* pInfo,		//!< [in,out] 作画情報
		CLayoutInt		nLineTo			//!< [in]     作画終了するレイアウト行番号
	);

	//! レイアウト行を1行描画
	bool DrawLayoutLine(SColorStrategyInfo* pInfo);

	//色分け
public:
	CColor3Setting GetColorIndex( const CLayout* pcLayout, CLayoutYInt nLineNum, int nIndex, SColorStrategyInfo* pInfo, bool bPrev = false );	/* 指定位置のColorIndexの取得 02/12/13 ai */
	void SetCurrentColor( CGraphics& gr, EColorIndexType, EColorIndexType, EColorIndexType);
	COLORREF GetTextColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);
	COLORREF GetBackColorByColorInfo2(const ColorInfo& info, const ColorInfo& info2);

	//画面バッファ
protected:
	bool CreateOrUpdateCompatibleBitmap( int cx, int cy );	//!< メモリBMPを作成または更新
	void UseCompatibleDC(BOOL fCache);
public:
	void DeleteCompatibleBitmap();							//!< メモリBMPを削除

public:
	void DispTextSelected( HDC hdc, CLayoutInt nLineNum, const CMyPoint& ptXY, CLayoutInt nX_Layout );	/* テキスト反転 */
	void RedrawAll();											/* フォーカス移動時の再描画 */
	void Redraw();										// 2001/06/21 asa-o 再描画
	void RedrawLines( CLayoutYInt top, CLayoutYInt bottom );
	void CaretUnderLineON(bool bDraw, bool bDrawPaint, bool DisalbeUnderLine);						/* カーソル行アンダーラインのON */
	void CaretUnderLineOFF(bool bDraw, bool bDrawPaint, bool bResetFlag, bool DisalbeUnderLine);				/* カーソル行アンダーラインのOFF */
	bool GetDrawSwitch() const
	{
		return m_bDrawSWITCH;
	}
	bool SetDrawSwitch(bool b)
	{
		bool bOld = m_bDrawSWITCH;
		m_bDrawSWITCH = b;
		return bOld;
	}
	bool IsDrawCursorVLinePos(int posX);
	void DrawBracketCursorLine(bool bDraw);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AdjustScrollBars( BOOL bRedraw = TRUE );						/* スクロールバーの状態を更新する */
	BOOL CreateScrollBar();												/* スクロールバー作成 */	// 2006.12.19 ryoji
	void DestroyScrollBar();											/* スクロールバー破棄 */	// 2006.12.19 ryoji
	CLayoutInt GetWrapOverhang( void ) const;							/* 折り返し桁以後のぶら下げ余白計算 */	// 2008.06.08 ryoji
	CKetaXInt ViewColNumToWrapColNum( CLayoutXInt nViewColNum ) const;	/* 「右端で折り返す」用にビューの桁数から折り返し桁数を計算する */	// 2008.06.08 ryoji
	CLayoutInt GetRightEdgeForScrollBar( void );								/* スクロールバー制御用に右端座標を取得する */		// 2009.08.28 nasukoji

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           IME                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 25, 2002 genta protected->publicに移動
	bool IsImeON( void );	// IME ONか	// 2006.12.04 ryoji
	
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        スクロール                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	CLayoutInt  ScrollAtV(CLayoutInt nPos, BOOL bRedrawScrollBar = TRUE);		/* 指定上端行位置へスクロール */
	CLayoutInt  ScrollAtH(CLayoutInt nPos, BOOL bRedrawScrollBar = TRUE);		/* 指定左端桁位置へスクロール */
	//	From Here Sep. 11, 2004 genta ずれ維持の同期スクロール
	CLayoutInt  ScrollByV( CLayoutInt vl ){	return ScrollAtV( GetTextArea().GetViewTopLine() + vl );}	/* 指定行スクロール*/
	CLayoutInt  ScrollByH( CLayoutInt hl ){	return ScrollAtH( GetTextArea().GetViewLeftCol() + hl );}	/* 指定桁スクロール */
	void ScrollDraw(CLayoutInt nScrollRowNum, CLayoutInt nScrollColNum, const RECT& rcScroll, const RECT& rcClip, const RECT& rcClip2);
	void MiniMapRedraw(bool bUpdateAll);
public:
	void SyncScrollV(CLayoutInt line);									/* 垂直同期スクロール */
	void SyncScrollH(CLayoutInt col);									/* 水平同期スクロール */

	void SetBracketPairPos(bool flag);								/* 対括弧の強調表示位置設定 03/02/18 ai */

	void AutoScrollEnter();
	void AutoScrollExit();
	void AutoScrollMove( CMyPoint& point );
	void AutoScrollOnTimer();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        過去の遺産                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void SetIMECompFormPos( void );								/* IME編集エリアの位置を変更 */
	void SetIMECompFormFont( void );							/* IME編集エリアの表示フォントを変更 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       テキスト選択                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	// 2002/01/19 novice public属性に変更
	bool GetSelectedDataSimple( CNativeW& cmemBuf );// 選択範囲のデータを取得
	bool GetSelectedDataOne( CNativeW& cmemBuf, int nMaxLen );
	bool GetSelectedData( CNativeW* cmemBuf, BOOL bLineOnly, const wchar_t* pszQuote, BOOL bWithLineNumber, bool bAddCRLFWhenCopy, EEolType neweol = EEolType::none);/* 選択範囲のデータを取得 */
	int IsCurrentPositionSelected( CLayoutPoint ptCaretPos );					/* 指定カーソル位置が選択エリア内にあるか */
	int IsCurrentPositionSelectedTEST( const CLayoutPoint& ptCaretPos, const CLayoutRange& sSelect ) const;/* 指定カーソル位置が選択エリア内にあるか */
	// 2006.07.09 genta 行桁指定によるカーソル移動(選択領域を考慮)
	void MoveCursorSelecting( CLayoutPoint ptWk_CaretPos, bool bSelect, int = _CARETMARGINRATE );
	void ConvSelectedArea(EFunctionCode nFuncCode);								/* 選択エリアのテキストを指定方法で変換 */
	//!指定位置または指定範囲がテキストの存在しないエリアかチェックする		// 2008.08.03 nasukoji
	bool IsEmptyArea( CLayoutPoint ptFrom, CLayoutPoint ptTo = CLayoutPoint( CLayoutInt(-1), CLayoutInt(-1) ), bool bSelect = false, bool bBoxSelect = false ) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         各種判定                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	bool IsCurrentPositionURL( const CLayoutPoint& ptCaretPos, CLogicRange* pUrlRange, std::wstring* pstrURL );/* カーソル位置にURLが有る場合のその範囲を調べる */
	BOOL CheckTripleClick( CMyPoint ptMouse );							/* トリプルクリックをチェックする */	// 2007.10.02 nasukoji

	bool ExecCmd(const WCHAR* pszCmd, int nFlgOpt, const WCHAR* pszCurDir, COutputAdapter* customOa = NULL ) ;							// 子プロセスの標準出力をリダイレクトする
	void AddToCmdArr(const WCHAR* szCmd);
	BOOL ChangeCurRegexp(bool bRedrawIfChanged= true);									// 2002.01.16 hor 正規表現の検索パターンを必要に応じて更新する(ライブラリが使用できないときはFALSEを返す)
	void SendStatusMessage( const WCHAR* msg );					// 2002.01.26 hor 検索／置換／ブックマーク検索時の状態をステータスバーに表示する
	LRESULT SetReconvertStruct(PRECONVERTSTRING pReconv, bool bUnicode, bool bDocumentFeed = false);	/* 再変換用構造体を設定する 2002.04.09 minfu */
	LRESULT SetSelectionFromReonvert(const PRECONVERTSTRING pReconv, bool bUnicode);				/* 再変換用構造体の情報を元に選択範囲を変更する 2002.04.09 minfu */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           D&D                               //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public: /* テスト用にアクセス属性を変更 */
	/* IDropTarget実装 */
	STDMETHODIMP DragEnter(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragOver(DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP DragLeave( void );
	STDMETHODIMP Drop(LPDATAOBJECT pDataObject, DWORD dwKeyState, POINTL pt, LPDWORD pdwEffect);
	STDMETHODIMP PostMyDropFiles( LPDATAOBJECT pDataObject );		/* 独自ドロップファイルメッセージをポストする */	// 2008.06.20 ryoji
	void OnMyDropFiles( HDROP hDrop );								/* 独自ドロップファイルメッセージ処理 */	// 2008.06.20 ryoji
	CLIPFORMAT GetAvailableClipFormat( LPDATAOBJECT pDataObject );
	DWORD TranslateDropEffect( CLIPFORMAT cf, DWORD dwKeyState, POINTL pt, DWORD dwEffect );
	bool IsDragSource( void );

	void _SetDragMode(BOOL b)
	{
		m_bDragMode = b;
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           編集                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* 指定位置の指定長データ削除 */
	void DeleteData2(
		const CLayoutPoint&	ptCaretPos,
		CLogicInt			nDelLen,
		CNativeW*			pcMem
	);

	/* 現在位置のデータ削除 */
	void DeleteData( bool bRedraw );

	/* 現在位置にデータを挿入 */
	void InsertData_CEditView(
		CLayoutPoint	ptInsertPos,
		const wchar_t*	pData,
		int				nDataLen,
		CLayoutPoint*	pptNewPos,	//挿入された部分の次の位置のデータ位置
		bool			bRedraw
	);

	/* データ置換 削除&挿入にも使える */
	void ReplaceData_CEditView(
		const CLayoutRange&	sDelRange,			// 削除範囲。レイアウト単位。
		const wchar_t*		pInsData,			// 挿入するデータ
		CLogicInt			nInsDataLen,		// 挿入するデータの長さ
		bool				bRedraw,
		COpeBlk*			pcOpeBlk,
		bool				bFastMode = false,
		const CLogicRange*	psDelRangeLogicFast = NULL
	);
	void ReplaceData_CEditView2(
		const CLogicRange&	sDelRange,			// 削除範囲。ロジック単位。
		const wchar_t*		pInsData,			// 挿入するデータ
		CLogicInt			nInsDataLen,		// 挿入するデータの長さ
		bool				bRedraw,
		COpeBlk*			pcOpeBlk,
		bool				bFastMode = false
	);
	bool ReplaceData_CEditView3(
		CLayoutRange	sDelRange,			// 削除範囲。レイアウト単位。
		COpeLineData*	pcmemCopyOfDeleted,	// 削除されたデータのコピー(NULL可能)
		COpeLineData*	pInsData,
		bool			bRedraw,
		COpeBlk*		pcOpeBlk,
		int				nDelSeq,
		int*			pnInsSeq,
		bool			bFastMode = false,
		const CLogicRange*	psDelRangeLogicFast = NULL
	);
	void RTrimPrevLine( void );		/* 2005.10.11 ryoji 前の行にある末尾の空白を削除 */

	//	Oct. 2, 2005 genta 挿入モードの設定・取得
	bool IsInsMode() const;
	void SetInsMode(bool mode);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2004.10.13 インクリメンタルサーチ関係
	void TranslateCommand_isearch(EFunctionCode& nCommand, bool& bRedraw, LPARAM& lparam1, LPARAM& lparam2, LPARAM& lparam3,  LPARAM& lparam4 );
	bool ProcessCommand_isearch(int	nCommand, bool	bRedraw, LPARAM	lparam1, LPARAM	lparam2, LPARAM	lparam3, LPARAM	lparam4 );

	//	Jan. 10, 2005 genta HandleCommandからgrep関連処理を分離
	void TranslateCommand_grep(EFunctionCode& nCommand, bool& bRedraw, LPARAM& param1, LPARAM& param2, LPARAM& param3, LPARAM& param4 );

	//	Jan. 10, 2005 インクリメンタルサーチ
	bool IsISearchEnabled(int nCommand) const;

	BOOL KeySearchCore( const CNativeW* pcmemCurText );	// 2006.04.10 fon
	bool MiniMapCursorLineTip( POINT* po, RECT* rc, bool* pbHide );

	/*!	CEditView::KeyWordHelpSearchDictのコール元指定用ローカルID
		@date 2006.04.10 fon 新規作成
	*/
	enum LID_SKH {
		LID_SKH_ONTIMER		= 1,	/*!< CEditView::OnTimer */
		LID_SKH_POPUPMENU_R = 2,	/*!< CEditView::CreatePopUpMenu_R */
	};
	BOOL KeyWordHelpSearchDict( LID_SKH nID, POINT* po, RECT* rc );	// 2006.04.10 fon

	int IsSearchString(const CStringRef& cStr, CLogicInt nPos, CLogicInt* pnSearchStart, CLogicInt* pnSearchEnd) const;	/* 現在位置が検索文字列に該当するか */	//2002.02.08 hor 引数追加

	void GetCurrentTextForSearch( CNativeW& cmemCurText, bool bStripMaxPath = true, bool bTrimSpaceTab = false );			/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	bool GetCurrentTextForSearchDlg(CNativeW& cmemCurText, bool bGetHistory = false );		/* 現在カーソル位置単語または選択範囲より検索等のキーを取得（ダイアログ用） 2006.08.23 ryoji */

private:
	/* インクリメンタルサーチ */ 
	//2004.10.24 isearch migemo
	void ISearchEnter( ESearchMode mode, ESearchDirection direction);
	void ISearchExit();
	void ISearchExec(DWORD wChar);
	void ISearchExec(LPCWSTR pszText);
	void ISearchExec(bool bNext);
	void ISearchBack(void) ;
	void ISearchWordMake(void);
	void ISearchSetStatusMsg(CNativeW* msg) const;

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           括弧                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Jun. 16, 2000 genta
	bool  SearchBracket( const CLayoutPoint& ptPos, CLayoutPoint* pptLayoutNew, int* mode );	// 対括弧の検索		// modeの追加 02/09/18 ai
	bool  SearchBracketForward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* upChar, const wchar_t* dnChar, int* mode );	//	対括弧の前方検索	// modeの追加 02/09/19 ai
	bool  SearchBracketBackward( CLogicPoint ptPos, CLayoutPoint* pptLayoutNew,
						const wchar_t* dnChar, const wchar_t* upChar, int* mode );	//	対括弧の後方検索	// modeの追加 02/09/19 ai
	void DrawBracketPair(bool bDraw);								/* 対括弧の強調表示 02/09/18 ai */
	bool IsBracket(const wchar_t *pLine, CLogicInt x, CLogicInt size);					/* 括弧判定 03/01/09 ai */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           補完                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/* 支援 */
	//	Jan. 10, 2005 genta HandleCommandから補完関連処理を分離
	void PreprocessCommand_hokan( int nCommand );
	void PostprocessCommand_hokan(void);

	// 補完ウィンドウを表示する。Ctrl+Spaceや、文字の入力/削除時に呼び出されます。 YAZAKI 2002/03/11
	void ShowHokanMgr( CNativeW& cmemData, BOOL bAutoDecided );

	int HokanSearchByFile(const wchar_t* pszKey, bool bHokanLoHiCase, vector_ex<std::wstring>& vKouho, int nMaxKouho ); // 2003.06.25 Moca

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         ジャンプ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//@@@ 2003.04.13 MIK, Apr. 21, 2003 genta bClose追加
	//	Feb. 17, 2007 genta 相対パスの基準ディレクトリ指示を追加
	bool TagJumpSub( const WCHAR* pszJumpToFile, CMyPoint ptJumpTo,bool bClose = false,
		bool bRelFromIni = false, bool* pbJumpToSelf = NULL );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         メニュー                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

	int	CreatePopUpMenu_R( void );		/* ポップアップメニュー(右クリック) */
	int	CreatePopUpMenuSub( HMENU hMenu, int nMenuIdx, int* pParentMenus, EKeyHelpRMenuType eRmenuType );		/* ポップアップメニュー */
	void AddKeyHelpMenu(HMENU hMenu, EKeyHelpRMenuType eRmenuType);

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           DIFF                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void AnalyzeDiffInfo( const char* pszDiffInfo, int nFlgFile12);	/* DIFF情報の解析 */	//@@@ 2002.05.25 MIK
	BOOL MakeDiffTmpFile( WCHAR* filename, HWND hWnd, ECodeType code, bool bBom);	/* DIFF一時ファイル作成 */	//@@@ 2002.05.28 MIK	//2005.10.29 maru
	BOOL MakeDiffTmpFile2( WCHAR* tmpName, const WCHAR* orgName, ECodeType code, ECodeType saveCode);
	void ViewDiffInfo( const WCHAR* pszFile1, const WCHAR* pszFile2, int nFlgOpt, bool bUTF8);		/* DIFF差分表示 */		//2005.10.29 maru

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           履歴                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//	Aug. 31, 2000 genta
	void AddCurrentLineToHistory(void);	//現在行を履歴に追加する

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          その他                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	BOOL OPEN_ExtFromtoExt( BOOL bCheckOnly, BOOL bBeepWhenMiss,
							const WCHAR* file_ext[], const WCHAR* open_ext[],
							int file_extno, int open_extno, const WCHAR* errmes ); // 指定拡張子のファイルに対応するファイルを開く補助関数 // 2003.08.12 Moca
	//	Jan.  8, 2006 genta 折り返しトグル動作判定
	enum TOGGLE_WRAP_ACTION {
		TGWRAP_NONE = 0,
		TGWRAP_FULL,
		TGWRAP_WINDOW,
		TGWRAP_PROP,
	};
	TOGGLE_WRAP_ACTION GetWrapMode( CKetaXInt* newKetas );
	void SmartIndent_CPP(wchar_t wcChar);	/* C/C++スマートインデント処理 */
	/* コマンド操作 */
	void SetFont( void );										/* フォントの変更 */
	void SplitBoxOnOff(BOOL bVert, BOOL bHorz, BOOL bSizeBox);						/* 縦・横の分割ボックス・サイズボックスのＯＮ／ＯＦＦ */

//	2001/06/18 asa-o
	bool  ShowKeywordHelp( POINT po, LPCWSTR pszHelp, LPRECT prcHokanWin);	// 補完ウィンドウ用のキーワードヘルプ表示
	void SetUndoBuffer( bool bPaintLineNumber = false );			// アンドゥバッファの処理
	HWND StartProgress();

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         アクセサ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//主要構成部品アクセス
	CTextArea& GetTextArea()
	{
		assert(m_pcTextArea);
		return *m_pcTextArea;
	}
	const CTextArea& GetTextArea() const
	{
		assert(m_pcTextArea);
		return *m_pcTextArea;
	}
	CCaret& GetCaret()
	{
		assert(m_pcCaret);
		return *m_pcCaret;
	}
	const CCaret& GetCaret() const
	{
		assert(m_pcCaret);
		return *m_pcCaret;
	}
	CRuler& GetRuler()
	{
		assert(m_pcRuler);
		return *m_pcRuler;
	}
	const CRuler& GetRuler() const
	{
		assert(m_pcRuler);
		return *m_pcRuler;
	}

	//主要属性アクセス
	CTextMetrics& GetTextMetrics(){ return m_cTextMetrics; }
	const CTextMetrics& GetTextMetrics() const{ return m_cTextMetrics; }
	CViewSelect& GetSelectionInfo(){ return m_cViewSelect; }
	const CViewSelect& GetSelectionInfo() const{ return m_cViewSelect; }

	//主要オブジェクトアクセス
	CViewFont& GetFontset()
	{
		assert(m_pcViewFont);
		return *m_pcViewFont;
	}
	const CViewFont& GetFontset() const
	{
		assert(m_pcViewFont);
		return *m_pcViewFont;
	}

	//主要ヘルパアクセス
	const CViewParser& GetParser() const{ return m_cParser; }
	const CTextDrawer& GetTextDrawer() const{ return m_cTextDrawer; }
	CViewCommander& GetCommander(){ return m_cCommander; }

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                       メンバ変数群                          //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//参照
	CEditDoc*		m_pcEditDoc;	//!< ドキュメント
	const STypeConfig*	m_pTypeData;

	//主要構成部品
	CTextArea*		m_pcTextArea;
	CCaret*			m_pcCaret;
	CRuler*			m_pcRuler;

	//主要属性
	CTextMetrics	m_cTextMetrics;
	CViewSelect		m_cViewSelect;

	//主要オブジェクト
	CViewFont*		m_pcViewFont;

	//主要ヘルパ
	CViewParser		m_cParser;
	CTextDrawer		m_cTextDrawer;
	CViewCommander	m_cCommander;

public:
	//ウィンドウ
	HWND			m_hwndParent;		/* 親ウィンドウハンドル */
	HWND			m_hwndVScrollBar;	/* 垂直スクロールバーウィンドウハンドル */
	int				m_nVScrollRate;		/* 垂直スクロールバーの縮尺 */
	HWND			m_hwndHScrollBar;	/* 水平スクロールバーウィンドウハンドル */
	HWND			m_hwndSizeBox;		/* サイズボックスウィンドウハンドル */
	HWND			m_hwndSizeBoxPlaceholder;	/* サイズボックス代替スタティックウィンドウハンドル */
	CSplitBoxWnd*	m_pcsbwVSplitBox;	/* 垂直分割ボックス */
	CSplitBoxWnd*	m_pcsbwHSplitBox;	/* 水平分割ボックス */
	CAutoScrollWnd	m_cAutoScrollWnd;	//!< オートスクロール

public:
	//描画
	bool			m_bDrawSWITCH;
	COLORREF		m_crBack;				/* テキストの背景色 */			// 2006.12.07 ryoji
	COLORREF		m_crBack2;				// テキストの背景(キャレット用)
	CLayoutInt		m_nOldUnderLineY;		// 前回作画したカーソルアンダーラインの位置 0未満=非表示
	CLayoutInt		m_nOldUnderLineYBg;
	int				m_nOldUnderLineYMargin;
	int				m_nOldUnderLineYHeight;
	int				m_nOldUnderLineYHeightReal;
	int				m_nOldCursorLineX;		/* 前回作画したカーソル位置縦線の位置 */ // 2007.09.09 Moca
	int				m_nOldCursorVLineWidth;	// カーソル位置縦線の太さ(px)

public:
	//画面バッファ
	HDC				m_hdcCompatDC;		/* 再描画用コンパチブルＤＣ */
	HBITMAP			m_hbmpCompatBMP;	/* 再描画用メモリＢＭＰ */
	HBITMAP			m_hbmpCompatBMPOld;	/* 再描画用メモリＢＭＰ(OLD) */
	int				m_nCompatBMPWidth;  /* 再作画用メモリＢＭＰの幅 */	// 2007.09.09 Moca 互換BMPによる画面バッファ
	int				m_nCompatBMPHeight; /* 再作画用メモリＢＭＰの高さ */	// 2007.09.09 Moca 互換BMPによる画面バッファ

public:
	//D&D
	CDropTarget*	m_pcDropTarget;
	BOOL			m_bDragMode;	/* 選択テキストのドラッグ中か */
	CLIPFORMAT		m_cfDragData;	/* ドラッグデータのクリップ形式 */	// 2008.06.20 ryoji
	BOOL			m_bDragBoxData;	/* ドラッグデータは矩形か */
	CLayoutPoint	m_ptCaretPos_DragEnter;			/* ドラッグ開始時のカーソル位置 */	// 2007.12.09 ryoji
	CLayoutInt		m_nCaretPosX_Prev_DragEnter;	/* ドラッグ開始時のX座標記憶 */	// 2007.12.09 ryoji

	//括弧
	CLogicPoint		m_ptBracketCaretPos_PHY;	// 前カーソル位置の括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	CLogicPoint		m_ptBracketPairPos_PHY;		// 対括弧の位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	BOOL			m_bDrawBracketPairFlag;		/* 対括弧の強調表示を行なうか */						// 03/02/18 ai

	//マウス
	bool			m_bActivateByMouse;		//!< マウスによるアクティベート	//2007.10.02 nasukoji
	DWORD			m_dwTripleClickCheck;	//!< トリプルクリックチェック用時刻	//2007.10.02 nasukoji
	CMyPoint		m_cMouseDownPos;	//!< クリック時のマウス座標
	int				m_nWheelDelta;	//!< ホイール変化量
	EFunctionCode	m_eWheelScroll; //!< スクロールの種類
	int				m_nMousePause;	// マウス停止時間
	CMyPoint		m_cMousePausePos;	// マウスの停止位置
	bool			m_bHideMouse;

	int				m_nAutoScrollMode;			//!< オートスクロールモード
	bool			m_bAutoScrollDragMode;		//!< ドラッグモード
	CMyPoint		m_cAutoScrollMousePos;		//!< オートスクロールのマウス基準位置
	bool			m_bAutoScrollVertical;		//!< 垂直スクロール可
	bool			m_bAutoScrollHorizontal;	//!< 水平スクロール可

	//検索
	CSearchStringPattern m_sSearchPattern;
	mutable CBregexp	m_CurRegexp;				/*!< コンパイルデータ */
	bool				m_bCurSrchKeyMark;			/* 検索文字列のマーク */
	bool				m_bCurSearchUpdate;			//!< コンパイルデータ更新要求
	int					m_nCurSearchKeySequence;	//!< 検索キーシーケンス
	std::wstring		m_strCurSearchKey;			//!< 検索文字列
	SSearchOption		m_sCurSearchOption;			// 検索／置換  オプション
	CLogicPoint			m_ptSrchStartPos_PHY;		// 検索/置換開始時のカーソル位置 (改行単位行先頭からのバイト数(0開始), 改行単位行の行番号(0開始))
	BOOL				m_bSearch;					/* 検索/置換開始位置を登録するか */											// 02/06/26 ai
	ESearchDirection	m_nISearchDirection;		//!< 検索方向
	ESearchMode			m_nISearchMode;				//!< 検索モード
	bool				m_bISearchWrap;
	bool				m_bISearchFlagHistory[256];
	int					m_nISearchHistoryCount;
	bool				m_bISearchFirst;
	CLayoutRange		m_sISearchHistory[256];

	//マクロ
	bool			m_bExecutingKeyMacro;		/* キーボードマクロの実行中 */
	BOOL			m_bCommandRunning;	/* コマンドの実行中 */

	// 入力補完
	BOOL			m_bHokan;			//	補完中か？＝補完ウィンドウが表示されているか？かな？

	//編集
	bool			m_bDoing_UndoRedo;	/* アンドゥ・リドゥの実行中か */

	// 辞書Tip関連
	DWORD			m_dwTipTimer;			/* Tip起動タイマー */
	CTipWnd			m_cTipWnd;				/* Tip表示ウィンドウ */
	POINT			m_poTipCurPos;			/* Tip起動時のマウスカーソル位置 */
	BOOL			m_bInMenuLoop;			/* メニュー モーダル ループに入っています */
	CDicMgr			m_cDicMgr;				/* 辞書マネージャ */

	WCHAR			m_szComposition[512]; // IMR_DOCUMENTFEED用入力中文字列データ

	// IME
private:
	int				m_nLastReconvLine;             //2002.04.09 minfu 再変換情報保存用;
	int				m_nLastReconvIndex;            //2002.04.09 minfu 再変換情報保存用;

public:
	// その他
	CAutoMarkMgr*	m_cHistory;	//	Jump履歴
	CRegexKeyword*	m_cRegexKeyword;	//@@@ 2001.11.17 add MIK
	int				m_nMyIndex;	/* 分割状態 */
	CMigemo*		m_pcmigemo;
	bool			m_bMiniMap;
	bool			m_bMiniMapMouseDown;
	CLayoutInt		m_nPageViewTop;
	CLayoutInt		m_nPageViewBottom;

	DISALLOW_COPY_AND_ASSIGN(CEditView);
};

class COutputAdapter
{
public:
	COutputAdapter(){};
	virtual  ~COutputAdapter(){};

	virtual bool OutputW(const WCHAR* pBuf, int size = -1) = 0;
	virtual bool OutputA(const ACHAR* pBuf, int size = -1) = 0;
	virtual bool IsEnableRunningDlg(){ return true; }
	virtual bool IsActiveDebugWindow(){ return true; }
};
#endif /* SAKURA_CEDITVIEW_54DE503F_6F97_4A16_8165_27F5F0D232E2_H_ */
