#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentTagjumpKeyword.h"
#include <string.h>
#include "my_icmp.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentTagjumpKeyword::CRecentTagjumpKeyword()
{
	Create(
		&GetShareData()->m_aTagJumpKeywords[0],
		&GetShareData()->m_aTagJumpKeywords._GetSizeRef(),
		NULL /*GetShareData()->m_bTagJumpKeywordArrFavorite*/,
		MAX_TAGJUMP_KEYWORD,
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
const TCHAR* CRecentTagjumpKeyword::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

int CRecentTagjumpKeyword::CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentTagjumpKeyword::CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
