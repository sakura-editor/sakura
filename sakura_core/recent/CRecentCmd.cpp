﻿/*! @file */
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

#include "StdAfx.h"
#include "CRecentCmd.h"
#include "config/maxdata.h"
#include "env/DLLSHAREDATA.h"
#include <string.h>

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCmd::CRecentCmd()
{
	Create(GetShareData()->m_sHistory.m_aCommands.dataPtr(),
		   GetShareData()->m_sHistory.m_aCommands.dataPtr()->GetBufferCount(),
		   &GetShareData()->m_sHistory.m_aCommands._GetSizeRef(), NULL, MAX_CMDARR, NULL);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const WCHAR *CRecentCmd::GetItemText(int nIndex) const { return *GetItem(nIndex); }

bool CRecentCmd::DataToReceiveType(LPCWSTR *dst, const CCmdString *src) const
{
	*dst = *src;
	return true;
}

bool CRecentCmd::TextToDataType(CCmdString *dst, LPCWSTR pszText) const
{
	if (false == ValidateReceiveType(pszText)) { return false; }
	CopyItem(dst, pszText);
	return true;
}

int CRecentCmd::CompareItem(const CCmdString *p1, LPCWSTR p2) const { return wcscmp(*p1, p2); }

void CRecentCmd::CopyItem(CCmdString *dst, LPCWSTR src) const { wcscpy(*dst, src); }

bool CRecentCmd::ValidateReceiveType(LPCWSTR p) const
{
	if (GetTextMaxLength() <= wcslen(p)) { return false; }
	return true;
}

size_t CRecentCmd::GetTextMaxLength() const { return m_nTextMaxLength; }
