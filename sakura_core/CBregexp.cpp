//	$Id$
/*!	@file
	@brief BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
	@date 2002/2/1 hor		ReleaseCompileBuffer��K�X�ǉ�
	@date Jul. 25, 2002 genta �s���������l�������������s���悤�ɁD(�u���͂܂�)
	@date 2003.05.22 ����� ���K�Ȑ��K�\���ɋ߂Â���
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, novice, hor
	Copyright (C) 2003, �����

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

// Compile���A�s���u��(len=0)�̎��Ƀ_�~�[������(�P�ɓ���) by �����
char CBregexp::tmpBuf[2] = "\0";

// ���g�p�Ȃ̂ō폜
//const char BREGEXP_OPT_KI[]	= "ki";
//const char BREGEXP_OPT_K[]	= "k";

CBregexp::CBregexp() : m_sRep( NULL ),
	m_ePatType( PAT_NORMAL )	//	Jul, 25, 2002 genta
{
}

CBregexp::~CBregexp()
{
	//<< 2002/03/27 Azumaiya
	// �ꉞ�A�N���X�̏I�����ɃR���p�C���o�b�t�@������B
	DeinitDll();
	//>> 2002/03/27 Azumaiya
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
	//	Apr. 15, 2002 genta
	//	CPPA.cpp ���Q�l�ɐݒ��z�񉻂���
	
	const ImportTable table[] = {
		{ &BMatch,		"BMatch" },
		{ &BSubst,		"BSubst" },
		{ &BTrans,		"BTrans" },
		{ &BSplit,		"BSplit" },
		{ &BRegfree, 	"BRegfree" },
		{ &BRegexpVersion,	"BRegexpVersion" },
		{ NULL, 0 }
	};
	
	if( ! RegisterEntries( table )){
		return 1;
	}
	
#if 0
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
#endif

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

/*! @brief ���C�u�����ɓn�����߂̌����E�u���p�^�[�����쐬����
**
** @note szPattern2: == NULL:���� != NULL:�u��
** 
** @param szPattern [in] �����p�^�[��
** @param szPattern2 [in] �u���p�^�[��(NULL�Ȃ猟��)
** @param szAdd2	[in] �u���p�^�[���̌��ɕt��������p�^�[��($1�Ȃ�) 
** @param bOption	[in] �����I�v�V����
**
** @retval ���C�u�����ɓn�������p�^�[���ւ̃|�C���^��Ԃ�
** @note �Ԃ��|�C���^�́A�Ăяo������ delete ���邱��
** 
** @date 2003.05.03 ����� �֐��ɐ؂�o��
*/
char* CBregexp::MakePatternSub( const char* szPattern, const char* szPattern2, const char* szAdd2, int bOption ) 
{
	static const char DELIMITER = '\xFF';		//<! �f���~�^
	int nLen;									//!< szPattern�̒���
	int nLen2;									//!< szPattern2 + szAdd2 �̒���

	// �����p�^�[���쐬
	char *szNPattern;		//!< ���C�u�����n���p�̌����p�^�[��������
	char *pPat;				//!< �p�^�[�������񑀍�p�̃|�C���^

	nLen = strlen(szPattern);
	if (szPattern2 == NULL) {
		// ����(BMatch)��
		szNPattern = new char[ nLen + 15 ];	//	15�F�us///option�v���]�T�ł͂���悤�ɁB
		pPat = szNPattern;
		*pPat++ = 'm';
	} else {
		// �u��(BSubst)��
		nLen2 = strlen(szPattern2) + strlen(szAdd2);
		szNPattern = new char[ nLen + nLen2 + 15 ];
		pPat = szNPattern;
		*pPat++ = 's';
	}
	*pPat++ = DELIMITER;
	while (*szPattern != '\0') { *pPat++ = *szPattern++; }
	*pPat++ = DELIMITER;
	if (szPattern2 != NULL) {
		while (*szPattern2 != '\0') { *pPat++ = *szPattern2++; }
		while (*szAdd2 != '\0') { *pPat++ = *szAdd2++; }
		*pPat++ = DELIMITER;
	}
	*pPat++ = 'k';			// �����Ή�
	*pPat++ = 'm';			// �����s�Ή�(�A���A�Ăяo�����������s�Ή��łȂ�)
	if( !(bOption & 0x01) ) {		// 2002/2/1 hor IgnoreCase �I�v�V�����ǉ� �}�[�W�Faroka
		*pPat++ = 'i';		// ����
	}
	*pPat = '\0';
	return szNPattern;
}


