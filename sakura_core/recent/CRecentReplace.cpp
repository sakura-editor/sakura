/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentReplace.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentReplace::CRecentReplace()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aReplaceKeys._GetSizeRef(),
		NULL,
		MAX_REPLACEKEY,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentReplace::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentReplace::DataToReceiveType( LPCWSTR* dst, const CReplaceString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentReplace::TextToDataType( CReplaceString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentReplace::CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentReplace::CopyItem( CReplaceString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentReplace::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentReplace::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
