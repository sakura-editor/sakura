#include "StdAfx.h"
#include "CRecentTagjumpKeyword.h"
#include "env/DLLSHAREDATA.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentTagjumpKeyword::CRecentTagjumpKeyword()
{
	Create(
		GetShareData()->m_sTagJump.m_aTagJumpKeywords.dataPtr(),
		&GetShareData()->m_sTagJump.m_aTagJumpKeywords._GetSizeRef(),
		NULL,
		MAX_TAGJUMP_KEYWORD,
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
const TCHAR* CRecentTagjumpKeyword::GetItemText( int nIndex ) const
{
	return to_tchar(*GetItem(nIndex));
}

bool CRecentTagjumpKeyword::DataToReceiveType( LPCWSTR* dst, const CTagjumpKeywordString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentTagjumpKeyword::TextToDataType( CTagjumpKeywordString* dst, LPCTSTR pszText ) const
{
	CopyItem(dst, to_wchar(pszText));
	return true;
}

int CRecentTagjumpKeyword::CompareItem( const CTagjumpKeywordString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentTagjumpKeyword::CopyItem( CTagjumpKeywordString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}
