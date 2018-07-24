/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CSHAREDATA_IO_AA81C249_631D_40B0_AAFF_2F163748954B9_H_
#define SAKURA_CSHAREDATA_IO_AA81C249_631D_40B0_AAFF_2F163748954B9_H_

class CDataProfile;
class CMenuDrawer;
struct CommonSetting_CustomMenu; // defined CommonSetting.h
struct CommonSetting_MainMenu; // defined CommonSetting.h
struct CommonSetting_KeyBind;	// defined CommonSetting.h
struct ColorInfo; // defined doc/CDocTypeSetting.h
struct SFileTree;

// 2008.XX.XX kobake CShareDataから分離
// 2008.05.24 Uchi   ShareData_IO_CustMenu, ShareData_IO_KeyBind  move Export、Importに使用
// 2010.08.21 Moca アクセス権、関数名の整理
class CShareData_IO{
public:
	//セーブ・ロード
	static bool LoadShareData();	/* 共有データのロード */
	static void SaveShareData();	/* 共有データの保存 */

protected:
	static bool ShareData_IO_2( bool );	/* 共有データの保存 */

	// Feb. 12, 2006 D.S.Koba
	static void ShareData_IO_Mru( CDataProfile& );
	static void ShareData_IO_Keys( CDataProfile& );
	static void ShareData_IO_Grep( CDataProfile& );
	static void ShareData_IO_Folders( CDataProfile& );
	static void ShareData_IO_Cmd( CDataProfile& );
	static void ShareData_IO_Nickname( CDataProfile& );
	static void ShareData_IO_Common( CDataProfile& );
	static void ShareData_IO_Toolbar( CDataProfile&, CMenuDrawer* );
	static void ShareData_IO_CustMenu( CDataProfile& );
	static void ShareData_IO_Font( CDataProfile& );
	static void ShareData_IO_KeyBind( CDataProfile& );
	static void ShareData_IO_Print( CDataProfile& );
	static void ShareData_IO_Types( CDataProfile& );
	static void ShareData_IO_KeyWords( CDataProfile& );
	static void ShareData_IO_Macro( CDataProfile& );
	static void ShareData_IO_Statusbar( CDataProfile& );	// 2008/6/21 Uchi
	static void ShareData_IO_Plugin( CDataProfile&, CMenuDrawer* );		// 2009/11/30 syat
	static void ShareData_IO_MainMenu( CDataProfile& );		// 2010/5/15 Uchi
	static void ShareData_IO_Other( CDataProfile& );

public:
	static void ShareData_IO_FileTree( CDataProfile&, SFileTree&, const WCHAR* );
	static void ShareData_IO_FileTreeItem( CDataProfile&, SFileTreeItem&, const WCHAR*, int i );
	static void ShareData_IO_Type_One( CDataProfile&, STypeConfig& , const WCHAR* );	// 2010/04/12 Uchi 分離

public:
	static void IO_CustMenu( CDataProfile&, CommonSetting_CustomMenu&, bool );
	static void IO_KeyBind( CDataProfile&, CommonSetting_KeyBind&, bool);		// 2012.11.22 aroka
	static void IO_MainMenu( CDataProfile& c, CommonSetting_MainMenu& s, bool b ){		// 2010/5/15 Uchi
		IO_MainMenu(c, NULL, s, b);
	}
	static void IO_MainMenu( CDataProfile& cProfile, std::vector<std::wstring>* pData,
		CommonSetting_MainMenu& mainmenu, bool bOutCmdName);
	static void IO_ColorSet( CDataProfile* , const WCHAR* , ColorInfo* );	/* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba
};

#endif /* SAKURA_CSHAREDATA_IO_AA81C249_631D_40B0_AAFF_2F163748954B9_H_ */
/*[EOF]*/
