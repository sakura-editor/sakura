#pragma once

#include "CRecentImp.h"
#include "StaticType.h"
#include "config/maxdata.h" //MAX_CMDLEN

typedef StaticString<TCHAR, MAX_CMDLEN> CCmdString;

//! コマンドの履歴を管理 (RECENT_FOR_CMD)
class CRecentCmd : public CRecentImp<CCmdString, LPCTSTR>{
public:
	//生成
	CRecentCmd();

	//オーバーライド
	int				CompareItem( const CCmdString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CCmdString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
