//	$Id$
/*!	@file
	BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001, genta

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

#include "CBregexp.h"
//#include "CMemory.h"

CBregexp::CBregexp() : m_sRep( NULL )
{
}

CBregexp::~CBregexp()
{
}

//	Jul. 5, 2001 genta �����ǉ��B�������A�����ł͎g��Ȃ��B
char *
CBregexp::GetDllName( char* str )
{
	return "BREGEXP.DLL";
}
/*!
	DLL�̏�����

	�֐��̃A�h���X���擾���ă����o�ɕۊǂ���D

	@retval 0 ����
	@retval 1 �A�h���X�擾�Ɏ��s
*/
int CBregexp::InitDll()
{
	//	�A�h���X�擾
	if( (BMatch = (BREGEXP_BMatch)GetProcAddress( GetInstance(), "BMatch" )) == NULL )
		return 1;
	if( (BSubst = (BREGEXP_BSubst)GetProcAddress( GetInstance(), "BSubst" )) == NULL )
		return 1;
	if( (BTrans = (BREGEXP_BTrans)GetProcAddress( GetInstance(), "BTrans" )) == NULL )
		return 1;
	if( (BSplit = (BREGEXP_BSplit)GetProcAddress( GetInstance(), "BSplit" )) == NULL )
		return 1;
	if( (BRegfree = (BREGEXP_BRegfree)GetProcAddress( GetInstance(), "BRegfree" )) == NULL )
		return 1;
	if( (BRegexpVersion = (BREGEXP_BRegexpVersion)GetProcAddress( GetInstance(), "BRegexpVersion" )) == NULL )
		return 1;

	return 0;
}

/*!
	BREGEXP�\���̂̉��
*/
int CBregexp::DeinitDll( void )
{
	ReleaseCompileBuffer();
	return 0;
}

/*!
	JRE32�̃G�~�����[�V�����֐��D��̕�����ɑ΂��Č������s�����Ƃɂ��
	BREGEXP�\���̂̐����݂̂��s���D

	�����������perl�`�� i.e. /pattern/option �܂��� m/pattern/option

	@param str[in] �����p�^�[��

	@retval true ����
	@retval false ���s
*/
bool CBregexp::Compile( const char* szPattern )
{
	static char tmp[2] = "\0";	//	�����ΏۂƂȂ�󕶎���

	//	DLL�����p�\�łȂ��Ƃ��̓G���[�I��
	if( !IsAvailable() )
		return false;

	//	BREGEXP�\���̂̉��
	ReleaseCompileBuffer();

	BMatch( szPattern, tmp, tmp+1, &m_sRep, m_szMsg );

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}

	return true;
}

/*!
	JRE32�̃G�~�����[�V�����֐��D���ɂ���R���p�C���\���̂𗘗p���Č����i1�s�j��
	�s���D

	@param target [in] �����Ώۗ̈�擪�A�h���X
	@param len [in] �����Ώۗ̈�T�C�Y
	@param nStart [in] �����J�n�ʒu�D(�擪��0)
	@param rep [out] �R���p�C���\���̂ւ̃A�h���X��Ԃ��B
	�����ɂ͓����ŕێ����Ă���\���̂��p�����邽�߁A�����ɕʂ̍\���̂�ݒ肵�Ă�
	����͌����ɂ͗p�����Ȃ��B

	@retval true Match
	@retval false No Match �܂��� �G���[�B�G���[��rep == NULL�ɂ�蔻��\�B

	@note �߂�l==-1

	@par rep�ɕԂ������


*/
bool CBregexp::GetMatchInfo( const char* target, int len, int nStart, BREGEXP**rep )
{
	//	DLL�����p�\�łȂ��Ƃ��A�܂��͍\���̂����ݒ�̎��̓G���[�I��
	if( (!IsAvailable()) || m_sRep == NULL ){
		*rep = NULL;
		return false;
	}

	*rep = m_sRep;
	//	����������NULL���w�肷��ƑO��Ɠ���̕�����ƌ��Ȃ����
	// BMatch( m_cPtn.c_str(), target + nStart, target + nStart - len, &m_sRep, m_szMsg );
	if( BMatch( NULL, target + nStart, target + len, &m_sRep, m_szMsg ) ){
		return true;
	}

	return false;
}

#if 0
/*!
	@note ���̊֐��͖��g�p�̂��ߎ��������B

	@param str1[in] ������1
	@param str2[in] ������2�D�����ꍇ��NULL�D
	@retval ���p�\�ȋ��E����(01-FF)�D�G���[�̏ꍇ��0��Ԃ��B

	�ʏ�̐��K�\������p�^�[��������𐶐�����Ƃ��̋��E�̑I�����s���B
	���E�ɂ͔C�ӂ̕������g���邪�Astr1/str2�Ɍ���Ȃ�������I�΂Ȃ��Ă͂Ȃ�Ȃ��B
	2�ݒ�ł���̂͒u������Ƃ��ɒu���O�ƒu����̗����Ɍ���Ȃ��������I�Ԃ��߁B

	@par �I����@
	����������Ɍ���0x21����0x7E�̊Ԃ̕������o���������ǂ������L�^���Ă����B
	���̌�A�g���Ă��Ȃ������̒��ł����Ƃ��擪�ɂ�����̂��g���B

*/
int CBregexp::ChooseBoundary( const char* str1, const char* str2 )
{
	return '/';
	//	�Ƃ肠������������
}
#endif


/*[EOF]*/
