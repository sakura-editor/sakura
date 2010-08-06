/*!	@file
	@brief アウトライン解析ダイアログボックス

	@author Norio Nakatani
	@date 1998/06/23 新規作成
	@date 1998/12/04 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, genta, hor
	Copyright (C) 2002, aroka, hor, YAZAKI, frozen
	Copyright (C) 2003, little YOSHI
	Copyright (C) 2005, genta
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgFuncList;

#ifndef _CDLGFUNCLIST_H_
#define _CDLGFUNCLIST_H_

#include <windows.h>
#include "dlg/CDialog.h"
#include "CFuncInfo.h"
#include "env/CShareData.h"
#include "doc/CEditDoc.h"
class CFuncInfoArr; // 2002/2/10 aroka

#define OUTLINE_LAYOUT_FOREGROUND (0)
#define OUTLINE_LAYOUT_BACKGROUND (1)
#define OUTLINE_LAYOUT_FILECHANGED (2)

//!	アウトライン解析ダイアログボックス
class SAKURA_CORE_API CDlgFuncList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFuncList();
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );	// 2007.11.07 ryoji 標準以外のメッセージを捕捉する
	HWND DoModeless( HINSTANCE, HWND, LPARAM, CFuncInfoArr*, CLayoutInt, CLayoutInt, int, bool );/* モードレスダイアログの表示 */
	void ChangeView( LPARAM );	/* モードレス時：検索対象となるビューの変更 */
	bool IsDocking() { return m_eDockSide > DOCKSIDE_FLOAT; }
	EDockSide GetDockSide() { return m_eDockSide; }

	CommonSetting_OutLine& CommonSet(void){ return m_pShareData->m_Common.m_sOutline; }
	STypeConfig& TypeSet(void){ return CEditDoc::GetInstance(0)->m_cDocType.GetDocumentAttribute(); }
	int& ProfDockSet() { return CommonSet().m_nOutlineDockSet; }
	BOOL& ProfDockSync() { return CommonSet().m_bOutlineDockSync; }
	BOOL& ProfDockDisp() { return (ProfDockSet() == 0)? CommonSet().m_bOutlineDockDisp: TypeSet().m_bOutlineDockDisp; }
	EDockSide& ProfDockSide() { return (ProfDockSet() == 0)? CommonSet().m_eOutlineDockSide: TypeSet().m_eOutlineDockSide; }
	int& ProfDockLeft() { return (ProfDockSet() == 0)? CommonSet().m_cxOutlineDockLeft: TypeSet().m_cxOutlineDockLeft; }
	int& ProfDockTop() { return (ProfDockSet() == 0)? CommonSet().m_cyOutlineDockTop: TypeSet().m_cyOutlineDockTop; }
	int& ProfDockRight() { return (ProfDockSet() == 0)? CommonSet().m_cxOutlineDockRight: TypeSet().m_cxOutlineDockRight; }
	int& ProfDockBottom() { return (ProfDockSet() == 0)? CommonSet().m_cyOutlineDockBottom: TypeSet().m_cyOutlineDockBottom; }

	/*! 現在の種別と同じなら
	*/
	bool CheckListType( int nOutLineType ) const { return nOutLineType == m_nListType; }
	void Redraw( int nOutLineType, CFuncInfoArr*, CLayoutInt nCurLine, CLayoutInt nCurCol );
	void Refresh( void );
	bool ChangeLayout( int nId );
	void OnOutlineNotify( WPARAM wParam, LPARAM lParam );
	void SyncColor( void );
	void SetWindowText( const TCHAR* szTitle );		//ダイアログタイトルの設定
	bool m_bInChangeLayout;

	CFuncInfoArr*	m_pcFuncInfoArr;	/* 関数情報配列 */
	CLayoutInt		m_nCurLine;			/* 現在行 */
	CLayoutInt		m_nCurCol;			/* 現在桁 */
	int				m_nSortCol;			/* ソートする列番号 */
	int				m_nDocType;			/* ドキュメントの種類 */
	int				m_nListType;		/* 一覧の種類 */
	CNativeW		m_cmemClipText;		/* クリップボードコピー用テキスト */
	int				m_bLineNumIsCRLF;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
	bool			m_bEditWndReady;	/* エディタ画面の準備完了 */
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM, LPARAM );
	BOOL OnSize( WPARAM, LPARAM );
	BOOL OnDestroy(void); // 20060201 aroka
	BOOL OnCbnSelChange( HWND hwndCtl, int wID ); // 2002/11/1 frozen
	BOOL OnContextMenu( WPARAM, LPARAM );
	void SetData();	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnJump( bool bCheckAutoClose = true );	//	bCheckAutoClose：「このダイアログを自動的に閉じる」をチェックするかどうか
	void SetTreeCpp( HWND );	/* ツリーコントロールの初期化：C++メソッドツリー */
	void SetTreeJava( HWND, BOOL );	/* ツリーコントロールの初期化：Javaメソッドツリー */
	void SetTree(bool tagjump = false);		/* ツリーコントロールの初期化：汎用品 */
	void SetListVB( void );			/* リストビューコントロールの初期化：VisualBasic */		// Jul 10, 2003  little YOSHI

	// 2002/11/1 frozen 
	void SortTree(HWND hWndTree,HTREEITEM htiParent);//!< ツリービューの項目をソートする（ソート基準はm_nSortTypeを使用）
