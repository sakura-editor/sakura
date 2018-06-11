/*!	@file
	@brief ファイル一覧ダイアログボックス

	@author Moca
	@date 2015.03.07
*/
/*
	Copyright (C) 2015, Moca

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

#ifndef SAKURA_CWINDOWLIST_H_
#define SAKURA_CWINDOWLIST_H_

#include "dlg/CDialog.h"
class CDlgWindowList : public CDialog
{
public:
	CDlgWindowList();

	int DoModal(HINSTANCE, HWND, LPARAM);
protected:
	BOOL	OnBnClicked(int);
	LPVOID	GetHelpIdTable();
	INT_PTR DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnSize(WPARAM wParam, LPARAM lParam);
	BOOL OnMove(WPARAM wParam, LPARAM lParam);
	BOOL OnMinMaxInfo(LPARAM lParam);
	BOOL OnActivate(WPARAM wParam, LPARAM lParam);

	void SetData();
	int  GetData();

	void GetDataListView(std::vector<HWND>&);
	void CommandClose();
	void CommandSave();
private:
	POINT		m_ptDefaultSize;
	RECT		m_rcItems[5];
};

#endif /* SAKURA_CWINDOWLIST_H_ */
