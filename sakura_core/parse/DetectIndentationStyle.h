/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

class CDocLineMgr;
class CFilePath;

struct IndentationStyle
{
	enum class Character {
		Spaces,
		Tabs,
		Unknown,
	} character;

	int tabSpace = -1;
};

void DetectIndentationStyle(const CFilePath& cFilePath, const CDocLineMgr& cDocLineMgr, size_t nMaxLinesToCheck, IndentationStyle& style);

