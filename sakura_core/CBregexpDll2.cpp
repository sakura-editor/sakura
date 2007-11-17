#include "stdafx.h"
#include "CBregexpDll2.h"

//	2007.07.22 genta : DLL���ʗp
static const TCHAR P_BREG[] = _T("BREGEXP.DLL");
static const TCHAR P_ONIG[] = _T("bregonig.dll");


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
LPCTSTR CBregexpDll2::GetDllNameInOrder( LPCTSTR str, int index )
{

	switch( index ){
	case 0:
		//	NULL�̓��X�g�̏I�����Ӗ�����̂ŁC
		//	str == NULL�̏ꍇ�ɂ��̂܂ܕԂ��Ă͂����Ȃ��D
		return str == NULL || str[0] == _T('\0') ? P_ONIG : str ;

// 2007.11.04 kobake ������Ǘ���UNICODE�ɂȂ�ABREGEXP �͎g���Ȃ��Ȃ����̂ŁA�R�����g�A�E�g�B
//	case 1:
//		return str == NULL || str[0] == _T('\0') ? P_BREG : NULL;
	}
	return NULL;
}


/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
int CBregexpDll2::InitDll(void)
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
		return 1;
	}
	
	return 0;
}

/*!
	BREGEXP_W�\���̂̉��
*/
int CBregexpDll2::DeinitDll( void )
{
	return 0;
}
