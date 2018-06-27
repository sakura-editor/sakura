/*!	@file
	@brief Editorオブジェクト

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
#ifndef SAKURA_CEDITORIFOBJ_87C246CB_1387_4893_A26F_190155191A96D_H_
#define SAKURA_CEDITORIFOBJ_87C246CB_1387_4893_A26F_190155191A96D_H_

#include "_os/OleTypes.h"
#include "macro/CWSHIfObj.h"


class CEditorIfObj : public CWSHIfObj
{
	// コンストラクタ
public:
	CEditorIfObj() : CWSHIfObj( L"Editor", true ){}

	// 実装
	MacroFuncInfoArray GetMacroCommandInfo() const;	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const;	//関数情報を取得する
	bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result);	//関数を処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize);	//コマンドを処理する
};


#endif /* SAKURA_CEDITORIFOBJ_87C246CB_1387_4893_A26F_190155191A96D_H_ */
/*[EOF]*/
