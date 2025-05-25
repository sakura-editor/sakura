/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_
#define SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

using CCurDirString = StaticString<_MAX_PATH>;

//! コマンドの履歴を管理 (RECENT_FOR_CUR_DIR)
class CRecentCurDir final : public CRecentImp<CCurDirString, LPCWSTR>{
public:
	//生成
	CRecentCurDir();

	//オーバーライド
	int				CompareItem( const CCurDirString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CCurDirString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CCurDirString* src ) const override;
	bool			TextToDataType( CCurDirString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTCURDIR_A5846FA5_5608_4E6A_9A57_65DE2133E40A_H_ */
