#include "StdAfx.h"
#include "CRecentGrepFile.h"
#include <string.h>
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFile::CRecentGrepFile()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFiles.dataPtr(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFiles._GetSizeRef(),
		NULL,
		MAX_GREPFILE,
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
const TCHAR* CRecentGrepFile::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

int CRecentGrepFile::CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentGrepFile::CopyItem( CGrepFileString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
