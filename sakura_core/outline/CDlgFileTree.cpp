/*!	@file
	@brief ファイルツリー設定ダイアログ

	@author Moca
	@date 2014.06.07
*/
/*
	Copyright (C) 2010, Uchi
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

#include "StdAfx.h"
#include "outline/CDlgFileTree.h"
#include "outline/CDlgFuncList.h"
#include "dlg/CDlgOpenFile.h"
#include "dlg/CDlgInput1.h"
#include "env/CDocTypeManager.h"
#include "typeprop/CImpExpManager.h"
#include "CDataProfile.h"
#include "util/shell.h"
#include "util/window.h"
#include "util/os.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {	//13300
	IDC_CHECK_LOADINI,      HIDC_CHECK_FILETREE_LOADINI,
	IDC_EDIT_DEFINI,        HIDC_EDIT_FILETREE_DEFINI,
	IDC_BUTTON_REF1,        HIDC_BUTTON_FILETREE_REF1,
	IDC_BUTTON_LOAD,        HIDC_BUTTON_FILETREE_LOAD,
	IDC_RADIO_GREP,         HIDC_RADIO_FILETREE_GREP,
	IDC_RADIO_FILE,         HIDC_RADIO_FILETREE_FILE,
	IDC_RADIO_FOLDER,       HIDC_RADIO_FILETREE_FOLDER,
	IDC_STATIC_PATH,        HIDC_STATIC_FILETREE_PATH,
	IDC_EDIT_PATH,          HIDC_EDIT_FILETREE_PATH,
	IDC_BUTTON_REF2,        HIDC_BUTTON_FILETREE_REF2,
	IDC_BUTTON_PATH_MENU,   HIDC_BUTTON_FILETREE_PATH_MENU,
	IDC_EDIT_LABEL,         HIDC_EDIT_FILETREE_LABEL,
	IDC_STATIC_FILE,        HIDC_STATIC_FILETREE_FILE,
	IDC_EDIT_FILE,          HIDC_EDIT_FILETREE_FILE,
	IDC_CHECK_HIDDEN,       HIDC_CHECK_FILETREE_HIDDEN,
	IDC_CHECK_READONLY,     HIDC_CHECK_FILETREE_READONLY,
	IDC_CHECK_SYSTEM,       HIDC_CHECK_FILETREE_SYSTEM,
	IDC_BUTTON_DELETE,      HIDC_BUTTON_FILETREE_DELETE,
	IDC_BUTTON_INSERT,      HIDC_BUTTON_FILETREE_INSERT,
	IDC_BUTTON_INSERT_A,    HIDC_BUTTON_FILETREE_INSERT_A,
	IDC_BUTTON_ADD,         HIDC_BUTTON_FILETREE_ADD,
	IDC_BUTTON_UPDATE,      HIDC_BUTTON_FILETREE_UPDATE,
	IDC_BUTTON_FILEADD,     HIDC_BUTTON_FILETREE_FILEADD,
	IDC_BUTTON_REPLACE,     HIDC_BUTTON_FILETREE_REPLACE,
	IDC_BUTTON_UP,          HIDC_BUTTON_FILETREE_UP,
	IDC_BUTTON_DOWN,        HIDC_BUTTON_FILETREE_DOWN,
	IDC_BUTTON_RIGHT,       HIDC_BUTTON_FILETREE_RIGHT,
	IDC_BUTTON_LEFT,        HIDC_BUTTON_FILETREE_LEFT,
	IDC_TREE_FL,            HIDC_TREE_FILETREE_FL,
	IDC_BUTTON_IMPORT,      HIDC_BUTTON_FILETREE_IMPORT,
	HIDC_BUTTON_EXPORT,     HIDC_BUTTON_FILETREE_EXPORT,
	IDOK,                   HIDC_FILETREE_IDOK,
	IDCANCEL,               HIDC_FILETREE_IDCANCEL,
	IDC_BUTTON_HELP,        HIDC_BUTTON_FILETREE_HELP,
//	IDC_STATIC,				-1,
	0, 0
};

CDlgFileTree::CDlgFileTree()
{
	m_bInMove = false;
}

/*! モーダルダイアログの表示
	lParam は CDlgFuncList*
	入力はlParam経由で取得。
	結果の設定はCDlgFileTreeが直接共通設定・タイプ別・設定ファイルに書き込みをして
	呼び出し元は、再表示で設定される
*/
int CDlgFileTree::DoModal(
	HINSTANCE	hInstance,
	HWND		hwndParent,
	LPARAM		lParam
)
{
	m_pcDlgFuncList = reinterpret_cast<CDlgFuncList*>(lParam);
	m_nDocType = m_pcDlgFuncList->m_nDocType;
	return (int)CDialog::DoModal(hInstance, hwndParent, IDD_FILETREE, lParam);
}


// LS()を使用しているのですぐ使うこと
static TCHAR* GetFileTreeLabel( const SFileTreeItem& item )
{
	const TCHAR* pszLabel;
	if( item.m_eFileTreeItemType != EFileTreeItemType_Folder ){
		pszLabel = item.m_szLabelName;
		if( item.m_szLabelName[0] == _T('\0') ){
			pszLabel = item.m_szTargetPath;
			if( 0 == auto_strcmp(pszLabel, _T("."))
			  || 0 == auto_strcmp(pszLabel, _T(".\\")) 
			  || 0 == auto_strcmp(pszLabel, _T("./")) ){
				pszLabel = LS(STR_FILETREE_CURDIR);
			}
		}
	}else{
		pszLabel = item.m_szLabelName;
		if( pszLabel[0] == _T('\0') ){
			pszLabel = _T("Folder");
		}
	}
	return const_cast<TCHAR*>(pszLabel);
}

