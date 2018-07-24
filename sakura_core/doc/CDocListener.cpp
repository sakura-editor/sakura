/*
	ObserverパターンのCEditDoc特化版。
	CDocSubjectは観察され、CDocListnerは観察を行う。
	観察の開始は CDocListener::Listen で行う。

	$Note:
		Listener (Observer) と Subject のリレーション管理は
		ジェネリックな汎用モジュールに分離できる。
*/
/*
	Copyright (C) 2008, kobake

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
#include <map>
#include "doc/CDocListener.h"
#include "doc/CEditDoc.h"

bool SLoadInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->cFilePath,pszPath)==0;
}
bool SSaveInfo::IsSamePath(LPCTSTR pszPath) const
{
	return _tcsicmp(this->cFilePath,pszPath)==0;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CDocSubject                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//(1)

CDocSubject::~CDocSubject()
{
}


#define DEF_NOTIFY(NAME) ECallbackResult CDocSubject::Notify##NAME() \
{ \
	int n = GetListenerCount(); \
	for(int i=0;i<n;i++){ \
		ECallbackResult eRet = GetListener(i)->On##NAME(); \
		if(eRet!=CALLBACK_CONTINUE)return eRet; \
	} \
	return CALLBACK_CONTINUE; \
}

#define DEF_NOTIFY2(NAME,ARGTYPE) ECallbackResult CDocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	for(int i=0;i<n;i++){ \
		ECallbackResult eRet = GetListener(i)->On##NAME(a); \
		if(eRet!=CALLBACK_CONTINUE)return eRet; \
	} \
	return CALLBACK_CONTINUE; \
}

#define VOID_NOTIFY(NAME) void CDocSubject::Notify##NAME() \
{ \
	int n = GetListenerCount(); \
	for(int i=0;i<n;i++){ \
		GetListener(i)->On##NAME(); \
	} \
}

#define VOID_NOTIFY2(NAME,ARGTYPE) void CDocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	for(int i=0;i<n;i++){ \
		GetListener(i)->On##NAME(a); \
	} \
}

//######仮
#define CORE_NOTIFY2(NAME,ARGTYPE) ELoadResult CDocSubject::Notify##NAME(ARGTYPE a) \
{ \
	int n = GetListenerCount(); \
	ELoadResult eRet = LOADED_FAILURE; \
	for(int i=0;i<n;i++){ \
		ELoadResult e = GetListener(i)->On##NAME(a); \
		if(e==LOADED_NOIMPLEMENT)continue; \
		if(e==LOADED_FAILURE)return e; \
		eRet = e; \
	} \
	return eRet; \
}

DEF_NOTIFY2(CheckLoad,SLoadInfo*)
VOID_NOTIFY2(BeforeLoad,SLoadInfo*)
CORE_NOTIFY2(Load,const SLoadInfo&)
VOID_NOTIFY2(Loading,int)
VOID_NOTIFY2(AfterLoad,const SLoadInfo&)
VOID_NOTIFY2(FinalLoad,ELoadResult)

DEF_NOTIFY2(CheckSave,SSaveInfo*)
DEF_NOTIFY2(PreBeforeSave,SSaveInfo*)
VOID_NOTIFY2(BeforeSave,const SSaveInfo&)
VOID_NOTIFY2(Save,const SSaveInfo&)
VOID_NOTIFY2(Saving,int)
VOID_NOTIFY2(AfterSave,const SSaveInfo&)
VOID_NOTIFY2(FinalSave,ESaveResult)

DEF_NOTIFY(BeforeClose)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       CDocListener                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//(多)

CDocListener::CDocListener(CDocSubject* pcDoc)
{
	if(pcDoc==NULL)pcDoc = CEditDoc::GetInstance(0); //$$ インチキ
	assert( pcDoc );
	Listen(pcDoc);
}

CDocListener::~CDocListener()
{
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CDocListenerEx                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#include "doc/CEditDoc.h"

CEditDoc* CDocListenerEx::GetListeningDoc() const
{
	return static_cast<CEditDoc*>(CDocListener::GetListeningDoc());
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CProgressSubject                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
void CProgressSubject::NotifyProgress(int nPer)
{
	int n = GetListenerCount();
	for(int i=0;i<n;i++){
		GetListener(i)->OnProgress(nPer);
	}
}

