//	$Id$
/*!	@file
	外部コマンド実行ダイアログ

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
