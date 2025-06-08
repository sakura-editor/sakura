/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "types/CType.h"

void CType_Other::InitTypeConfigImp(STypeConfig* pType)
{
	//名前と拡張子
	auto_sprintf( pType->m_szTypeName, L"設定%d", pType->m_nIdx + 1 );
	pType->m_szTypeExts[0] = L'\0';
}
