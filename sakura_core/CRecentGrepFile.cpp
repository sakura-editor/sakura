#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentGrepFile.h"
#include <string.h>
#include "my_icmp.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFile::CRecentGrepFile()
{
	Create(
		&GetShareData()->m_aGrepFiles[0],
		&GetShareData()->m_aGrepFiles._GetSizeRef(),
		NULL,
		MAX_GREPFILE,
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
const TCHAR* CRecentGrepFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

int CRecentGrepFile::CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentGrepFile::CopyItem( CGrepFileString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
