/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CMAINTOOLBAR_FEA7E388_DFEC_4E15_94CC_90A7E779797B_H_
#define SAKURA_CMAINTOOLBAR_FEA7E388_DFEC_4E15_94CC_90A7E779797B_H_
#pragma once

#include "dlg/CDialog.h"
#include "recent/CRecentSearch.h"

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

	CRecentSearch			m_cRecentSearch;
	CImageListMgr*			m_pcIcons;
};
#endif /* SAKURA_CMAINTOOLBAR_FEA7E388_DFEC_4E15_94CC_90A7E779797B_H_ */
