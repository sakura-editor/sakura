//	$Id$
/************************************************************************

	CDlgExec.h
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/
#include "CDialog.h"

#ifndef _CDLGEXEC_H_
#define _CDLGEXEC_H_

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgExec : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgExec();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */

	char	m_szCommand[1024 + 1];	/* コマンドライン */
	BOOL	m_bGetStdout;	// 標準出力を得る


protected:
	/* オーバーライド? */
	int GetData( void );	/* ダイアログデータの取得 */
	void SetData( void );	/* ダイアログデータの設定 */
	BOOL OnBnClicked( int );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGEXEC_H_ */

/*[EOF]*/
