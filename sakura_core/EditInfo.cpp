/*! @file */
/*
	Copyright (C) 2018-2020 Sakura Editor Organization

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
    : m_szPath{0}
    , m_nCharCode(CODE_AUTODETECT)
    , m_bBom(false)
    , m_szDocType{0}
    , m_nTypeId(-1)
    , m_nViewTopLine(-1)
    , m_nViewLeftCol(-1)
    , m_ptCursor{-1, -1}
    , m_bIsModified(false)
    , m_bIsGrep(false)
    , m_szGrepKey{0}
    , m_bIsDebug(false)
    , m_szMarkLines{0}
    , m_nWindowSizeX(-1)
    , m_nWindowSizeY(-1)
    , m_nWindowOriginX(CW_USEDEFAULT) //	2004.05.13 Moca “指定無し”を-1からCW_USEDEFAULTに変更
    , m_nWindowOriginY(CW_USEDEFAULT)
{
}
