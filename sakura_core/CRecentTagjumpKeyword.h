#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<WCHAR, _MAX_PATH> CTagjumpKeywordString;

//! �^�O�W�����v�L�[���[�h�̗������Ǘ� (RECENT_FOR_TAGJUMP_KEYWORD)
class CRecentTagjumpKeyword : public CRecentImp<CTagjumpKeywordString, LPCWSTR>{
public:
	//����
	CRecentTagjumpKeyword();

	//�I�[�o�[���C�h
	int				CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const;
	void			CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