/* ダイアログデータの設定 */
void CDlgFileTree::SetData()
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	std::vector<HTREEITEM> hParentTree;
	hParentTree.push_back(TVI_ROOT);
	HTREEITEM hSelect = NULL;
	m_aItemRemoveList.clear();
	TreeView_DeleteAllItems(hwndTree);
	bool bSaveShareData = (m_fileTreeSetting.m_szLoadProjectIni[0] == _T('\0'));
	for( int i = 0; i < (int)m_fileTreeSetting.m_aItems.size(); i++ ){
		int nMaxCount = _countof(GetDllShareData().m_Common.m_sOutline.m_sFileTree.m_aItems);
		if( bSaveShareData && nMaxCount < i + 1 ){
			::InfoMessage(GetHwnd(), LS(STR_FILETREE_MAXCOUNT), nMaxCount);
		}
		const SFileTreeItem& item = m_fileTreeSetting.m_aItems[i];
		while( item.m_nDepth < (int)hParentTree.size() - 1 ){
			hParentTree.resize(hParentTree.size() - 1);
		}
		TVINSERTSTRUCT tvis;
		tvis.hParent      = hParentTree.back();
		tvis.item.mask    = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvis.item.lParam  = i;
		tvis.item.pszText = GetFileTreeLabel(item);
		tvis.item.cChildren = (item.m_eFileTreeItemType == EFileTreeItemType_Folder) ? 1 : 0;
		HTREEITEM hParent = TreeView_InsertItem(hwndTree, &tvis);
		if( item.m_eFileTreeItemType == EFileTreeItemType_Folder ){
			hParentTree.push_back(hParent);
		}
		if( hSelect == NULL ){
			hSelect = hParent;
		}
	}
	if( hSelect != NULL ){
		TreeView_SelectItem(hwndTree, hSelect);
	}
	int nIndex = (m_fileTreeSetting.m_aItems.size() == 0 ? -1: 0);
	SetDataItem(nIndex);
	ChangeEnableAddInsert();
	return;
}

void CDlgFileTree::SetDataItem(int nItemIndex)
{
	HWND hwndDlg = GetHwnd();
	bool bDummy = false;
	if( nItemIndex < 0 || (int)m_fileTreeSetting.m_aItems.size() <= nItemIndex ){
		bDummy = true;
	}
	SFileTreeItem itemDummy;
	const SFileTreeItem& item = (bDummy ? itemDummy : m_fileTreeSetting.m_aItems[nItemIndex]);
	itemDummy.m_szTargetFile = _T("*.*");
	int nIDs[] ={IDC_RADIO_GREP, IDC_RADIO_FILE, IDC_RADIO_FOLDER};
	int nID1;
	int nID2, nID3;
	switch( item.m_eFileTreeItemType ){
	case EFileTreeItemType_Grep:   nID1 = 0; nID2 = 1; nID3 = 2; break;
	case EFileTreeItemType_File:   nID1 = 1; nID2 = 0; nID3 = 2; break;
	case EFileTreeItemType_Folder: nID1 = 2; nID2 = 0; nID3 = 1; break;
	}
	::CheckDlgButton(hwndDlg, nIDs[nID1], TRUE);
	::CheckDlgButton(hwndDlg, nIDs[nID2], FALSE);
	::CheckDlgButton(hwndDlg, nIDs[nID3], FALSE);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_PATH, item.m_szTargetPath);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_LABEL, item.m_szLabelName);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_FILE, item.m_szTargetFile);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_HIDDEN, item.m_bIgnoreHidden);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_READONLY, item.m_bIgnoreReadOnly);
	::CheckDlgButtonBool(hwndDlg, IDC_CHECK_SYSTEM, item.m_bIgnoreSystem);
	ChangeEnableItemType();
	return;
}

void CDlgFileTree::ChangeEnableItemType()
{
	HWND hwndDlg = GetHwnd();
	BOOL bGrepEnable = FALSE;
	BOOL bPathEnable = FALSE;
	if( IsDlgButtonCheckedBool(hwndDlg, IDC_RADIO_GREP) ){ 
		bGrepEnable = TRUE;
		bPathEnable = TRUE;
	}else if( IsDlgButtonCheckedBool(hwndDlg, IDC_RADIO_FILE) ){ 
		bPathEnable = TRUE;
	}
	::EnableWindow(GetItemHwnd(IDC_STATIC_PATH), bPathEnable);
	::EnableWindow(GetItemHwnd(IDC_EDIT_PATH), bPathEnable);
	::EnableWindow(GetItemHwnd(IDC_BUTTON_REF2), bPathEnable);
	::EnableWindow(GetItemHwnd(IDC_BUTTON_PATH_MENU), bPathEnable);
	::EnableWindow(GetItemHwnd(IDC_STATIC_FILE), bGrepEnable);
	::EnableWindow(GetItemHwnd(IDC_EDIT_FILE), bGrepEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_HIDDEN), bGrepEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_READONLY), bGrepEnable);
	::EnableWindow(GetItemHwnd(IDC_CHECK_SYSTEM), bGrepEnable);
}

