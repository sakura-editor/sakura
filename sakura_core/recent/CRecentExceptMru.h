#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CMetaPath;

//! フォルダの履歴を管理 (RECENT_FOR_FOLDER)
class CRecentExceptMRU : public CRecentImp<CMetaPath, LPCTSTR>{
public:
	//生成
	CRecentExceptMRU();

	//オーバーライド
	int				CompareItem( const CMetaPath* p1, LPCTSTR p2 ) const;
	void			CopyItem( CMetaPath* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
	bool			DataToReceiveType( LPCTSTR* dst, const CMetaPath* src ) const;
	bool			TextToDataType( CMetaPath* dst, LPCTSTR pszText ) const;
};