/*! 
** �s�������̈Ӗ������C�u�����ł� \n�Œ�Ȃ̂ŁA
** ��������܂������߂ɁA���C�u�����ɓn�����߂̌����E�u���p�^�[�����H�v����
**
** �s������($)�������p�^�[���̍Ō�ɂ���A���̒��O�� [\r\n] �łȂ��ꍇ�ɁA
** �s������($)�̎�O�� ([\r\n]+)�����āA�u���p�^�[���� $(nParen+1)��₤
** �Ƃ����A���S���Y����p���āA���܂����B
**
** @note szPattern2: == NULL:���� != NULL:�u��
** 
** @param szPattern [in] �����p�^�[��
** @param szPattern2 [in] �u���p�^�[��(NULL�Ȃ猟��)
** @param bOption [in] �����I�v�V����
**
** @retval ���C�u�����ɓn�������p�^�[���ւ̃|�C���^��Ԃ�
** @note �Ԃ��|�C���^�́A�Ăяo������ delete ���邱��
**
** @date 2003.05.03 ����� �֐��ɐ؂�o��
*/
char* CBregexp::MakePattern( const char* szPattern, const char* szPattern2, int bOption ) 
{
	static const char CRLF[] = "\r\n";			//!< ���A�E���s
	static const char CR[] = "\r";				//!< ���A
	static const char LF[] = "\n";				//!< ���s
	static const char LFCR[] = "\n\r";			//!< ���s�E���A
	static const char TOP_MATCH[] = "/^\\(*\\^/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const char DOL_MATCH[] = "/\\\\\\$$/k";							//!< \$(�s���p�^�[���łȂ�)�`�F�b�N�p�p�^�[��
	static const char BOT_MATCH[] = "/\\$\\)*$/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const char BOT_SUBST[] = "s/\\$(\\)*)$/([\\\\r\\\\n]+)\\$$1/k";	//!< �s���p�^�[���̒u���p�p�^�[��
	static const char TAB_MATCH[] = "/^\\(*\\^\\$\\)*$/k";					//!< "^$"�p�^�[�������`�F�b�N�p�p�^�[��
	int nLen;									//!< szPattern�̒���
	BREGEXP*	sReg = NULL;					//!< �R���p�C���\����
	char szMsg[80] = "";						//!< �G���[���b�Z�[�W
	char szAdd2[5] = "";						//!< �s������u���� $���� �i�[�p
	int nParens = 0;							//!< �����p�^�[��(szPattern)���̊��ʂ̐�(�s�����Ɏg�p)
	char *szNPattern;							//!< �����p�^�[��

	nLen = strlen( szPattern );
	// �p�^�[����ʂ̐ݒ�
	if( BMatch( TOP_MATCH, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
		// �s���p�^�[���Ƀ}�b�`����
		m_ePatType |= PAT_TOP;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( TAB_MATCH, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
		// �s���s���p�^�[���Ƀ}�b�`����
		m_ePatType |= PAT_TAB;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( DOL_MATCH, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
		// �s����\$ �Ƀ}�b�`����
		// PAT_NORMAL
	} else {
		BRegfree(sReg);
		sReg = NULL;
		if( BMatch( BOT_MATCH, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
			// �s���p�^�[���Ƀ}�b�`����
			m_ePatType |= PAT_BOTTOM;
		} else {
			// ���̑�
			// PAT_NORMAL
		}
	}
	BRegfree(sReg);
	sReg = NULL;

	if( (m_ePatType & PAT_BOTTOM) != 0 ) {
		bool bJustDollar = false;			// �s���w���$�݂̂ł���t���O($�̑O�� \r\n���w�肳��Ă��Ȃ�)
		szNPattern = MakePatternSub(szPattern, NULL, NULL, bOption);
		int matched = BMatch( szNPattern, CRLF, CRLF+sizeof(CRLF)-1, &sReg, szMsg );
		if( matched >= 0 ) {
			// szNPattern���s���ȃp�^�[�����̃G���[�łȂ�����
			// �G���[���ɂ� sReg��NULL�̂܂܂Ȃ̂ŁAsReg->nparens�ւ̃A�N�Z�X�͕s��
			nParens = sReg->nparens;			// �I���W�i���̌��������񒆂�()�̐����L��
			if( matched > 0 ) {
				if( sReg->startp[0] == &CRLF[1] && sReg->endp[0] == &CRLF[1] ) {
					if( BMatch( NULL, CR, CR+sizeof(CR)-1, &sReg, szMsg ) > 0 && sReg->startp[0] == &CR[1] && sReg->endp[0] == &CR[1] ) {
						if( BMatch( NULL, LF, LF+sizeof(LF)-1, &sReg, szMsg ) > 0 && sReg->startp[0] == &LF[0] && sReg->endp[0] == &LF[0] ) {
							if( BMatch( NULL, LFCR, LFCR+sizeof(LFCR)-1, &sReg, szMsg ) > 0 && sReg->startp[0] == &LFCR[0] && sReg->endp[0] == &LFCR[0] ) {
								// ����������� �s��($)�݂̂�����
								bJustDollar = true;
							}
						}
					}
				}
			} else {
				if( BMatch( NULL, CR, CR+sizeof(CR)-1, &sReg, szMsg ) <= 0 ) {
					if( BMatch( NULL, LF, LF+sizeof(LF)-1, &sReg, szMsg ) <= 0 ) {
						if( BMatch( NULL, LFCR, LFCR+sizeof(LFCR)-1, &sReg, szMsg ) <= 0 ) {
							// ����������́A�����{�s��($)������
							bJustDollar = true;
						}
					}
				}
			}
			BRegfree(sReg);
			sReg = NULL;
		}
		delete [] szNPattern;

		if( bJustDollar == true || (m_ePatType & PAT_TAB) != 0 ) {
			// �s���w���$ or �s���s���w�� �Ȃ̂ŁA�����������u��
			if( BSubst( BOT_SUBST, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
				szPattern = sReg->outp;
				if( szPattern2 != NULL ) {
					// �u���p�^�[��������̂ŁA�u���p�^�[���̍Ō�� $(nParens+1)��ǉ�
					wsprintf( szAdd2, "$%d", nParens + 1 );
				}
			}
			// sReg->outp �̃|�C���^���Q�Ƃ��Ă���̂ŁAsReg���������͍̂Ō��
		}
	}

	szNPattern = MakePatternSub( szPattern, szPattern2, szAdd2, bOption );
	if( sReg != NULL ) {
		BRegfree(sReg);
	}
	return szNPattern;
}


/*!
	JRE32�̃G�~�����[�V�����֐��D��̕�����ɑ΂��Č������s�����Ƃɂ��
	BREGEXP�\���̂̐����݂̂��s���D

	�����������perl�`�� i.e. /pattern/option �܂��� m/pattern/option

	@param szPattern [in] �����p�^�[��
	@param bOption [in]
		0x01�F�啶���������̋�ʂ�����B

	@retval true ����
	@retval false ���s
*/
bool CBregexp::Compile( const char* szPattern, int bOption )
{

	//	DLL�����p�\�łȂ��Ƃ��̓G���[�I��
	if( !IsAvailable() )
		return false;

	//	BREGEXP�\���̂̉��
	ReleaseCompileBuffer();

	// ���C�u�����ɓn�������p�^�[�����쐬
	// �ʊ֐��ŋ��ʏ����ɕύX 2003.05.03 by �����
	char *szNPattern = MakePattern( szPattern, NULL, bOption );
	BMatch( szNPattern, tmpBuf, tmpBuf+1, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}
	
	// �s�������`�F�b�N�́AMakePattern�Ɏ�荞�� 2003.05.03 by �����

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

	// �s�̐擪("^")�̌������̓��ʏ��� by �����
	/*
	** �s��(^)�ƃ}�b�`����̂́AnStart=0�̎������Ȃ̂ŁA����ȊO�� false
	*/
	if( (m_ePatType & PAT_TOP) != 0 && nStart != 0 ) {
		// nStart!=0�ł��ABMatch()�ɂƂ��Ă͍s���ɂȂ�̂ŁA������false�ɂ���K�v������
		return false;
	}
			

	*rep = m_sRep;
	//	����������NULL���w�肷��ƑO��Ɠ���̕�����ƌ��Ȃ����
	int matched = BMatch( NULL, target + nStart, target + len, &m_sRep, m_szMsg );
	if ( matched < 0 ) {
		// BMatch�G���[
		// �G���[���������Ă��Ȃ������̂ŁAnStart>=len�̂悤�ȏꍇ�ɁA�}�b�`�����ɂȂ�
		// �����u�����̕s��ɂȂ��Ă��� 2003.05.03 by �����
		*rep = NULL;
		return false;
	} else if ( matched == 0 ) {
		return false;
	} else {
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
	@param bOption [in]
		0x01�F�啶���������̋�ʂ�����B

	@retval true ����
	@retval false ���s
*/
bool CBregexp::Replace( const char* szPattern0, const char* szPattern1, const char *target, int len, char **out, int bOption)
{
	int result;

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	// ���C�u�����ɓn�������p�^�[�����쐬
	// �ʊ֐��ŋ��ʏ����ɕύX 2003.05.03 by �����
	char *szNPattern = MakePattern( szPattern0, szPattern1, bOption );

	// nLen���O���ƁABSubst()���u���Ɏ��s���Ă��܂��̂ŁA��p�f�[�^(tmpBuf)���g�� 2003.05.03 �����
	if( len == 0 ) {
		target = tmpBuf;
		len = 1;
	}
	// �u�����s
	result = BSubst( szNPattern, target, target + len, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
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

		ReleaseCompileBuffer();
		return true;
	}

	ReleaseCompileBuffer();
	return false;
}

//<< 2002/03/27 Azumaiya
/*!
	JRE32�̃G�~�����[�V�����֐��D��̕�����ɑ΂��Ēu�����s�����Ƃɂ��
	BREGEXP�\���̂̐����݂̂��s���D

	@param szPattern0 [in] �u���p�^�[��
	@param szPattern1 [in] �u���㕶����p�^�[��
	@param bOption [in]
		0x01�F�啶���������̋�ʂ�����B

	@retval true ����
	@retval false ���s
*/
bool CBregexp::CompileReplace( const char* szPattern0, const char* szPattern1, int bOption )
{

	if( !IsAvailable() ){
		return false;
	}

	ReleaseCompileBuffer();

	// ���C�u�����ɓn�������p�^�[�����쐬
	// �ʊ֐��ŋ��ʏ����ɕύX 2003.05.03
	char *szNPattern = MakePattern(szPattern0, szPattern1, bOption);
	// �u�����s
	BSubst( szNPattern, tmpBuf, tmpBuf + 1, &m_sRep, m_szMsg );
	delete [] szNPattern;

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ){
		ReleaseCompileBuffer();
		return false;
	}

	// �s�������`�F�b�N�́AMakePattern�Ɏ�荞�� 2003.05.03
	return true;
}

/*!
	���K�\���ɂ�镶����u��
	���ɂ���R���p�C���\���̂𗘗p���Ēu���i1�s�j��
	�s���D

	@param szTarget [in] �u���Ώۃf�[�^
	@param nLen [in] �u���Ώۃf�[�^��
	@param pszOut [out] �u���㕶����
	@param pnOutLen [out] �u���㕶����̒���

	@retval true ����
	@retval false ���s
*/
bool CBregexp::GetReplaceInfo(char *szTarget, int nLen, char **pszOut, int *pnOutLen)
{
	if( !IsAvailable() || m_sRep == NULL )
	{
		return false;
	}

	//	From Here 2003.05.03 �����
	// nLen���O���ƁABSubst()���u���Ɏ��s���Ă��܂��̂ŁA��p�f�[�^(tmpBuf)���g��
	if( nLen == 0 ) {
		szTarget = tmpBuf;
		nLen = 1;
	}
	//	To Here 2003.05.03 �����

	int result = BSubst( NULL, szTarget, szTarget + nLen, &m_sRep, m_szMsg );

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] )
	{
		// �t�����ւ̒u�����ČĂяo�����l�����āA�R���p�C���o�b�t�@�̓N���A���Ȃ� by �����
//		ReleaseCompileBuffer();
		return false;
	}

	if( !result ) {
		// �u��������̂��Ȃ�����
		return false;
	}

	if( m_sRep->outp != NULL && m_sRep->outp[0] != '\0' )
	{
		int i = strlen(m_sRep->outp);
		*pszOut = new char[i+1];
		memcpy( *pszOut, m_sRep->outp, i );
		(*pszOut)[i] = '\0';
		*pnOutLen = i;
	}
	else
	{
		*pszOut = new char[1];
		(*pszOut)[0] = '\0';
		*pnOutLen = 0;
	}

	return true;
}
//>> 2002/03/27 Azumaiya

/*[EOF]*/
