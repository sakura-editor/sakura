//	$Id$
/*!	@file
	@brief バージョン情報ダイアログ

	@author Norio Nakatani
	@date 1998/05/22 作成
	@date 1999/12/05 再作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "CDialog.h"
/*!
	@brief About Box管理
	
	DispatchEventを独自に定義することで，CDialogでサポートされていない
	メッセージを捕捉する．
*/
class CDlgAbout : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND );	/* モーダルダイアログの表示 */
	//	Nov. 7, 2000 genta	標準以外のメッセージを捕捉する
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
private:
	int	 nCursorState;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */


/*[EOF]*/
