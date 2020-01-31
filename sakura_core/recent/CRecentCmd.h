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

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h" //MAX_CMDLEN

typedef StaticString<WCHAR, MAX_CMDLEN> CCmdString;

//! コマンドの履歴を管理 (RECENT_FOR_CMD)
class CRecentCmd final : public CRecentImp<CCmdString, LPCWSTR>
{
  public:
    //生成
    CRecentCmd();

    //オーバーライド
    int CompareItem(const CCmdString *p1, LPCWSTR p2) const override;
    void CopyItem(CCmdString *dst, LPCWSTR src) const override;
    const WCHAR *GetItemText(int nIndex) const;
    bool DataToReceiveType(LPCWSTR *dst, const CCmdString *src) const override;
    bool TextToDataType(CCmdString *dst, LPCWSTR pszText) const override;
    bool ValidateReceiveType(LPCWSTR p) const override;
    size_t GetTextMaxLength() const;
};
