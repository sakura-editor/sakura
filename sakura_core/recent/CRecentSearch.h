/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTSEARCH_52B7DE14_4B4A_49DD_9374_27ED71D22ED7_H_
#define SAKURA_CRECENTSEARCH_52B7DE14_4B4A_49DD_9374_27ED71D22ED7_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CSearchString = StaticString<_MAX_PATH>;

//! 検索の履歴を管理 (RECENT_FOR_SEARCH)
class CRecentSearch final : public CRecentImp<CSearchString, LPCWSTR>{
public:
	//生成
	CRecentSearch();

	//オーバーライド
	int				CompareItem( const CSearchString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CSearchString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CSearchString* src ) const override;
	bool			TextToDataType( CSearchString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTSEARCH_52B7DE14_4B4A_49DD_9374_27ED71D22ED7_H_ */
