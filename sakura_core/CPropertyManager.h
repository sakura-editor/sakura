#pragma once

#include "CPropCommon.h"
#include "CPropTypes.h"

class CPropertyManager{
public:
	CPropertyManager();
	/*
	|| ���̑�
	*/
	BOOL OpenPropertySheet( int nPageNum );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int nPageNum, CDocumentType nSettingType );	/* �^�C�v�ʐݒ� */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};
