#include "stdafx.h"
#include "global.h"
#include "CShareData.h"
#include "CRecentSearch.h"
#include <string.h>





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentSearch::CRecentSearch()
{
	Create(
		&GetShareData()->m_aSearchKeys[0],
		&GetShareData()->m_aSearchKeys._GetSizeRef(),
		NULL /*GetShareData()->m_bSEARCHKEYArrFavorite*/,
		MAX_SEARCHKEY,
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
const TCHAR* CRecentSearch::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

int CRecentSearch::CompareItem( const CSearchString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentSearch::CopyItem( CSearchString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
