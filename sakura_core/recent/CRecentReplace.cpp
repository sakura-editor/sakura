#include "stdafx.h"
#include "CRecentReplace.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentReplace::CRecentReplace()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr(),
		&GetShareData()->m_sSearchKeywords.m_aReplaceKeys._GetSizeRef(),
		NULL /*GetShareData()->m_bREPLACEKEYArrFavorite*/,
		MAX_REPLACEKEY,
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
const TCHAR* CRecentReplace::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

int CRecentReplace::CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentReplace::CopyItem( CReplaceString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
