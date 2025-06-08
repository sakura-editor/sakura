/*!	@file
	@brief MRUリストと呼ばれるリストを管理する

	@author YAZAKI
	@date 2001/12/23  新規作成
*/
/*
	Copyright (C) 1998-2001, YAZAKI
	Copyright (C) 2000, jepro
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2003, MIK
	Copyright (C) 2004, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_CMRUFILE_41099ADB_562E_457B_873D_8F81AC958AC2_H_
#define SAKURA_CMRUFILE_41099ADB_562E_457B_873D_8F81AC958AC2_H_
#pragma once

#include <Windows.h> /// BOOL,HMENU // 2002/2/10 aroka
#include <vector>
#include "recent/CRecentFile.h"

struct EditInfo; // 2004.04.11 genta パラメータ内のstructを削除するため．doxygen対策
class CMenuDrawer;

//	@date 2002.2.17 YAZAKI CShareDataのインスタンスは、CProcessにひとつあるのみ。
class CMRUFile {
	using Me = CMRUFile;

public:
	//	コンストラクタ
	CMRUFile();
	CMRUFile(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CMRUFile(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CMRUFile();

	//	メニューを取得する
	HMENU CreateMenu( CMenuDrawer* pCMenuDrawer ) const;	//	うーん。pCMenuDrawerが必要なくなるといいなぁ。
	HMENU CreateMenu( HMENU hMenu, CMenuDrawer* pCMenuDrawer ) const;	//	2010/5/21 Uchi
	BOOL DestroyMenu( HMENU hMenu ) const;
	
	//	ファイル名の一覧を教えて
	std::vector<LPCWSTR> GetPathList() const;

	//	アクセス関数
	int Length(void) const;	//	アイテムの数。
	int MenuLength(void) const { return t_min(Length(), m_cRecentFile.GetViewCount()); }	//	メニューに表示されるアイテムの数
	void ClearAll(void);//	アイテムを削除～。
	bool GetEditInfo( int num, EditInfo* pfi ) const;				//	番号で指定したEditInfo（情報をまるごと）
	bool GetEditInfo( const WCHAR* pszPath, EditInfo* pfi ) const;	//	ファイル名で指定したEditInfo（情報をまるごと）
	void Add( EditInfo* pEditInfo );		//	*pEditInfoを追加する。

protected:
	//	共有メモリアクセス用。
	struct DLLSHAREDATA*	m_pShareData;		//	共有メモリを参照するよ。

private:
	CRecentFile	m_cRecentFile;	//履歴	//@@@ 2003.04.08 MIK
};
#endif /* SAKURA_CMRUFILE_41099ADB_562E_457B_873D_8F81AC958AC2_H_ */
