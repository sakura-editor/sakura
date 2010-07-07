#include "StdAfx.h"
#include "types/CType.h"

void CType_Other::InitTypeConfigImp(STypeConfig* pType)
{
	//–¼‘O‚ÆŠg’£Žq
	auto_sprintf( pType->m_szTypeName, _T("Ý’è%d"), pType->m_nIdx + 1 );
	_tcscpy( pType->m_szTypeExts, _T("") );

}
