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
#include "GrepInfo.h"

/*!
 * コンストラクタ
 */
GrepInfo::GrepInfo() noexcept
	: cmGrepKey()
	, cmGrepRep()
	, cmGrepFile()
	, cmGrepFolder()
	, cmExcludeFile()
	, cmExcludeFolder()
	, sGrepSearchOption()
	, bGrepCurFolder(false)
	, bGrepStdout(false)
	, bGrepHeader(true)
	, bGrepSubFolder(false)
	, nGrepCharSet(CODE_SJIS)
	, nGrepOutputStyle(1)
	, nGrepOutputLineType(0)
	, bGrepOutputFileOnly(false)
	, bGrepOutputBaseFolder(false)
	, bGrepSeparateFolder(false)
	, bGrepReplace(false)
	, bGrepPaste(false)
	, bGrepBackup(false)
{
}

/*!
 * 同型との等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しい
 * @retval false 等しくない
 */
bool GrepInfo::operator == (const GrepInfo& rhs) const noexcept {
	if (this == &rhs) return true;
	return cmGrepKey == rhs.cmGrepKey
		&& cmGrepRep == rhs.cmGrepRep
		&& cmGrepFile == rhs.cmGrepFile
		&& cmGrepFolder == rhs.cmGrepFolder
		&& cmExcludeFile == rhs.cmExcludeFile
		&& cmExcludeFolder == rhs.cmExcludeFolder
		&& sGrepSearchOption == rhs.sGrepSearchOption
		&& bGrepCurFolder == rhs.bGrepCurFolder
		&& bGrepStdout == rhs.bGrepStdout
		&& bGrepHeader == rhs.bGrepHeader
		&& bGrepSubFolder == rhs.bGrepSubFolder
		&& nGrepCharSet == rhs.nGrepCharSet
		&& nGrepOutputStyle == rhs.nGrepOutputStyle
		&& nGrepOutputLineType == rhs.nGrepOutputLineType
		&& bGrepOutputFileOnly == rhs.bGrepOutputFileOnly
		&& bGrepOutputBaseFolder == rhs.bGrepOutputBaseFolder
		&& bGrepSeparateFolder == rhs.bGrepSeparateFolder
		&& bGrepReplace == rhs.bGrepReplace
		&& bGrepPaste == rhs.bGrepPaste
		&& bGrepBackup == rhs.bGrepBackup;
}

/*!
 * 同型との否定の等価比較
 *
 * @param rhs 比較対象
 * @retval true 等しくない
 * @retval false 等しい
 */
bool GrepInfo::operator != (const GrepInfo& rhs) const noexcept
{
	return !(*this == rhs);
}
