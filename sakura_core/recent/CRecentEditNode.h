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
struct EditNode;

//! EditNode(ウィンドウリスト)の履歴を管理 (RECENT_FOR_EDITNODE)
class CRecentEditNode final : public CRecentImp<EditNode>
{
  public:
    //生成
    CRecentEditNode();

    //オーバーライド
    int CompareItem(const EditNode *p1, const EditNode *p2) const override;
    void CopyItem(EditNode *dst, const EditNode *src) const override;
    const WCHAR *GetItemText(int nIndex) const;
    bool DataToReceiveType(const EditNode **dst, const EditNode *src) const override;
    bool TextToDataType(EditNode *dst, LPCWSTR pszText) const override;
    bool ValidateReceiveType(const EditNode *) const override;
    size_t GetTextMaxLength() const;
    //固有インターフェース
    int FindItemByHwnd(HWND hwnd) const;
    void DeleteItemByHwnd(HWND hwnd);
};
