/*!	@file
	@brief ファイルツリー設定

	@author Moca
	@date 2014.06.07
*/
/*
	Copyright (C) 2014, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CDLGFILETREE_58915B5A_6480_4286_AB4F_28A7A6502AEC_H_
#define SAKURA_CDLGFILETREE_58915B5A_6480_4286_AB4F_28A7A6502AEC_H_
#pragma once

#include "dlg/CDialog.h"
#include "outline/CDlgFuncList.h"

/*!
	@brief ファイルツリー設定ダイアログ
*/
class CDlgFileTree final : public CDialog
{
public:
	CDlgFileTree();

	int DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam);

private:
	BOOL	OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam) override;
	BOOL	OnBnClicked(int wID) override;
	BOOL	OnNotify(NMHDR* pNMHDR) override;
	LPVOID	GetHelpIdTable() override;
	void	SetData() override;
	int		GetData() override;

	void	SetDataInit();
	void	SetDataItem(int nItemIndex);
	void	ChangeEnableItemType();
	void	ChangeEnableAddInsert();
	int		GetDataItem(SFileTreeItem& item);
	bool	GetDataTree(std::vector<SFileTreeItem>& data, HTREEITEM hItem, int nLevel, int nMaxCount);
	HTREEITEM InsertTreeItem(SFileTreeItem& item, HTREEITEM htiParent, HTREEITEM htiInsert);

private:
	CDlgFuncList*		m_pcDlgFuncList;
	CFileTreeSetting	m_fileTreeSetting;
	std::vector<int>	m_aItemRemoveList;
	int					m_nDocType;

	int					m_bInMove;
};
#endif /* SAKURA_CDLGFILETREE_58915B5A_6480_4286_AB4F_28A7A6502AEC_H_ */
