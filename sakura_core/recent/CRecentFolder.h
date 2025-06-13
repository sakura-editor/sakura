/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_
#define SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CPathString = StaticString<_MAX_PATH>;

//! フォルダーの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentFolder final : public CRecentImp<CPathString, LPCWSTR>{
public:
	//生成
	CRecentFolder();

	//オーバーライド
	int				CompareItem( const CPathString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CPathString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CPathString* src ) const override;
	bool			TextToDataType( CPathString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTFOLDER_E26A46E2_C8DF_4228_A0D6_24A2712392E9_H_ */
