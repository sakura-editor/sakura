/*! @file

	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentExcludeFile.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExcludeFile::CRecentExcludeFile()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aExcludeFiles.dataPtr(),
		GetShareData()->m_sSearchKeywords.m_aExcludeFiles.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aExcludeFiles._GetSizeRef(),
		NULL,
		MAX_EXCLUDEFILE,
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
const WCHAR* CRecentExcludeFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExcludeFile::DataToReceiveType( LPCWSTR* dst, const CExcludeFileString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExcludeFile::TextToDataType( CExcludeFileString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentExcludeFile::CompareItem( const CExcludeFileString* p1, LPCWSTR p2 ) const
{
	return _wcsicmp(*p1,p2);
}

void CRecentExcludeFile::CopyItem( CExcludeFileString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentExcludeFile::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentExcludeFile::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
