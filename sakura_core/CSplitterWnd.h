//	$Id$
/************************************************************************

	CSplitterWnd.h
	Copyright (C) 1998-2000, Norio Nakatani


    CREATE: 1998/7/7  新規作成
************************************************************************/
class CSplitterWnd;


#ifndef _CSPLITTERWND_H_
#define _CSPLITTERWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CShareData.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CSplitterWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CSplitterWnd();
	~CSplitterWnd();
	/*
	||  Attributes & Operations
	*/
	char*			m_pszClassName;			/* クラス名 */
//	HINSTANCE		m_hInstance;			/* アプリケーションインスタンスのハンドル */
//	HWND			m_hwndParent;			/* オーナーウィンドウのハンドル */
//	HWND			m_hWnd;					/* このダイアログのハンドル */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	void*			m_pCEditWnd;	
	int				m_nAllSplitRows;		/* 分割行数 */
	int				m_nAllSplitCols;		/* 分割桁数 */
	int				m_nVSplitPos;			/* 垂直分割位置 */
	int				m_nHSplitPos;			/* 水平分割位置 */
	HWND			m_ChildWndArr[4];		/* 子ウィンドウ配列 */
	HCURSOR			m_hcurOld;				/* もとのマウスカーソル */
	int				m_bDragging;			/* 分割バーをドラッグ中か */
	int				m_nDragPosX;			/* ドラッグ位置Ｘ */
	int				m_nDragPosY;			/* ドラッグ位置Ｙ */
	int				m_nActivePane;			/* アクティブなペイン */

	HWND Create( HINSTANCE, HWND, void* pCEditWnd );	/* 初期化 */
	void SetChildWndArr( HWND* );	/* 子ウィンドウの設定 */
	void DoSplit( int, int );	/* ウィンドウの分割 */
	void SetActivePane( int );	/* アクティブペインの設定 */
	int GetPrevPane( void );	/* 前のペインを返す */ 
	int GetNextPane( void );	/* 次のペインを返す */ 
	int GetFirstPane( void );	/* 最初のペインを返す */
	int GetLastPane( void );	/* 最後のペインを返す */

	void VSplitOnOff( void );	/* 縦分割ＯＮ／ＯＦＦ */
	void HSplitOnOff( void );	/* 横分割ＯＮ／ＯＦＦ */
	void VHSplitOnOff( void );	/* 縦横分割ＯＮ／ＯＦＦ */
//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */
protected:
	/* 仮想関数 */
	virtual LRESULT DispatchEvent_WM_APP( HWND, UINT, WPARAM, LPARAM );/* アプリケーション定義のメッセージ(WM_APP <= msg <= 0xBFFF) */
	
	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );	/* ウィンドウサイズの変更処理 */
	virtual LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );	/* 描画処理 */
//	virtual LRESULT OnMove( HWND, UINT, WPARAM, LPARAM );	/* ウィンドウ移動時の処理 */
	virtual LRESULT OnMouseMove( HWND, UINT, WPARAM, LPARAM ); /* マウス移動時の処理 */
	virtual LRESULT OnLButtonDown( HWND, UINT, WPARAM, LPARAM );	/* マウス左ボタン押下時の処理 */
	virtual LRESULT OnLButtonUp( HWND, UINT, WPARAM, LPARAM );	/* マウス左ボタン解放時の処理 */
	virtual LRESULT OnLButtonDblClk( HWND, UINT, WPARAM, LPARAM );	/* マウス左ボタンダブルクリック時の処理 */
	/*
	||  実装ヘルパ関数
	*/
	void DrawFrame( HDC , RECT* );	/* 分割フレーム描画 */
	int HitTestSplitter( int , int );	/* 分割バーへのヒットテスト */
	void DrawSplitter( int , int , int );	/* 分割トラッカーの表示 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CSPLITTERWND_H_ */

/*[EOF]*/
