#pragma once

#include "CRecentImp.h"
#include "util/StaticType.h"
#include "config/maxdata.h" //MAX_CMDLEN

typedef StaticString<TCHAR, MAX_CMDLEN> CCmdString;

//! �R�}���h�̗������Ǘ� (RECENT_FOR_CMD)
class CRecentCmd : public CRecentImp<CCmdString, LPCTSTR>{
public:
	//����
	CRecentCmd();

	//�I�[�o�[���C�h
	int				CompareItem( const CCmdString* p1, LPCTSTR p2 ) const;
	void			CopyItem( CCmdString* dst, LPCTSTR src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;
};
