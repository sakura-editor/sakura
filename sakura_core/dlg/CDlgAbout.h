/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date 1998/05/22 作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "dlg/CDialog.h"
/*!
	@brief About Box管理
	
	DispatchEventを独自に定義することで，CDialogでサポートされていない
	メッセージを捕捉する．
*/

class CUrlWnd
{
public:
	CUrlWnd() { m_hWnd = NULL; m_hFont = NULL; m_bHilighted = FALSE; m_pOldProc = NULL; }
	virtual ~CUrlWnd() { ; }
	BOOL SetSubclassWindow( HWND hWnd );
	HWND GetHwnd() const{ return m_hWnd; }
protected:
	HFONT GetFont() const { return m_hFont; }
protected:
	static LRESULT CALLBACK UrlWndProc( HWND hWnd, UINT msg, WPARAM wp, LPARAM lp );
protected:
	bool OnSetText( _In_opt_z_ LPCTSTR pchText, _In_opt_ size_t cchText = 0 ) const;
protected:
	HWND m_hWnd;
	HFONT m_hFont;
	BOOL m_bHilighted;
	WNDPROC m_pOldProc;
};

class CDlgAbout : public CDialog
{
public:
	int DoModal(HINSTANCE hInstance, HWND hwndParent);	/* モーダルダイアログの表示 */
	//	Nov. 7, 2000 genta	標準以外のメッセージを捕捉する
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
	BOOL OnBnClicked(int wID);
	BOOL OnStnClicked(int wID);
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
private:
	CUrlWnd m_UrlUrWnd;
	CUrlWnd m_UrlGitWnd;
	CUrlWnd m_UrlBuildLinkWnd;
	CUrlWnd m_UrlGitHubCommitWnd;
	CUrlWnd m_UrlGitHubPRWnd;
	CUrlWnd m_UrlOrgWnd;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */

