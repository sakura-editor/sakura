/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentGrepFolder.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFolder::CRecentGrepFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFolders._GetSizeRef(),
		NULL,
		MAX_GREPFOLDER,
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
const WCHAR* CRecentGrepFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentGrepFolder::DataToReceiveType( LPCWSTR* dst, const CGrepFolderString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFolder::TextToDataType( CGrepFolderString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFolder::CompareItem( const CGrepFolderString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentGrepFolder::CopyItem( CGrepFolderString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentGrepFolder::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentGrepFolder::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
