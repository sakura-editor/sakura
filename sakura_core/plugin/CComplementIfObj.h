/*!	@file
	@brief Complementオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2011, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CCOMPLEMENTIFOBJ_674B3C16_A1FD_4FE6_B3F5_054A5D1C71DB_H_
#define SAKURA_CCOMPLEMENTIFOBJ_674B3C16_A1FD_4FE6_B3F5_054A5D1C71DB_H_
#pragma once

#include "macro/CWSHIfObj.h"
#include "util/ole_convert.h"

#include <array>
#include <string>
#include <string_view>

class CHokanMgr;

class CComplementIfObj : public CWSHIfObj {
	// 型定義
	enum FuncId {
		F_OL_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_OL_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_CM_GETCURRENTWORD,					//補完対象の文字列を取得
		F_CM_GETOPTION,							//オプションを取得
		F_CM_ADDLIST,							//候補に追加
	};

	// コンストラクタ
public:
	CComplementIfObj(
		std::wstring_view curWord,
		CHokanMgr* pMgr,
		int option
	)
		: CWSHIfObj( L"Complement", false )
		, m_sCurrentWord( curWord )
		, m_pHokanMgr( pMgr )
		, m_nOption( option )
	{
	}

	// デストラクタ
public:
	~CComplementIfObj() override = default;

	// 実装
public:
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const override {
		static std::array<MacroFuncInfo, 4> macroFuncInfoNotCommandArr = 
		{
			//			  ID					関数名				引数										戻り値の型	
			MacroFuncInfo{F_CM_GETCURRENTWORD,	L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	}, //補完対象の文字列を取得
			MacroFuncInfo{F_CM_GETOPTION,		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		}, //補完対象の文字列を取得
			MacroFuncInfo{F_CM_ADDLIST,			L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		}, //候補に追加する
			//	終端
			MacroFuncInfo{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoNotCommandArr.data();
	}

	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result) override
	{
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch(LOWORD(ID)){
		case F_CM_GETCURRENTWORD:	//補完対象の文字列を取得
			{
				SysString s( m_sCurrentWord.c_str(), m_sCurrentWord.length() );
				Wrap( &Result )->Receive( s );
			}
			break;

		case F_CM_GETOPTION:	//オプションを取得
			{
				Wrap( &Result )->Receive( m_nOption );
			}
			break;

		case F_CM_ADDLIST:		//候補に追加する
			{
				std::wstring keyword;
				if( variant_to_wstr( Arguments[0], keyword ) != true) return false;
				const wchar_t* word = keyword.c_str();
				const auto nWordLen = keyword.length();
				if( nWordLen <= 0 ) return false;
				std::wstring strWord(word, nWordLen);
				if( CHokanMgr::AddKouhoUnique( m_pHokanMgr->m_vKouho, strWord ) ){
					Wrap( &Result )->Receive( (int)m_pHokanMgr->m_vKouho.size() );
				}else{
					Wrap( &Result )->Receive( -1 );
				}
			}
			break;

		default:
			return CWSHIfObj::HandleFunction(View, ID, Arguments, ArgSize, Result);
		}
		return true;
	}

	// メンバ変数
private:
	std::wstring m_sCurrentWord;
	CHokanMgr* m_pHokanMgr;
	int m_nOption; // 0x01 == IgnoreCase
};

#endif /* SAKURA_CCOMPLEMENTIFOBJ_674B3C16_A1FD_4FE6_B3F5_054A5D1C71DB_H_ */
