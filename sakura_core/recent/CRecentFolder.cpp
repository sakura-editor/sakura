#include "stdafx.h"
#include "global.h"
#include "env/CShareData.h"
#include "CRecentFolder.h"
#include <string.h>





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentFolder::CRecentFolder()
{
	Create(
		&GetShareData()->m_sHistory.m_szOPENFOLDERArr[0],
		&GetShareData()->m_sHistory.m_nOPENFOLDERArrNum,
		GetShareData()->m_sHistory.m_bOPENFOLDERArrFavorite,
		MAX_OPENFOLDER,
		&(GetShareData()->m_Common.m_sGeneral.m_nOPENFOLDERArrNum_MAX)
	);
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      �I�[�o�[���C�h                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*
	�A�C�e���̔�r�v�f���擾����B

	@note	�擾��̃|�C���^�̓��[�U�Ǘ��̍\���̂ɃL���X�g���ĎQ�Ƃ��Ă��������B
*/
const TCHAR* CRecentFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

int CRecentFolder::CompareItem( const CPathString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentFolder::CopyItem( CPathString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
