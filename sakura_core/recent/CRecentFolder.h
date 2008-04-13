#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

//StaticVector< StaticString<TCHAR, _MAX_PATH>, MAX_GREPFOLDER, const TCHAR*>

typedef StaticString<TCHAR, _MAX_PATH> CPathString;

//! �t�H���_�̗������Ǘ� (RECENT_FOR_FOLDER)
class CRecentFolder : public CRecentImp<CPathString, LPCTSTR>{
public:
	//����
	CRecentFolder();

	//�I�[�o�[���C�h
	int				CompareItem( const CPathString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CPathString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
