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



class CDlgPluginOption;

#ifndef	_CDLGPLUGINOPTION_H_
#define	_CDLGPLUGINOPTION_H_

#include "dlg/CDialog.h"
#include "plugin/CPluginManager.h"

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

class SAKURA_CORE_API CDlgPluginOption : public CDialog
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
	int DoModal( HINSTANCE, HWND, int );	/* モーダルダイアログの表示 */

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

	void	ChangeListPosition( void );
	void	SetToEdit( int );
	void	SetFromEdit( int );
	void	SelectEdit( int );							// 編集領域の切り替え
	void	SepSelect( wstring, wstring*, wstring* );	// 選択用文字列分解

private:
	CPlugin*		m_cPlugin;
	int 			m_ID;			// プラグイン番号（エディタがふる番号）
	int				m_Line;			// 現在編集中のオプション行番号
};

#endif	//_CDLGPLUGINOPTION_H_