void CDlgFileTree::ChangeEnableAddInsert()
{
	bool bSaveShareData = (m_fileTreeSetting.m_szLoadProjectIni[0] == _T('\0'));
	if( bSaveShareData ){
		int nCount = TreeView_GetCount(GetItemHwnd(IDC_TREE_FL));
		bool bEnable = true;
		int nMaxCount = _countof(GetDllShareData().m_Common.m_sOutline.m_sFileTree.m_aItems);
		if( nMaxCount < nCount ){
			bEnable = false;
		}
		::EnableWindow(GetItemHwnd(IDC_BUTTON_ADD), bEnable);
		::EnableWindow(GetItemHwnd(IDC_BUTTON_INSERT), bEnable);
		::EnableWindow(GetItemHwnd(IDC_BUTTON_INSERT_A), bEnable);
	}
}

/* ダイアログデータの取得 */
/* TRUE==正常  FALSE==入力エラー */
int CDlgFileTree::GetData()
{
	HWND hwndDlg = GetHwnd();
	SFileTree* pFileTree;
	STypeConfig type;
	bool bTypeError = false;
	if( m_fileTreeSetting.m_eFileTreeSettingOrgType == EFileTreeSettingFrom_Common ){
		pFileTree = &GetDllShareData().m_Common.m_sOutline.m_sFileTree;
	}else{
		if( false == CDocTypeManager().GetTypeConfig(CTypeConfig(m_nDocType), type) ){
			bTypeError = true;
			pFileTree = NULL;
		}else{
			pFileTree = &type.m_sFileTree;
		}
	}
	bool bSaveShareData = (m_fileTreeSetting.m_szLoadProjectIni[0] == _T('\0'));
	std::vector<SFileTreeItem> items;
	if( !GetDataTree(items, TreeView_GetRoot(GetItemHwnd(IDC_TREE_FL)), 0, (bSaveShareData ? _countof(pFileTree->m_aItems) : 0) ) ){
		InfoMessage(GetHwnd(), LS(STR_FILETREE_MAXCOUNT));
	}
	if( pFileTree ){
		pFileTree->m_bProject = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_LOADINI);
		DlgItem_GetText(hwndDlg, IDC_EDIT_DEFINI, pFileTree->m_szProjectIni, pFileTree->m_szProjectIni.GetBufferCount());
		if( bSaveShareData ){
			pFileTree->m_nItemCount = (int)items.size();
			assert(pFileTree->m_nItemCount <= _countof(pFileTree->m_aItems));
			for( int i = 0; i < pFileTree->m_nItemCount; i++ ){
				pFileTree->m_aItems[i] = items[i];
			}
		}
		if( m_fileTreeSetting.m_eFileTreeSettingOrgType == EFileTreeSettingFrom_Type ){
			CDocTypeManager().SetTypeConfig(CTypeConfig(m_nDocType), type);
		}
	}
	if( false == bSaveShareData ){
		// 元のiniに保存
		CImpExpFileTree cImpExp(items);
		std::wstring strIni = to_wchar(m_fileTreeSetting.m_szLoadProjectIni);
		std::wstring strError;
		if( false == cImpExp.Export(strIni, strError) ){
			ErrorMessage(hwndDlg, _T("%ls"), strError.c_str());
		}
	}
	return TRUE;
}

bool CDlgFileTree::GetDataTree(std::vector<SFileTreeItem>& data, HTREEITEM hItem, int nLevel, int nMaxCount)
{
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	for( HTREEITEM s = hItem; s != NULL; s = TreeView_GetNextSibling(hwndTree, s) ){
		TV_ITEM	tvi;
		tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		if( !TreeView_GetItem(hwndTree, &tvi) ){
			return false;
		}
		if( 0 < nMaxCount && nMaxCount <= (int)data.size() ){
			return false;
		}
		data.push_back(m_fileTreeSetting.m_aItems[tvi.lParam]);
		data.back().m_nDepth = nLevel;
		if( 0 < tvi.cChildren ){
			HTREEITEM ts = TreeView_GetChild(hwndTree, s);
			if( ts != NULL ){
				if( !GetDataTree(data, ts, nLevel+1, nMaxCount) ){
					return false;
				}
			}
		}
	}
	return true;
}

int CDlgFileTree::GetDataItem( SFileTreeItem& item )
{
	HWND hwndDlg = GetHwnd();
	item = SFileTreeItem(); // 初期化
	BOOL bGrepEnable = FALSE;
	BOOL bPathEnable = FALSE;
	if( IsDlgButtonCheckedBool(hwndDlg, IDC_RADIO_GREP) ){
		bGrepEnable = TRUE;
		bPathEnable = TRUE;
		item.m_eFileTreeItemType = EFileTreeItemType_Grep;
	}else if( IsDlgButtonCheckedBool(hwndDlg, IDC_RADIO_FILE) ){ 
		bPathEnable = TRUE;
		item.m_eFileTreeItemType = EFileTreeItemType_File;
	}else{
		item.m_eFileTreeItemType = EFileTreeItemType_Folder;
	}
	if( bPathEnable ){
		::DlgItem_GetText(hwndDlg, IDC_EDIT_PATH, item.m_szTargetPath, item.m_szTargetPath.GetBufferCount());
	}
	::DlgItem_GetText(hwndDlg, IDC_EDIT_LABEL, item.m_szLabelName, item.m_szLabelName.GetBufferCount());
	if( bGrepEnable ){
		::DlgItem_GetText(hwndDlg, IDC_EDIT_FILE, item.m_szTargetFile, item.m_szTargetFile.GetBufferCount());
		item.m_bIgnoreHidden = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_HIDDEN);
		item.m_bIgnoreReadOnly = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_READONLY);
		item.m_bIgnoreSystem = IsDlgButtonCheckedBool(hwndDlg, IDC_CHECK_SYSTEM);	
	}
	return TRUE;
}

