/*! @file */
#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CColor_Url.h"
#include "parse/CWordParse.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Url::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	int	nUrlLen;

	// nPos が URL の先頭である場合
	if( IsURL( cStr.GetPtr(), nPos, cStr.GetLength(), &nUrlLen ) )
	{
		this->m_nCOMMENTEND = nPos + nUrlLen;
		return true;
	}
	return false;
}

bool CColor_Url::EndColor(const CStringRef& cStr, int nPos)
{
	if( nPos == this->m_nCOMMENTEND ){
		return true;
	}
	return false;
}
