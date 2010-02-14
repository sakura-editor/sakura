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

#include <string>
#include <list>
#include <vector>


//COM一般

template<class Base>
class ImplementsIUnknown: public Base
{
private:
	int m_RefCount;
	ImplementsIUnknown(const ImplementsIUnknown &);
	ImplementsIUnknown& operator = (const ImplementsIUnknown &);
public:
	#ifdef __BORLANDC__
	#pragma argsused
	#endif
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject) 
	{ 
		return E_NOINTERFACE; 
	}
	virtual ULONG STDMETHODCALLTYPE AddRef() { ++ m_RefCount; return m_RefCount; };
	virtual ULONG STDMETHODCALLTYPE Release() { -- m_RefCount; int R = m_RefCount; if(m_RefCount == 0) delete this; return R; };
public:
	ImplementsIUnknown(): m_RefCount(0) {};
	virtual ~ImplementsIUnknown(){};
};

//WSH一般
class CWSHIfObj;
enum EFunctionCode;
typedef HRESULT (CWSHIfObj::*CIfObjMethod)(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);
typedef void (*ScriptErrorHandler)(BSTR Description, BSTR Source, void *Data);

//CWSHIfObjが必要とするWSHClientのインタフェース
class IWSHClient
{
public:
	virtual void* GetData() const = 0;
};

//スクリプトに渡されるオブジェクト
class CWSHIfObj: public ImplementsIUnknown<IDispatch>
{
	// 型定義
private:
	typedef std::wstring wstring;
public:
	typedef std::list<CWSHIfObj*> List;		//インタフェースオブジェクトのリスト
	typedef List::const_iterator ListIter;	//そのイテレータ
	struct CMethodInfo
	{
		FUNCDESC				Desc;
		wchar_t					Name[64];
		CIfObjMethod			Method;
		ELEMDESC				Arguments[8];
		EFunctionCode			ID;
	};
	typedef std::vector<CMethodInfo> CMethodInfoList;

	// コンストラクタ
public:
	CWSHIfObj(WCHAR* name);

	// デストラクタ
public:
	virtual ~CWSHIfObj();

	// 操作
public:
	void AddMethod(wchar_t *Name, EFunctionCode ID, VARTYPE *ArgumentTypes,
		int ArgumentCount, VARTYPE ResultType, CIfObjMethod Method);
	void ReserveMethods(int Count)
	{
		m_Methods.reserve(Count);
	}
	virtual void ReadyMethods( int flags );	//コマンド・関数を準備する
protected:
	//	2007.07.20 genta : flags追加
	//  2009.09.05 syat CWSHManagerから移動
	void ReadyCommands(MacroFuncInfo *Info, int flags);
	HRESULT MacroCommand(EFunctionCode ID, DISPPARAMS *Arguments, VARIANT* Result, void *Data);

	// 属性
public:
	virtual bool IsGlobalMember() { return false; }	//オブジェクト名の省略可否

	// 実装
public:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
	virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(
					REFIID riid,
					OLECHAR FAR* FAR* rgszNames,
					UINT cNames,
					LCID lcid,
					DISPID FAR* rgdispid);
	virtual HRESULT STDMETHODCALLTYPE Invoke(
					DISPID dispidMember,
					REFIID riid,
					LCID lcid,
					WORD wFlags,
					DISPPARAMS FAR* pdispparams,
					VARIANT FAR* pvarResult,
					EXCEPINFO FAR* pexcepinfo,
					UINT FAR* puArgErr);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfo( 
					/* [in] */ UINT iTInfo,
					/* [in] */ LCID lcid,
					/* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
	virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount( 
					/* [out] */ UINT __RPC_FAR *pctinfo);

	// メンバ変数
public:
	CMethodInfoList m_Methods;			//メソッド情報リスト
	IWSHClient *m_Owner;				//オーナーCWSHClient
	wstring m_sName;					//インタフェースオブジェクト名
private:
	ITypeInfo* m_TypeInfo;

	// 非実装提供
protected:
	virtual bool HandleFunction(CEditView* View, EFunctionCode ID, const VARIANT *Arguments, const int ArgSize, VARIANT &Result) = 0;	//関数を処理する
	virtual void HandleCommand(CEditView* View, EFunctionCode ID, const WCHAR* Arguments[], const int ArgSize) = 0;	//コマンドを処理する
	virtual MacroFuncInfoArray GetMacroCommandInfo() const = 0;	//コマンド情報を取得する
	virtual MacroFuncInfoArray GetMacroFuncInfo() const = 0;	//関数情報を取得する
};


#endif /* SAKURA_CWSHIFOBJ_EAA6C3E3_1442_4940_B8A3_2AAB324D8788D_H_ */
/*[EOF]*/
