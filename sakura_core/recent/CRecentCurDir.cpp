/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "CRecentCurDir.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCurDir::CRecentCurDir()
{
	Create(
		GetShareData()->m_sHistory.m_aCurDirs.dataPtr(),
		GetShareData()->m_sHistory.m_aCurDirs.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sHistory.m_aCurDirs._GetSizeRef(),
		NULL,
		MAX_CMDARR,
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
const WCHAR* CRecentCurDir::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentCurDir::DataToReceiveType( LPCWSTR* dst, const CCurDirString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentCurDir::TextToDataType( CCurDirString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentCurDir::CompareItem( const CCurDirString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentCurDir::CopyItem( CCurDirString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentCurDir::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentCurDir::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
