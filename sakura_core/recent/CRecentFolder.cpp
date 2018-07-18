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
#include "CRecentFolder.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFolder::CRecentFolder()
{
	Create(
		&GetShareData()->m_sHistory.m_szOPENFOLDERArr[0],
		GetShareData()->m_sHistory.m_szOPENFOLDERArr[0].GetBufferCount(),
		&GetShareData()->m_sHistory.m_nOPENFOLDERArrNum,
		GetShareData()->m_sHistory.m_bOPENFOLDERArrFavorite,
		MAX_OPENFOLDER,
		&(GetShareData()->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX)
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      オーバーライド                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* CRecentFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentFolder::DataToReceiveType( LPCTSTR* dst, const CPathString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentFolder::TextToDataType( CPathString* dst, LPCTSTR pszText ) const
{
	if( false == ValidateReceiveType(pszText) ){
		return false;
	}
	CopyItem(dst, pszText);
	return true;
}

int CRecentFolder::CompareItem( const CPathString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentFolder::CopyItem( CPathString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}

bool CRecentFolder::ValidateReceiveType( LPCTSTR p ) const
{
	if( GetTextMaxLength() <= _tcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentFolder::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
