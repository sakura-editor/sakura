#include "StdAfx.h"
#include "CRecentEditNode.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentEditNode::CRecentEditNode()
{
	Create(
		GetShareData()->m_sNodes.m_pEditArr,
		&GetShareData()->m_sNodes.m_nEditArrNum,
		NULL,
		MAX_EDITWINDOWS,
		NULL
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �I�[�o�[���C�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* CRecentEditNode::GetItemText( int nIndex ) const
{
	return _T("WIN"); //���e�L�X�g���͖��� (GetWindowText���Ă����Ă��ǂ����ǁA���̊֐��͎��s����Ȃ��̂ŁA�Ӗ��͖���)
}

bool CRecentEditNode::DataToReceiveType( const EditNode** dst, const EditNode* src ) const
{
	*dst = src;
	return true;
}

bool CRecentEditNode::TextToDataType( EditNode* dst, LPCTSTR pszText ) const
{
	return false;
}

int CRecentEditNode::CompareItem( const EditNode* p1, const EditNode* p2 ) const
{
	return p1->m_hWnd - p2->m_hWnd;
}

void CRecentEditNode::CopyItem( EditNode* dst, const EditNode* src ) const
{
	*dst = *src;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �ŗL�C���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentEditNode::FindItemByHwnd(HWND hwnd) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if(GetItem(i)->m_hWnd == hwnd)return i;
	}
	return -1;
}

void CRecentEditNode::DeleteItemByHwnd(HWND hwnd)
{
	int n = FindItemByHwnd(hwnd);
	if(n!=-1){
		DeleteItem(n);
	}
	else{
		DBPRINT_A("DeleteItemByHwnd���s\n");
	}
}
