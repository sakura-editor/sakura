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

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
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
