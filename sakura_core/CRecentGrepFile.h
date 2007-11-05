#pragma once

#include "CRecentImp.h"
#include "StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CGrepFileString;

//! GREPファイルの履歴を管理 (RECENT_FOR_GREP_FILE)
class CRecentGrepFile : public CRecentImp<CGrepFileString, LPCTSTR>{
public:
	//生成
	CRecentGrepFile();

	//オーバーライド
	int				CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFileString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
