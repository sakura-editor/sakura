#pragma once

#include "CRecentImp.h"
#include "EditInfo.h" //EditInfo

//! EditInfoの履歴を管理 (RECENT_FOR_FILE)
class CRecentFile : public CRecentImp<EditInfo>{
public:
	//生成
	CRecentFile();

	//オーバーライド
	int				CompareItem( const EditInfo* p1, const EditInfo* p2 ) const;
	void			CopyItem( EditInfo* dst, const EditInfo* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//固有インターフェース
	int FindItemByPath(const TCHAR* pszPath) const;
};
