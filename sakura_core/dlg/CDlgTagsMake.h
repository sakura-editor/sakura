/*!	@file
	@brief タグファイル作成ダイアログボックス

	@author MIK
	@date 2003.5.12
*/
/*
	Copyright (C) 2003, MIK

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

class CDlgTagsMake;

#include "dlg/CDialog.h"
/*!
	@brief タグファイル作成ダイアログボックス
*/
class CDlgTagsMake final : public CDialog
{
  public:
    /*
	||  Constructors
	*/
    CDlgTagsMake();

    /*
	||  Attributes & Operations
	*/
    int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, const WCHAR *pszPath); /* モーダルダイアログの表示 */

    WCHAR m_szPath[_MAX_PATH + 1]; /* フォルダ */
    WCHAR m_szTagsCmdLine[_MAX_PATH]; /* コマンドラインオプション(個別) */
    int m_nTagsOpt; /* CTAGSオプション(チェック) */

  protected:
    /*
	||  実装ヘルパ関数
	*/
    BOOL OnBnClicked(int wID) override;
    LPVOID GetHelpIdTable(void) override;

    void SetData(void) override; /* ダイアログデータの設定 */
    int GetData(void) override; /* ダイアログデータの取得 */

  private:
    void SelectFolder(HWND hwndDlg);
};
