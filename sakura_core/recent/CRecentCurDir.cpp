/*
	Copyright (C) 2008, kobake
	Copyright (C) 2013, Moca

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
#include "CRecentCurDir.h"
#include "config/maxdata.h"
#include "env/DLLSHAREDATA.h"
#include <string.h>


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

CRecentCurDir::CRecentCurDir()
{
	Create(
		GetShareData()->m_sHistory.m_aCurDirs.dataPtr(),
		&GetShareData()->m_sHistory.m_aCurDirs._GetSizeRef(),
		NULL,
		MAX_CMDARR,
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
const TCHAR* CRecentCurDir::GetItemText( int nIndex ) const
{
	return *GetItem(nIndex);
}

bool CRecentCurDir::DataToReceiveType( LPCTSTR* dst, const CCurDirString* src ) const
{
	*dst = *src;
	return true;
}

bool CRecentCurDir::TextToDataType( CCurDirString* dst, LPCTSTR pszText ) const
{
	CopyItem(dst, pszText);
	return true;
}

int CRecentCurDir::CompareItem( const CCurDirString* p1, LPCTSTR p2 ) const
{
	return _tcscmp(*p1,p2);
}

void CRecentCurDir::CopyItem( CCurDirString* dst, LPCTSTR src ) const
{
	_tcscpy(*dst,src);
}
