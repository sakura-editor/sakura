//	$Id$
/************************************************************************

	CDlgFuncList.h

    Update: 1998/12/04  再作成
    CREATE: 1998/06/23  新規作成
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

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

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFUNCLIST_H_ */

/*[EOF]*/
