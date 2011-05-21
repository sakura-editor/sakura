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

#include "stdafx.h"
#include "macro/CWSHIfObj.h"
#include "macro/CSMacroMgr.h" // MacroFuncInfo
#include "Funccode_enum.h" // EFunctionCode::FA_FROMMACRO


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

	@date 2007.07.20 genta flags追加．flagはコマンド登録段階で混ぜておく．
*/
void CWSHIfObj::ReadyCommands(MacroFuncInfo *Info, int flags)
{
	while(Info->m_nFuncID != -1)	// Aug. 29, 2002 genta 番人の値が変更されたのでここも変更
	{
		wchar_t FuncName[256];
		wcscpy(FuncName, Info->m_pszFuncName);

		int ArgCount = 0;
		for(int I = 0; I < 4; ++I)
			if(Info->m_varArguments[I] != VT_EMPTY) 
				++ArgCount;
		
		//	2007.07.21 genta : flagを加えた値を登録する
		this->AddMethod(
			FuncName,
			(Info->m_nFuncID | flags),
			Info->m_varArguments,
			ArgCount,
			Info->m_varResult,
			reinterpret_cast<CIfObjMethod>(&CWSHIfObj::MacroCommand)
			/* CWSHIfObjを継承したサブクラスからReadyCommandsを呼び出した場合、
			 * サブクラスのMacroCommandが呼び出される。 */
		);
		
		++Info;
	}
}

/*!
	マクロコマンドの実行

	@date 2005.06.27 zenryaku 戻り値の受け取りが無くてもエラーにせずに関数を実行する
*/
HRESULT CWSHIfObj::MacroCommand(int IntID, DISPPARAMS *Arguments, VARIANT* Result, void *Data)
{
	int I;
	int ArgCount = Arguments->cArgs;
	if(ArgCount > 4) ArgCount = 4;

	const EFunctionCode ID = static_cast<EFunctionCode>(IntID);
	//	2007.07.22 genta : コマンドは下位16ビットのみ
	if(LOWORD(ID) >= F_FUNCTION_FIRST)
	{
		VARIANT ret; // 2005.06.27 zenryaku 戻り値の受け取りが無くても関数を実行する
		VariantInit(&ret);

		// 2011.3.18 syat 引数の順序を正しい順にする
		VARIANTARG rgvargBak[4];
		memcpy( rgvargBak, Arguments->rgvarg, sizeof(VARIANTARG) * 4 );
		for(I = 0; I < ArgCount; I++){
			Arguments->rgvarg[ArgCount-I-1] = rgvargBak[I];
		}

		// 2009.9.5 syat HandleFunctionはサブクラスでオーバーライドする
		bool r = HandleFunction(m_pView, ID, Arguments->rgvarg, Arguments->cArgs, ret);
		if(Result) {::VariantCopyInd(Result, &ret);}
		VariantClear(&ret);
		return r ? S_OK : E_FAIL;
	}
	else
	{
		//	Nov. 29, 2005 FILE 引数を文字列で取得する
		WCHAR *StrArgs[4] = {NULL, NULL, NULL, NULL};	// 初期化必須
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
			}
			StrArgs[ArgCount - I - 1] = S;			// DISPPARAMSは引数の順序が逆転しているため正しい順に直す
		}

		// 2009.10.29 syat HandleCommandはサブクラスでオーバーライドする
		HandleCommand(m_pView, ID, const_cast<WCHAR const **>(StrArgs), ArgCount);

		//	Nov. 29, 2005 FILE 配列の破棄なので、[括弧]を追加
		for(int J = 0; J < ArgCount; ++J)
			delete [] StrArgs[J];

		return S_OK;
	}
}
