/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.cppから切り出し
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2003, FILE
	Copyright (C) 2004, genta
	Copyright (C) 2005, FILE, zenryaku
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#include "StdAfx.h"
#include "macro/CWSHIfObj.h"

#include <memory>
#include <string_view>
#include <vector>

#include "macro/CSMacroMgr.h" // MacroFuncInfo
#include "Funccode_enum.h" // EFunctionCode::FA_FROMMACRO

// コンストラクタ
CWSHIfObj::CWSHIfObj(const wchar_t* name, bool isGlobal)
	: CIfObj(name, isGlobal)
{
}

//コマンド・関数を準備する
void CWSHIfObj::ReadyMethods( CEditView* pView, int flags )
{
	this->m_pView = pView;
	//	 2007.07.20 genta : コマンドに混ぜ込むフラグを渡す
	ReadyCommands(GetMacroCommandInfo(), flags | FA_FROMMACRO );
	ReadyCommands(GetMacroFuncInfo(), 0);
	/* CWSHIfObjを継承したサブクラスからReadyMethodsを呼び出した場合、
	 * サブクラスのGetMacroCommandInfo,GetMacroFuncInfoが呼び出される。 */
}

/** WSHマクロエンジンへコマンド登録を行う

	@date 2002/08/29 genta 番人の値が変更されたのでここも変更
	@date 2007/07/20 genta flags追加．flagはコマンド登録段階で混ぜておく．
	@date 2009/09/05 syat CWSHManagerから移動
*/
void CWSHIfObj::ReadyCommands(MacroFuncInfoArray pFuncInfo, int flags)
{
	std::wstring_view funcName;
	std::vector<VARTYPE> varArgs(_countof(pFuncInfo->m_varArguments), VT_EMPTY);

	while (pFuncInfo->m_nFuncID != (int)F_INVALID)
	{
		funcName = pFuncInfo->m_pszFuncName;

		int ArgCount = 0;
		if (pFuncInfo->m_pData) {
			ArgCount = pFuncInfo->m_pData->m_nArgMinSize;
		}
		else {
			for (int i = 0; i < _countof(pFuncInfo->m_varArguments); ++i) {
				if (pFuncInfo->m_varArguments[i] != VT_EMPTY)
					++ArgCount;
			}
		}

		varArgs.resize(ArgCount, VT_EMPTY);

		for (int i = 0; i < ArgCount; i++) {
			if (i < _countof(pFuncInfo->m_varArguments)) {
				varArgs[i] = pFuncInfo->m_varArguments[i];
			}
			else {
				varArgs[i] = pFuncInfo->m_pData->m_pVarArgEx[i - _countof(pFuncInfo->m_varArguments)];
			}
		}

		AddMethod(
			funcName.data(),
			pFuncInfo->m_nFuncID | flags,
			varArgs.data(),
			ArgCount,
			pFuncInfo->m_varResult,
			reinterpret_cast<CIfObjMethod>(&CWSHIfObj::MacroCommand)
		);

		++pFuncInfo;
	}
}

/*!
	マクロコマンドの実行

	@date 2005.06.27 zenryaku 戻り値の受け取りが無くてもエラーにせずに関数を実行する
	@date 2013.06.07 Moca 5つ以上の引数の時ずれるのを修正。NULを含む文字列対応
*/
HRESULT CWSHIfObj::MacroCommand(int IntID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	int I;
	int ArgCount = Arguments->cArgs;

	const EFunctionCode ID = static_cast<EFunctionCode>(IntID);
	//	2007.07.22 genta : コマンドは下位16ビットのみ
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku 戻り値の受け取りが無くても関数を実行する
		VariantInit(&ret);

		// 2011.3.18 syat 引数の順序を正しい順にする
		auto rgvargParam = std::make_unique<VARIANTARG[]>(ArgCount);
		for(I = 0; I < ArgCount; I++){
			::VariantInit(&rgvargParam[ArgCount - I - 1]);
			::VariantCopy(&rgvargParam[ArgCount - I - 1], &Arguments->rgvarg[I]);
		}

		// 2009.9.5 syat HandleFunctionはサブクラスでオーバーライドする
		bool r = HandleFunction(m_pView, ID, &rgvargParam[0], ArgCount, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		for(I = 0; I < ArgCount; I++){
			::VariantClear(&rgvargParam[I]);
		}
		return r ? S_OK : E_FAIL;
	}
	else
	{
		// 最低4つは確保
		int argCountMin = t_max(4, ArgCount);
		//	Nov. 29, 2005 FILE 引数を文字列で取得する
		auto StrArgs = std::make_unique<LPWSTR[]>(argCountMin);
		auto strLengths = std::make_unique<int[]>(argCountMin);
		for(I = ArgCount; I < argCountMin; I++ ){
			StrArgs[I] = NULL;
			strLengths[I] = 0;
		}
		WCHAR *S = NULL;								// 初期化必須
		Variant varCopy;							// VT_BYREFだと困るのでコピー用
		int Len;
		for(I = 0; I < ArgCount; ++I)
		{
			if(VariantChangeType(&varCopy.Data, &(Arguments->rgvarg[I]), 0, VT_BSTR) == S_OK)
			{
				Wrap(&varCopy.Data.bstrVal)->GetW(&S, &Len);
			}
			else
			{
				S = new WCHAR[1];
				S[0] = 0;
				Len = 0;
			}
			StrArgs[ArgCount - I - 1] = S;			// DISPPARAMSは引数の順序が逆転しているため正しい順に直す
			strLengths[ArgCount - I - 1] = Len;
		}

		// 2009.10.29 syat HandleCommandはサブクラスでオーバーライドする
		HandleCommand(m_pView, ID, const_cast<WCHAR const **>(&StrArgs[0]), &strLengths[0], ArgCount);

		//	Nov. 29, 2005 FILE 配列の破棄なので、[括弧]を追加
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}