#if 0
2002.04.01 YAZAKI SetTreeTxt()、SetTreeTxtNest()は廃止。GetTreeTextNextはもともと使用されていなかった。
	void SetTreeTxt( HWND );	/* ツリーコントロールの初期化：テキストトピックツリー */
	int SetTreeTxtNest( HWND, HTREEITEM, int, int, HTREEITEM*, int );
	void GetTreeTextNext( HWND, HTREEITEM, int );
#endif

	//	Apr. 23, 2005 genta リストビューのソートを関数として独立させた
	void SortListView(HWND hwndList, int sortcol);

	// 2001.12.03 hor
//	void SetTreeBookMark( HWND );		/* ツリーコントロールの初期化：ブックマーク */
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
	void Key2Command( WORD );		//	キー操作→コマンド変換
	bool HitTestSplitter( int xPos, int yPos );
	int HitTestCaptionButton( int xPos, int yPos );
	INT_PTR OnNcCalcSize( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcHitTest( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnMouseMove( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcLButtonDown( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnLButtonUp( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnNcPaint( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	INT_PTR OnTimer( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void GetDockSpaceRect( LPRECT pRect );
	void GetCaptionRect( LPRECT pRect );
	bool GetCaptionButtonRect( int nButton, LPRECT pRect );
	void DoMenu( POINT pt, HWND hwndFrom );
	BOOL PostOutlineNotifyToAllEditors( WPARAM wParam, LPARAM lParam );
	EDockSide GetDropRect( POINT ptDrag, POINT ptDrop, LPRECT pRect, bool bForceFloat );
	BOOL Track( POINT ptDrag );

private:
	//	May 18, 2001 genta
	/*!
		@brief アウトライン解析種別

		0: List, 1: Tree
	*/
	int	m_nViewType;

	// 2002.02.16 hor Treeのダブルクリックでフォーカス移動できるように 1/4
	// (無理矢理なのでどなたか修正お願いします)
	bool m_bWaitTreeProcess;

	// 2002/11/1 frozen
	//! ツリービューをソートする基準
	// 0 デフォルト(ノードに関連づけれられた値順)
	// 1 アルファベット順
	int m_nSortType;

	// 選択中の関数情報
	CFuncInfo* m_cFuncInfo;

	EDockSide	m_eDockSide;	// 現在の画面の表示位置
	HWND		m_hwndToolTip;	/*!< ツールチップ（ボタン用） */
	bool		m_bStretching;
	bool		m_bHovering;
	int			m_nHilightedBtn;
	int			m_nCapturingBtn;

	static LPDLGTEMPLATE m_pDlgTemplate;
	static DWORD m_dwDlgTmpSize;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */



