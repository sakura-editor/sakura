/*! @file */
/*
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "basis/GrepInfo.h"

/*!
 * コンストラクタ
 */
GrepInfo::GrepInfo() noexcept
{
}

/*!	正規化した GrepInfo を返す

	@return 出力・置換の挙動が矛盾しないよう補正した複製
	@note Grep置換では「一致しなかった行を出力」が成立しないため、行単位出力に落とす。
*/
GrepInfo GrepInfo::Normalized() const
{
	GrepInfo gi = *this;
	// Grep否定行はGrep置換では無効
	if( gi.bGrepReplace && gi.nGrepOutputLineType == 2 ){
		gi.nGrepOutputLineType = 1; // 行単位
	}
	return gi;
}
