#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentFolder.h"
#include <string.h>
#include "my_icmp.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           生成                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFolder::CRecentFolder()
{
	Create(
		&GetShareData()->m_szOPENFOLDERArr[0],
		&GetShareData()->m_nOPENFOLDERArrNum,
		GetShareData()->m_bOPENFOLDERArrFavorite,
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

int CRecentFolder::CompareItem( const CPathString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentFolder::CopyItem( CPathString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
