/*!	@file
	@brief ファイルプロパティダイアログ

	@author Norio Nakatani
	@date 1999/02/31 新規作成
	@date 1999/12/05 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgProperty;

#ifndef _CDLGPROPERTY_H_
#define _CDLGPROPERTY_H_

#include "dlg/CDialog.h"
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
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};
///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPROPERTY_H_ */

/*[EOF]*/
