#include "StdAfx.h"
#include "types/CType.h"

#define IMPLEMENT_CTYPE_OTHER(CLASS_NAME, TYPE_NAME) \
void CLASS_NAME::InitTypeConfigImp(STypeConfig* pType) \
{ \
	_tcscpy( pType->m_szTypeName, _T(#TYPE_NAME) ); \
	_tcscpy( pType->m_szTypeExts, _T("") ); \
}

IMPLEMENT_CTYPE_OTHER(CType_Other17, ê›íË17)
IMPLEMENT_CTYPE_OTHER(CType_Other18, ê›íË18)
IMPLEMENT_CTYPE_OTHER(CType_Other19, ê›íË19)
IMPLEMENT_CTYPE_OTHER(CType_Other20, ê›íË20)
IMPLEMENT_CTYPE_OTHER(CType_Other21, ê›íË21)
IMPLEMENT_CTYPE_OTHER(CType_Other22, ê›íË22)
IMPLEMENT_CTYPE_OTHER(CType_Other23, ê›íË23)
IMPLEMENT_CTYPE_OTHER(CType_Other24, ê›íË24)
IMPLEMENT_CTYPE_OTHER(CType_Other25, ê›íË25)
IMPLEMENT_CTYPE_OTHER(CType_Other26, ê›íË26)
IMPLEMENT_CTYPE_OTHER(CType_Other27, ê›íË27)
IMPLEMENT_CTYPE_OTHER(CType_Other28, ê›íË28)
IMPLEMENT_CTYPE_OTHER(CType_Other29, ê›íË29)
IMPLEMENT_CTYPE_OTHER(CType_Other30, ê›íË30)

