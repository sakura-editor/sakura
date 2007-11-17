#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentReplace.h"
#include <string.h>
#include "my_icmp.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentReplace::CRecentReplace()
{
	Create(
		&GetShareData()->m_aReplaceKeys[0],
		&GetShareData()->m_aReplaceKeys._GetSizeRef(),
		NULL /*GetShareData()->m_bREPLACEKEYArrFavorite*/,
		MAX_REPLACEKEY,
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
const TCHAR* CRecentReplace::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

int CRecentReplace::CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentReplace::CopyItem( CReplaceString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