BOOL CDlgFileTree::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	_SetHwnd(hwndDlg);
	SFileTreeItem item;

	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_DEFINI), m_fileTreeSetting.m_szDefaultProjectIni.GetBufferCount() -1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_PATH), item.m_szTargetPath.GetBufferCount() -1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_LABEL), item.m_szLabelName.GetBufferCount() -1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_FILE), item.m_szTargetFile.GetBufferCount() -1);

	CFilePath path;
	m_pcDlgFuncList->LoadFileTreeSetting(m_fileTreeSetting, path);
	SetDataInit();

	/* 基底クラスメンバ */
	return CDialog::OnInitDialog(GetHwnd(), wParam, lParam);
}

void CDlgFileTree::SetDataInit()
{
	HWND hwndDlg = GetHwnd();
	BOOL bEnableDefIni = TRUE;
	if( m_fileTreeSetting.m_eFileTreeSettingLoadType != EFileTreeSettingFrom_File ){
		int id;
		if( m_fileTreeSetting.m_eFileTreeSettingLoadType == EFileTreeSettingFrom_Common ){
			id = STR_FILETREE_FROM_COMMON;
		}else{
			id = STR_FILETREE_FROM_TYPE;
		}
		std::tstring str = LS(id);
		if( m_fileTreeSetting.m_szLoadProjectIni[0] != _T('\0') ){
			str += _T("+");
			str += LS(F_FILE_TOPMENU);
		}
		::SetWindowText(GetItemHwnd(IDC_STATIC_SETTFING_FROM), str.c_str() );
	}else{
		TCHAR szMsg[_MAX_PATH+200];
		const TCHAR* pFile = m_fileTreeSetting.m_szLoadProjectIni;
		TCHAR szFilePath[_MAX_PATH];
		CTextWidthCalc calc(GetHwnd(), IDC_STATIC_SETTFING_FROM);
		RECT rc;
		GetWindowRect( GetItemHwnd(IDC_STATIC_SETTFING_FROM), &rc);
		const int xWidth = calc.GetTextWidth(_T("x"));
		const int ctrlWidth = rc.right - rc.left;
		int nMaxCch = ctrlWidth / xWidth;
		CFileNameManager::getInstance()->GetTransformFileNameFast(pFile, szFilePath, _countof(szFilePath), calc.GetDC(), true, nMaxCch);
		wsprintf(szMsg, LS(STR_FILETREE_FROM_FILE), szFilePath);
		::SetWindowText(GetItemHwnd(IDC_STATIC_SETTFING_FROM), szMsg);
		bEnableDefIni = FALSE;
	}
	::EnableWindow(GetItemHwnd(IDC_BUTTON_LOAD), bEnableDefIni);
	CheckDlgButtonBool(hwndDlg, IDC_CHECK_LOADINI, m_fileTreeSetting.m_bProject);
	::DlgItem_SetText(hwndDlg, IDC_EDIT_DEFINI, m_fileTreeSetting.m_szDefaultProjectIni); 
}

HTREEITEM CDlgFileTree::InsertTreeItem(SFileTreeItem& item, HTREEITEM htiParent, HTREEITEM htiInsert )
{
	int nlParam;
	if( m_aItemRemoveList.empty() ){
		nlParam = m_fileTreeSetting.m_aItems.size();
		m_fileTreeSetting.m_aItems.push_back(item);
	}else{
		// 削除リストから復活させる
		nlParam = m_aItemRemoveList.back();
		m_aItemRemoveList.pop_back();
		m_fileTreeSetting.m_aItems[nlParam] = item;
	}
	TV_INSERTSTRUCT	tvis;
	tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
	tvis.hParent = htiParent;
	tvis.hInsertAfter = htiInsert;
	tvis.item.pszText = GetFileTreeLabel(item);
	tvis.item.lParam = nlParam;
	tvis.item.cChildren = (item.m_eFileTreeItemType == EFileTreeItemType_Folder) ? 1 : 0;
	return TreeView_InsertItem(GetItemHwnd(IDC_TREE_FL), &tvis);
}


