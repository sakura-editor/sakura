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

bool CRecentGrepFile::DataToReceiveType( LPCTSTR* dst, const CGrepFileString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentGrepFile::TextToDataType( CGrepFileString* dst, LPCTSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentGrepFile::CompareItem( const CGrepFileString* p1, LPCTSTR p2 ) const
{
	return _tcsicmp(*p1,p2);
}

void CRecentGrepFile::CopyItem( CGrepFileString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
