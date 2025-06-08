/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_
#define SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_
#pragma once

#include "CRecentImp.h"
struct EditNode;

//! EditNode(ウィンドウリスト)の履歴を管理 (RECENT_FOR_EDITNODE)
class CRecentEditNode final : public CRecentImp<EditNode>{
public:
	//生成
	CRecentEditNode();

	//オーバーライド
	int				CompareItem( const EditNode* p1, const EditNode* p2 ) const override;
	void			CopyItem( EditNode* dst, const EditNode* src ) const override;
	const WCHAR*	GetItemText( int nIndex ) const override;
	bool			DataToReceiveType( const EditNode** dst, const EditNode* src ) const override;
	bool			TextToDataType( EditNode* dst, LPCWSTR pszText ) const override;
	bool			ValidateReceiveType( const EditNode* ) const override;
	size_t			GetTextMaxLength() const override;
	//固有インターフェース
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};
#endif /* SAKURA_CRECENTEDITNODE_51FF7E34_DFF5_45BA_AB77_7845F21F7A85_H_ */
