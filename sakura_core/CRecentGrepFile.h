#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CGrepFileString;

//! GREP�t�@�C���̗������Ǘ� (RECENT_FOR_GREP_FILE)
class CRecentGrepFile : public CRecentImp<CGrepFileString, LPCTSTR>{
public:
	//����
	CRecentGrepFile();

	//�I�[�o�[���C�h
	int				CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFileString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
