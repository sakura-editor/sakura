//	$Id$
/*!	@file
	@brief BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, hor

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

#include <stdio.h>
#include <string.h>
#include "CBregexp.h"
#include "etc_uty.h"

const char BREGEXP_OPT_KI[]	= "ki";
const char BREGEXP_OPT_K[]	= "k";

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

	@param szPattern [in] �����p�^�[��

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

	//	Jan. 31, 2002 genta
	//	/�̃G�X�P�[�v
	//	�������m�ہD
	char *szNPattern = new char[ strlen(szPattern) * 2 + 5 ];
	szNPattern[0] = '/';
	char *pEnd = szNPattern + 1 + cescape( szPattern, szNPattern + 1, '/', '\\' );
	*pEnd = '/';
	*++pEnd = 'k';
	*++pEnd = '\0';

	BMatch( szNPattern, tmp, tmp+1, &m_sRep, m_szMsg );
	delete [] szNPattern;

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

// 2002/01/19 novice
/*!
	���K�\���ɂ�镶����u��

	@param szPattern0 [in] �}�b�`�p�^�[��
	@param szPattern1 [in] �u��������
	@param target [in] �u���Ώۃf�[�^
	@param len [in] �u���Ώۃf�[�^��
	@param out [out] �u���㕶����		// 2002.01.26 hor
	@retval true ����
	@retval false ���s

*/
bool CBregexp::Replace( const char* szPattern0, const char* szPattern1, char *target, int len , char **out)
{
	int result;

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	//	From Here Feb. 01, 2002 genta
	//	/�̃G�X�P�[�v
	char *szNPattern = new char[ ( strlen( szPattern0 ) + strlen( szPattern1 )) * 2 + 5 ];
	szNPattern[0] = 's';
	szNPattern[1] = '/';
	char *pEnd = szNPattern + 2;

	pEnd = pEnd + cescape( szPattern0, pEnd, '/', '\\' );
	*pEnd = '/';
	++pEnd;

	pEnd = pEnd + cescape( szPattern1, pEnd, '/', '\\' );
	*pEnd = '/';
	*++pEnd = 'k';
	*++pEnd = 'm';
	*++pEnd = '\0';
	//	To Here Feb. 01, 2002 genta

	result = BSubst( szNPattern, target, target + len, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ){
		return false;
	}

	if( result ){
		if( m_sRep->outp != NULL && m_sRep->outp != '\0' ){
//			strcpy( target, m_sRep->outp );
			int i=lstrlen(m_sRep->outp);
			*out = new char[i+1];
			strcpy( *out, m_sRep->outp );
			(*out)[i] = '\0';
		}else{
//			strcpy( target, "" );
			*out = new char[1];
			(*out)[0] = '\0';
		}

		return true;
	}

	return false;
}

/*[EOF]*/
