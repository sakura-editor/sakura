//	$Id$
/************************************************************************

	CDlgAbout.h

    バージョン情報ダイアログ

    UPDATE: 1999.12/05 再作成
    CREATE: 1998.05/22 新規作成
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgAbout;

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgAbout : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND );	/* モーダルダイアログの表示 */
	//	Nov. 7, 2000 genta	標準以外のメッセージを捕捉する
	BOOL DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
private:
	int	 nCursorState;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */

/*[EOF]*/
