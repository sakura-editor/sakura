/*!	@file
	@brief Editorオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CEDITORIFOBJ_1C8AA37E_D9FB_4C26_AE83_22E62D9B7C3D_H_
#define SAKURA_CEDITORIFOBJ_1C8AA37E_D9FB_4C26_AE83_22E62D9B7C3D_H_
#pragma once

#include "_os/OleTypes.h"
#include "macro/CWSHIfObj.h"

class CEditorIfObj : public CWSHIfObj
{
	// コンストラクタ
public:
	CEditorIfObj() : CWSHIfObj( L"Editor", true ){}

	// 実装
	MacroFuncInfoArray GetMacroCommandInfo() const override;	//コマンド情報を取得する
	MacroFuncInfoArray GetMacroFuncInfo() const override;	//関数情報を取得する
	bool HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result) override;	//関数を処理する
	bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize) override;	//コマンドを処理する
};
#endif /* SAKURA_CEDITORIFOBJ_1C8AA37E_D9FB_4C26_AE83_22E62D9B7C3D_H_ */
