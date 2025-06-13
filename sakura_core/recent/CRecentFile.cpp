/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "recent/CRecentFile.h"
#include "config/maxdata.h"

/*
	アイテムの比較要素を取得する。

	@note	取得後のポインタはユーザー管理の構造体にキャストして参照してください。
*/
const WCHAR* CRecentFile::GetItemText( int nIndex ) const
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

bool CRecentFile::TextToDataType( EditInfo* dst, LPCWSTR pszText ) const
{
	if( _countof(dst->m_szPath) < wcslen(pszText) + 1 ){
		return false;
	}
	wcscpy(dst->m_szPath, pszText);
	return true;
}

int CRecentFile::CompareItem( const EditInfo* p1, const EditInfo* p2 ) const
{
	return _wcsicmp(p1->m_szPath, p2->m_szPath);
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

int CRecentFile::FindItemByPath(const WCHAR* pszPath) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if(_wcsicmp(GetItem(i)->m_szPath,pszPath)==0)return i;
	}
	return -1;
}
