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

class CDlgInput1;

#ifndef _CDLGINPUT1_H_
#define _CDLGINPUT1_H_

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
	BOOL DoModal( HINSTANCE , HWND , const TCHAR* , const TCHAR* , int , TCHAR*  );	/* モードレスダイアログの表示 */
	BOOL DoModal( HINSTANCE , HWND , const TCHAR* , const TCHAR* , int , NOT_TCHAR*  );	/* モードレスダイアログの表示 */
	/*
	||  Attributes & Operations
	*/
	INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND		m_hWnd;			/* このダイアログのハンドル */

	const TCHAR*	m_pszTitle;		/* ダイアログタイトル */
	const TCHAR*	m_pszMessage;	/* メッセージ */
	int			m_nMaxTextLen;	/* 入力サイズ上限 */
//	char*		m_pszText;		/* テキスト */
	CNativeT	m_cmemText;		/* テキスト */
protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGINPUT1_H_ */


