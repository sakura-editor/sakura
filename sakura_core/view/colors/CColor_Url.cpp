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

	// なんかの条件
	const bool someCondition = _IsPosKeywordHead( cStr, nPos );
	//　↓
	// return (nPos == 0 || !IS_KEYWORD_CHAR( cStr.At( nPos - 1 ) ));
	//　↓
	// nPos が 行頭、または、nPos の 直前の文字が キーワードに使える文字 でも ユーザー定義キーワード文字 でもない

	int	nUrlLen;

	// nPos が URL の先頭である場合
	if( someCondition && IsURL( cStr.GetPtr(), nPos, cStr.GetLength(), &nUrlLen ) )
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
