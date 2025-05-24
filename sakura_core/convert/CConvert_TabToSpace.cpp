/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CConvert_TabToSpace.h"

#include <algorithm>
#include <string_view>
#include "charset/charcode.h"
#include "CEol.h"
#include "util/string_ex2.h"
#include "mem/CNativeW.h"

//! TAB→空白
bool CConvert_TabToSpace::DoConvert(CNativeW* pcData)
{
	const std::wstring_view source(pcData->GetStringPtr(), pcData->GetStringLength());
	const auto numOfTabs = std::count(source.cbegin(), source.cend(), L'\t');
	std::wstring buffer;
	buffer.reserve(source.length() + numOfTabs * (m_nTabWidth - 1));

	int begin = 0;
	while (true) {
		int lineLength;
		CEol eol;
		/* CRLFで区切られる「行」を返す。CRLFは行長に加えない */
		const wchar_t* line = GetNextLineW(source.data(),
			static_cast<int>(source.length()), &lineLength, &begin, &eol, m_bExtEol);
		if (!line)
			break;
		// 先頭行については開始桁位置を考慮する（さらに折り返し関連の対策が必要？）
		int pos = (source.data() == line) ? m_nStartColumn : 0;
		for (int i = 0; i < lineLength; ++i) {
			if (line[i] == L'\t') {
				const int width = m_nTabWidth - (pos % m_nTabWidth);
				buffer.append(width, L' ');
				pos += width;
			} else {
				buffer.push_back(line[i]);
				pos += WCODE::IsZenkaku(line[i]) ? 2 : 1;  //全角文字ずれ対応 2008.10.15 matsumo
			}
		}
		buffer.append(eol.GetValue2(), eol.GetLen());
	}
	if (buffer.empty())
		return false;
	pcData->SetString(buffer.c_str(), buffer.length());
	return true;
}
