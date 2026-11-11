/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTREPLACE_F287196B_19A2_4CEA_9225_9E9C681774F7_H_
#define SAKURA_CRECENTREPLACE_F287196B_19A2_4CEA_9225_9E9C681774F7_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CReplaceString = StaticString<_MAX_PATH>;

//! 置換の履歴を管理 (RECENT_FOR_REPLACE)
class CRecentReplace final : public CRecentImp<CReplaceString, LPCWSTR>{
public:
	//生成
	CRecentReplace();

	//オーバーライド
	int				CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CReplaceString* src ) const override;
};

#endif /* SAKURA_CRECENTREPLACE_F287196B_19A2_4CEA_9225_9E9C681774F7_H_ */
