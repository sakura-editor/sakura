#include "stdafx.h"
#include "types/CType.h"

#define IMPLEMENT_CTYPE_OTHER(CLASS_NAME, TYPE_NAME) \
void CLASS_NAME::InitTypeConfigImp(STypeConfig* pType) \
{ \
	_tcscpy( pType->m_szTypeName, _T(#TYPE_NAME) ); \
	_tcscpy( pType->m_szTypeExts, _T("") ); \
}

IMPLEMENT_CTYPE_OTHER(CType_Other1, �ݒ�17)
IMPLEMENT_CTYPE_OTHER(CType_Other2, �ݒ�18)
IMPLEMENT_CTYPE_OTHER(CType_Other3, �ݒ�19)
IMPLEMENT_CTYPE_OTHER(CType_Other4, �ݒ�20)
IMPLEMENT_CTYPE_OTHER(CType_Other5, �ݒ�21)
IMPLEMENT_CTYPE_OTHER(CType_Other6, �ݒ�22)
IMPLEMENT_CTYPE_OTHER(CType_Other7, �ݒ�23)
IMPLEMENT_CTYPE_OTHER(CType_Other8, �ݒ�24)
IMPLEMENT_CTYPE_OTHER(CType_Other9, �ݒ�25)
IMPLEMENT_CTYPE_OTHER(CType_Other10, �ݒ�26)
IMPLEMENT_CTYPE_OTHER(CType_Other11, �ݒ�27)
IMPLEMENT_CTYPE_OTHER(CType_Other12, �ݒ�28)
IMPLEMENT_CTYPE_OTHER(CType_Other13, �ݒ�29)
IMPLEMENT_CTYPE_OTHER(CType_Other14, �ݒ�30)

