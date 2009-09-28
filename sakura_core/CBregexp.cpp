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
#include <string>
#include <stdio.h>
#include <string.h>
#include "CBregexp.h"
#include "charset/charcode.h"
#include "env/CShareData.h"


// Compile���A�s���u��(len=0)�̎��Ƀ_�~�[������(�P�ɓ���) by �����
const wchar_t CBregexp::m_tmpBuf[2] = L"\0";





CBregexp::CBregexp()
: m_pRegExp( NULL )
, m_ePatType( PAT_NORMAL )	//	Jul, 25, 2002 genta
{
	m_szMsg[0] = L'\0';
}

CBregexp::~CBregexp()
{
	//�R���p�C���o�b�t�@�����
	ReleaseCompileBuffer();
}


/*! @brief �����p�^�[��������̌����p�^�[�����`�F�b�N����
**
** @param[in] szPattern �����p�^�[��
**
** @retval �����p�^�[��������
** 
** @date 2005.03.20 ����� �֐��ɐ؂�o��
*/
int CBregexp::CheckPattern(const wchar_t* szPattern)
{
	static const wchar_t TOP_MATCH[] = L"/^\\(*\\^/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const wchar_t DOL_MATCH[] = L"/\\\\\\$$/k";							//!< \$(�s���p�^�[���łȂ�)�`�F�b�N�p�p�^�[��
	static const wchar_t BOT_MATCH[] = L"/\\$\\)*$/k";							//!< �s���p�^�[���̃`�F�b�N�p�p�^�[��
	static const wchar_t TAB_MATCH[] = L"/^\\(*\\^\\$\\)*$/k";					//!< "^$"�p�^�[�������`�F�b�N�p�p�^�[��
	static const wchar_t LOOKAHEAD[] = L"/\\(\\?[=]/k";							//!< "(?=" ��ǂ� �̑��݃`�F�b�N�p�^�[��
	BREGEXP_W*	sReg = NULL;					//!< �R���p�C���\����
	wchar_t szMsg[80] = L"";					//!< �G���[���b�Z�[�W
	int nLen;									//!< �����p�^�[���̒���
	const wchar_t *szPatternEnd;				//!< �����p�^�[���̏I�[

	m_ePatType = PAT_NORMAL;	//!<�@�m�[�}���͊m��
	nLen = wcslen( szPattern );
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
** @retval ���C�u�����ɓn�������p�^�[���ւ̃|�C���^��Ԃ�
** @note �Ԃ��|�C���^�́A�Ăяo������ delete ���邱��
** 
** @date 2003.05.03 ����� �֐��ɐ؂�o��
*/
wchar_t* CBregexp::MakePatternSub(
	const wchar_t*	szPattern,	//!< �����p�^�[��
	const wchar_t*	szPattern2,	//!< �u���p�^�[��(NULL�Ȃ猟��)
	const wchar_t*	szAdd2,		//!< �u���p�^�[���̌��ɕt��������p�^�[��($1�Ȃ�) 
	int				nOption		//!< �����I�v�V����
) 
{
	static const wchar_t DELIMITER = WCODE::BREGEXP_DELIMITER;	//!< �f���~�^
	int nLen;									//!< szPattern�̒���
	int nLen2;									//!< szPattern2 + szAdd2 �̒���

	// �����p�^�[���쐬
	wchar_t *szNPattern;		//!< ���C�u�����n���p�̌����p�^�[��������
	wchar_t *pPat;				//!< �p�^�[�������񑀍�p�̃|�C���^

	nLen = wcslen(szPattern);
	if (szPattern2 == NULL) {
		// ����(BMatch)��
		szNPattern = new wchar_t[ nLen + 15 ];	//	15�F�us///option�v���]�T�ł͂���悤�ɁB
		pPat = szNPattern;
		*pPat++ = L'm';
	}
	else {
		// �u��(BSubst)��
		nLen2 = wcslen(szPattern2) + wcslen(szAdd2);
		szNPattern = new wchar_t[ nLen + nLen2 + 15 ];
		pPat = szNPattern;
		*pPat++ = L's';
	}
	*pPat++ = DELIMITER;
	while (*szPattern != L'\0') { *pPat++ = *szPattern++; }
	*pPat++ = DELIMITER;
	if (szPattern2 != NULL) {
		while (*szPattern2 != L'\0') { *pPat++ = *szPattern2++; }
		while (*szAdd2 != L'\0') { *pPat++ = *szAdd2++; }
		*pPat++ = DELIMITER;
	}
	*pPat++ = L'k';			// �����Ή�
	*pPat++ = L'm';			// �����s�Ή�(�A���A�Ăяo�����������s�Ή��łȂ�)
	// 2006.01.22 ����� �_���t�Ȃ̂� bIgnoreCase -> optCaseSensitive�ɕύX
	if( !(nOption & optCaseSensitive) ) {		// 2002/2/1 hor IgnoreCase �I�v�V�����ǉ� �}�[�W�Faroka
		*pPat++ = L'i';		// �啶���������𓯈ꎋ(����)����
	}
	// 2006.01.22 ����� �s�P�ʒu���̂��߂ɁA�S��I�v�V�����ǉ�
	if( (nOption & optGlobal) ) {
		*pPat++ = L'g';			// �S��(global)�I�v�V�����A�s�P�ʂ̒u�������鎞�Ɏg�p����
	}
	*pPat = L'\0';
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
wchar_t* CBregexp::MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption ) 
{
	using namespace WCODE;
	static const wchar_t* szCRLF = CRLF;		//!< ���A�E���s
	static const wchar_t szCR[] = {CR,0};				//!< ���A
	static const wchar_t szLF[] = {LF,0};				//!< ���s
	static const wchar_t BOT_SUBST[] = L"s/\\$(\\)*)$/([\\\\r\\\\n]+)\\$$1/k";	//!< �s���p�^�[���̒u���p�p�^�[��
	int nLen;									//!< szPattern�̒���
	BREGEXP_W*	sReg = NULL;					//!< �R���p�C���\����
	wchar_t szMsg[80] = L"";						//!< �G���[���b�Z�[�W
	wchar_t szAdd2[5] = L"";						//!< �s������u���� $���� �i�[�p
	int nParens = 0;							//!< �����p�^�[��(szPattern)���̊��ʂ̐�(�s�����Ɏg�p)
	wchar_t *szNPattern;							//!< �����p�^�[��

	nLen = CheckPattern( szPattern );
	if( (m_ePatType & PAT_BOTTOM) != 0 ) {
		bool bJustDollar = false;			// �s���w���$�݂̂ł���t���O($�̑O�� \r\n���w�肳��Ă��Ȃ�)
		szNPattern = MakePatternSub(szPattern, NULL, NULL, nOption);
		int matched = BMatch( szNPattern, szCRLF, szCRLF+wcslen(szCRLF), &sReg, szMsg );
		if( matched >= 0 ) {
			// szNPattern���s���ȃp�^�[�����̃G���[�łȂ�����
			// �G���[���ɂ� sReg��NULL�̂܂܂Ȃ̂ŁAsReg->nparens�ւ̃A�N�Z�X�͕s��
			nParens = sReg->nparens;			// �I���W�i���̌��������񒆂�()�̐����L��
			if( matched > 0 ) {
				if( sReg->startp[0] == &szCRLF[1] && sReg->endp[0] == &szCRLF[1] ) {
					if( BMatch( NULL, szCR, szCR+wcslen(szCR), &sReg, szMsg ) > 0 && sReg->startp[0] == &szCR[1] && sReg->endp[0] == &szCR[1] ) {
						if( BMatch( NULL, szLF, szLF+wcslen(szLF), &sReg, szMsg ) > 0 && sReg->startp[0] == &szLF[0] && sReg->endp[0] == &szLF[0] ) {
							// ����������� �s��($)�݂̂�����
							bJustDollar = true;
						}
					}
				}
			} else {
				if( BMatch( NULL, szCR, szCR+wcslen(szCR), &sReg, szMsg ) <= 0 ) {
					if( BMatch( NULL, szLF, szLF+wcslen(szLF), &sReg, szMsg ) <= 0 ) {
						// ����������́A�����{�s��($)������
						bJustDollar = true;
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
					auto_sprintf( szAdd2, L"$%d", nParens + 1 );
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
	CBregexp::MakePattern()�̑�ցB
	* �G�X�P�[�v����Ă��炸�A�����W���̒��ɂ��Ȃ� . �� [^\r\n] �ɒu������B
	* �G�X�P�[�v����Ă��炸�A�����W���̒��ɂ��Ȃ� $ �� (?<![\r\n])(?=\r|$) �ɒu������B
	����́u���s�v�̈Ӗ��� LF �̂�(BREGEXP.DLL�̎d�l)����ACR, LF, CRLF �Ɋg�����邽�߂̕ύX�ł���B
	�܂��A$ �͉��s�̌��A�s�����񖖔��Ƀ}�b�`���Ȃ��Ȃ�B�Ō�̈�s�̏ꍇ���̂����āA
	���K�\��DLL�ɗ^�����镶����̖����͕������Ƃ͂������A$ ���}�b�`����K�v�͂Ȃ����낤�B
	$ ���s�����񖖔��Ƀ}�b�`���Ȃ����Ƃ́A�ꊇ�u���ł̊��҂��Ȃ��u����h�����߂ɕK�v�ł���B
*/
wchar_t* CBregexp::MakePatternAlternate( const wchar_t* const szSearch, const wchar_t* const szReplace, int nOption )
{
	this->CheckPattern( szSearch );

	const bool nestedBracketIsAllowed = true;
	const wchar_t szDotAlternative[] = L"[^\\r\\n]";
	const wchar_t szDollarAlternative[] = L"(?<![\\r\\n])(?=\\r|$)";

	// ���ׂĂ� . �� [^\r\n] �ցA���ׂĂ� $ �� (?<![\r\n])(?=\r|$) �֒u������Ɖ��肵�āAstrModifiedSearch�̍ő咷�����肷��B
	std::wstring::size_type modifiedSearchSize = 0;
	for( const wchar_t* p = szSearch; *p; ++p ) {
		if( *p == L'.') {
			modifiedSearchSize += (sizeof szDotAlternative) / (sizeof szDotAlternative[0]) - 1;
		} else if( *p == L'$' ) {
			modifiedSearchSize += (sizeof szDollarAlternative) / (sizeof szDollarAlternative[0]) - 1;
		} else {
			modifiedSearchSize += 1;
		}
	}
	++modifiedSearchSize; // '\0'

	std::wstring strModifiedSearch;
	strModifiedSearch.reserve( modifiedSearchSize );

	// szSearch�� strModifiedSearch�ցA�Ƃ���ǂ���u�����Ȃ��珇���R�s�[���Ă����B
	struct Sequence {
		enum State { None = 0, Escaped, SmallC } state;
		Sequence() : state( None ) {}
		bool EatCharacter( const wchar_t ch )
		{
			const wchar_t acceptChars[] = { L'\\', L'c', 0 };
			const wchar_t acceptChar = acceptChars[this->state];
			if( acceptChar && acceptChar == ch ) {
				// ����̕�����H�ׂĎ��̏�ԂցB
				this->state =  State( state + 1 );
				return true;
			} else if( this->state == Escaped || this->state == SmallC ) {
				// ���ł��ꕶ������ďI���B
				this->state = None;
				return true;
			}
			return false; // �֌W�Ȃ������������B
		}
	} seq;
	int charsetLevel = 0; // �u���P�b�g�̐[���BPOSIX�u���P�b�g�\���ȂǁA�G�X�P�[�v����Ă��Ȃ� [] ������q�ɂȂ邱�Ƃ�����B
	const wchar_t *left = szSearch, *right = szSearch;
	for( ; *right; ++right ) { // CNativeW::GetSizeOfChar()�͎g��Ȃ��Ă��������ȁH
		if( seq.EatCharacter( *right ) ) {
			// (�u���P�b�g�̓��O�ŗL����)�G�X�P�[�v�V�[�N�G���X( \X, \cX )�̈ꕔ�������B
		} else if( *right == L'[' ) { // �S�Ԃł͕����W���̒��� [ �� POSIX�u���P�b�g�\���Ȃǂ̈Ӗ������̂ŁA�K���G�X�P�[�v����Ă���B����̓G�X�P�[�v����Ă��Ȃ��̂ŁA�������Ń��x���𑝂��B
			++charsetLevel;
		} else if( *right == L']' ) {
			if( 0 < charsetLevel ) { // �����W���̊O�̃G�X�P�[�v����Ă��Ȃ� ] �͍��@�Ȃ̂ōl������B
				charsetLevel -= nestedBracketIsAllowed ? 1 : charsetLevel;
			}
		} else {
			if( *right == L'.' && charsetLevel == 0 ) {
				strModifiedSearch.append( left, right );
				left = right + 1;
				strModifiedSearch.append( szDotAlternative );
			} else if( *right == L'$' && charsetLevel == 0 ) {
				strModifiedSearch.append( left, right );
				left = right + 1;
				strModifiedSearch.append( szDollarAlternative );
			}
		}
	}
	strModifiedSearch.append( left, right + 1 ); // right + 1 �� '\0' �̎����w��(�����I�� '\0' ���R�s�[)�B

	return this->MakePatternSub( strModifiedSearch.c_str(), szReplace, L"", nOption );
}


/*!
	JRE32�̃G�~�����[�V�����֐��D��̕�����ɑ΂��Č����E�u�����s�����Ƃɂ��
	BREGEXP_W�\���̂̐����݂̂��s���D

	@param[in] szPattern0	����or�u���p�^�[��
	@param[in] szPattern1	�u���㕶����p�^�[��(��������NULL)
	@param[in] nOption		�����E�u���I�v�V����

	@retval true ����
	@retval false ���s
*/
bool CBregexp::Compile( const wchar_t *szPattern0, const wchar_t *szPattern1, int nOption )
{

	//	DLL�����p�\�łȂ��Ƃ��̓G���[�I��
	if( !IsAvailable() )
		return false;

	//	BREGEXP_W�\���̂̉��
	ReleaseCompileBuffer();

	// ���C�u�����ɓn�������p�^�[�����쐬
	// �ʊ֐��ŋ��ʏ����ɕύX 2003.05.03 by �����
	wchar_t *szNPattern = MakePatternAlternate( szPattern0, szPattern1, nOption );
	m_szMsg[0] = L'\0';		//!< �G���[����
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
bool CBregexp::Match( const wchar_t* target, int len, int nStart )
{
	int matched;		//!< ������v������? >0:Match, 0:NoMatch, <0:Error

	//	DLL�����p�\�łȂ��Ƃ��A�܂��͍\���̂����ݒ�̎��̓G���[�I��
	if( (!IsAvailable()) || m_pRegExp == NULL ){
		return false;
	}

	m_szMsg[0] = '\0';		//!< �G���[����
	// �g���֐����Ȃ��ꍇ�́A�s�̐擪("^")�̌������̓��ʏ��� by �����
	if (!ExistBMatchEx()) {
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
int CBregexp::Replace(const wchar_t *szTarget, int nLen, int nStart)
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
	if (!ExistBSubstEx()) {
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


const TCHAR* CBregexp::GetLastMessage() const
{
	return to_tchar(m_szMsg);
}












//	From Here Jun. 26, 2001 genta
/*!
	�^����ꂽ���K�\�����C�u�����̏��������s���D
	���b�Z�[�W�t���O��ON�ŏ������Ɏ��s�����Ƃ��̓��b�Z�[�W��\������D

	@retval true ����������
	@retval false �������Ɏ��s

	@date 2007.08.12 genta ���ʐݒ肩��DLL�����擾����
*/
bool InitRegexp(
	HWND		hWnd,			//!< [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	CBregexp&	rRegexp,		//!< [in] �`�F�b�N�ɗ��p����CBregexp�N���X�ւ̎Q��
	bool		bShowMessage	//!< [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
)
{
	//	From Here 2007.08.12 genta
	CShareData* pInstance = NULL;
	DLLSHAREDATA* pShareData = NULL;
	
	LPCTSTR RegexpDll = _T("");
	
	if( (pInstance = CShareData::getInstance()) && (pShareData = pInstance->GetShareData()) ){
		RegexpDll = pShareData->m_Common.m_sSearch.m_szRegexpLib;
	}
	//	To Here 2007.08.12 genta

	EDllResult eDllResult = rRegexp.InitDll(RegexpDll);
	if( DLL_SUCCESS != eDllResult && bShowMessage ){
		LPCTSTR pszMsg = _T("");
		if(eDllResult==DLL_LOADFAILURE){
			pszMsg =
				_T("BREGONIG.DLL �̃��[�h�Ɏ��s���܂����B\r\n")
				_T("���K�\���𗘗p����ɂ� UNICODE �ł� BREGONIG.DLL ���K�v�ł��B\r\n")
				_T("������@�̓w���v���Q�Ƃ��Ă��������B");
		}
		else if(eDllResult==DLL_INITFAILURE){
			pszMsg =
				_T("BREGONIG.DLL �̗��p�Ɏ��s���܂����B\r\n")
				_T("���K�\���𗘗p����ɂ� UNICODE �ł� BREGONIG.DLL ���K�v�ł��B\r\n")
				_T("������@�̓w���v���Q�Ƃ��Ă��������B");
		}
		else{
			pszMsg =
				_T("BREGONIG.DLL �̃��[�h�ŗ\�����ʃG���[���������܂����B");
			assert(0);
		}
		WarningBeep();
		::MessageBox( hWnd, pszMsg, _T("���"), MB_OK | MB_ICONEXCLAMATION );
		return false;
	}
	return true;
}

/*!
	���K�\�����C�u�����̑��݂��m�F���A����΃o�[�W���������w��R���|�[�l���g�ɃZ�b�g����B
	���s�����ꍇ�ɂ͋󕶎�����Z�b�g����B

	@retval true �o�[�W�����ԍ��̐ݒ�ɐ���
	@retval false ���K�\�����C�u�����̏������Ɏ��s
*/
bool CheckRegexpVersion(
	HWND	hWnd,			//!< [in] �_�C�A���O�{�b�N�X�̃E�B���h�E�n���h���B�o�[�W�����ԍ��̐ݒ肪�s�v�ł����NULL�B
	int		nCmpId,			//!< [in] �o�[�W�����������ݒ肷��R���|�[�l���gID
	bool	bShowMessage	//!< [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
)
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		if( hWnd != NULL ){
			::DlgItem_SetText( hWnd, nCmpId, _T(" "));
		}
		return false;
	}
	if( hWnd != NULL ){
		::DlgItem_SetText( hWnd, nCmpId, cRegexp.GetVersionT() );
	}
	return true;
}

/*!
	���K�\�����K���ɏ]���Ă��邩���`�F�b�N����B

	@param szPattern [in] �`�F�b�N���鐳�K�\��
	@param hWnd [in] ���b�Z�[�W�{�b�N�X�̐e�E�B���h�E
	@param bShowMessage [in] ���������s���ɃG���[���b�Z�[�W���o���t���O
	@param nOption [in] �啶���Ə������𖳎����Ĕ�r����t���O // 2002/2/1 hor�ǉ�

	@retval true ���K�\���͋K���ʂ�
	@retval false ���@�Ɍ�肪����B�܂��́A���C�u�������g�p�ł��Ȃ��B
*/
bool CheckRegexpSyntax(
	const wchar_t*	szPattern,
	HWND			hWnd,
	bool			bShowMessage,
	int				nOption )
{
	CBregexp cRegexp;

	if( !InitRegexp( hWnd, cRegexp, bShowMessage ) ){
		return false;
	}
	if( !cRegexp.Compile( szPattern, nOption ) ){	// 2002/2/1 hor�ǉ�
		if( bShowMessage ){
			::MessageBox( hWnd, cRegexp.GetLastMessage(),
				_T("���K�\���G���["), MB_OK | MB_ICONEXCLAMATION );
		}
		return false;
	}
	return true;
}
//	To Here Jun. 26, 2001 genta
