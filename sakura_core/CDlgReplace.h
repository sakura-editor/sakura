//	$Id$
/*!	@file
	@brief 置換ダイアログ

	@author Norio Nakatani
	@date 1998/10/02  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CDlgReplace;

#ifndef _CDLGREPLACE_H_
#define _CDLGREPLACE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 置換ダイアログボックス
*/
class SAKURA_CORE_API CDlgReplace : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgReplace();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, BOOL );	/* モーダルダイアログの表示 */
	void ChangeView( LPARAM );	/* モードレス時：置換・検索対象となるビューの変更 */

	int				m_bLoHiCase;	/* 英大文字と英小文字を区別する */
	int				m_bWordOnly;	/* 一致する単語のみ検索する */
	int				m_bRegularExp;	/* 正規表現 */
	char			m_szText[_MAX_PATH + 1];	/* 検索文字列 */
	char			m_szText2[_MAX_PATH + 1];	/* 置換後文字列 */
	BOOL			m_bSelectedArea;	/* 選択範囲内置換 */
	int				m_bNOTIFYNOTFOUND;				/* 検索／置換  見つからないときメッセージを表示 */
	int				m_nSettingType;
	BOOL			m_bSelected;	/* テキスト選択中か */
	int				m_nReplaceTarget;	/* 置換対象 */	// 2001.12.03 hor
	int				m_nPaste;			/* 張付け？ */	// 2001.12.03 hor
protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );

	void SetData( void );	/* ダイアログデータの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGREPLACE_H_ */


/*[EOF]*/
