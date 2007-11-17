#pragma once

#include "CRecentImp.h"
#include "StaticType.h"

//StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>

typedef StaticString<TCHAR, _MAX_PATH> CPathString;

//! フォルダの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentFolder : public CRecentImp<CPathString, LPCTSTR>{
public:
	//生成
	CRecentFolder();

	//オーバーライド
	int				CompareItem( const CPathString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CPathString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
