//	$Id$
/************************************************************************

	CDlgProperty.h
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE: 1999.12/05  再作成
    CREATE: 1999.02/31  新規作成
************************************************************************/

class CDlgProperty;

#ifndef _CDLGPROPERTY_H_
#define _CDLGPROPERTY_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgProperty : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND, LPARAM  );	/* モーダルダイアログの表示 */
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnBnClicked( int );
	void SetData( void );	/* ダイアログデータの設定 */
};
///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPROPERTY_H_ */

/*[EOF]*/
