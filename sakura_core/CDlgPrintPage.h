//	$Id$
/************************************************************************

	CDlgPrintPage.h
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/
#include "CDialog.h"

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


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTPAGE_H_ */

/*[EOF]*/
