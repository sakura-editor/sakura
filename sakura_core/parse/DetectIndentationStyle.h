/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#pragma once

struct IndentationStyle
{
	enum class Character {
		Spaces,
		Tabs,
		Unknown,
	} character;

	int tabSpace = -1;
};

void DetectIndentationStyle(const CEditDoc* pcDoc, size_t nMaxLinesToCheck, IndentationStyle& style);

