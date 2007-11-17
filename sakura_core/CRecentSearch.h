#pragma once

#include "CRecentImp.h"
#include "StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CSearchString;

//! 検索の履歴を管理 (RECENT_FOR_SEARCH)
class CRecentSearch : public CRecentImp<CSearchString, LPCWSTR>{
public:
	//生成
	CRecentSearch();

	//オーバーライド
	int				CompareItem( const CSearchString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CSearchString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
