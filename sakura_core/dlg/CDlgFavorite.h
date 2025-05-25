/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2010, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGFAVORITE_D3C5AE69_3DBD_4C29_9918_BCC0D3BE61FE_H_
#define SAKURA_CDLGFAVORITE_D3C5AE69_3DBD_4C29_9918_BCC0D3BE61FE_H_
#pragma once

#include "dlg/CDialog.h"
#include "recent/CRecentFile.h"
#include "recent/CRecentFolder.h"
#include "recent/CRecentExceptMru.h"
#include "recent/CRecentSearch.h"
#include "recent/CRecentReplace.h"
#include "recent/CRecentGrepFile.h"
#include "recent/CRecentGrepFolder.h"
#include "recent/CRecentCmd.h"
#include "recent/CRecentCurDir.h"

//!「履歴とお気に入りの管理」ダイアログ
//アクセス方法：[設定] - [履歴の管理]
class CDlgFavorite final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFavorite();
	~CDlgFavorite();

	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnDestroy( void ) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	BOOL	OnActivate( WPARAM wParam, LPARAM lParam ) override;
	LPVOID	GetHelpIdTable( void ) override;
	INT_PTR DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam ) override;	// 標準以外のメッセージを捕捉する
	BOOL	OnSize( WPARAM wParam, LPARAM lParam ) override;
	BOOL	OnMinMaxInfo( LPARAM lParam );

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */

	void	TabSelectChange(bool bSetFocus);
	bool	RefreshList( void );
	void	SetDataOne( int nIndex, int nLvItemIndex );	/* ダイアログデータの設定 */
	bool	RefreshListOne( int nIndex );
	//void	ChangeSlider( int nIndex );
	void	UpdateUIState();
	
	void    GetFavorite( int nIndex );
	int     DeleteSelected();
	void	AddItem();
	void	EditItem();
	void	RightMenu(POINT &menuPos);

private:
	CRecentFile			m_cRecentFile;
	CRecentFolder		m_cRecentFolder;
	CRecentExceptMRU	m_cRecentExceptMRU;
	CRecentSearch		m_cRecentSearch;
	CRecentReplace		m_cRecentReplace;
	CRecentGrepFile		m_cRecentGrepFile;
	CRecentGrepFolder	m_cRecentGrepFolder;
	CRecentCmd			m_cRecentCmd;
	CRecentCurDir		m_cRecentCurDir;

	enum {
		// ! 管理数
		FAVORITE_INFO_MAX = 10 // 管理数 +1(番兵)
	};

	struct FavoriteInfo {
		CRecent*	m_pRecent;			//オブジェクトへのポインタ
		std::wstring	m_strCaption;	//キャプション
		const WCHAR*	m_pszCaption;	//キャプション
		int			m_nId;				//コントロールのID
		bool		m_bHaveFavorite;	//お気に入りを持っているか？
		bool		m_bHaveView;		//表示数変更機能をもっているか？
		bool		m_bFilePath;		//ファイル/フォルダーか？
		bool		m_bEditable;		//編集可能
		bool		m_bAddExcept;		//除外へ追加
		int			m_nViewCount;		//カレントの表示数
		FavoriteInfo():
			m_pRecent(NULL)
			,m_pszCaption(NULL)
			,m_nId(0)
			,m_bHaveFavorite(false)
			,m_bHaveView(false)
			,m_bFilePath(false)
			,m_bEditable(false)
			,m_bAddExcept(false)
			,m_nViewCount(0)
		{};
	};
	struct ListViewSortInfo {
		HWND	hListView; //!< リストビューの HWND
		int		nSortColumn; //!< ソート列 -1で未指定
		bool	bSortAscending; //!< ソートが昇順
	};

	FavoriteInfo        m_aFavoriteInfo[FAVORITE_INFO_MAX];
	ListViewSortInfo    m_aListViewInfo[FAVORITE_INFO_MAX];
	POINT				m_ptDefaultSize;
	RECT				m_rcListDefault;
	RECT				m_rcItems[10];

	int		m_nCurrentTab;
	int		m_nExceptTab;
	WCHAR	m_szMsg[1024];

	static void  ListViewSort(ListViewSortInfo& info, const CRecent* pRecent, int column, bool bReverse);
};
#endif /* SAKURA_CDLGFAVORITE_D3C5AE69_3DBD_4C29_9918_BCC0D3BE61FE_H_ */
