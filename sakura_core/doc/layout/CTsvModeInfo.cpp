/*
	Copyright (C) 2015, syat

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
#include "CTsvModeInfo.h"
#include "doc/logic/CDocLine.h"
#include "doc/logic/CDocLineMgr.h"

// タブ位置を再計算する
void CTsvModeInfo::CalcTabLength(CDocLineMgr* cDocLineMgr)
{
	int i;
	unsigned int ui;

	m_tabLength.clear();
	if (m_nTsvMode == TSV_MODE_NONE) return;

	CLogicInt nLine;
	CLogicInt nLineNum = cDocLineMgr->GetLineCount();
	wchar_t delimiter = m_nTsvMode == TSV_MODE_TSV ? L'\t' : L',';
	int nFieldWidth = 0;

	for (nLine = CLogicInt(0); nLine < nLineNum; nLine++) {
		CDocLine* cDocLine = cDocLineMgr->GetLine(nLine);

		int nLineLen;
		int nCharChars;
		int nField = 0;
		int nFieldWidth = 0;
		LPCWSTR pcLine = cDocLine->GetDocLineStrWithEOL(&nLineLen);
		for (i = 0; i < nLineLen; ) {
			if (WCODE::IsLineDelimiter(pcLine[i], true)) break;
			if (pcLine[i] == delimiter) {
				if (nField == m_tabLength.size()) {
					m_tabLength.push_back(nFieldWidth);
				} else if (m_tabLength[nField] < nFieldWidth) {
					m_tabLength[nField] = nFieldWidth;
				}
				nField++;
				nFieldWidth = 0;
				i++;
				continue;
			}
			if( pcLine[i] != WCODE::TAB ){
				CKetaXInt nKeta = CNativeW::GetKetaOfChar(pcLine, nLineLen, i);
				nFieldWidth += Int(nKeta);
			} else {
				nFieldWidth++;
			}
			nCharChars = CNativeW::GetSizeOfChar( pcLine, nLineLen, i );
			i += nCharChars;
		}
		if (nField == m_tabLength.size()) {
			m_tabLength.push_back(nFieldWidth);
		} else if (m_tabLength[nField] < nFieldWidth) {
			m_tabLength[nField] = nFieldWidth;
			nField++;
		}
	}

	for (ui = 0; ui<m_tabLength.size(); ui++) {
		if (ui == 0) {
			m_tabLength[0] += 2;
		} else {
			m_tabLength[ui] += m_tabLength[ui-1] + 2;
		}
	}
}

// 指定したレイアウト位置のタブ幅を取得（折り返しは考慮しない）
CLayoutInt CTsvModeInfo::GetActualTabLength(CLayoutInt pos, CLayoutInt px) const
{
	unsigned int i;
	for (i = 0; i < m_tabLength.size(); i++) {
		if (pos < m_tabLength[i] * px) {
			return CLayoutInt(m_tabLength[i] * px) - pos;
		}
	}
	return CLayoutInt(px*2);
}
