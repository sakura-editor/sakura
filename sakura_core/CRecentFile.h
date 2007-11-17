#pragma once

#include "CRecentImp.h"
#include "FileInfo.h" //FileInfo

//! FileInfo�̗������Ǘ� (RECENT_FOR_FILE)
class CRecentFile : public CRecentImp<FileInfo>{
public:
	//����
	CRecentFile();

	//�I�[�o�[���C�h
	int				CompareItem( const FileInfo* p1, const FileInfo* p2 ) const;
	void			CopyItem( FileInfo* dst, const FileInfo* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//�ŗL�C���^�[�t�F�[�X
	int FindItemByPath(const TCHAR* pszPath) const;
};
