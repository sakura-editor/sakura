/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
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
		GetShareData()->m_sSearchKeywords.m_aReplaceKeys.dataPtr()->GetBufferCount(),
		&GetShareData()->m_sSearchKeywords.m_aReplaceKeys._GetSizeRef(),
		NULL,
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

bool CRecentReplace::DataToReceiveType( LPCWSTR* dst, const CReplaceString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentReplace::TextToDataType( CReplaceString* dst, LPCTSTR pszText ) const
{
	if( false == ValidateReceiveType(to_wchar(pszText)) ){
		return false;
	}
	CopyItem(dst, to_wchar(pszText));
	return true;
}


int CRecentReplace::CompareItem( const CReplaceString* p1, LPCWSTR p2 ) const
{
	return wcscmp(*p1,p2);
}

void CRecentReplace::CopyItem( CReplaceString* dst, LPCWSTR src ) const
{
	wcscpy(*dst,src);
}

bool CRecentReplace::ValidateReceiveType( LPCWSTR p ) const
{
	if( GetTextMaxLength() <= wcslen(p) ){
		return false;
	}
	return true;
}

size_t CRecentReplace::GetTextMaxLength() const
{
	return m_nTextMaxLength;
}
