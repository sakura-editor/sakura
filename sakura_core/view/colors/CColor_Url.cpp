﻿/*! @file */
#include "StdAfx.h"
#include "CColor_Url.h"
#include "doc/CEditDoc.h"
#include "doc/layout/CLayout.h"
#include "parse/CWordParse.h"
#include "types/CTypeSupport.h"
#include "view/CEditView.h" // SColorStrategyInfo

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           URL                               //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Url::BeginColor(const CStringRef &cStr, int nPos)
{
	if (!cStr.IsValid()) return false;

	int nUrlLen;

	if (_IsPosKeywordHead(cStr, nPos) /* URLを表示する */
		&& IsURL(cStr.GetPtr(), nPos, cStr.GetLength(),
				 &nUrlLen) /* 指定アドレスがURLの先頭ならばTRUEとその長さを返す */
	) {
		this->m_nCOMMENTEND = nPos + nUrlLen;
		return true;
	}
	return false;
}

bool CColor_Url::EndColor(const CStringRef &cStr, int nPos)
{
	if (nPos == this->m_nCOMMENTEND) { return true; }
	return false;
}
