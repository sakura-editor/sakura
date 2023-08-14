#include "StdAfx.h"
#include "DetectIndentationStyle.h"

#include "doc/CEditDoc.h"
#include "config/app_constants.h"

void DetectIndentationStyle(CEditDoc* pcDoc, size_t nMaxLinesToCheck, IndentationStyle& style)
{
	const auto& cDocLineMgr = pcDoc->m_cDocLineMgr;
	int nSpaceUsed = 0;
	int nTabUsed = 0;
	style.character = IndentationStyle::Character::Unknown;
	for (size_t i=0; i<nMaxLinesToCheck; ++i) {
		const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt(i));
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
	if (nSpaceUsed > nTabUsed * 4) style.character = IndentationStyle::Character::Spaces;
	else if (nTabUsed > nSpaceUsed * 4) style.character = IndentationStyle::Character::Tabs;

	if (style.character == IndentationStyle::Character::Spaces) {
		// https://heathermoor.medium.com/detecting-code-indentation-eff3ed0fb56b
		std::array<int, TABSPACE_MAX> indents{}; // # spaces indent -> # times seen
		int last = 0; // # leading spaces in the last line we saw
		for (size_t i=0; i<nMaxLinesToCheck; ++i) {
			const CDocLine* pLine = cDocLineMgr.GetLine(CLogicInt(i));
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
			if (indent > 1) {
				++indents[indent];
			}
			last = width;
		}
		// find most frequent non-zero width difference
		int max = 0;
		int maxIdx = -1;
		for (int i=0; i<indents.size(); ++i) {
			auto freq = indents[i];
			if (freq > max) {
				max = freq;
				maxIdx = i;
			}
		}
		if (maxIdx != -1) {
			style.tabSpace = maxIdx;
		}
	}
}
