/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_
#define SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_

#include "recent/CRecent.h"
#include "dlg/CDialog.h"

class CEditWnd;
class CImageListMgr;

class CMainToolBar{
public:
	CMainToolBar(CEditWnd* pOwner);

	void Create( CImageListMgr* pcIcons );

	//作成・破棄
	void CreateToolBar( void );		//!< ツールバー作成
	void DestroyToolBar( void );	//!< ツールバー破棄

	//メッセージ
	bool EatMessage(MSG* msg);		//!< メッセージ処理。なんか処理したなら true を返す。
	void ProcSearchBox( MSG* );		//!< 検索コンボボックスのメッセージ処理

	//イベント
	void OnToolbarTimer( void );	//!< タイマーの処理 20060128 aroka
	void UpdateToolbar( void );		//!< ツールバーの表示を更新する		// 2008.09.23 nasukoji

	//描画
	LPARAM ToolBarOwnerDraw( LPNMCUSTOMDRAW pnmh );

	//共有データとの同期
	void AcceptSharedSearchKey();

	//取得
	HWND GetToolbarHwnd() const{ return m_hwndToolBar; }
	HWND GetRebarHwnd() const{ return m_hwndReBar; }
	HWND GetSearchHwnd() const{ return m_hwndSearchBox; }
	int GetSearchKey(std::wstring&); //!< 検索キーを取得。戻り値は検索キーの文字数。

	//操作
	void SetFocusSearchBox( void ) const;		/* ツールバー検索ボックスへフォーカスを移動 */	// 2006.06.04 yukihane

private:
	CEditWnd*	m_pOwner;
    HWND		m_hwndToolBar;

	//子ウィンドウ
    HWND		m_hwndReBar;		//!< Rebar ウィンドウ	//@@@ 2006.06.17 ryoji
	HWND		m_hwndSearchBox;	//!< 検索コンボボックス

	//フォント
	HFONT		m_hFontSearchBox;	//!< 検索コンボボックスのフォント

	SComboBoxItemDeleter	m_comboDel;
	CRecentSearch			m_cRecentSearch;
	CImageListMgr*			m_pcIcons;
};

#endif /* SAKURA_CMAINTOOLBAR_F8D148A4_02B1_42E7_8B00_B51B3DB49E749_H_ */
/*[EOF]*/
