/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_
#define SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CTagjumpKeywordString = StaticString<_MAX_PATH>;

//! タグジャンプキーワードの履歴を管理 (RECENT_FOR_TAGJUMP_KEYWORD)
class CRecentTagjumpKeyword final : public CRecentImp<CTagjumpKeywordString, LPCWSTR>{
public:
	//生成
	CRecentTagjumpKeyword();

	//オーバーライド
	int				CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CTagjumpKeywordString* src ) const override;
	bool			TextToDataType( CTagjumpKeywordString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTTAGJUMPKEYWORD_1416AC30_3714_4760_A313_76588D26A0A1_H_ */
