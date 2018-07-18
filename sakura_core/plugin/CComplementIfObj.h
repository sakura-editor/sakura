/*!	@file
	@brief Complementオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2011, Moca
	

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
#ifndef SAKURA_CCOMPLEMENTIFOBJ_CE6919C7_F8F8_479E_BC70_7BA4651351D27_H_
#define SAKURA_CCOMPLEMENTIFOBJ_CE6919C7_F8F8_479E_BC70_7BA4651351D27_H_

#include "macro/CWSHIfObj.h"
#include "util/ole_convert.h"

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
	CComplementIfObj( std::wstring& curWord, CHokanMgr* pMgr, int option )
		: CWSHIfObj( L"Complement", false )
		, m_sCurrentWord( curWord )
		, m_pHokanMgr( pMgr )
		, m_nOption( option )
	{
	}

	// デストラクタ
public:
	~CComplementIfObj(){}

	// 実装
public:
	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroCommandInfo() const{ return m_MacroFuncInfoCommandArr; }
	//関数情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const{ return m_MacroFuncInfoArr; };
	//関数を処理する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result)
	{
		Variant varCopy;	// VT_BYREFだと困るのでコピー用

		switch(LOWORD(ID)){
		case F_CM_GETCURRENTWORD:	//補完対象の文字列を取得
			{
				SysString s( m_sCurrentWord.c_str(), m_sCurrentWord.length() );
				Wrap( &Result )->Receive( s );
			}
			return true;
		case F_CM_GETOPTION:	//オプションを取得
			{
				Wrap( &Result )->Receive( m_nOption );
			}
			return true;
		case F_CM_ADDLIST:		//候補に追加する
			{
				std::wstring keyword;
				if( variant_to_wstr( Arguments[0], keyword ) != true) return false;
				const wchar_t* word = keyword.c_str();
				int nWordLen = keyword.length();
				if( nWordLen <= 0 ) return false;
				std::wstring strWord = std::wstring(word, nWordLen);
				if( CHokanMgr::AddKouhoUnique( m_pHokanMgr->m_vKouho, strWord ) ){
					Wrap( &Result )->Receive( m_pHokanMgr->m_vKouho.size() );
				}else{
					Wrap( &Result )->Receive( -1 );
				}
				return true;
			}
		}
		return false;
	}
	//コマンドを処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize)
	{
		return false;
	}

	// メンバ変数
private:
	std::wstring m_sCurrentWord;
	CHokanMgr* m_pHokanMgr;
	int m_nOption; // 0x01 == IgnoreCase

private:
	static MacroFuncInfo m_MacroFuncInfoCommandArr[];	// コマンド情報(戻り値なし)
	static MacroFuncInfo m_MacroFuncInfoArr[];	// 関数情報(戻り値あり)
};

//コマンド情報
MacroFuncInfo CComplementIfObj::m_MacroFuncInfoCommandArr[] = 
{
	//ID									関数名							引数										戻り値の型	m_pszData
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

//関数情報
MacroFuncInfo CComplementIfObj::m_MacroFuncInfoArr[] = 
{
	//ID								関数名				引数										戻り値の型	m_pszData
	{EFunctionCode(F_CM_GETCURRENTWORD),L"GetCurrentWord",	{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_BSTR,	NULL }, //補完対象の文字列を取得
	{EFunctionCode(F_CM_GETOPTION),		L"GetOption",		{VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //補完対象の文字列を取得
	{EFunctionCode(F_CM_ADDLIST),		L"AddList",			{VT_BSTR,  VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_I4,		NULL }, //候補に追加する
	//	終端
	{F_INVALID,	NULL, {VT_EMPTY, VT_EMPTY, VT_EMPTY, VT_EMPTY},	VT_EMPTY,	NULL}
};

#endif /* SAKURA_CCOMPLEMENTIFOBJ_CE6919C7_F8F8_479E_BC70_7BA4651351D27_H_ */
