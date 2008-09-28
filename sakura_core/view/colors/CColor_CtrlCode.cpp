#include "stdafx.h"
#include "CColor_CtrlCode.h"
#include "parse/CWordParse.h"
#include "util/string_ex2.h"
#include "doc/CLayout.h"
#include "types/CTypeSupport.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                        制御コード                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_CtrlCode::BeginColor(const CStringRef& cStr, int nPos)
{
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();

	if(TypeDataPtr->m_ColorInfoArr[COLORIDX_CTRLCODE].m_bDisp	/* コントロールコードを色分け */
		&& WCODE::IsControlCode(cStr.At(nPos)))
	{
		return true;
	}

	return false;
}

bool CColor_CtrlCode::EndColor(const CStringRef& cStr, int nPos)
{
	if( !WCODE::IsControlCode(cStr.At(nPos)) ){
		return true;
	}
	return false;
}

