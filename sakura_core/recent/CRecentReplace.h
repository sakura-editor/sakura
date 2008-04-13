#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CReplaceString;

//! �u���̗������Ǘ� (RECENT_FOR_REPLACE)
class CRecentReplace : public CRecentImp<CReplaceString, LPCWSTR>{
public:
	//����
	CRecentReplace();

	//�I�[�o�[���C�h
	int				CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CReplaceString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
