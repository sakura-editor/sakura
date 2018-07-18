/*!	@file
	@brief ファイルツリー設定

	@author Moca
	@date 2014.06.07
*/
/*
	Copyright (C) 2014, Moca

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

#ifndef SAKURA_CDLGFILETREE_H_
#define SAKURA_CDLGFILETREE_H_

#include "dlg/CDialog.h"
#include "outline/CDlgFuncList.h"

/*!
	@brief ファイルツリー設定ダイアログ
*/
class CDlgFileTree : public CDialog
{
public:
	CDlgFileTree();

	int DoModal( HINSTANCE, HWND, LPARAM );

private:
	BOOL	OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM, LPARAM );
	LPVOID	GetHelpIdTable();
	void	SetData();
	int		GetData();

	void	SetDataInit();
	void	SetDataItem(int);
	void	ChangeEnableItemType();
	void	ChangeEnableAddInsert();
	int		GetDataItem( SFileTreeItem& );
	bool	GetDataTree( std::vector<SFileTreeItem>&, HTREEITEM, int, int );
	HTREEITEM InsertTreeItem( SFileTreeItem&, HTREEITEM, HTREEITEM );

private:
	CDlgFuncList*		m_pcDlgFuncList;
	CFileTreeSetting	m_fileTreeSetting;
	std::vector<int>	m_aItemRemoveList;
	int					m_nlParamCount;
	int					m_nDocType;

	int					m_bInMove;
};



#endif /* SAKURA_CDLGFILETREE_H_ */


