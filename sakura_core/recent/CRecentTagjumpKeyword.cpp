/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CRecentTagjumpKeyword.h"
#include "config/maxdata.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentTagjumpKeyword::CRecentTagjumpKeyword()
{
	Create(
		GetShareData()->m_sTagJump.m_aTagJumpKeywords.dataPtr(),
		GetShareData()->m_sTagJump.m_aTagJumpKeywords.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sTagJump.m_aTagJumpKeywords._GetSizeRef(),
		NULL,
		MAX_TAGJUMP_KEYWORD,
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
const WCHAR* CRecentTagjumpKeyword::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentTagjumpKeyword::DataToReceiveType( LPCWSTR* dst, const CTagjumpKeywordString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentTagjumpKeyword::TextToDataType( CTagjumpKeywordString* dst, LPCWSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentTagjumpKeyword::CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentTagjumpKeyword::CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentTagjumpKeyword::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentTagjumpKeyword::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