// ツリーのコピー
//		fChildがtrueの時はdstの子としてコピー, そうでなければdstの兄弟としてdstの後ろにコピー
//		fOnryOneがtrueの時は1つだけコピー（子があったらコピー）
static HTREEITEM FileTreeCopy( HWND hwndTree, HTREEITEM dst, HTREEITEM src, bool fChild, bool fOnryOne )
{
	HTREEITEM		s;
	HTREEITEM		ts;
	HTREEITEM		td = NULL;
	TV_INSERTSTRUCT	tvis;		// 挿入用
	TV_ITEM			tvi;		// 取得用
	int				n = 0;
	TCHAR			szLabel[_MAX_PATH];

	for (s = src; s != NULL; s = fOnryOne ? NULL:TreeView_GetNextSibling( hwndTree, s )) {
		tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		tvi.hItem = s;
		tvi.pszText = szLabel;
		tvi.cchTextMax = _countof(szLabel);
		if (!TreeView_GetItem( hwndTree, &tvi )) {
			// Error
			break;
		}
		tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;
		if (fChild || n != 0) {
			// dstの子供として作成
			tvis.hParent = dst;
			tvis.hInsertAfter = TVI_LAST;
		}
		else {
			//	dstの兄弟として作成
			tvis.hParent = TreeView_GetParent( hwndTree, dst );
			tvis.hInsertAfter = dst;
		}
		tvis.item.pszText = szLabel;
		tvis.item.lParam = tvi.lParam;
		tvis.item.cChildren = tvi.cChildren;
		td = TreeView_InsertItem( hwndTree, &tvis );	//	Itemの作成

		if (tvi.cChildren) {
			ts = TreeView_GetChild( hwndTree, s );	//	子の取得
			if (ts != NULL) {
				FileTreeCopy( hwndTree, td, ts, true, false );
			}
			// 展開
			if (tvi.state & TVIS_EXPANDEDONCE) {
				TreeView_Expand( hwndTree, td, TVE_EXPAND );
			}
		}
		n++;
	}

	return td;
}


