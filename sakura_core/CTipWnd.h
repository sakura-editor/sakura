//	$Id$
/************************************************************************

	CTipWnd.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/10/30  新規作成


************************************************************************/

class CTipWnd;

#ifndef _CTIPWND_H_
#define _CTIPWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CMemory.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CTipWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTipWnd();
	~CTipWnd();
	void Create( HINSTANCE, HWND );	/* 初期化 */

	/*
	||  Attributes & Operations
	*/
	void Show( int, int, char* );	/* Tipを表示 */
	void Hide( void );	/* Tipを消す */
//	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	char*		m_pszClassName;	/* Mutex作成用・ウィンドウクラス名 */
//	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
//	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
//	HWND		m_hWnd;			/* このダイアログのハンドル */
	HFONT		m_hFont;
//	HFONT		m_hFontOld;

	CMemory		m_cKey;			/* キーの内容データ */
	BOOL		m_KeyWasHit;	/* キーがヒットしたか */
	
	CMemory		m_cInfo;		/* Tipの内容データ */

protected:
	/*
	||  実装ヘルパ関数
	*/
	void ComputeWindowSize( HDC, HFONT, const char*, RECT* );	/* ウィンドウのサイズを決める */
	void DrawTipText( HDC, HFONT, const char* );	/* ウィンドウのテキストを表示 */

	/* 仮想関数 */

	/* 仮想関数 メッセージ処理 詳しくは実装を参照 */
	LRESULT OnPaint( HWND, UINT, WPARAM, LPARAM );/* 描画処理 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CTIPWND_H_ */

/*[EOF]*/
