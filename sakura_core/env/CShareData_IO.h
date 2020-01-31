/*! @file */
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
#pragma once

class CDataProfile;
class CMenuDrawer;
struct CommonSetting_CustomMenu; // defined CommonSetting.h
struct CommonSetting_MainMenu; // defined CommonSetting.h
struct CommonSetting_KeyBind; // defined CommonSetting.h
struct ColorInfo; // defined doc/CDocTypeSetting.h
struct SFileTree;

// 2008.XX.XX kobake CShareDataから分離
// 2008.05.24 Uchi   ShareData_IO_CustMenu, ShareData_IO_KeyBind  move Export、Importに使用
// 2010.08.21 Moca アクセス権、関数名の整理
class CShareData_IO
{
  public:
    //セーブ・ロード
    static bool LoadShareData(); /* 共有データのロード */
    static void SaveShareData(); /* 共有データの保存 */

  protected:
    static bool ShareData_IO_2(bool bRead); /* 共有データの保存 */

    // Feb. 12, 2006 D.S.Koba
    static void ShareData_IO_Mru(CDataProfile &cProfile);
    static void ShareData_IO_Keys(CDataProfile &cProfile);
    static void ShareData_IO_Grep(CDataProfile &cProfile);
    static void ShareData_IO_Folders(CDataProfile &cProfile);
    static void ShareData_IO_Cmd(CDataProfile &cProfile);
    static void ShareData_IO_Nickname(CDataProfile &cProfile);
    static void ShareData_IO_Common(CDataProfile &cProfile);
    static void ShareData_IO_Toolbar(CDataProfile &cProfile, CMenuDrawer *pcMenuDrawer);
    static void ShareData_IO_CustMenu(CDataProfile &cProfile);
    static void ShareData_IO_Font(CDataProfile &cProfile);
    static void ShareData_IO_KeyBind(CDataProfile &cProfile);
    static void ShareData_IO_Print(CDataProfile &cProfile);
    static void ShareData_IO_Types(CDataProfile &cProfile);
    static void ShareData_IO_KeyWords(CDataProfile &cProfile);
    static void ShareData_IO_Macro(CDataProfile &cProfile);
    static void ShareData_IO_Statusbar(CDataProfile &cProfile); // 2008/6/21 Uchi
    static void ShareData_IO_Plugin(CDataProfile &cProfile, CMenuDrawer *pcMenuDrawer); // 2009/11/30 syat
    static void ShareData_IO_MainMenu(CDataProfile &cProfile); // 2010/5/15 Uchi
    static void ShareData_IO_Other(CDataProfile &cProfile);

  public:
    static void ShareData_IO_FileTree(CDataProfile &cProfile, SFileTree &fileTree, const WCHAR *pszSecName);
    static void ShareData_IO_FileTreeItem(CDataProfile &cProfile, SFileTreeItem &item, const WCHAR *pszSecName, int i);
    static void ShareData_IO_Type_One(CDataProfile &cProfile, STypeConfig &types, const WCHAR *pszSecName); // 2010/04/12 Uchi 分離

  public:
    static void IO_CustMenu(CDataProfile &cProfile, CommonSetting_CustomMenu &menu, bool bOutCmdName);
    static void IO_KeyBind(CDataProfile &cProfile, CommonSetting_KeyBind &sKeyBind, bool bOutCmdName); // 2012.11.22 aroka
    static void IO_MainMenu(CDataProfile &c, CommonSetting_MainMenu &s, bool b)
    { // 2010/5/15 Uchi
        IO_MainMenu(c, NULL, s, b);
    }
    static void IO_MainMenu(CDataProfile &cProfile, std::vector<std::wstring> *pData, CommonSetting_MainMenu &mainmenu, bool bOutCmdName);
    static void IO_ColorSet(CDataProfile *pcProfile, const WCHAR *pszSecName, ColorInfo *pColorInfoArr); /* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba
};
