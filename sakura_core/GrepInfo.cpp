/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
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
