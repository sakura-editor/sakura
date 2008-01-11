#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CGrepFolderString;

//! GREPフォルダの履歴を管理 (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder : public CRecentImp<CGrepFolderString, LPCTSTR>{
public:
	//生成
	CRecentGrepFolder();

	//オーバーライド
	int				CompareItem( const CGrepFolderString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFolderString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
