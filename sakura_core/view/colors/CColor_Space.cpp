#include "stdafx.h"
#include "CColor_Space.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           タブ                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_Tab::BeginColor(const CStringRef& cStr, int nPos)
{
#ifdef NEW_ZENSPACE
	return false;
#endif
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( cStr.At(nPos) == WCODE::TAB ){
		return true;
	}
	return false;
}

bool CColor_Tab::EndColor(const CStringRef& cStr, int nPos)
{
	if( cStr.At(nPos) != WCODE::TAB ){
		return true;
	}
	return false;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       全角スペース                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_ZenSpace::BeginColor(const CStringRef& cStr, int nPos)
{
#ifdef NEW_ZENSPACE
	return false;
#endif
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if( WCODE::IsZenkakuSpace(cStr.At(nPos)) )
	{
		return true;
	}
	return false;
}

bool CColor_ZenSpace::EndColor(const CStringRef& cStr, int nPos)
{
	if( !WCODE::IsZenkakuSpace(cStr.At(nPos)) ){
		return true;
	}
	return false;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       半角スペース                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CColor_HanSpace::BeginColor(const CStringRef& cStr, int nPos)
{
#ifdef NEW_ZENSPACE
	return false;
#endif
	if(!cStr.IsValid())return false;

	const CEditDoc* pcDoc = CEditDoc::GetInstance(0);
	const STypeConfig* TypeDataPtr = &pcDoc->m_cDocType.GetDocumentAttribute();
	if (cStr.At(nPos) == L' ' && TypeDataPtr->m_ColorInfoArr[COLORIDX_SPACE].m_bDisp )
	{
		return true;
	}
	return false;
}

bool CColor_HanSpace::EndColor(const CStringRef& cStr, int nPos)
{
	if( cStr.At(nPos) != L' ' ){
		return true;
	}
	return false;
}

