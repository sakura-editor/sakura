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
#include "recent/CRecentFile.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザ管理の構造体にキャストして参照してください。
*/
const TCHAR* CRecentFile::GetItemText( int nIndex ) const
{
	return GetItem(nIndex)->m_szPath;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFile::CRecentFile()
{
	Create(
		GetShareData()->m_sHistory.m_fiMRUArr,
		_countof(GetShareData()->m_sHistory.m_fiMRUArr[0].m_szPath),
		&GetShareData()->m_sHistory.m_nMRUArrNum,
		GetShareData()->m_sHistory.m_bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->m_Common.m_sGeneral.m_nMRUArrNum_MAX)
	);
}

bool CRecentFile::DataToReceiveType( const EditInfo** dst, const EditInfo* src ) const
{
	*dst = src;
	return true;
}

bool CRecentFile::TextToDataType( EditInfo* dst, LPCTSTR pszText ) const
{
	if( _countof(dst->m_szPath) < auto_strlen(pszText) + 1 ){
		return false;
	}
	_tcscpy(dst->m_szPath, pszText);
	return true;
}

int CRecentFile::CompareItem( const EditInfo* p1, const EditInfo* p2 ) const
{
	return _tcsicmp(p1->m_szPath, p2->m_szPath);
}

void CRecentFile::CopyItem( EditInfo* dst, const EditInfo* src ) const
{
	*dst = *src;
}

bool CRecentFile::ValidateReceiveType( const EditInfo* ) const
{
	return true;
}

size_t CRecentFile::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   固有インターフェース                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentFile::FindItemByPath(const TCHAR* pszPath) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if(_tcsicmp(GetItem(i)->m_szPath,pszPath)==0)return i;
	}
	return -1;
}

