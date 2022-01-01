﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2021, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#define SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_
#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h"

typedef StaticString<WCHAR, MAX_GREP_PATH> CGrepFolderString;

//! GREPフォルダの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder final : public CRecentImp<CGrepFolderString, LPCWSTR>{
public:
	//生成
	CRecentGrepFolder();

	//オーバーライド
	int				CompareItem( const CGrepFolderString* p1, LPCWSTR p2 ) const override;
	void			CopyItem( CGrepFolderString* dst, LPCWSTR src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCWSTR* dst, const CGrepFolderString* src ) const override;
	bool			TextToDataType( CGrepFolderString* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( LPCWSTR p ) const override;
	size_t			GetTextMaxLength() const;
};
#endif /* SAKURA_CRECENTGREPFOLDER_A0D1E75B_4587_4587_9A33_A5EA13349BAB_H_ */
