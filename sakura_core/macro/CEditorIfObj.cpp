/*!	@file
	@brief Editorオブジェクト

*/
/*
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CEditorIfObj.h"
#include "macro/CMacro.h"
#include "macro/CSMacroMgr.h"

//コマンド情報を取得する
MacroFuncInfoArray CEditorIfObj::GetMacroCommandInfo() const
{
	return CSMacroMgr::m_MacroFuncInfoCommandArr;
}
//関数情報を取得する
MacroFuncInfoArray CEditorIfObj::GetMacroFuncInfo() const
{
	return CSMacroMgr::m_MacroFuncInfoArr;
}

//関数を処理する
bool CEditorIfObj::HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result)
{
	return CMacro::HandleFunction( View, ID, Arguments, ArgSize, Result );
}

//コマンドを処理する
bool CEditorIfObj::HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize)
{
	return CMacro::HandleCommand( View, ID, Arguments, ArgLengths, ArgSize );
}
