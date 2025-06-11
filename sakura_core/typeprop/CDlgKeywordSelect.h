/*!	@file
	@brief 強調キーワード選択ダイアログ

	@author MIK
	@date 2005/01/13 作成
*/
/*
	Copyright (C) 2005, MIK, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGKEYWORDSELECT_3430FB9A_96EE_492E_A8FD_FEB4CDB553AA_H_
#define SAKURA_CDLGKEYWORDSELECT_3430FB9A_96EE_492E_A8FD_FEB4CDB553AA_H_
#pragma once

#include "dlg/CDialog.h"
#include "config/maxdata.h" // MAX_KEYWORDSET_PER_TYPE
 
class CKeyWordSetMgr;

/*
	強調キーワード選択可能数
	1～10個の範囲で指定できる。
	ただし、ソースの修正は必要です。
*/

//	2005.01.13 genta ShareDataの定義と連動させる
const int KEYWORD_SELECT_NUM = MAX_KEYWORDSET_PER_TYPE;

class CDlgKeywordSelect final : public CDialog
{
public:
	CDlgKeywordSelect();
	~CDlgKeywordSelect();
	int DoModal( HINSTANCE hInstance, HWND hwndParent, int* pnSet );

protected:

	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnBnClicked(int wID) override;
	int  GetData( void ) override;
	void SetData( void ) override;
	LPVOID GetHelpIdTable( void ) override;

	int m_nSet[ KEYWORD_SELECT_NUM ];
	CKeyWordSetMgr*	m_pCKeyWordSetMgr;
};
#endif /* SAKURA_CDLGKEYWORDSELECT_3430FB9A_96EE_492E_A8FD_FEB4CDB553AA_H_ */
