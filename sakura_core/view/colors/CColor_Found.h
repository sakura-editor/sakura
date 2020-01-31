/*! @file */
/*
	Copyright (C) 2008, kobake

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

#include "view/colors/CColorStrategy.h"

class CColor_Select final : public CColorStrategy
{
  public:
    EColorIndexType GetStrategyColor() const override
    {
        return COLORIDX_SELECT;
    }
    //色替え
    void InitStrategyStatus() override
    {
    }
    bool BeginColor(const CStringRef &cStr, int nPos) override;
    bool Disp() const override
    {
        return true;
    }
    bool EndColor(const CStringRef &cStr, int nPos) override;

    virtual bool BeginColorEx(const CStringRef &cStr, int nPos, CLayoutInt, const CLayout *);

    //イベント
    void OnStartScanLogic() override;

  private:
    CLayoutInt m_nSelectLine;
    CLogicInt m_nSelectStart;
    CLogicInt m_nSelectEnd;
};

class CColor_Found final : public CColorStrategy
{
  public:
    CColor_Found();
    EColorIndexType GetStrategyColor() const override
    {
        return this->validColorNum != 0 ? this->highlightColors[(m_nSearchResult - 1) % this->validColorNum] : COLORIDX_DEFAULT;
    }
    //色替え
    void InitStrategyStatus() override
    {
    } //############要検証
    bool BeginColor(const CStringRef &cStr, int nPos) override;
    bool Disp() const override
    {
        return true;
    }
    bool EndColor(const CStringRef &cStr, int nPos) override;
    //イベント
    void OnStartScanLogic() override;

  private:
    int m_nSearchResult;
    CLogicInt m_nSearchStart;
    CLogicInt m_nSearchEnd;
    EColorIndexType highlightColors[COLORIDX_SEARCHTAIL - COLORIDX_SEARCH + 1]; ///< チェックが付いている検索文字列色の配列。
    unsigned validColorNum; ///< highlightColorsの何番目の要素までが有効か。
};
