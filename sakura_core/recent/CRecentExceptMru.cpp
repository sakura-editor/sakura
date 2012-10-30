#include "stdafx.h"
#include "CRecentExceptMRU.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentExceptMRU::CRecentExceptMRU()
{
	Create(
		GetShareData()->m_sHistory.m_aExceptMRU.dataPtr(),
		&GetShareData()->m_sHistory.m_aExceptMRU._GetSizeRef(),
		NULL,
		MAX_MRU,
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
const TCHAR* CRecentExceptMRU::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentExceptMRU::DataToReceiveType( LPCTSTR* dst, const CMetaPath* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentExceptMRU::TextToDataType( CMetaPath* dst, LPCTSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentExceptMRU::CompareItem( const CMetaPath* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentExceptMRU::CopyItem( CMetaPath* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
