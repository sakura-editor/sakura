﻿/*! @file

	Copyright (C) 2018-2022, Sakura Editor Organization

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
#ifndef SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#define SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

typedef StaticString<WCHAR, MAX_EXCLUDE_PATH> CExcludeFileString;

//! Excludeファイルの履歴を管理 (RECENT_FOR_Exclude_FILE)
class CRecentExcludeFile final : public CRecentImp<CExcludeFileString, LPCWSTR>{
public:
	//生成
	CRecentExcludeFile();

	//オーバーライド
	int				CompareItem( const CExcludeFileString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CExcludeFileString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCWSTR* dst, const CExcludeFileString* src ) const override;
	bool			TextToDataType( CExcludeFileString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const;
};
#endif /* SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_ */
