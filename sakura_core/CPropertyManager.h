#pragma once

#include "prop/CPropCommon.h"
#include "types/CPropTypes.h"

class CPropertyManager{
public:
	CPropertyManager();
	/*
	|| ���̑�
	*/
	BOOL OpenPropertySheet( int nPageNum );	/* ���ʐݒ� */
	BOOL OpenPropertySheetTypes( int nPageNum, CTypeConfig nSettingType );	/* �^�C�v�ʐݒ� */

public:
	CPropCommon			m_cPropCommon;
	CPropTypes			m_cPropTypes;
};
