/*!	@file
	@brief プラグイン設定ダイアログボックス

	@author Uchi
	@date 2010/3/22
*/
/*
	Copyright (C) 2010, Uchi

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
#pragma once

#include "dlg/CDialog.h"
#include "plugin/CPluginManager.h"

class CPropPlugin;

/*!	@brief 「プラグイン設定」ダイアログ

	共通設定のプラグイン設定で，指定プラグインのオプションを設定するために
	使用されるダイアログボックス
*/

// 編集最大長
#define MAX_LENGTH_VALUE	1024

typedef std::wstring wstring;

// 型 
static const wstring	OPTION_TYPE_BOOL = wstring( L"bool" );
static const wstring	OPTION_TYPE_INT  = wstring( L"int" );
static const wstring	OPTION_TYPE_SEL  = wstring( L"sel" );
static const wstring	OPTION_TYPE_DIR  = wstring( L"dir" );

class CDlgPluginOption final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgPluginOption();
	~CDlgPluginOption();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE hInstance, HWND hwndParent, CPropPlugin* cPropPlugin, int ID );	/* モーダルダイアログの表示 */

protected:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam ) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	BOOL	OnCbnSelChange( HWND hwndCtl, int wID ) override;
	BOOL	OnEnChange( HWND hwndCtl, int wID ) override;
	BOOL	OnActivate( WPARAM wParam, LPARAM lParam ) override;
	LPVOID	GetHelpIdTable( void ) override;

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */

	void	ChangeListPosition( void );					// 編集領域をリストビューに合せて切替える
	void	MoveFocusToEdit( void );					// 編集領域にフォーカスを移す
	void	SetToEdit(int iLine);
	void	SetFromEdit(int iLine);
	void	SelectEdit(int IDCenable);							// 編集領域の切り替え
	void	SepSelect(wstring sTrg, wstring* spView, wstring* spValue);	// 選択用文字列分解
	void	SelectDirectory( int iLine );				// ディレクトリを選択する

private:
	CPlugin*		m_cPlugin;
	CPropPlugin*	m_cPropPlugin;
	int 			m_ID;			// プラグイン番号（エディタがふる番号）
	int				m_Line;			// 現在編集中のオプション行番号
	std::wstring	m_sReadMeName;	// ReadMe ファイル名
};
