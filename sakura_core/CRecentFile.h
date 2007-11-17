#pragma once

#include "CRecentImp.h"
#include "FileInfo.h" //FileInfo

//! FileInfoの履歴を管理 (RECENT_FOR_FILE)
class CRecentFile : public CRecentImp<FileInfo>{
public:
	//生成
	CRecentFile();

	//オーバーライド
	int				CompareItem( const FileInfo* p1, const FileInfo* p2 ) const;
	void			CopyItem( FileInfo* dst, const FileInfo* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//固有インターフェース
	int FindItemByPath(const TCHAR* pszPath) const;
};
