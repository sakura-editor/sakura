/*! @file

	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#define SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

using CExcludeFileString = StaticString<MAX_EXCLUDE_PATH>;

//! Excludeファイルの履歴を管理 (RECENT_FOR_Exclude_FILE)
class CRecentExcludeFile final : public CRecentImp<CExcludeFileString, LPCWSTR>{
public:
	//生成
	CRecentExcludeFile();

	//オーバーライド
	int				CompareItem( const CExcludeFileString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CExcludeFileString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CExcludeFileString* src ) const override;
	bool			TextToDataType( CExcludeFileString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTEXCLUDEFILE_74BD9C61_4E41_4D1D_A8CE_8C78B4DDDEBA_H_ */
