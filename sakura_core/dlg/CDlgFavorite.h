/*!	@file
	@brief 履歴の管理ダイアログボックス

	@author MIK
	@date 2003.4.8
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2010, Moca

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



#ifndef SAKURA_CDLGFAVORITE_H_
#define SAKURA_CDLGFAVORITE_H_

#include "dlg/CDialog.h"
#include "recent/CRecent.h"

//!「履歴とお気に入りの管理」ダイアログ
//アクセス方法：[設定] - [履歴の管理]
class SAKURA_CORE_API CDlgFavorite : public CDialog
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
	int DoModal( HINSTANCE, HWND, LPARAM );	/* モーダルダイアログの表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL	OnActivate( WPARAM wParam, LPARAM lParam );
	LPVOID	GetHelpIdTable( void );

	void	SetData( void );	/* ダイアログデータの設定 */
	int		GetData( void );	/* ダイアログデータの取得 */

	bool	RefreshList( void );
	void	SetDataOne( int nIndex, int nLvItemIndex );	/* ダイアログデータの設定 */
	bool	RefreshListOne( int nIndex );
	//void	ChangeSlider( int nIndex );
	void	UpdateUIState();
	
	void    GetFavorite( int nIndex );
	int     DeleteSelected();
	void	AddItem();
	void	EditItem();

private:
	CRecentFile			m_cRecentFile;
	CRecentFolder		m_cRecentFolder;
	CRecentExceptMRU	m_cRecentExceptMRU;
	CRecentSearch		m_cRecentSearch;
	CRecentReplace		m_cRecentReplace;
	CRecentGrepFile		m_cRecentGrepFile;
	CRecentGrepFolder	m_cRecentGrepFolder;
	CRecentCmd			m_cRecentCmd;

	enum {
		// ! 管理数
		FAVORITE_INFO_MAX = 9 // 管理数 +1(番兵)
	};

	struct FavoriteInfo {
		CRecent*	m_pRecent;			//オブジェクトへのポインタ
		TCHAR*		m_pszCaption;		//キャプション
		int			m_nId;				//コントロールのID
		bool		m_bHaveFavorite;	//お気に入りを持っているか？
		bool		m_bHaveView;		//表示数変更機能をもっているか？
		bool		m_bFilePath;		//ファイル/フォルダか？
		bool		m_bEditable;		//編集可能
		int			m_nViewCount;		//カレントの表示数
	};

	struct ListViewSortInfo {
		HWND	hListView; //!< リストビューの HWND
		int		nSortColumn; //!< ソート列 -1で未指定
		bool	bSortAscending; //!< ソートが昇順
	};

	FavoriteInfo        m_aFavoriteInfo[FAVORITE_INFO_MAX];
	ListViewSortInfo    m_aListViewInfo[FAVORITE_INFO_MAX];

	int		m_nCurrentTab;
	TCHAR	m_szMsg[1024];

	static void  ListViewSort(ListViewSortInfo&, const CRecent* , int, bool);
};

#endif	//SAKURA_CDLGFAVORITE_H_

