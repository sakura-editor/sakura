#pragma once

#include "doc/CDocTypeSetting.h" // ColorInfo typedefされている

class CDataProfile;
class CMenuDrawer;
struct CommonSetting_CustomMenu; // defined CommonSetting.h
struct CommonSetting_MainMenu; // defined CommonSetting.h
struct KEYDATA; // defined CKeyBind.h
// struct ColorInfo; // defined doc/CDocTypeSetting.h

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
	static void ShareData_IO_Type_One( CDataProfile&, int, const WCHAR* );	// 2010/04/12 Uchi 分離

public:
	static void IO_CustMenu( CDataProfile&, CommonSetting_CustomMenu&, bool );
	static void IO_KeyBind( CDataProfile&, int, KEYDATA[], bool);
	static void IO_MainMenu( CDataProfile&, CommonSetting_MainMenu&, bool );		// 2010/5/15 Uchi
	static void IO_ColorSet( CDataProfile* , const WCHAR* , ColorInfo* );	/* 色設定 I/O */ // Feb. 12, 2006 D.S.Koba
};

