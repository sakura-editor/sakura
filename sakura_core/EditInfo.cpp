/*! @file */
/*
	Copyright (C) 2018-2019 Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool EditInfo::operator == (const EditInfo& rhs) const noexcept {
	if (this == &rhs) return true;
	return 0 == wcsncmp(m_szPath, rhs.m_szPath, _countof(m_szPath))
		&& m_nCharCode == rhs.m_nCharCode
		&& m_bBom == rhs.m_bBom
		&& 0 == wcsncmp(m_szDocType, rhs.m_szDocType, _countof(m_szDocType))
		&& m_nTypeId == rhs.m_nTypeId
		&& m_nViewTopLine == rhs.m_nViewTopLine
		&& m_nViewLeftCol == rhs.m_nViewLeftCol
		&& m_ptCursor == rhs.m_ptCursor
		&& m_bIsModified == rhs.m_bIsModified
		&& m_bIsGrep == rhs.m_bIsGrep
		&& 0 == wcsncmp(m_szGrepKey, rhs.m_szGrepKey, _countof(m_szGrepKey))
		&& m_bIsDebug == rhs.m_bIsDebug
		&& 0 == wcsncmp(m_szMarkLines, rhs.m_szMarkLines, _countof(m_szMarkLines))
		&& m_nWindowSizeX == rhs.m_nWindowSizeX
		&& m_nWindowSizeY == rhs.m_nWindowSizeY
		&& m_nWindowOriginX == rhs.m_nWindowOriginX
		&& m_nWindowOriginY == rhs.m_nWindowOriginY;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool EditInfo::operator != (const EditInfo& rhs) const noexcept
{
	return !(*this == rhs);
}
