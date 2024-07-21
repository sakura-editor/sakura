#include "StdAfx.h"
#include "DetectIndentationStyle.h"

#include "doc/CEditDoc.h"
#include "config/app_constants.h"

void DetectIndentationStyle(const CEditDoc* pcDoc, size_t nMaxLinesToCheck, IndentationStyle& style)
{
	const auto& cDocLineMgr = pcDoc->m_cDocLineMgr;
	int nSpaceUsed = 0;
	int nTabUsed = 0;
	style.character = IndentationStyle::Character::Unknown;
	// 各行の行頭の文字が半角空白かタブ文字かをカウントする
	for (size_t i=0; i<nMaxLinesToCheck; ++i) {
		const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt((int)i));
		if (pLine == nullptr) {
			break;
		}
		auto len = pLine->GetLengthWithoutEOL();
		// check leading letters
		if (len == 0) {
			continue;
		}
		const wchar_t* str = pLine->GetPtr();
		wchar_t c = str[0];
		if (c == '\t') ++nTabUsed;
		else if (c == ' ') ++nSpaceUsed;
	}
	// 4倍以上行数に差がある場合は明確な差があると判断して、インデントに使われている文字種別を決定する
	if (nSpaceUsed > nTabUsed * 4) style.character = IndentationStyle::Character::Spaces;
	else if (nTabUsed > nSpaceUsed * 4) style.character = IndentationStyle::Character::Tabs;

	// 半角空白でインデントが行われていると判断した場合、前の行とのインデント差の頻度を調べて最頻値のインデント差をタブ幅とする
	if (style.character == IndentationStyle::Character::Spaces) {
		// https://heathermoor.medium.com/detecting-code-indentation-eff3ed0fb56b
		std::array<int, TABSPACE_MAX+1> indents{}; // # spaces indent -> # times seen
		int last = 0; // # leading spaces in the last line we saw
		for (size_t i=0; i<nMaxLinesToCheck; ++i) {
			const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt((int)i));
			if (pLine == nullptr) {
				break;
			}
			auto len = pLine->GetLengthWithoutEOL();
			// check leading letters
			if (len == 0) {
				continue;
			}
			const wchar_t* str = pLine->GetPtr();
			int width = 0;
			while (*str++ == ' ')
				++width;
			int indent = abs(width - last);
			if (indent > 1 && indent < indents.size()) {
				++indents[indent];
			}
			last = width;
		}
		// find most frequent non-zero width difference
		int max = 0;
		int maxIdx = -1;
		for (size_t i=1; i<indents.size(); ++i) {
			auto freq = indents[i];
			if (freq > max) {
				max = freq;
				maxIdx = (int)i;
			}
		}
		if (maxIdx != -1) {
			style.tabSpace = maxIdx;
		}
	}
}
