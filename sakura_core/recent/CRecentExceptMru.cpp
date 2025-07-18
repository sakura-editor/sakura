﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExceptMru.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExceptMRU::CRecentExceptMRU()
{
	Create(
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr(),
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sHistory.m_aExceptMRU._GetSizeRef(),
		nullptr,
		MAX_MRU,
		nullptr
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentExceptMRU::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExceptMRU::DataToReceiveType( LPCWSTR* dst, const CMetaPath* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExceptMRU::TextToDataType( CMetaPath* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentExceptMRU::CompareItem( const CMetaPath* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentExceptMRU::CopyItem( CMetaPath* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentExceptMRU::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentExceptMRU::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
