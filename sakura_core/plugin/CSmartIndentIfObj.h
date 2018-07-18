/*!	@file
	@brief SmartIndentオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2010, syat

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
#ifndef SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_
#define SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_

#include "macro/CWSHIfObj.h"

// スマートインデント用WSHオブジェクト
class CSmartIndentIfObj : public CWSHIfObj
{
	// 型定義
	enum FuncId {
		F_SI_COMMAND_FIRST = 0,					//↓コマンドは以下に追加する
		F_SI_FUNCTION_FIRST = F_FUNCTION_FIRST,	//↓関数は以下に追加する
		F_SI_GETCHAR							//押下したキーを取得する
	};
	typedef std::string string;
	typedef std::wstring wstring;

	// コンストラクタ
public:
	CSmartIndentIfObj( wchar_t ch )
		: CWSHIfObj( L"Indent", false )
		, m_wcChar( ch )
	{
	}

	// デストラクタ
public:
	~CSmartIndentIfObj(){}

	// 実装
public:
	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const{
		static MacroFuncInfo macroFuncInfoArr[] = {
			//	終端
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
		};
		return macroFuncInfoArr;
	}
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const{
		static MacroFuncInfo macroFuncInfoNotCommandArr[] = {
			//ID									関数名							引数										戻り値の型	m_pszData
			{EFunctionCode(F_SI_GETCHAR),			LTEXT("GetChar"),				{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //押下したキーを取得する
			//	終端
			{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
		};
		return macroFuncInfoNotCommandArr;
	}
	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		switch ( LOWORD(ID) ) 
		{
		case F_SI_GETCHAR:						//押下したキーを取得する
			{
				wstring sValue;
				sValue += m_wcChar;
				SysString S(sValue.c_str(), sValue.size());
				Wrap(&Result)->Receive(S);
			}
			return true;
		}
		return false;
	}
	//コマンドを処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize)
	{
		return false;
	}

	// メンバ変数
public:
	wchar_t m_wcChar;
};

#endif /* SAKURA_CSMARTINDENTIFOBJ_5708577D_5A1D_4374_91FD_5E180C9F4E20_H_ */
