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
#pragma once

#ifndef SAKURA_CDLGFIND_H_
#define SAKURA_CDLGFIND_H_

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "_main/global.h"
#include "dlg/CDialog.h"

// 依存クラスの前方定義
class CEditView;


/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
class CDlgFind : public CDialog
{
	enum InternalConstants
	{
		IDT_AUTO_COUNT = 100,			//タイマーID
		TIMESPAN_AUTO_COUNT = 1000,		//起動間隔
	};

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
	bool					m_bNotifyNotFound;		// 検索／置換  見つからないときメッセージを表示
	bool					m_bAutoClose;			// 検索ダイアログを自動的に閉じる
	bool					m_bSearchAll;			// 先頭（末尾）から再検索

	HFONT					m_hFont;				// ドキュメント設定から作成したフォント

	CLogicPoint				m_ptEscCaretPos_PHY;	// 検索開始時のカーソル位置退避エリア
	CEditView*&				m_pcEditView;
	std::thread				m_threadAutoCount;		// 自動カウント用スレッド
	std::mutex				m_mtxAutoCount;
	std::atomic<bool>		m_bAutoCountCanceled;
	std::wstring			m_strOriginalTitle;		// ダイアログタイトル退避用

protected:
	/* オーバーライド */
	BOOL OnInitDialog( HWND wParam, LPARAM lParam ) override;
	BOOL OnDestroy( void ) override;
	void OnNcDestroy( void ) noexcept override;
	void SetData( void ) const noexcept override;	/* ダイアログデータの設定 */
	int GetData( void ) override;					/* ダイアログデータの取得 */

	BOOL OnActivate( WPARAM wParam, LPARAM lParam );	// 2009.11.29 ryoji
	BOOL OnBnClicked( int wID ) override;
	BOOL OnCbnDropDown( HWND hwndCtl, int wID ) override;
	BOOL OnCbnEditChange( HWND hwndCtl, int wID ) override;
	BOOL OnCbnSelChange( HWND hwndCtl, int wID ) override;
	BOOL OnTimer( WPARAM nTimerId ) override;

	void ApplySharedSearchKey() noexcept;

	void DoSearch( ESearchDirection direction ) noexcept;
	void DoSetMark( void ) noexcept;

	void StartAutoCounter() noexcept;
	void StopAutoCounter() noexcept;
	void CountMatches() const noexcept;


	LPVOID GetHelpIdTable(void);	//@@@ 2002.01.18 add
};



///////////////////////////////////////////////////////////////////////
#endif /* SAKURA_CDLGFIND_H_ */



