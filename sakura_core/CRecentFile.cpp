#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentFile.h"
#include <string.h>
#include "my_icmp.h"


/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* CRecentFile::GetItemText( int nIndex ) const
{
	return GetItem(nIndex)->m_szPath;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFile::CRecentFile()
{
	Create(
		GetShareData()->m_fiMRUArr,
		&GetShareData()->m_nMRUArrNum,
		GetShareData()->m_bMRUArrFavorite,
		MAX_MRU,
		&(GetShareData()->m_Common.m_sGeneral.m_nMRUArrNum_MAX)
	);
}



int CRecentFile::CompareItem( const FileInfo* p1, const FileInfo* p2 ) const
{
	return _tcsicmp(p1->m_szPath, p2->m_szPath);
}

void CRecentFile::CopyItem( FileInfo* dst, const FileInfo* src ) const
{
	memcpy_raw(dst,src,sizeof(*dst));
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   �ŗL�C���^�[�t�F�[�X                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int CRecentFile::FindItemByPath(const TCHAR* pszPath) const
{
	int n = GetItemCount();
	for(int i=0;i<n;i++){
		if(_tcsicmp(GetItem(i)->m_szPath,pszPath)==0)return i;
	}
	return -1;
}

