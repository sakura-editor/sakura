﻿/*!	@file
	@brief 最近使ったリスト

	お気に入りを含む最近使ったリストを管理する。

	@author MIK
	@date Apr. 05, 2003
	@date Apr. 03, 2005

	@date Oct. 19, 2007 kobake 型チェックが働くように、再設計
*/
/*
	Copyright (C) 2003, MIK
	Copyright (C) 2005, MIK
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
#ifndef SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_
#define SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_
#pragma once

#include "env/DLLSHAREDATA.h"

class CRecent{
public:
	virtual ~CRecent(){}

	//インスタンス管理
	virtual void	Terminate() = 0;

	//アイテム
	virtual const WCHAR*	GetItemText( int nIndex ) const = 0;
	virtual int				GetArrayCount() const = 0;
	virtual int				GetItemCount() const = 0;
	virtual void			DeleteAllItem() = 0;
	virtual bool			DeleteItemsNoFavorite() = 0;
	virtual bool			DeleteItem( int nIndex ) = 0;	//!< アイテムをクリア
	virtual bool			AppendItemText(const WCHAR* pszText) = 0;
	virtual bool			EditItemText( int nIndex, const WCHAR* pszText) = 0;
	virtual size_t			GetTextMaxLength() const = 0;

	int FindItemByText(const WCHAR* pszText) const
	{
		int n = GetItemCount();
		for(int i=0;i<n;i++){
			if(wcscmp(GetItemText(i),pszText)==0)return i;
		}
		return -1;
	}

	//お気に入り
	virtual bool	SetFavorite( int nIndex, bool bFavorite = true ) = 0;	//!< お気に入りに設定
	virtual bool	IsFavorite(int nIndex) const = 0;						//!< お気に入りか調べる

	//その他
	virtual int		GetViewCount() const = 0;
	virtual bool	UpdateView() = 0;

	// 共有メモリアクセス
	DLLSHAREDATA*	GetShareData()
	{
		return &GetDllShareData();
	}
};

#endif /* SAKURA_CRECENT_F4D70310_9FAF_4F07_9431_2B011A47142D_H_ */
