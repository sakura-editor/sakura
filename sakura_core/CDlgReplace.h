/*!	@file
	@brief 置換ダイアログ

	@author Norio Nakatani
	@date 1998/10/02  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, hor
	Copyright (C) 2002, hor
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji

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

	int				m_bLoHiCase;	// 英大文字と英小文字を区別する
	int				m_bWordOnly;	// 一致する単語のみ検索する
	int				m_bConsecutiveAll;	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
	int				m_bRegularExp;	/* 正規表現 */
	char			m_szText[_MAX_PATH + 1];	/* 検索文字列 */
	char			m_szText2[_MAX_PATH + 1];	/* 置換後文字列 */
	BOOL			m_bSelectedArea;	/* 選択範囲内置換 */
	int				m_bNOTIFYNOTFOUND;				/* 検索／置換  見つからないときメッセージを表示 */
	int				m_nSettingType;
	BOOL			m_bSelected;	/* テキスト選択中か */
	int				m_nReplaceTarget;	/* 置換対象 */	// 2001.12.03 hor
	int				m_nPaste;			/* 貼り付け？ */	// 2001.12.03 hor
	int				m_nReplaceCnt;		//すべて置換の実行結果		// 2002.02.08 hor
	bool			m_bCanceled;		//すべて置換で中断したか	// 2002.02.08 hor
	int				m_nEscCaretPosX_PHY;	// 検索/置換開始時のカーソル位置退避エリア 02/07/28 ai
	int				m_nEscCaretPosY_PHY;	// 検索/置換開始時のカーソル位置退避エリア 02/07/28 ai

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );	/* ダイアログデータの設定 */
	void SetCombosList( void );	/* 検索文字列/置換後文字列リストの設定 */
	int GetData( void );	/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGREPLACE_H_ */


/*[EOF]*/
