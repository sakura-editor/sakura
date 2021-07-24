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
#pragma once

#include "view/CGetSelectedData.h"

#include <tuple>

#include "view/CEditView.h"

/*!
 *  通常選択範囲のデータを取得するクラス
 */
class CGetLinearSelectedData : public CGetSelectedData
{
	const CEditView* m_pcEditView;
	CLayoutPoint ptSelectFrom;
	CLayoutPoint ptSelectTo;
	std::wstring_view quoteMark = L"";
	size_t nLineNumCols = 0;
	bool bInsertEolAtWrap = false;
	EEolType newEolType = EEolType::none;

public:
	explicit CGetLinearSelectedData(
		const CEditView* m_pcEditView,
		const CViewSelect& cSelection,
		std::wstring_view quoteMark,
		size_t nLineNumCols,
		bool bInsertEolAtWrap,
		EEolType newEolType
	) noexcept;

protected:
	[[nodiscard]] size_t _CountData() const override;
	[[nodiscard]] bool _GetData( CNativeW& cmemBuf ) const  override;

private:
	[[nodiscard]] std::tuple<CLogicXInt, CLogicXInt> LineColumnsToIndexes(const CLayoutInt nLineNum, const CLayout* pcLayout) const {
		// 行内の桁位置を行頭からのオフセットに変換
		const auto nIdxFrom = nLineNum == ptSelectFrom.y
			? m_pcEditView->LineColumnToIndex(pcLayout, ptSelectFrom.x)
			: CLogicInt(0);
		const auto nIdxTo = nLineNum == ptSelectTo.y
			? m_pcEditView->LineColumnToIndex(pcLayout, ptSelectTo.x)
			: pcLayout->GetLengthWithEOL();
		return std::make_tuple(nIdxFrom, nIdxTo);
	}
};
