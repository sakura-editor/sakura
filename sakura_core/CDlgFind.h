//	$Id$
/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/02 再作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "CDialog.h"

#ifndef _CDLGFIND_H_
#define _CDLGFIND_H_


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */
	HWND DoModeless( HINSTANCE, HWND, LPARAM );	/* モードレスダイアログの表示 */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ダイアログのメッセージ処理 */

	void ChangeView( LPARAM );

	int		m_bLoHiCase;	/* 英大文字と英小文字を区別する */
	int		m_bWordOnly;	/* 一致する単語のみ検索する */
	int		m_bRegularExp;	/* 正規表現 */
	int		m_bNOTIFYNOTFOUND;	/* 検索／置換  見つからないときメッセージを表示 */
	char	m_szText[_MAX_PATH + 1];	/* 検索文字列 */
	int		m_nEscCaretPosX_PHY;	/* 検索開始時のカーソル位置退避エリア 02/07/28 ai */
	int		m_nEscCaretPosY_PHY;	/* 検索開始時のカーソル位置退避エリア 02/07/28 ai */

protected:
//@@@ 2002.2.2 YAZAKI CShareDataに移動
//	void AddToSearchKeyArr( const char* );
	/* オーバーライド? */
	int GetData( void );	/* ダイアログデータの取得 */
	void SetData( void );	/* ダイアログデータの設定 */
	BOOL OnBnClicked( int );

	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFIND_H_ */


/*[EOF]*/
