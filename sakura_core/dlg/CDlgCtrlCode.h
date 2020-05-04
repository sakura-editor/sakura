/*!	@file
	@brief コントロールコード入力ダイアログボックス

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002, MIK

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

class CDlgCtrlCode;

#include "dlg/CDialog.h"
/*!
	@brief コントロールコード入力ダイアログボックス
*/
//2007.10.18 kobake GetCharCode()を作成。
class CDlgCtrlCode final : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCtrlCode();

	/*
	||  Attributes & Operations
	*/
	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);	/* モーダルダイアログの表示 */

	wchar_t GetCharCode() const{ return m_nCode; } //!< 選択された文字コードを取得

private:
	/*
	||  実装ヘルパ関数
	*/
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	LPVOID	GetHelpIdTable( void ) override;

	void	SetData( void ) override;	/* ダイアログデータの設定 */
	int		GetData( void ) override;	/* ダイアログデータの取得 */

private:
	/*
	|| メンバ変数
	*/
	wchar_t		m_nCode;	// コード
};
