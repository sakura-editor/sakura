/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_
#define SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CMetaPath = StaticString<_MAX_PATH>;

//! フォルダーの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentExceptMRU final : public CRecentImp<CMetaPath, LPCWSTR>{
public:
	//生成
	CRecentExceptMRU();

	//オーバーライド
	int				CompareItem( const CMetaPath* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CMetaPath* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CMetaPath* src ) const override;
	bool			TextToDataType( CMetaPath* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTEXCEPTMRU_4DF7E5C5_2EC1_4A19_B31C_74EF43DC08AE_H_ */
