/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_
#define SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

using CGrepFileString = StaticString<MAX_GREP_PATH>;

//! GREPファイルの履歴を管理 (RECENT_FOR_GREP_FILE)
class CRecentGrepFile final : public CRecentImp<CGrepFileString, LPCWSTR>{
public:
	//生成
	CRecentGrepFile();

	//オーバーライド
	int				CompareItem( const CGrepFileString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CGrepFileString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CGrepFileString* src ) const override;
	bool			TextToDataType( CGrepFileString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTGREPFILE_E23BE08A_1B53_492D_85EE_4370AA956BB5_H_ */
