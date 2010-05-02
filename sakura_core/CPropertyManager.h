#pragma once

#include "prop/CPropCommon.h"
#include "typeprop/CPropTypes.h"

class CPropertyManager{
public:
	CPropertyManager();
	/*
	|| ÇªÇÃëº
	*/
	BOOL OpenPropertySheet( int nPageNum );	/* ã§í ê›íË */
	BOOL OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );	/* É^ÉCÉvï ê›íË */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};
