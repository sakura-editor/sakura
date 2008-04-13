#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentCmd.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCmd::CRecentCmd()
{
	Create(
		&GetShareData()->m_aCommands[0],
		&GetShareData()->m_aCommands._GetSizeRef(),
		NULL /*GetShareData()->m_bCmdArrFavorite*/,
		MAX_CMDARR,
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
const TCHAR* CRecentCmd::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

int CRecentCmd::CompareItem( const CCmdString* p1, LPCTSTR p2 ) const
{
	return _tcscmp(*p1,p2);
}

void CRecentCmd::CopyItem( CCmdString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
