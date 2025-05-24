/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTCMD_4EB34D07_2F92_4BE4_9AB1_767141022C54_H_
#define SAKURA_CRECENTCMD_4EB34D07_2F92_4BE4_9AB1_767141022C54_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h" //MAX_CMDLEN

using CCmdString = StaticString<MAX_CMDLEN>;

//! コマンドの履歴を管理 (RECENT_FOR_CMD)
class CRecentCmd final : public CRecentImp<CCmdString, LPCWSTR>{
public:
	//生成
	CRecentCmd();

	//オーバーライド
	int				CompareItem( const CCmdString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CCmdString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CCmdString* src ) const override;
	bool			TextToDataType( CCmdString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTCMD_4EB34D07_2F92_4BE4_9AB1_767141022C54_H_ */
