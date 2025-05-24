/*! @file

	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExcludeFolder.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExcludeFolder::CRecentExcludeFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aExcludeFolders.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aExcludeFolders.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aExcludeFolders._GetSizeRef(),
		NULL,
		MAX_EXCLUDEFOLDER,
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
const WCHAR* CRecentExcludeFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExcludeFolder::DataToReceiveType( LPCWSTR* dst, const CExcludeFolderString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExcludeFolder::TextToDataType( CExcludeFolderString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentExcludeFolder::CompareItem( const CExcludeFolderString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentExcludeFolder::CopyItem( CExcludeFolderString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentExcludeFolder::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentExcludeFolder::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
