/*!	@file
	@brief WSH Handler

	@author 鬼
	@date 2002年4月28日,5月3日,5月5日,5月6日,5月13日,5月16日
	@date 2002.08.25 genta リンクエラー回避のためCWSHManager.hにエディタの
		マクロインターフェース部を分離．
	@date 2009.10.29 syat インターフェースオブジェクト部分をCWSHIfObj.hに分離
*/
/*
	Copyright (C) 2002, 鬼, genta
	Copyright (C) 2009, syat
	Copyright (C) 2018-2022, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.

*/

#ifndef SAKURA_CWSH_B4802BFB_233E_4104_AEA7_AE3721801C27_H_
#define SAKURA_CWSH_B4802BFB_233E_4104_AEA7_AE3721801C27_H_
#pragma once

#include <ActivScp.h>
//↑Microsoft Platform SDK より
#include "macro/CIfObj.h"

/* 2009.10.29 syat インターフェースオブジェクト部分をCWSHIfObj.hに分離
template<class Base>
class ImplementsIUnknown: public Base

class CInterfaceObject: public ImplementsIUnknown<IDispatch>
 */
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

class CWSHClient final : IWSHClient
{
	using Me = CWSHClient;

public:
	// 型定義
	typedef std::vector<CIfObj*> List;      // 所有しているインターフェースオブジェクトのリスト
	typedef List::const_iterator ListIter;	// そのイテレータ

	// コンストラクタ・デストラクタ
	CWSHClient(const wchar_t *AEngine, ScriptErrorHandler AErrorHandler, void *AData);
	CWSHClient(const Me&) = delete;
	Me& operator = (const Me&) = delete;
	CWSHClient(Me&&) noexcept = delete;
	Me& operator = (Me&&) noexcept = delete;
	~CWSHClient();

	// フィールド・アクセサ
	ScriptErrorHandler m_OnError;
	void *m_Data;
	bool m_Valid; ///< trueの場合スクリプトエンジンが使用可能。falseになる場合は ScriptErrorHandlerにエラー内容が通知されている。
	void* GetData() const override{ return this->m_Data; }
	const List& GetInterfaceObjects() {	return this->m_IfObjArr; }

	// 操作
	void AddInterfaceObject( CIfObj* obj );
	bool Execute(const wchar_t *AScript);
	void Error(BSTR Description, BSTR Source); ///< ScriptErrorHandlerを呼び出す。
	void Error(const wchar_t* Description);          ///< ScriptErrorHandlerを呼び出す。

private:
	IActiveScript *m_Engine;
	List m_IfObjArr;
};
#endif /* SAKURA_CWSH_B4802BFB_233E_4104_AEA7_AE3721801C27_H_ */
