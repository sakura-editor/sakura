#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"

typedef StaticString<TCHAR, _MAX_PATH> CGrepFolderString;

//! GREP�t�H���_�̗������Ǘ� (RECENT_FOR_GREP_FOLDER)
class CRecentGrepFolder : public CRecentImp<CGrepFolderString, LPCTSTR>{
public:
	//����
	CRecentGrepFolder();

	//�I�[�o�[���C�h
	int				CompareItem( const CGrepFolderString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CGrepFolderString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
