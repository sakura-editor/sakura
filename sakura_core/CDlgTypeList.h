//	$Id$
/************************************************************************

	CDlgTypeList.h
	Copyright (C) 1998-2000, Norio Nakatani

    Update: 1999/12/05  再作成
    CREATE: 1998/12/23  新規作成
************************************************************************/

class CDlgTypeList;

#ifndef _CDLGTYPELIST_H_
#define _CDLGTYPELIST_H_

const int PROP_TEMPCHANGE_FLAG = 0x10000;

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgTypeList : public CDialog
{
public:
	BOOL DoModal( HINSTANCE, HWND, int* );	/* モーダルダイアログの表示 */

	int				m_nSettingType;
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnLbnDblclk( int );
	BOOL OnBnClicked( int );
	void SetData();	/* ダイアログデータの設定 */

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTYPELIST_H_ */

/*[EOF]*/
