//	$Id$
/************************************************************************

	CDlgInput1.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgInput1;

#ifndef _CDLGINPUT1_H_
#define _CDLGINPUT1_H_

#include <windows.h>
#include "CMemory.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgInput1
{
public:
	/*
	||  Constructors
	*/
	CDlgInput1();
	~CDlgInput1();
	BOOL DoModal( HINSTANCE , HWND , const char* , const char* , int , char*  );	/* モードレスダイアログの表示 */
	/*
	||  Attributes & Operations
	*/
	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	HINSTANCE	m_hInstance;	/* アプリケーションインスタンスのハンドル */
	HWND		m_hwndParent;	/* オーナーウィンドウのハンドル */
	HWND		m_hWnd;			/* このダイアログのハンドル */

	const char*	m_pszTitle;		/* ダイアログタイトル */
	const char*	m_pszMessage;	/* メッセージ */
	int			m_nMaxTextLen;	/* 入力サイズ上限 */
//	char*		m_pszText;		/* テキスト */
	CMemory		m_cmemText;		/* テキスト */
protected:
	/*
	||  実装ヘルパ関数
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGINPUT1_H_ */

/*[EOF]*/
