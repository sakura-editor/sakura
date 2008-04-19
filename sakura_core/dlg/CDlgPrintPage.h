/*!	@file
	@brief 印刷ダイアログボックス

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"

#ifndef _CDLGPRINTPAGE_H_
#define _CDLGPRINTPAGE_H_

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgPrintPage : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgPrintPage();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */


	int	m_nPageMin;
	int	m_nPageMax;

	BOOL m_bAllPage;
	int	m_nPageFrom;
	int	m_nPageTo;



protected:
	/* オーバーライド? */
	int GetData( void );	/* ダイアログデータの取得 */
	void SetData( void );	/* ダイアログデータの設定 */
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTPAGE_H_ */


