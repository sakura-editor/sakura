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

#ifndef SAKURA_CDLGREPLACE_H_
#define SAKURA_CDLGREPLACE_H_

#include "dlg/CDialog.h"
#include "recent/CRecent.h"
#include "util/window.h"

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!
	@brief 置換ダイアログボックス
*/
class CDlgReplace : public CDialog
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

	SSearchOption	m_sSearchOption;	// 検索オプション
	int				m_bConsecutiveAll;	/* 「すべて置換」は置換の繰返し */	// 2007.01.16 ryoji
	std::wstring	m_strText;	// 検索文字列
	std::wstring	m_strText2;	// 置換後文字列
	int				m_nReplaceKeySequence;	//置換後シーケンス
	BOOL			m_bSelectedArea;	/* 選択範囲内置換 */
	int				m_bNOTIFYNOTFOUND;				/* 検索／置換  見つからないときメッセージを表示 */
	BOOL			m_bSelected;	/* テキスト選択中か */
	int				m_nReplaceTarget;	/* 置換対象 */	// 2001.12.03 hor
	int				m_nPaste;			/* 貼り付け？ */	// 2001.12.03 hor
	int				m_nReplaceCnt;		//すべて置換の実行結果		// 2002.02.08 hor
	bool			m_bCanceled;		//すべて置換で中断したか	// 2002.02.08 hor

	CLogicPoint		m_ptEscCaretPos_PHY;	// 検索/置換開始時のカーソル位置退避エリア

protected:
	CRecentSearch			m_cRecentSearch;
	SComboBoxItemDeleter	m_comboDelText;
	CRecentReplace			m_cRecentReplace;
	SComboBoxItemDeleter	m_comboDelText2;
	CFontAutoDeleter		m_cFontText;
	CFontAutoDeleter		m_cFontText2;

	/*
	||  実装ヘルパ関数
	*/
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnDestroy();
	BOOL OnBnClicked( int );
	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add

	void SetData( void );		/* ダイアログデータの設定 */
	void SetCombosList( void );	/* 検索文字列/置換後文字列リストの設定 */
	int GetData( void );		/* ダイアログデータの取得 */
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGREPLACE_H_ */



