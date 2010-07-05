#pragma once

#include "doc/CDocTypeSetting.h" // ColorInfo typedef����Ă���

class CDataProfile;
struct CommonSetting_CustomMenu; // defined CommonSetting.h
struct CommonSetting_MainMenu; // defined CommonSetting.h
struct KEYDATA; // defined CKeyBind.h
// struct ColorInfo; // defined doc/CDocTypeSetting.h

// 2008.XX.XX kobake CShareData���番��
// 2008.05.24 Uchi   ShareData_IO_CustMenu, ShareData_IO_KeyBind  move Export�AImport�Ɏg�p
class CShareData_IO{
public:
	//�Z�[�u�E���[�h
	static bool LoadShareData();	/* ���L�f�[�^�̃��[�h */
	static void SaveShareData();	/* ���L�f�[�^�̕ۑ� */

protected:
	static bool ShareData_IO_2( bool );	/* ���L�f�[�^�̕ۑ� */

public:
	// Feb. 12, 2006 D.S.Koba
	static void ShareData_IO_Mru( CDataProfile& );
	static void ShareData_IO_Keys( CDataProfile& );
	static void ShareData_IO_Grep( CDataProfile& );
	static void ShareData_IO_Folders( CDataProfile& );
	static void ShareData_IO_Cmd( CDataProfile& );
	static void ShareData_IO_Nickname( CDataProfile& );
	static void ShareData_IO_Common( CDataProfile& );
	static void ShareData_IO_Toolbar( CDataProfile& );
	static void ShareData_IO_CustMenu( CDataProfile&, CommonSetting_CustomMenu&, bool );
	static void ShareData_IO_Font( CDataProfile& );
	static void ShareData_IO_KeyBind( CDataProfile&, int, KEYDATA[], bool);
	static void ShareData_IO_Print( CDataProfile& );
	static void ShareData_IO_Types( CDataProfile& );
	static void ShareData_IO_Type_One( CDataProfile&, int, const WCHAR* );	// 2010/04/12 Uchi ����
	static void ShareData_IO_KeyWords( CDataProfile& );
	static void ShareData_IO_Macro( CDataProfile& );
	static void ShareData_IO_Statusbar( CDataProfile& );	// 2008/6/21 Uchi
	static void ShareData_IO_Plugin( CDataProfile& );		// 2009/11/30 syat
	static void ShareData_IO_MainMenu( CDataProfile&, CommonSetting_MainMenu&, bool );		// 2010/5/15 Uchi
	static void ShareData_IO_Other( CDataProfile& );


public:
	static void IO_ColorSet( CDataProfile* , const WCHAR* , ColorInfo* );	/* �F�ݒ� I/O */ // Feb. 12, 2006 D.S.Koba
};

