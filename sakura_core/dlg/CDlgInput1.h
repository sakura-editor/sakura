/*!	@file
	@brief 1行入力ダイアログボックス

	@author Norio Nakatani
	@date	1998/05/31 作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#pragma once

class CDlgInput1;

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief １行入力ダイアログボックス
*/
class CDlgInput1
{
  public:
    /*
	||  Constructors
	*/
    CDlgInput1();
    ~CDlgInput1();
    BOOL DoModal(HINSTANCE hInstApp, HWND hwndParent, const WCHAR *pszTitle, const WCHAR *pszMessage, int nMaxTextLen, WCHAR *pszText); /* モードレスダイアログの表示 */

    /*
	||  Attributes & Operations
	*/
    INT_PTR DispatchEvent(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); /* ダイアログのメッセージ処理 */

    HINSTANCE m_hInstance; /* アプリケーションインスタンスのハンドル */
    HWND m_hwndParent; /* オーナーウィンドウのハンドル */
    HWND m_hWnd; /* このダイアログのハンドル */

    const WCHAR *m_pszTitle; /* ダイアログタイトル */
    const WCHAR *m_pszMessage; /* メッセージ */
    int m_nMaxTextLen; /* 入力サイズ上限 */
    //	char*		m_pszText;		/* テキスト */
    CNativeW m_cmemText; /* テキスト */
  protected:
    /*
	||  実装ヘルパ関数
	*/
};