BOOL CDlgFileTree::OnBnClicked( int wID )
{
	switch( wID )
	{
	case IDC_BUTTON_REF1:
		{
			CDlgOpenFile::SelectFile( GetHwnd(), GetItemHwnd(IDC_EDIT_DEFINI), _T("*.ini"), true, EFITER_NONE );
		}
		return TRUE;
	case IDC_BUTTON_LOAD:
		{
			DlgItem_GetText( GetHwnd(), IDC_EDIT_DEFINI, m_fileTreeSetting.m_szDefaultProjectIni, m_fileTreeSetting.m_szDefaultProjectIni.GetBufferCount() );
			if( m_fileTreeSetting.m_szDefaultProjectIni[0] != _T('\0') ){
				CDataProfile cProfile;
				cProfile.SetReadingMode();
				const TCHAR* pszIniFileName;
				TCHAR szDir[_MAX_PATH * 2];
				if( _IS_REL_PATH( m_fileTreeSetting.m_szDefaultProjectIni ) ){
					// sakura.iniからの相対パス
					GetInidirOrExedir( szDir, m_fileTreeSetting.m_szDefaultProjectIni );
					pszIniFileName = szDir;
				}else{
					pszIniFileName = m_fileTreeSetting.m_szDefaultProjectIni;
				}
				if( cProfile.ReadProfile(pszIniFileName) ){
					CImpExpFileTree::IO_FileTreeIni(cProfile, m_fileTreeSetting.m_aItems);
					m_fileTreeSetting.m_szLoadProjectIni = pszIniFileName;
				}
			}
			SetDataInit();
			SetData();
		}
		return TRUE;
	case IDC_RADIO_GREP:
	case IDC_RADIO_FILE:
	case IDC_RADIO_FOLDER:
		{
			ChangeEnableItemType();
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			BOOL bEnableUpdate = TRUE;
			if( htiItem != NULL && !IsDlgButtonCheckedBool(GetHwnd(), IDC_RADIO_FOLDER) ){
				TV_ITEM tvi;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if( TreeView_GetItem(hwndTree, &tvi)
				  && m_fileTreeSetting.m_aItems[tvi.lParam].m_eFileTreeItemType ==  EFileTreeItemType_Folder
				  && NULL != TreeView_GetChild(hwndTree, htiItem) ){
					// [Folder]以外を子がいるFolderに上書きするの禁止
					bEnableUpdate = FALSE;
				}
			}
			EnableWindow( GetItemHwnd(IDC_BUTTON_UPDATE), bEnableUpdate );
		}
		return TRUE;
	case IDC_BUTTON_REF2:
		{
			HWND hwndDlg = GetHwnd();
			if( IsDlgButtonCheckedBool(hwndDlg, IDC_RADIO_GREP) ){
				// RADIO_GREP == folder
				TCHAR szDir[MAX_PATH];
				DlgItem_GetText(GetHwnd(), IDC_EDIT_PATH, szDir, _countof(szDir) );
				if( SelectDir(hwndDlg, LS(STR_DLGGREP1), szDir, szDir) ){
					DlgItem_SetText(GetHwnd(), IDC_EDIT_PATH, szDir );
				}
			}else{
				// RADIO_FILE == file
				CDlgOpenFile dlg;
				TCHAR szDir[_MAX_PATH];
				GetInidir(szDir);
				dlg.Create( G_AppInstance(), hwndDlg, _T("*.*"), szDir,
					std::vector<LPCTSTR>(), std::vector<LPCTSTR>() );
				TCHAR szFile[_MAX_PATH];
				if( dlg.DoModal_GetOpenFileName(szFile) ){
					CNativeT cmemFile = szFile;
					cmemFile.ReplaceT(_T("%"), _T("%%"));
					DlgItem_SetText( GetHwnd(), IDC_EDIT_PATH, cmemFile.GetStringPtr() );
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_PATH_MENU:
		{
			const int MENU_ROOT = 0x100;
			const int MENU_MYDOC = 0x101;
			const int MENU_MYMUSIC = 0x102;
			const int MENU_MYVIDEO = 0x103;
			const int MENU_DESK = 0x104;
			const int MENU_TEMP = 0x105;
			const int MENU_SAKURA = 0x106;
			const int MENU_SAKURADATA = 0x107;
			HMENU hMenu = ::CreatePopupMenu();
			int iPos = 0;
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_ROOT, LS(STR_FILETREE_MENU_ROOT) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYDOC, LS(STR_FILETREE_MENU_MYDOC) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYMUSIC, LS(STR_FILETREE_MENU_MYMUSIC) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_MYVIDEO, LS(STR_FILETREE_MENU_MYVIDEO) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_DESK, LS(STR_FILETREE_MENU_DESK) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_TEMP, LS(STR_FILETREE_MENU_TEMP) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_SAKURA, LS(STR_FILETREE_MENU_SAKURA) );
			::InsertMenu( hMenu, iPos++, MF_BYPOSITION | MF_STRING, MENU_SAKURADATA, LS(STR_FILETREE_MENU_SAKURADATA) );
			POINT pt;
			RECT rc;
			::GetWindowRect( GetItemHwnd(IDC_BUTTON_PATH_MENU), &rc );
			pt.x = rc.left;
			pt.y = rc.bottom;
			RECT rcWork;
			GetMonitorWorkRect( pt, &rcWork );	// モニタのワークエリア
			int nId = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD,
										( pt.x > rcWork.left )? pt.x: rcWork.left,
										( pt.y < rcWork.bottom )? pt.y: rcWork.bottom,
										0, GetHwnd(), NULL);
			::DestroyMenu( hMenu );
			if( nId != 0 ){
				int index = nId - MENU_ROOT;
				const TCHAR* pszPaths[] = { _T("<iniroot>"), _T("%MYDOC%"), _T("%MYMUSIC%"), _T("%MYVIDEO%"),
					_T("%DESKTOP%"), _T("%TEMP%"), _T("%SAKURA%"), _T("%SAKURADATA%") };
				EditCtl_ReplaceSel(GetItemHwnd(IDC_EDIT_PATH), pszPaths[index]);
			}
		}
		return TRUE;
	case IDC_BUTTON_DELETE:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if( htiItem != NULL ){
				if( TreeView_GetChild(hwndTree, htiItem) != NULL
				  && IDCANCEL == ::MYMESSAGEBOX(GetHwnd(), MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
					LS(STR_PROPCOMMAINMENU_DEL)) ){
					return TRUE;
				}
				HTREEITEM htiTemp = TreeView_GetNextSibling(hwndTree, htiItem);
				if( htiTemp == NULL ){
					// 末尾ならば、前を取る
					htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
				}
				TreeView_DeleteItem(hwndTree, htiItem);
				if( htiTemp != NULL ){
					TreeView_SelectItem(hwndTree, htiTemp);
				}
			}
			ChangeEnableAddInsert();
		}
		return TRUE;
	case IDC_BUTTON_INSERT:
	case IDC_BUTTON_INSERT_A:
	case IDC_BUTTON_ADD:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiInsert = NULL;
			HTREEITEM htiParent = NULL;
			// 挿入位置検索
			HTREEITEM htiTemp = TreeView_GetSelection(hwndTree);
			TV_ITEM tvi;
			if( htiTemp == NULL ){
			}else if( wID == IDC_BUTTON_ADD ){
				// 追加
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiTemp;
				if( TreeView_GetItem(hwndTree, &tvi) ){
					if( m_fileTreeSetting.m_aItems[tvi.lParam].m_eFileTreeItemType ==  EFileTreeItemType_Folder ){
						// ノード
						htiParent = htiTemp;
					}else{
						// 子を付けられないので親に付ける（選択アイテムの下に付く）
						htiParent = TreeView_GetParent(hwndTree, htiTemp);
					}
				}
			}else if( wID == IDC_BUTTON_INSERT_A ){
				// ノード挿入、挿入(下)
				// 追加先を探る
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiTemp;
				if( TreeView_GetItem(hwndTree, &tvi) ){
					if( m_fileTreeSetting.m_aItems[tvi.lParam].m_eFileTreeItemType ==  EFileTreeItemType_Folder ){
						// ノード
						htiParent = htiTemp;
						htiInsert = TVI_FIRST;
					}else{
						// 子を付けられないので親に付ける（選択アイテムの下に付く）
						htiParent = TreeView_GetParent(hwndTree, htiTemp);
						htiInsert = htiTemp;
					}
				}
			}else{
				assert(wID == IDC_BUTTON_INSERT);
				// 挿入(上)
				// 挿入先を探る
				htiParent = TreeView_GetParent(hwndTree, htiTemp);
				if( htiParent == NULL ){
					htiInsert = TVI_FIRST;
				}else{
					htiInsert = TreeView_GetPrevSibling(hwndTree, htiTemp);
					if( htiInsert == NULL ){
						htiInsert = TVI_FIRST;
					}
				}
			}
			if( htiParent == NULL ){
				htiParent = TVI_ROOT;
			}
			if( htiInsert == NULL ){
				htiInsert = TVI_LAST;
			}
			SFileTreeItem item;
			GetDataItem(item);
			HTREEITEM htiItem = InsertTreeItem(item, htiParent, htiInsert);
			// 展開
			if( htiParent != TVI_ROOT ){
				TreeView_Expand(hwndTree, htiParent, TVE_EXPAND);
			}
			TreeView_SelectItem(hwndTree, htiItem);
			ChangeEnableAddInsert();
		}
		return TRUE;
	case IDC_BUTTON_UPDATE:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiSelect = TreeView_GetSelection(hwndTree);
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiSelect;
			if( TreeView_GetItem(hwndTree, &tvi) ){
				SFileTreeItem item;
				GetDataItem(item);
				m_fileTreeSetting.m_aItems[tvi.lParam] = item;
				
				tvi.mask = TVIF_HANDLE | TVIF_TEXT;
				tvi.hItem = htiSelect;
				tvi.pszText = GetFileTreeLabel(item);
				TreeView_SetItem(hwndTree, &tvi);
			}
		}
		return TRUE;
	case IDC_BUTTON_FILEADD:
		{
			CDlgOpenFile dlg;
			SLoadInfo sLoadInfo;
			std::vector<std::tstring> aFileNames;
			dlg.Create( G_AppInstance(), GetHwnd(), _T("*.*"), _T("."),
				std::vector<LPCTSTR>(), std::vector<LPCTSTR>() );
			if( dlg.DoModalOpenDlg(&sLoadInfo, &aFileNames, false) ){
				if( 0 < aFileNames.size() ){
					HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
					HTREEITEM htiInsert = NULL;
					HTREEITEM htiParent = NULL;
					// 挿入位置検索
					HTREEITEM htiTemp = TreeView_GetSelection(hwndTree);
					TV_ITEM tvi;
					if( htiTemp == NULL ){
					}else{
						// ノード挿入、挿入(下)
						// 追加先を探る
						tvi.mask = TVIF_HANDLE | TVIF_PARAM;
						tvi.hItem = htiTemp;
						if( TreeView_GetItem(hwndTree, &tvi) ){
							if( m_fileTreeSetting.m_aItems[tvi.lParam].m_eFileTreeItemType ==  EFileTreeItemType_Folder ){
								// ノード
								htiParent = htiTemp;
								htiInsert = TVI_FIRST;
							}else{
								// 子を付けられないので親に付ける（選択アイテムの下に付く）
								htiParent = TreeView_GetParent(hwndTree, htiTemp);
							}
						}
					}
					if( htiParent == NULL ){
						htiParent = TVI_ROOT;
					}
					if( htiInsert == NULL ){
						htiInsert = TVI_LAST;
					}
					HTREEITEM htiItemFirst = NULL;
					for( int i = 0; i < (int)aFileNames.size(); i++ ){
						CNativeT cmemFile = aFileNames[i].c_str();
						cmemFile.ReplaceT(_T("%"), _T("%%"));
						SFileTreeItem item;
						item.m_eFileTreeItemType = EFileTreeItemType_File;
						item.m_szTargetPath = cmemFile.GetStringPtr();
						item.m_szLabelName = GetFileTitlePointer(aFileNames[i].c_str());
						htiInsert = InsertTreeItem(item, htiParent, htiInsert);
						if( htiItemFirst == NULL ){
							htiItemFirst = htiInsert;
						}
					}
					// 展開
					if( htiParent != TVI_ROOT ){
						TreeView_Expand(hwndTree, htiParent, TVE_EXPAND);
					}
					TreeView_SelectItem(hwndTree, htiItemFirst);
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_REPLACE:
		{
			CDlgInput1 dlgInput;
			std::tstring strMsg = LS(STR_FILETREE_REPLACE_PATH_FROM);
			std::tstring strTitle = LS(STR_DLGREPLC_STR);
			TCHAR szPathFrom[_MAX_PATH];
			szPathFrom[0] = _T('\0');
			if( dlgInput.DoModal(G_AppInstance(), GetHwnd(), strTitle.c_str(), strMsg.c_str(), _countof(szPathFrom), szPathFrom) ){
				TCHAR szPathTo[_MAX_PATH];
				szPathTo[0] = _T('\0');
				strMsg = LS(STR_FILETREE_REPLACE_PATH_TO);
				if( dlgInput.DoModal( G_AppInstance(), GetHwnd(), strTitle.c_str(), strMsg.c_str(), _countof(szPathTo), szPathTo) ){
					int nItemsCount = (int)m_fileTreeSetting.m_aItems.size();
					for( int i = 0; i < nItemsCount; i++ ){
						SFileTreeItem& item =  m_fileTreeSetting.m_aItems[i];
						CNativeT str(item.m_szTargetPath);
						str.Replace(szPathFrom, szPathTo);
						if( str.GetStringLength() < (int)item.m_szTargetPath.GetBufferCount() ){
							item.m_szTargetPath = str.GetStringPtr();
						}
					}
				}
				HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
				HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
				if( htiItem != NULL ){
					TV_ITEM tvi;
					tvi.mask = TVIF_HANDLE | TVIF_PARAM;
					tvi.hItem = htiItem;
					if( TreeView_GetItem( hwndTree, &tvi) ){
						SetDataItem(tvi.lParam);
					}
				}
			}
		}
		return TRUE;
	case IDC_BUTTON_UP:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if(htiItem == NULL ){
				break;
			}
			HTREEITEM htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
			if( htiTemp == NULL ){
				// そのエリアで最初
				break;
			}
			// コピー
			m_bInMove = true;
			FileTreeCopy(hwndTree, htiItem, htiTemp, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiTemp);
			m_bInMove = false;
		}
		return TRUE;
	case IDC_BUTTON_DOWN:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if( htiItem == NULL ){
				break;
			}
			HTREEITEM htiTemp = TreeView_GetNextSibling(hwndTree, htiItem);
			if( htiTemp == NULL ){
				// そのエリアで最後
				break;
			}
			// コピー
			m_bInMove = true;
			FileTreeCopy(hwndTree, htiTemp, htiItem, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiItem);
			m_bInMove = false;
			// 選択
			htiItem = TreeView_GetNextSibling(hwndTree, htiTemp);
			if( htiItem != NULL ){
				TreeView_SelectItem(hwndTree, htiItem);
			}
		}
		return TRUE;
	case IDC_BUTTON_RIGHT:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if( htiItem == NULL ){
				break;
			}
			HTREEITEM htiTemp = TreeView_GetPrevSibling(hwndTree, htiItem);
			if( htiTemp == NULL ){
				// そのエリアで最初
				break;
			}
			// ノード確認
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiTemp;
			if( !TreeView_GetItem(hwndTree, &tvi) ){
				// エラー
				break;
			}
			if( m_fileTreeSetting.m_aItems[tvi.lParam].m_eFileTreeItemType == EFileTreeItemType_Folder ){
				// 直前がノード
				// コピー
				m_bInMove = true;
				HTREEITEM htiTemp2 = FileTreeCopy(hwndTree, htiTemp, htiItem, true, true);
				// 削除
				TreeView_DeleteItem(hwndTree, htiItem);
				m_bInMove = false;
				// 選択
				TreeView_SelectItem(hwndTree, htiTemp2);
			}
		}
		return TRUE;
	case IDC_BUTTON_LEFT:
		{
			HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
			HTREEITEM htiItem = TreeView_GetSelection(hwndTree);
			if( htiItem == NULL ){
				break;
			}
			HTREEITEM htiParent = TreeView_GetParent(hwndTree, htiItem);
			if(htiParent == NULL ){
				// Root
				break;
			}
			// コピー
			m_bInMove = true;
			HTREEITEM htiTemp2 = FileTreeCopy(hwndTree, htiParent, htiItem, false, true);
			// 削除
			TreeView_DeleteItem(hwndTree, htiItem);
			m_bInMove = false;
			// 選択
			TreeView_SelectItem(hwndTree, htiTemp2);
		}
		return TRUE;
	case IDC_BUTTON_IMPORT:
		{
			CImpExpFileTree cImpExp(m_fileTreeSetting.m_aItems);
			cImpExp.ImportUI(G_AppInstance(), GetHwnd());
			SetData();
		}
		return TRUE;
	case IDC_BUTTON_EXPORT:
		{
			std::vector<SFileTreeItem> items;
			GetDataTree(items, TreeView_GetRoot(GetItemHwnd(IDC_TREE_FL)), 0, 0);
			CImpExpFileTree cImpExp(items);
			cImpExp.ExportUI(G_AppInstance(), GetHwnd());
		}
		return TRUE;

	case IDC_BUTTON_HELP:
		/* ヘルプ */
		MyWinHelp( GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_FILETREE) );
		return TRUE;

	case IDOK:
		::EndDialog( GetHwnd(), GetData() );
		return TRUE;

	case IDCANCEL:
		::EndDialog( GetHwnd(), FALSE );
		return TRUE;

	}

	/* 基底クラスメンバ */
	return CDialog::OnBnClicked( wID );
}

