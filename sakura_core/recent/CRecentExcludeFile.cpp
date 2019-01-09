/*
	Copyright (C) 2018-2019 Sakura Editor Organization

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
#include "CRecentExcludeFile.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"

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

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* CRecentExcludeFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExcludeFile::DataToReceiveType( LPCTSTR* dst, const CExcludeFileString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExcludeFile::TextToDataType( CExcludeFileString* dst, LPCTSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentExcludeFile::CompareItem( const CExcludeFileString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentExcludeFile::CopyItem( CExcludeFileString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}

bool CRecentExcludeFile::ValidateReceiveType( LPCTSTR p ) const
{
	if( GetTextMaxLength() <= _tcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentExcludeFile::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
