﻿/*!	@file
	@brief ファイル一覧ダイアログボックス

	@author Moca
	@date 2015.03.07
*/
/*
	Copyright (C) 2015, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CDLGWINDOWLIST_EBED03A4_1BD8_4EEE_93D0_32E8C3A1F5FD_H_
#define SAKURA_CDLGWINDOWLIST_EBED03A4_1BD8_4EEE_93D0_32E8C3A1F5FD_H_
#pragma once

#include "dlg/CDialog.h"
class CDlgWindowList final : public CDialog
{
public:
	CDlgWindowList();

	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);
protected:
	BOOL	OnBnClicked(int wID) override;
	LPVOID	GetHelpIdTable() override;
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL OnDestroy( void ) override;
	BOOL OnSize(WPARAM wParam, LPARAM lParam) override;
	BOOL OnMinMaxInfo(LPARAM lParam);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam) override;

	void SetData() override;
	int  GetData() override;

	void GetDataListView(std::vector<HWND>& aHwndList);
	void CommandClose();
	void CommandSave();
private:
	POINT		m_ptDefaultSize;
	RECT		m_rcItems[5];
};
#endif /* SAKURA_CDLGWINDOWLIST_EBED03A4_1BD8_4EEE_93D0_32E8C3A1F5FD_H_ */
