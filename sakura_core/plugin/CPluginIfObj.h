/*!	@file
	@brief Pluginオブジェクト

*/
/*
	Copyright (C) 2009, syat

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
#ifndef SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_
#define SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_

#include "macro/CWSHIfObj.h"
#include "_os/OleTypes.h"
#include "util/ole_convert.h"

// cppへ移動予定
#include "window/CEditWnd.h"
#include "view/CEditView.h"

class CPluginIfObj : public CWSHIfObj {
	// 型定義
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_PL_SETOPTION,							//オプションファイルに値を書く
		F_PL_ADDCOMMAND,						//コマンドを追加する
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_PL_GETPLUGINDIR,						//プラグインフォルダパスを取得する
		F_PL_GETDEF,							//設定ファイルから値を読む
		F_PL_GETOPTION,							//オプションファイルから値を読む
		F_PL_GETCOMMANDNO,						//実行中プラグの番号を取得する
		F_PL_GETSTRING,							//設定ファイルから文字列を読みだす(多言語対応)
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	CPluginIfObj( CPlugin& cPlugin )
		: CWSHIfObj( L"Plugin", false )
		, m_cPlugin( cPlugin )
	{
	}

	// デストラクタ
public:
	~CPluginIfObj(){}

	// 操作
public:
	void SetPlugIndex(int nIndex) { m_nPlugIndex = nIndex; }
	// 実装
public:
	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const
	{
		return m_MacroFuncInfoCommandArr;
	}
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const
	{
		return m_MacroFuncInfoArr;
	}
	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch(LOWORD(ID))
		{
		case F_PL_GETPLUGINDIR:			//プラグインフォルダパスを取得する
			{
				SysString S(m_cPlugin.m_sBaseDir.c_str(), m_cPlugin.m_sBaseDir.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETDEF:				//設定ファイルから値を読む
		case F_PL_GETOPTION:			//オプションファイルから値を読む
			{
				CDataProfile cProfile;
				wstring sSection;
				wstring sKey;
				wstring sValue;
				if( variant_to_wstr( Arguments[0], sSection ) != true) return false;
				if( variant_to_wstr( Arguments[1], sKey ) != true) return false;

				cProfile.SetReadingMode();
				if( LOWORD(ID) == F_PL_GETDEF ){
					cProfile.ReadProfile( m_cPlugin.GetPluginDefPath().c_str() );
				}else{
					cProfile.ReadProfile( m_cPlugin.GetOptionPath().c_str() );
				}
				if (!cProfile.IOProfileData( sSection.c_str(), sKey.c_str(), sValue )
					&& LOWORD(ID) == F_PL_GETOPTION ) {
					// 設定されていなければデフォルトを取得 
					CPluginOption::ArrayIter it;
					for (it = m_cPlugin.m_options.begin(); it != m_cPlugin.m_options.end(); it++) {
						wstring sSectionTmp;
						wstring sKeyTmp;
						(*it)->GetKey(&sSectionTmp, &sKeyTmp);
						if (sSection == sSectionTmp && sKey == sKeyTmp) {
							sValue = (*it)->GetDefaultVal();
							break;
						}
					}
				}

				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		case F_PL_GETCOMMANDNO:			//実行中プラグの番号を取得する
			{
				Wrap(&Result)->Receive(m_nPlugIndex);
			}
			return true;
		case F_PL_GETSTRING:
			{
				int num;
				if(variant_to_int( Arguments[0], num ) == false) return false;
				if( 0 < num && num < MAX_PLUG_STRING ){
					std::wstring& str = m_cPlugin.m_aStrings[num];
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
					return true;
				}else if( 0 == num ){
					std::wstring str = to_wchar(m_cPlugin.m_sLangName.c_str());
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
					return true;
				}
			}
		}
		return false;
	}
	//コマンドを処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize)
	{
		switch ( LOWORD(ID) ) 
		{
		case F_PL_SETOPTION:			//オプションファイルに値を書く
			{
				if( Arguments[0] == NULL )return false;
				if( Arguments[1] == NULL )return false;
				if( Arguments[2] == NULL )return false;
				CDataProfile cProfile;

				cProfile.ReadProfile( m_cPlugin.GetOptionPath().c_str() );
				cProfile.SetWritingMode();
				wstring tmp(Arguments[2]);
				cProfile.IOProfileData( Arguments[0], Arguments[1], tmp );
				cProfile.WriteProfile( m_cPlugin.GetOptionPath().c_str(), (m_cPlugin.m_sName + L" プラグイン設定ファイル").c_str() );
			}
			break;
		case F_PL_ADDCOMMAND:			//コマンドを追加する
			{
				int id = m_cPlugin.AddCommand( Arguments[0], Arguments[1], Arguments[2], true );
				View->m_pcEditWnd->RegisterPluginCommand( id );
			}
			break;
		}
		return true;
	}

	// メンバ変数
public:
private:
	CPlugin& m_cPlugin;
	static MacroFuncInfo m_MacroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo m_MacroFuncInfoArr[];	// 関数情報(戻り値あり)
	int m_nPlugIndex;	//実行中プラグの番号
};

//コマンド情報
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoCommandArr[] = 
{
	//ID									関数名							引数										戻り値の型	m_pszData
	{EFunctionCode(F_PL_SETOPTION),			LTEXT("SetOption"),				{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	NULL }, //オプションファイルに値を書く
	{EFunctionCode(F_PL_ADDCOMMAND),		LTEXT("AddCommand"),			{VT_BSTR, VT_BSTR, VT_BSTR, VT_EMPTY},		VT_EMPTY,	NULL }, //コマンドを追加する
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

//関数情報
MacroFuncInfo CPluginIfObj::m_MacroFuncInfoArr[] = 
{
	//ID									関数名							引数										戻り値の型	m_pszData
	{EFunctionCode(F_PL_GETPLUGINDIR),		LTEXT("GetPluginDir"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //プラグインフォルダパスを取得する
	{EFunctionCode(F_PL_GETDEF),			LTEXT("GetDef"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, //設定ファイルから値を読む
	{EFunctionCode(F_PL_GETOPTION),			LTEXT("GetOption"),				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	NULL }, //オプションファイルから値を読む
	{EFunctionCode(F_PL_GETCOMMANDNO),		LTEXT("GetCommandNo"),			{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //オプションファイルから値を読む
	{EFunctionCode(F_PL_GETSTRING),			LTEXT("GetString"),				{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //設定ファイルから文字列を読む
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

#endif /* SAKURA_CPLUGINIFOBJ_2205DDE8_330B_49AC_AC5E_4C02F07DDCD5D_H_ */
/*[EOF]*/
