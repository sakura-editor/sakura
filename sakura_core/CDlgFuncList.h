//	$Id$
/*!	@file
	アウトライン解析ダイアログボックス
	
	@author Norio Nakatani
    @date 1998/06/23 新規作成
    @date 1998/12/04 再作成
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

class CDlgFuncList;

#ifndef _CDLGFUNCLIST_H_
#define _CDLGFUNCLIST_H_

#include <windows.h>
#include "CDialog.h"
#include "CFuncInfoArr.h"
#include "CShareData.h"


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFuncList : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFuncList();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, CFuncInfoArr*, int, int, int );/* モードレスダイアログの表示 */
	void ChangeView( LPARAM );	/* モードレス時：検索対象となるビューの変更 */

	CFuncInfoArr*	m_pcFuncInfoArr;	/* 関数情報配列 */
	int				m_nCurLine;			/* 現在行 */
	int				m_nSortCol;			/* ソートする列番号 */
	int				m_nListType;		/* 一覧の種類 */
	CMemory			m_cmemClipText;		/* クリップボードコピー用テキスト */
	int				m_bLineNumIsCRLF;	/* 行番号の表示 FALSE=折り返し単位／TRUE=改行単位 */
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM, LPARAM );
	BOOL OnSize( WPARAM, LPARAM );
	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnJump( void );
	void SetTreeCpp( HWND );	/* ツリーコントロールの初期化：　C++メソッドツリー */
	void SetTreeJava( HWND, BOOL );	/* ツリーコントロールの初期化：　Javaメソッドツリー */
	void SetTreeTxt( HWND );	/* ツリーコントロールの初期化：　テキストトピックツリー */
	int SetTreeTxtNest( HWND, HTREEITEM, int, int, HTREEITEM*, int );
	void GetTreeTextNext( HWND, HTREEITEM, int );

private:
	//	May 18, 2001 genta
	/*!
		@brief アウトライン解析種別
		
		0: List, 1: Tree
	*/
	int	m_nViewType;

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */

/*[EOF]*/
