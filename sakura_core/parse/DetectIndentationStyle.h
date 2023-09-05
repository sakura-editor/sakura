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

