#pragma once

#include "CRecentImp.h"
#include "env/CShareData.h" //EditNode

//! EditNode(�E�B���h�E���X�g)�̗������Ǘ� (RECENT_FOR_EDITNODE)
class CRecentEditNode : public CRecentImp<EditNode>{
public:
	//����
	CRecentEditNode();

	//�I�[�o�[���C�h
	int				CompareItem( const EditNode* p1, const EditNode* p2 ) const;
	void			CopyItem( EditNode* dst, const EditNode* src ) const;
	const TCHAR*	GetItemText( int nIndex ) const;

	//�ŗL�C���^�[�t�F�[�X
	int FindItemByHwnd(HWND hwnd) const;
	void DeleteItemByHwnd(HWND hwnd);
};
