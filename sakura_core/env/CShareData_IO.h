/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSHAREDATA_IO_B154E0E0_C606_468E_A3B7_767DDA1DE6EE_H_
#define SAKURA_CSHAREDATA_IO_B154E0E0_C606_468E_A3B7_767DDA1DE6EE_H_
#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>

class CBlockComment;
class CDataProfile;
class CMenuDrawer;
struct CommonSetting_CustomMenu; // defined CommonSetting.h
struct CommonSetting_MainMenu; // defined CommonSetting.h
struct CommonSetting_KeyBind;	// defined CommonSetting.h
struct ColorInfo; // defined doc/CDocTypeSetting.h
struct SFileTree;
struct SFileTreeItem;
struct STypeConfig;

using BlockComments = std::span<CBlockComment>;

// 2008.XX.XX kobake CShareDataから分離
// 2008.05.24 Uchi   ShareData_IO_CustMenu, ShareData_IO_KeyBind  move Export、Importに使用
// 2010.08.21 Moca アクセス権、関数名の整理
class CShareData_IO{
public:
	//セーブ・ロード
	static bool LoadShareData();	/* 共有データのロード */
	static void SaveShareData();	/* 共有データの保存 */

protected:
	static bool ShareData_IO_2( bool bRead );	/* 共有データの保存 */

	// Feb. 12, 2006 D.S.Koba
	static void ShareData_IO_Mru( CDataProfile& cProfile );
	static void ShareData_IO_Keys( CDataProfile& cProfile );
	static void ShareData_IO_Grep( CDataProfile& cProfile );
	static void ShareData_IO_Folders( CDataProfile& cProfile );
	static void ShareData_IO_Cmd( CDataProfile& cProfile );
	static void ShareData_IO_Nickname( CDataProfile& cProfile );
	static void ShareData_IO_Common( CDataProfile& cProfile );
	static void ShareData_IO_Toolbar( CDataProfile& cProfile, CMenuDrawer* pcMenuDrawer );
	static void ShareData_IO_CustMenu( CDataProfile& cProfile );
	static void ShareData_IO_Font( CDataProfile& cProfile );
	static void ShareData_IO_KeyBind( CDataProfile& cProfile );
	static void ShareData_IO_Print( CDataProfile& cProfile );
	static void ShareData_IO_Types( CDataProfile& cProfile );
	static void ShareData_IO_KeyWords( CDataProfile& cProfile );
	static void ShareData_IO_Macro( CDataProfile& cProfile );
	static void ShareData_IO_Statusbar( CDataProfile& cProfile );	// 2008/6/21 Uchi
	static void ShareData_IO_Plugin( CDataProfile& cProfile, CMenuDrawer* pcMenuDrawer );	// 2009/11/30 syat
	static void ShareData_IO_MainMenu( CDataProfile& cProfile );		// 2010/5/15 Uchi
	static void ShareData_IO_Other( CDataProfile& cProfile );

public:
	static void ShareData_IO_Type_One( CDataProfile& cProfile, STypeConfig& types, const WCHAR* pszSecName);	// 2010/04/12 Uchi 分離

public:
	static void IO_CustMenu( CDataProfile& cProfile, CommonSetting_CustomMenu& menu, bool bOutCmdName);
	static void IO_KeyBind( CDataProfile& cProfile, CommonSetting_KeyBind& sKeyBind, bool bOutCmdName);		// 2012.11.22 aroka
	static void IO_MainMenu( CDataProfile& c, CommonSetting_MainMenu& s, bool b ){		// 2010/5/15 Uchi
		IO_MainMenu(c, nullptr, s, b);
	}
	static void IO_MainMenu( CDataProfile& cProfile, std::vector<std::wstring>* pData,
		CommonSetting_MainMenu& mainmenu, bool bOutCmdName);
	static void IO_ColorSet( CDataProfile* pcProfile, const WCHAR* pszSecName, ColorInfo* pColorInfoArr );	/* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba
};

void ShareData_IO_BlockComments(
	CDataProfile&			cProfile,
	std::wstring_view		sectionName,	//!< [in] セクション名
	BlockComments			tEntryValues	//!< [in,out] エントリ値
);

void ShareData_IO_FileTree(
	CDataProfile&			cProfile,
	std::wstring_view		sectionName,	//!< [in] セクション名
	SFileTree&				fileTree
);

bool ShareData_IO_LogFont(
	CDataProfile&						cProfile,
	std::wstring_view					pszSecName,
	std::wstring_view					pszKeyLf,
	LOGFONT&							lf,
	INT&								nPointSize,
	const std::optional<std::wstring>&	optPointSizeKey = std::nullopt,
	const std::optional<std::wstring>&	optFaceNameKey = std::nullopt
);

template <typename T>
void ShareData_IO_OutlineDockRect(
	CDataProfile&			cProfile,
	std::wstring_view		sectionName,	//!< [in] セクション名
	std::wstring_view		entryKey,		//!< [in] エントリ名
	T&						tEntryValue		//!< [in,out] エントリ値
);

#endif /* SAKURA_CSHAREDATA_IO_B154E0E0_C606_468E_A3B7_767DDA1DE6EE_H_ */
