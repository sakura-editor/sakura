#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CSearchString;

//! �����̗������Ǘ� (RECENT_FOR_SEARCH)
class CRecentSearch : public CRecentImp<CSearchString, LPCWSTR>{
public:
	//����
	CRecentSearch();

	//�I�[�o�[���C�h
	int				CompareItem( const CSearchString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CSearchString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
