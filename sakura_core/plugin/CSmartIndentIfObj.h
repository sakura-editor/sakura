/*!	@file
	@brief SmartIndentオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2010, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CSMARTINDENTIFOBJ_7F0A25BE_E50A_45C4_B20E_C9683FD04BB8_H_
#define SAKURA_CSMARTINDENTIFOBJ_7F0A25BE_E50A_45C4_B20E_C9683FD04BB8_H_
#pragma once

#include "macro/CWSHIfObj.h"

#include <array>

// スマートインデント用WSHオブジェクト
class CSmartIndentIfObj final : public CWSHIfObj
{
	// 型定義
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_SI_GETCHAR							//押下したキーを取得する
	};

	// コンストラクタ
public:
	explicit CSmartIndentIfObj(
		wchar_t ch
	)
		: CWSHIfObj( L"Indent", false )
		, m_wcChar{ ch }
	{
	}

	// デストラクタ
	~CSmartIndentIfObj() override = default;

	// 実装
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const override{
		static std::array<MacroFuncInfo, 2> macroFuncInfoNotCommandArr = {
			//ID								関数名					引数										戻り値の型	
			MacroFuncInfo{F_SI_GETCHAR,			L"GetChar",				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	}, //押下したキーを取得する
			//	終端
			MacroFuncInfo{F_INVALID,	nullptr, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	nullptr}
		};
		return macroFuncInfoNotCommandArr.data();
	}
	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result) override
	{
		switch ( LOWORD(ID) ) 
		{
		case F_SI_GETCHAR:						//押下したキーを取得する
			{
				std::wstring sValue;
				sValue += m_wcChar;
				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		default:
			break;
		}
		return CWSHIfObj::HandleFunction(View, ID, Arguments, ArgSize, Result);
	}

	// メンバ変数
	wchar_t m_wcChar;
};

#endif /* SAKURA_CSMARTINDENTIFOBJ_7F0A25BE_E50A_45C4_B20E_C9683FD04BB8_H_ */
