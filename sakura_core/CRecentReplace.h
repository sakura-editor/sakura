#pragma once

#include "CRecentImp.h"
#include "StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CReplaceString;

//! 置換の履歴を管理 (RECENT_FOR_REPLACE)
class CRecentReplace : public CRecentImp<CReplaceString, LPCWSTR>{
public:
	//生成
	CRecentReplace();

	//オーバーライド
	int				CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CReplaceString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
