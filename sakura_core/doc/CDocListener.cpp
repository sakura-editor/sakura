// この行は文字化け対策用です。
#include "stdafx.h"
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

