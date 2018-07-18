/*
	Copyright (C) 2008, kobake

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

#include "StdAfx.h"
#include <string.h>
#include "CRecentGrepFolder.h"
#include "env/DLLSHAREDATA.h"


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

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* CRecentGrepFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentGrepFolder::DataToReceiveType( LPCTSTR* dst, const CGrepFolderString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFolder::TextToDataType( CGrepFolderString* dst, LPCTSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFolder::CompareItem( const CGrepFolderString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentGrepFolder::CopyItem( CGrepFolderString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}

bool CRecentGrepFolder::ValidateReceiveType( LPCTSTR p ) const
{
	if( GetTextMaxLength() <= _tcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentGrepFolder::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
