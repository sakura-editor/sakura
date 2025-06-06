﻿/*!	@file
	@brief WSHインターフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.hから切り出し

*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CWSHIFOBJ_7C4DEA59_C6E2_4814_9209_8818D90096DE_H_
#define SAKURA_CWSHIFOBJ_7C4DEA59_C6E2_4814_9209_8818D90096DE_H_
#pragma once

#include <list>
#include <ActivScp.h>
#include "_os/OleTypes.h"
#include "macro/CIfObj.h"
#include "macro/CWSH.h" // CWSHClient::List, ListIter
#include "macro/CSMacroMgr.h" // MacroFuncInfo, MacroFuncInfoArray
class CEditView;

/* CWSHIfObj - プラグインやマクロに公開するオブジェクト
 * 使用上の注意:
 *   1. 生成はnewで。
 *      参照カウンタを持つので、自動変数で生成するとスコープ抜けて解放されるときにヒープエラーが出ます。
 *   2. 生成したらAddRef()、不要になったらRelease()を呼ぶこと。
 *   3. 新しいIfObjを作る時はCWSHIfObjを継承し、以下の4つをオーバーライドすること。
 *      GetMacroCommandInfo, GetMacroFuncInfo, HandleCommand, HandleFunction
 */
class CWSHIfObj
: public CIfObj
{
public:
	// 型定義
	typedef std::list<CWSHIfObj*> List;
	typedef List::const_iterator ListIter;

	// コンストラクタ
	CWSHIfObj(const wchar_t* name, bool isGlobal)
	: CIfObj(name, isGlobal)
	{}

	virtual void ReadyMethods( CEditView* pView, int flags );

protected:
	// 操作
	//	2007.07.20 genta : flags追加
	//  2009.09.05 syat CWSHManagerから移動
	void ReadyCommands(MacroFuncInfo *Info, int flags);
	HRESULT MacroCommand(int ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);

	// 非実装提供
	virtual bool HandleFunction(CEditView* View, EFunctionCode ID, VARIANT *Arguments, const int ArgSize, VARIANT &Result) = 0;	//関数を処理する
	virtual bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize) = 0;	//コマンドを処理する
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	//コマンド情報を取得する
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	//関数情報を取得する

	CEditView* m_pView;
};
#endif /* SAKURA_CWSHIFOBJ_7C4DEA59_C6E2_4814_9209_8818D90096DE_H_ */
