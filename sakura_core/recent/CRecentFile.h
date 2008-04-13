#pragma once

#include "CRecentImp.h"
#include "EditInfo.h" //EditInfo

//! EditInfo�̗������Ǘ� (RECENT_FOR_FILE)
class CRecentFile : public CRecentImp<EditInfo>{
public:
	//����
	CRecentFile();

	//�I�[�o�[���C�h
	int				CompareItem( const EditInfo* p1, const EditInfo* p2 ) const;
	void			CopyItem( EditInfo* dst, const EditInfo* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//�ŗL�C���^�[�t�F�[�X
	int FindItemByPath(const TCHAR* pszPath) const;
};
