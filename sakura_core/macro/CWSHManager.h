/*!	@file
	@brief WSH Manager

	@author 鬼, genta
	@date 2002年4月28日,5月3日,5月5日,5月6日,5月13日,5月16日
	@date 2002.08.25 genta CWSH.hより分離

	@par TODO
	@li 未知のエンジンに対応できるようCMacroFactoryを変更 → 要議論
	@li CEditView::HandleCommandを使う → CMacro::HandleCommandでもなにかやってるようなのでいじらない方が？
	@li vector::reserveを使う → CSMacroMgrで個数が宣言されて無いので見送り
	@li 再描画の代わりにShowEditCaret → protectedですよー
*/
/*
	Copyright (C) 2002, 鬼, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef __CWSH_Manager_H__
#define __CWSH_Manager_H__

#include <Windows.h>
#include <string>
#include "macro/CSMacroMgr.h"
#include "macro/CMacroManagerBase.h"
#include "macro/CWSHIfObj.h"
class CEditView;

typedef void (*EngineCallback)(wchar_t *Ext, char *EngineName);

class CWSHMacroManager: public CMacroManagerBase
{
public:
	CWSHMacroManager(std::wstring const AEngineName);
	virtual ~CWSHMacroManager();

	//	2007.07.20 genta : flags追加
	virtual bool ExecKeyMacro(CEditView *EditView, int flags) const;
	virtual BOOL LoadKeyMacro(HINSTANCE hInstance, const TCHAR* pszPath);
	virtual BOOL LoadKeyMacroStr(HINSTANCE hInstance, const TCHAR* pszCode);

	static CMacroManagerBase* Creator(const TCHAR* FileExt);
	static void declare();

	void AddParam( CWSHIfObj* param );				//インタフェースオブジェクトを追加する
	void AddParam( CWSHIfObj::List& params );		//インタフェースオブジェクト達を追加する
	void ClearParam();								//インタフェースオブジェクトを削除する
protected:
	std::wstring m_Source;
	std::wstring m_EngineName;
	CWSHIfObj::List m_Params;
	//2009.10.29 syat CWSHIfObjへ移動
	////	2007.07.20 genta : flags追加
	//static void ReadyCommands(CIfObj *Object, MacroFuncInfo *Info, int flags);
};
#endif
