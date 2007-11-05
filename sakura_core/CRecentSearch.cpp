#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentSearch.h"
#include <string.h>
#include "my_icmp.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentSearch::CRecentSearch()
{
	Create(
		&GetShareData()->m_aSearchKeys[0],
		&GetShareData()->m_aSearchKeys._GetSizeRef(),
		NULL /*GetShareData()->m_bSEARCHKEYArrFavorite*/,
		MAX_SEARCHKEY,
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
const TCHAR* CRecentSearch::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

int CRecentSearch::CompareItem( const CSearchString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentSearch::CopyItem( CSearchString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
