//	$Id$
/*!	@file
	@brief 外部コマンド実行ダイアログ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
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
//@@@ 2002.01.08 YAZAKI 設定を保存するためにShareDataに移動
//	BOOL	m_bGetStdout;	// 標準出力を得る


protected:
	/* オーバーライド? */
	int GetData( void );	/* ダイアログデータの取得 */
	void SetData( void );	/* ダイアログデータの設定 */
	BOOL OnBnClicked( int );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGEXEC_H_ */

/*[EOF]*/
