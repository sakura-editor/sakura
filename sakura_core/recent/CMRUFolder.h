﻿/*!	@file
	@brief MRUリストと呼ばれるリストを管理する。フォルダー版。

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, YAZAKI
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2003, MIK
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

#ifndef SAKURA_CMRUFOLDER_32D69CDD_037F_4DE1_961E_B730F56F4189_H_
#define SAKURA_CMRUFOLDER_32D69CDD_037F_4DE1_961E_B730F56F4189_H_
#pragma once

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include "recent/CRecentFolder.h"

class CMenuDrawer;

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CMRUFolder {
	using Me = CMRUFolder;

public:
	//	コンストラクタ
	CMRUFolder();
	CMRUFolder(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CMRUFolder(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CMRUFolder();

	//	メニューを取得する
	HMENU CreateMenu( CMenuDrawer* pCMenuDrawer ) const;	//	うーん。pCMenuDrawerが必要なくなるといいなぁ。
	HMENU CreateMenu( HMENU hMenu, CMenuDrawer* pCMenuDrawer ) const;	//	2010/5/21 Uchi
	BOOL DestroyMenu( HMENU hMenu ) const;
	
	//	フォルダー名の一覧を教えて
	std::vector<LPCWSTR> GetPathList() const;

	//	アクセス関数
	int Length() const;	//	アイテムの数。
	int MenuLength(void) const{ return t_min(Length(), m_cRecentFolder.GetViewCount()); }	//	メニューに表示されるアイテムの数
	void ClearAll();					//	アイテムを削除～。
	void Add( const WCHAR* pszFolder );	//	pszFolderを追加する。
	const WCHAR* GetPath(int num) const;

protected:
	//	共有メモリアクセス用。
	struct DLLSHAREDATA*	m_pShareData;			//	共有メモリを参照するよ。

private:
	CRecentFolder	m_cRecentFolder;	//履歴	//@@@ 2003.04.08 MIK
};
#endif /* SAKURA_CMRUFOLDER_32D69CDD_037F_4DE1_961E_B730F56F4189_H_ */
