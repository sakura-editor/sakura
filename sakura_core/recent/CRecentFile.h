/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_
#define SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_
#pragma once

#include "CRecentImp.h"
#include "EditInfo.h" //EditInfo

//! EditInfoの履歴を管理 (RECENT_FOR_FILE)
class CRecentFile final : public CRecentImp<EditInfo>{
public:
	//生成
	CRecentFile();

	//オーバーライド
	int				CompareItem( const EditInfo* p1, const EditInfo* p2 ) const override;
	void			CopyItem( EditInfo* dst, const EditInfo* src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( const EditInfo** dst, const EditInfo* src ) const override;
	bool			TextToDataType( EditInfo* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( const EditInfo* ) const override;
	size_t			GetTextMaxLength() const override;
	//固有インターフェース
	int FindItemByPath(const WCHAR* pszPath) const;
};
#endif /* SAKURA_CRECENTFILE_11698DF0_9914_4163_8A68_8E611163D2E9_H_ */
