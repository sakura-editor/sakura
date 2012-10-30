#include "StdAfx.h"
#include <string.h>
#include "CRecentGrepFolder.h"
#include "env/DLLSHAREDATA.h"


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentGrepFolder::CRecentGrepFolder()
{
	Create(
		GetShareData()->m_sSearchKeywords.m_aGrepFolders.dataPtr(),
		&GetShareData()->m_sSearchKeywords.m_aGrepFolders._GetSizeRef(),
		NULL,
		MAX_GREPFOLDER,
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
const TCHAR* CRecentGrepFolder::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentGrepFolder::DataToReceiveType( LPCTSTR* dst, const CGrepFolderString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFolder::TextToDataType( CGrepFolderString* dst, LPCTSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFolder::CompareItem( const CGrepFolderString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentGrepFolder::CopyItem( CGrepFolderString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
