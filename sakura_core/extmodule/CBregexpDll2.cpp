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
#include "CBregexpDll2.h"

CBregexpDll2::CBregexpDll2()
{
}

CBregexpDll2::~CBregexpDll2()
{
}

/*!
	@date 2001.07.05 genta �����ǉ��B�������A�����ł͎g��Ȃ��B
	@date 2007.06.25 genta ������DLL���ɑΉ�
	@date 2007.09.13 genta �T�[�`���[����ύX
		@li �w��L��̏ꍇ�͂���݂̂�Ԃ�
		@li �w�薳��(NULL�܂��͋󕶎���)�̏ꍇ��BREGONIG, BREGEXP�̏��Ŏ��݂�
*/
LPCTSTR CBregexpDll2::GetDllNameImp( int index )
{
	return _T("bregonig.dll");
}


/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval true ����
	@retval false �A�h���X�擾�Ɏ��s
*/
bool CBregexpDll2::InitDllImp()
{
	//DLL���֐������X�g
	const ImportTable table[] = {
		{ &m_BMatch,			"BMatchW" },
		{ &m_BSubst,			"BSubstW" },
		{ &m_BTrans,			"BTransW" },
		{ &m_BSplit,			"BSplitW" },
		{ &m_BRegfree,			"BRegfreeW" },
		{ &m_BRegexpVersion,	"BRegexpVersionW" },
		{ &m_BMatchEx,			"BMatchExW" },
		{ &m_BSubstEx,			"BSubstExW" },
		{ NULL, 0 }
	};
	
	if( ! RegisterEntries( table )){
		return false;
	}
	
	return true;
}
