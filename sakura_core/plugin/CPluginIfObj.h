/*!	@file
	@brief Pluginオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CPLUGINIFOBJ_F083BB50_2D36_44C9_8F2B_6F2D3BA7AAF9_H_
#define SAKURA_CPLUGINIFOBJ_F083BB50_2D36_44C9_8F2B_6F2D3BA7AAF9_H_
#pragma once

#include "macro/CWSHIfObj.h"
#include "_os/OleTypes.h"
#include "util/ole_convert.h"

// cppへ移動予定
#include "window/CEditWnd.h"
#include "view/CEditView.h"

#include <array>

class CPluginIfObj : public CWSHIfObj {
	// 型定義
	enum FuncId {
		F_PL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_PL_SETOPTION,							//オプションファイルに値を書く
		F_PL_ADDCOMMAND,						//コマンドを追加する
		F_PL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_PL_GETPLUGINDIR,						//プラグインフォルダーパスを取得する
		F_PL_GETDEF,							//設定ファイルから値を読む
		F_PL_GETOPTION,							//オプションファイルから値を読む
		F_PL_GETCOMMANDNO,						//実行中プラグの番号を取得する
		F_PL_GETSTRING,							//設定ファイルから文字列を読みだす(多言語対応)
	};

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
	MacroFuncInfoArray GetMacroCommandInfo() const override {
		static std::array<MacroFuncInfo, 3> macroFuncInfoArr = {
			//ID									関数名					引数										戻り値の型	
			MacroFuncInfo{F_PL_SETOPTION,			L"SetOption",			{VT_BSTR, VT_BSTR, VT_VARIANT, VT_EMPTY},	VT_EMPTY,	}, //オプションファイルに値を書く
			MacroFuncInfo{F_PL_ADDCOMMAND,			L"AddCommand",			{VT_BSTR, VT_BSTR, VT_BSTR, VT_EMPTY},		VT_EMPTY,	}, //コマンドを追加する
			//	終端
			MacroFuncInfo{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoArr.data();
	}
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const override{
		static std::array<MacroFuncInfo, 6> macroFuncInfoNotCommandArr = {
			//			  ID					関数名					引数										戻り値の型	
			MacroFuncInfo{F_PL_GETPLUGINDIR,	L"GetPluginDir",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	}, //プラグインフォルダーパスを取得する
			MacroFuncInfo{F_PL_GETDEF,			L"GetDef",				{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	}, //設定ファイルから値を読む
			MacroFuncInfo{F_PL_GETOPTION,		L"GetOption",			{VT_BSTR, VT_BSTR, VT_EMPTY, VT_EMPTY},		VT_BSTR,	}, //オプションファイルから値を読む
			MacroFuncInfo{F_PL_GETCOMMANDNO,	L"GetCommandNo",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		}, //オプションファイルから値を読む
			MacroFuncInfo{F_PL_GETSTRING,		L"GetString",			{VT_I4,    VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	}, //設定ファイルから文字列を読む
			//	終端
			MacroFuncInfo{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoNotCommandArr.data();
	}

	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result) override
	{
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch(LOWORD(ID))
		{
		case F_PL_GETPLUGINDIR:			//プラグインフォルダーパスを取得する
			{
				SysString S(m_cPlugin.m_sBaseDir.c_str(), m_cPlugin.m_sBaseDir.size());
				Wrap(&Result)->Receive(S);
			}
			break;

		case F_PL_GETDEF:				//設定ファイルから値を読む
		case F_PL_GETOPTION:			//オプションファイルから値を読む
			{
				CDataProfile cProfile;
				std::wstring sSection;
				std::wstring sKey;
				std::wstring sValue;
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
					for (it = m_cPlugin.m_options.cbegin(); it != m_cPlugin.m_options.cend(); it++) {
						std::wstring sSectionTmp;
						std::wstring sKeyTmp;
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
			break;

		case F_PL_GETCOMMANDNO:			//実行中プラグの番号を取得する
			{
				Wrap(&Result)->Receive(m_nPlugIndex);
			}
			break;

		case F_PL_GETSTRING:
			{
				int num;
				if(variant_to_int( Arguments[0], num ) == false) return false;
				if( 0 < num && num < MAX_PLUG_STRING ){
					std::wstring& str = m_cPlugin.m_aStrings[num];
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
				}else if( 0 == num ){
					std::wstring str = m_cPlugin.m_sLangName;
					SysString S(str.c_str(), str.size());
					Wrap(&Result)->Receive(S);
				}
			}
			break;

		default:
			return CWSHIfObj::HandleFunction(View, ID, Arguments, ArgSize, Result);
		}
		return true;
	}
	//コマンドを処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, LPCWSTR* Arguments, const int* ArgLengths, const int ArgSize) override
	{
		switch ( LOWORD(ID) ) 
		{
		case F_PL_SETOPTION:			//オプションファイルに値を書く
			{
				if( Arguments[0] == nullptr )return false;
				if( Arguments[1] == nullptr )return false;
				if( Arguments[2] == nullptr )return false;
				CDataProfile cProfile;

				cProfile.ReadProfile( m_cPlugin.GetOptionPath().c_str() );
				cProfile.SetWritingMode();
				std::wstring tmp(Arguments[2]);
				cProfile.IOProfileData( Arguments[0], Arguments[1], tmp );
				cProfile.WriteProfile( m_cPlugin.GetOptionPath().c_str(), (m_cPlugin.m_sName + L" プラグイン設定ファイル").c_str() );
			}
			break;
		case F_PL_ADDCOMMAND:			//コマンドを追加する
			{
				int id = m_cPlugin.AddCommand( Arguments[0], Arguments[1], Arguments[2], true );
				GetEditWnd().RegisterPluginCommand( id );
			}
			break;

		default:
			return CWSHIfObj::HandleCommand(View, ID, Arguments, ArgLengths, ArgSize);
		}
		return true;
	}

	// メンバ変数
public:
private:
	CPlugin& m_cPlugin;
	int m_nPlugIndex;	//実行中プラグの番号
};

#endif /* SAKURA_CPLUGINIFOBJ_F083BB50_2D36_44C9_8F2B_6F2D3BA7AAF9_H_ */
