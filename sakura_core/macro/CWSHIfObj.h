/*!	@file
	@brief WSHインタフェースオブジェクト基本クラス

	@date 2009.10.29 syat CWSH.hから切り出し

*/
/*
	Copyright (C) 2002, 鬼, genta
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
#ifndef SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_
#define SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_
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
	virtual bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result) = 0;	//関数を処理する
	virtual bool HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgLengths[], const int ArgSize) = 0;	//コマンドを処理する
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	//コマンド情報を取得する
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	//関数情報を取得する

	CEditView* m_pView;
};


#endif /* SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_ */
/*[EOF]*/