BOOL CDlgFileTree::OnNotify( WPARAM wParam, LPARAM lParam )
{
	NMHDR* pNMHDR = (NMHDR*)lParam;
	TV_DISPINFO* ptdi = (TV_DISPINFO*)lParam;
	HWND hwndTree = GetItemHwnd(IDC_TREE_FL);
	HTREEITEM htiItem;

	switch( pNMHDR->code ){
	case TVN_DELETEITEM:
		if( !m_bInMove
		  && pNMHDR->hwndFrom == hwndTree
		  && (htiItem = TreeView_GetSelection( hwndTree )) != NULL ){
			//付属情報を削除
			TV_ITEM tvi;
			tvi.mask = TVIF_HANDLE | TVIF_PARAM;
			tvi.hItem = htiItem;
			if( TreeView_GetItem( hwndTree, &tvi) ){
				// リストから削除する代わりに番号を覚えて後で再利用
				m_aItemRemoveList.push_back(tvi.lParam);
			}
		}
		break;
	case TVN_SELCHANGED:
		if( !m_bInMove
			&& pNMHDR->hwndFrom == hwndTree ){
			htiItem = TreeView_GetSelection( hwndTree );
			if( htiItem != NULL ){
				TV_ITEM tvi;
				tvi.mask = TVIF_HANDLE | TVIF_PARAM;
				tvi.hItem = htiItem;
				if( TreeView_GetItem( hwndTree, &tvi) ){
					SetDataItem(tvi.lParam);
				}
			}else{
				SetDataItem(-1);
			}
		}
	}
	/* 基底クラスメンバ */
	return CDialog::OnNotify( wParam, lParam );
}

LPVOID CDlgFileTree::GetHelpIdTable( void )
{
	return (LPVOID)p_helpids;
}


