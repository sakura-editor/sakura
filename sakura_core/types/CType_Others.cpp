#include "StdAfx.h"
#include "types/CType.h"

#define IMPLEMENT_CTYPE_OTHER(CLASS_NAME, TYPE_NAME) \
void CLASS_NAME::InitTypeConfigImp(STypeConfig* pType) \
{ \
	_tcscpy( pType->m_szTypeName, _T(#TYPE_NAME) ); \
	_tcscpy( pType->m_szTypeExts, _T("") ); \
}

IMPLEMENT_CTYPE_OTHER(CType_Other17, �ݒ�17)
IMPLEMENT_CTYPE_OTHER(CType_Other18, �ݒ�18)
IMPLEMENT_CTYPE_OTHER(CType_Other19, �ݒ�19)
IMPLEMENT_CTYPE_OTHER(CType_Other20, �ݒ�20)
IMPLEMENT_CTYPE_OTHER(CType_Other21, �ݒ�21)
IMPLEMENT_CTYPE_OTHER(CType_Other22, �ݒ�22)
IMPLEMENT_CTYPE_OTHER(CType_Other23, �ݒ�23)
IMPLEMENT_CTYPE_OTHER(CType_Other24, �ݒ�24)
IMPLEMENT_CTYPE_OTHER(CType_Other25, �ݒ�25)
IMPLEMENT_CTYPE_OTHER(CType_Other26, �ݒ�26)
IMPLEMENT_CTYPE_OTHER(CType_Other27, �ݒ�27)
IMPLEMENT_CTYPE_OTHER(CType_Other28, �ݒ�28)
IMPLEMENT_CTYPE_OTHER(CType_Other29, �ݒ�29)
IMPLEMENT_CTYPE_OTHER(CType_Other30, �ݒ�30)

