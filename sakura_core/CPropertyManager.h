/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CPROPERTYMANAGER_E2156683_CAC9_4255_82A2_749A0D1D383A_H_
#define SAKURA_CPROPERTYMANAGER_E2156683_CAC9_4255_82A2_749A0D1D383A_H_
#pragma once

#include "prop/CPropCommon.h"
#include "typeprop/CPropTypes.h"

class CImageListMgr;
class CMenuDrawer;

class CPropertyManager{
public:
	void Create( HWND, CImageListMgr*, CMenuDrawer* );

	/*
	|| その他
	*/
	bool OpenPropertySheet( HWND hWnd, int nPageNum, bool bTrayProc );	/* 共通設定 */
	bool OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType );	/* タイプ別設定 */

private:
	HWND			m_hwndOwner;
	CImageListMgr*	m_pImageList;
	CMenuDrawer*	m_pMenuDrawer;

	int				m_nPropComPageNum;
	int				m_nPropTypePageNum;
};
#endif /* SAKURA_CPROPERTYMANAGER_E2156683_CAC9_4255_82A2_749A0D1D383A_H_ */
