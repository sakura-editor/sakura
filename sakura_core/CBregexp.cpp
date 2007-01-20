/*!	@file
	@brief BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
	@date 2002/2/1 hor		ReleaseCompileBuffer��K�X�ǉ�
	@date Jul. 25, 2002 genta �s���������l�������������s���悤�ɁD(�u���͂܂�)
	@date 2003.05.22 ����� ���K�Ȑ��K�\���ɋ߂Â���
	@date 2005.03.19 ����� ���t�@�N�^�����O�B�N���X�������B���B
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2002, novice, hor, Azumaiya
	Copyright (C) 2003, �����
	Copyright (C) 2005, �����
	Copyright (C) 2006, �����
	Copyright (C) 2007, ryoji

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

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CBregexp.h"
#include "etc_uty.h"

// Compile���A�s���u��(len=0)�̎��Ƀ_�~�[������(�P�ɓ���) by �����
const char CBregexp::m_tmpBuf[2] = "\0";

CBregexp::CBregexp() : m_pRegExp( NULL ),
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
int CBregexp::InitDll(void)
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
	// 2003.11.01 ����� �g�������֐��̃A�h���X�擾
	BMatchEx = (BREGEXP_BMatchEx)GetProcAddress( GetInstance(), "BMatchEx" );
	BSubstEx = (BREGEXP_BSubstEx)GetProcAddress( GetInstance(), "BSubstEx" );
	
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

/*! @brief �����p�^�[��������̌����p�^�[�����`�F�b�N����
**
** @param[in] szPattern �����p�^�[��
**
** @retval �����p�^�[��������
** 
** @date 2005.03.20 ����� �֐��ɐ؂�o��
*/
int CBregexp::CheckPattern(const char *szPattern)
{
	static const char TOP_MATCH[] = "/^\\(*\\^/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const char DOL_MATCH[] = "/\\\\\\$$/k";							//!< \$(�s���p�^�[���łȂ�)�`�F�b�N�p�p�^�[��
	static const char BOT_MATCH[] = "/\\$\\)*$/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const char TAB_MATCH[] = "/^\\(*\\^\\$\\)*$/k";					//!< "^$"�p�^�[�������`�F�b�N�p�p�^�[��
	static const char LOOKAHEAD[] = "/\\(\\?[=]/k";							//!< "(?=" ��ǂ� �̑��݃`�F�b�N�p�^�[��
	BREGEXP*	sReg = NULL;					//!< �R���p�C���\����
	char szMsg[80] = "";						//!< �G���[���b�Z�[�W
	int nLen;									//!< �����p�^�[���̒���
	const char *szPatternEnd;					//!< �����p�^�[���̏I�[

	m_ePatType = PAT_NORMAL;	//!<�@�m�[�}���͊m��
	nLen = strlen( szPattern );
	szPatternEnd = szPattern + nLen;
	// �p�^�[����ʂ̐ݒ�
	if( BMatch( TOP_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// �s���p�^�[���Ƀ}�b�`����
		m_ePatType |= PAT_TOP;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( TAB_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// �s���s���p�^�[���Ƀ}�b�`����
		m_ePatType |= PAT_TAB;
	}
	BRegfree(sReg);
	sReg = NULL;
	if( BMatch( DOL_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
		// �s����\$ �Ƀ}�b�`����
		// PAT_NORMAL
	} else {
		BRegfree(sReg);
		sReg = NULL;
		if( BMatch( BOT_MATCH, szPattern, szPatternEnd, &sReg, szMsg ) > 0 ) {
			// �s���p�^�[���Ƀ}�b�`����
			m_ePatType |= PAT_BOTTOM;
		} else {
			// ���̑�
			// PAT_NORMAL
		}
	}
	BRegfree(sReg);
	sReg = NULL;
	
	if( BMatch( LOOKAHEAD, szPattern, szPattern + nLen, &sReg, szMsg ) > 0 ) {
		// ��ǂ݃p�^�[���Ƀ}�b�`����
		m_ePatType |= PAT_LOOKAHEAD;
	}
	BRegfree(sReg);
	sReg = NULL;
	return (nLen);
}

/*! @brief ���C�u�����ɓn�����߂̌����E�u���p�^�[�����쐬����
**
** @note szPattern2: == NULL:���� != NULL:�u��
** 
** @param[in] szPattern �����p�^�[��
** @param[in] szPattern2 �u���p�^�[��(NULL�Ȃ猟��)
** @param[in] szAdd2	�u���p�^�[���̌��ɕt��������p�^�[��($1�Ȃ�) 
** @param[in] nOption	�����I�v�V����
**
** @retval ���C�u�����ɓn�������p�^�[���ւ̃|�C���^��Ԃ�
** @note �Ԃ��|�C���^�́A�Ăяo������ delete ���邱��
** 
** @date 2003.05.03 ����� �֐��ɐ؂�o��
*/
char* CBregexp::MakePatternSub( const char* szPattern, const char* szPattern2, const char* szAdd2, int nOption ) 
{
	static const char DELIMITER = '\xFF';		//!< �f���~�^
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
	// 2006.01.22 ����� �_���t�Ȃ̂� bIgnoreCase -> optCaseSensitive�ɕύX
	if( !(nOption & optCaseSensitive) ) {		// 2002/2/1 hor IgnoreCase �I�v�V�����ǉ� �}�[�W�Faroka
		*pPat++ = 'i';		// �啶���������𓯈ꎋ(����)����
	}
	// 2006.01.22 ����� �s�P�ʒu���̂��߂ɁA�S��I�v�V�����ǉ�
	if( (nOption & optGlobal) ) {
		*pPat++ = 'g';			// �S��(global)�I�v�V�����A�s�P�ʂ̒u�������鎞�Ɏg�p����
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
** @param[in] szPattern �����p�^�[��
** @param[in] szPattern2 �u���p�^�[��(NULL�Ȃ猟��)
** @param[in] nOption �����I�v�V����
**
** @retval ���C�u�����ɓn�������p�^�[���ւ̃|�C���^��Ԃ�
** @note �Ԃ��|�C���^�́A�Ăяo������ delete ���邱��
**
** @date 2003.05.03 ����� �֐��ɐ؂�o��
*/
char* CBregexp::MakePattern( const char* szPattern, const char* szPattern2, int nOption ) 
{
	static const char CRLF[] = "\r\n";			//!< ���A�E���s
	static const char CR[] = "\r";				//!< ���A
	static const char LF[] = "\n";				//!< ���s
	static const char LFCR[] = "\n\r";			//!< ���s�E���A
	static const char BOT_SUBST[] = "s/\\$(\\)*)$/([\\\\r\\\\n]+)\\$$1/k";	//!< �s���p�^�[���̒u���p�p�^�[��
	int nLen;									//!< szPattern�̒���
	BREGEXP*	sReg = NULL;					//!< �R���p�C���\����
	char szMsg[80] = "";						//!< �G���[���b�Z�[�W
	char szAdd2[5] = "";						//!< �s������u���� $���� �i�[�p
	int nParens = 0;							//!< �����p�^�[��(szPattern)���̊��ʂ̐�(�s�����Ɏg�p)
	char *szNPattern;							//!< �����p�^�[��

	nLen = CheckPattern( szPattern );
	if( (m_ePatType & PAT_BOTTOM) != 0 ) {
		bool bJustDollar = false;			// �s���w���$�݂̂ł���t���O($�̑O�� \r\n���w�肳��Ă��Ȃ�)
		szNPattern = MakePatternSub(szPattern, NULL, NULL, nOption);
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

	szNPattern = MakePatternSub( szPattern, szPattern2, szAdd2, nOption );
	if( sReg != NULL ) {
		BRegfree(sReg);
	}
	return szNPattern;
}


/*!
	JRE32�̃G�~�����[�V�����֐��D��̕�����ɑ΂��Č����E�u�����s�����Ƃɂ��
	BREGEXP�\���̂̐����݂̂��s���D

	@param[in] szPattern0	����or�u���p�^�[��
	@param[in] szPattern1	�u���㕶����p�^�[��(��������NULL)
	@param[in] nOption		�����E�u���I�v�V����

	@retval true ����
	@retval false ���s
*/
bool CBregexp::Compile( const char *szPattern0, const char *szPattern1, int nOption )
{

	//	DLL�����p�\�łȂ��Ƃ��̓G���[�I��
	if( !IsAvailable() )
		return false;

	//	BREGEXP�\���̂̉��
	ReleaseCompileBuffer();

	// ���C�u�����ɓn�������p�^�[�����쐬
	// �ʊ֐��ŋ��ʏ����ɕύX 2003.05.03 by �����
	char *szNPattern = MakePattern( szPattern0, szPattern1, nOption );
	m_szMsg[0] = '\0';		//!< �G���[����
	if (szPattern1 == NULL) {
		// �������s
		BMatch( szNPattern, m_tmpBuf, m_tmpBuf+1, &m_pRegExp, m_szMsg );
	} else {
		// �u�����s
		BSubst( szNPattern, m_tmpBuf, m_tmpBuf+1, &m_pRegExp, m_szMsg );
	}
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

	@param[in] target �����Ώۗ̈�擪�A�h���X
	@param[in] len �����Ώۗ̈�T�C�Y
	@param[in] nStart �����J�n�ʒu�D(�擪��0)

	@retval true Match
	@retval false No Match �܂��� �G���[�B�G���[�� GetLastMessage()�ɂ�蔻��\�B

*/
bool CBregexp::Match( const char* target, int len, int nStart )
{
	int matched;		//!< ������v������? >0:Match, 0:NoMatch, <0:Error

	//	DLL�����p�\�łȂ��Ƃ��A�܂��͍\���̂����ݒ�̎��̓G���[�I��
	if( (!IsAvailable()) || m_pRegExp == NULL ){
		return false;
	}

	m_szMsg[0] = '\0';		//!< �G���[����
	// �g���֐����Ȃ��ꍇ�́A�s�̐擪("^")�̌������̓��ʏ��� by �����
	if (BMatchEx == NULL) {
		/*
		** �s��(^)�ƃ}�b�`����̂́AnStart=0�̎������Ȃ̂ŁA����ȊO�� false
		*/
		if( (m_ePatType & PAT_TOP) != 0 && nStart != 0 ) {
			// nStart!=0�ł��ABMatch()�ɂƂ��Ă͍s���ɂȂ�̂ŁA������false�ɂ���K�v������
			return false;
		}
		//	����������NULL���w�肷��ƑO��Ɠ���̕�����ƌ��Ȃ����
		matched = BMatch( NULL, target + nStart, target + len, &m_pRegExp, m_szMsg );
	} else {
		//	����������NULL���w�肷��ƑO��Ɠ���̕�����ƌ��Ȃ����
		matched = BMatchEx( NULL, target, target + nStart, target + len, &m_pRegExp, m_szMsg );
	}
	m_szTarget = target;
			
	if ( matched < 0 || m_szMsg[0] ) {
		// BMatch�G���[
		// �G���[���������Ă��Ȃ������̂ŁAnStart>=len�̂悤�ȏꍇ�ɁA�}�b�`�����ɂȂ�
		// �����u�����̕s��ɂȂ��Ă��� 2003.05.03 by �����
		return false;
	} else if ( matched == 0 ) {
		// ��v���Ȃ�����
		return false;
	}

	return true;
}


//<< 2002/03/27 Azumaiya
/*!
	���K�\���ɂ�镶����u��
	���ɂ���R���p�C���\���̂𗘗p���Ēu���i1�s�j��
	�s���D

	@param[in] szTarget �u���Ώۃf�[�^
	@param[in] nLen �u���Ώۃf�[�^��
	@param[in] nStart �u���J�n�ʒu(0����nLen����)

	@retval �u����

	@date	2007.01.16 ryoji �߂�l��u�����ɕύX
*/
int CBregexp::Replace(const char *szTarget, int nLen, int nStart)
{
	int result;
	//	DLL�����p�\�łȂ��Ƃ��A�܂��͍\���̂����ݒ�̎��̓G���[�I��
	if( !IsAvailable() || m_pRegExp == NULL )
	{
		return false;
	}

	//	From Here 2003.05.03 �����
	// nLen���O���ƁABSubst()���u���Ɏ��s���Ă��܂��̂ŁA��p�f�[�^(m_tmpBuf)���g��
	//
	// 2007.01.19 ryoji ��p�f�[�^�g�p���R�����g�A�E�g
	// �g�p����ƌ���ł͌��ʂɂP�o�C�g�]���ȃS�~���t�������
	// �u���Ɏ��s����̂�nLen���O�Ɍ��炸 nLen = nStart �̂Ƃ��i�s���}�b�`�����΍􂵂Ă��D�D�D�j
	//
	//if( nLen == 0 ) {
	//	szTarget = m_tmpBuf;
	//	nLen = 1;
	//}
	//	To Here 2003.05.03 �����

	m_szMsg[0] = '\0';		//!< �G���[����
	if (BSubstEx == NULL) {
		result = BSubst( NULL, szTarget + nStart, szTarget + nLen, &m_pRegExp, m_szMsg );
	} else {
		result = BSubstEx( NULL, szTarget, szTarget + nStart, szTarget + nLen, &m_pRegExp, m_szMsg );
	}
	m_szTarget = szTarget;

	//	���b�Z�[�W���󕶎���łȂ���Ή��炩�̃G���[�����B
	//	�T���v���\�[�X�Q��
	if( m_szMsg[0] ) {
		return 0;
	}

	if( result < 0 ) {
		// �u��������̂��Ȃ�����
		return 0;
	}
	return result;
}
//>> 2002/03/27 Azumaiya

/*[EOF]*/
