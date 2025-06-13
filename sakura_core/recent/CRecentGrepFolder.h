/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#define SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

using CGrepFolderString = StaticString<MAX_GREP_PATH>;

//! GREPフォルダーの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder final : public CRecentImp<CGrepFolderString, LPCWSTR>{
public:
	//生成
	CRecentGrepFolder();

	//オーバーライド
	int				CompareItem( const CGrepFolderString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CGrepFolderString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( LPCWSTR* dst, const CGrepFolderString* src ) const override;
	bool			TextToDataType( CGrepFolderString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const override;
};
#endif /* SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_ */
