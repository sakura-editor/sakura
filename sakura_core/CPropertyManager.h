#pragma once

#include "CPropCommon.h"
#include "CPropTypes.h"

class CPropertyManager{
public:
	CPropertyManager();
	/*
	|| ÇªÇÃëº
	*/
	BOOL OpenPropertySheet( int nPageNum );	/* ã§í ê›íË */
	BOOL OpenPropertySheetTypes( int nPageNum, CDocumentType nSettingType );	/* É^ÉCÉvï ê›íË */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};
