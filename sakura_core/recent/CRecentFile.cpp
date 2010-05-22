#include "stdafx.h"
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
		&GetShareData()->m_sHistory.m_nMRUArrNum,
		GetShareData()->m_sHistory.m_bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->m_Common.m_sGeneral.m_nMRUArrNum_MAX)
	);
}



int CRecentFile::CompareItem( const EditInfo* p1, const EditInfo* p2 ) const
{
	return _tcsicmp(p1->m_szPath, p2->m_szPath);
}

void CRecentFile::CopyItem( EditInfo* dst, const EditInfo* src ) const
{
	memcpy_raw(dst,src,sizeof(*dst));
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

