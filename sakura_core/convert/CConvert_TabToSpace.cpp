/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
	const std::ptrdiff_t numOfTabs = std::count(source.begin(), source.end(), L'\t');
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
