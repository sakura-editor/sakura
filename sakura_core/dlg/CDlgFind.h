/*!	@file
	@brief 検索ダイアログボックス

	@author Norio Nakatani
	@date	1998/12/02 再作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI
	Copyright (C) 2009, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "dlg/CDialog.h"
#include "recent/CRecentSearch.h"
#include "util/window.h"
#ifndef SAKURA_CDLGFIND_H_
#define SAKURA_CDLGFIND_H_

// 依存クラスの前方定義
class CEditView;


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind() noexcept;
	virtual ~CDlgFind() noexcept = default;

	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( CEditView* pcEditView, const WCHAR* pszFind, size_t cchFind );	/* モードレスダイアログの表示 */

	void ChangeView( CEditView* pcEditView );

protected:
	std::wstring			m_strText;				// 検索文字列
	SSearchOption			m_sSearchOption;		// 検索オプション
	int						m_bNOTIFYNOTFOUND;		// 検索／置換  見つからないときメッセージを表示
	BOOL					m_bAutoCloseDlgFind;	// 検索ダイアログを自動的に閉じる
	BOOL					m_bSearchAll;			// 先頭（末尾）から再検索


	CLogicPoint				m_ptEscCaretPos_PHY;	// 検索開始時のカーソル位置退避エリア

	CRecentSearch			m_cRecentSearch;
	SComboBoxItemDeleter	m_comboDel;
	CFontAutoDeleter		m_cFontText;
	CEditView*&				m_pcEditView;

protected:
	/* オーバーライド */
	BOOL OnInitDialog( HWND hwnd, WPARAM wParam, LPARAM lParam ) override;
	BOOL OnDestroy() override;
	void SetData( void ) const noexcept;
	void SetData( void ) override		/* ダイアログデータの設定 */
	{
		const_cast<CDlgFind*>(this)->SetData();
	}
	int GetData( void ) override;		/* ダイアログデータの取得 */

	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji
	BOOL OnBnClicked( int wID );
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	void SetCombosList( void );	/* 検索文字列/置換後文字列リストの設定 */

	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );
	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGFIND_H_ */



