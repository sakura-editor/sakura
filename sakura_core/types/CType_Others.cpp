#include "stdafx.h"
#include "types/CType.h"

#define IMPLEMENT_CTYPE_OTHER(CLASS_NAME, TYPE_NAME) \
void CLASS_NAME::InitTypeConfigImp(STypeConfig* pType) \
{ \
	_tcscpy( pType->m_szTypeName, _T(#TYPE_NAME) ); \
	_tcscpy( pType->m_szTypeExts, _T("") ); \
}

IMPLEMENT_CTYPE_OTHER(CType_Other1, ê›íË17)
IMPLEMENT_CTYPE_OTHER(CType_Other2, ê›íË18)
IMPLEMENT_CTYPE_OTHER(CType_Other3, ê›íË19)
IMPLEMENT_CTYPE_OTHER(CType_Other4, ê›íË20)
IMPLEMENT_CTYPE_OTHER(CType_Other5, ê›íË21)
IMPLEMENT_CTYPE_OTHER(CType_Other6, ê›íË22)
IMPLEMENT_CTYPE_OTHER(CType_Other7, ê›íË23)
IMPLEMENT_CTYPE_OTHER(CType_Other8, ê›íË24)
IMPLEMENT_CTYPE_OTHER(CType_Other9, ê›íË25)
IMPLEMENT_CTYPE_OTHER(CType_Other10, ê›íË26)
IMPLEMENT_CTYPE_OTHER(CType_Other11, ê›íË27)
IMPLEMENT_CTYPE_OTHER(CType_Other12, ê›íË28)
IMPLEMENT_CTYPE_OTHER(CType_Other13, ê›íË29)
IMPLEMENT_CTYPE_OTHER(CType_Other14, ê›íË30)

