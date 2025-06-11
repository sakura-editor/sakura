/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "EditInfo.h"

/*!
 * コンストラクタ
 *
 * @date 2002/05/07 genta 確実に初期化するため追加
 */
EditInfo::EditInfo() noexcept
	: m_szPath{ 0 }
	, m_nCharCode(CODE_AUTODETECT)
	, m_bBom(false)
	, m_szDocType{ 0 }
	, m_nTypeId(-1)
	, m_nViewTopLine(-1)
	, m_nViewLeftCol(-1)
	, m_ptCursor{ -1, -1 }
	, m_bIsModified(false)
	, m_bIsGrep(false)
	, m_szGrepKey{ 0 }
	, m_bIsDebug(false)
	, m_szMarkLines{ 0 }
	, m_nWindowSizeX(-1)
	, m_nWindowSizeY(-1)
	, m_nWindowOriginX(CW_USEDEFAULT)	//	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
	, m_nWindowOriginY(CW_USEDEFAULT)
{
}
