/*!	@file
	@brief BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
	@date Jan. 05, 2002 genta �R�����g�ǉ�
	@date 2002/2/1 hor
	@date Jul. 25, 2002 genta �s�������̍l����ǉ�
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, novice, hor
	Copyright (C) 2002, novice, hor, Azumaiya
	Copyright (C) 2003, �����
	Copyright (C) 2005, �����, aroka
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

#ifndef _DLL_BREGEXP_H_
#define _DLL_BREGEXP_H_

#include "CDllHandler.h"

/*!
	BREGEXP ���K�\���̃R���p�C�����ʂ�ێ�����\����
*/
typedef struct bregexp {
	const char *outp;	//!< BSubst �u���f�[�^�̐擪�|�C���^
	const char *outendp;	//!< BSubst �u���f�[�^�̍ŏI�|�C���^+1
	int  splitctr;	//!< BSplit �z��
	const char **splitp;	//!< BSplit �f�[�^�|�C���^
	int	rsv1;		//!< ���U�[�u ���R�Ɏg�p�\
	char *parap;		//!< �p�^�[���f�[�^�|�C���^
	char *paraendp;		//!< �p�^�[���f�[�^�|�C���^+1
	char *transtblp;	//!< BTrans �ϊ��e�[�u���|�C���^
	char **startp;		//!< �}�b�`�����f�[�^�̐擪�|�C���^
	char **endp;		//!< �}�b�`�����f�[�^�̍ŏI�|�C���^+1
	int nparens;		//!< �p�^�[���̒���() �̐��B $1,$2, �𒲂ׂ�Ƃ��Ɏg�p
} BREGEXP;

/*!
	@brief Perl�݊����K�\�� BREGEXP.DLL ���T�|�[�g����N���X

	DLL�̓��I���[�h���s�����߁ADllHandler���p�����Ă���B

	CJre�ɋ߂�����������邽�߁A�o�b�t�@���N���X����1�ێ����A
	�f�[�^�̐ݒ�ƌ����̂Q�̃X�e�b�v�ɕ�������悤�ɂ��Ă���B
	Jre�G�~�����[�V�����֐����g���Ƃ��͓���q�ɂȂ�Ȃ��悤�ɒ��ӂ��邱�ƁB

	�{���͂��̂悤�ȕ����͕ʃN���X�Ƃ��ĕ������ׂ������A���̏ꍇ���̃N���X��
	�j�������O�ɑS�ẴN���X��j������K�v������B
	���̈��S����ۏ؂���̂�������߁A�����_�ł͗��҂��P�̃N���X�ɓ��ꂽ�B

	@note ���̃N���X��Thread Safe�ł͂Ȃ��B

	@date 2005.03.19 ����� ���t�@�N�^�����O�B�N���X�������B��
	@date 2006.01.22 ����� �I�v�V�����ǉ��E���̕ύX(�S�čs�u���pGlobal�I�v�V�����ǉ��̂���)
*/
class SAKURA_CORE_API CBregexp : public CDllHandler {
public:
	CBregexp();
	virtual ~CBregexp();

	// 2006.01.22 ����� �I�v�V�����ǉ��E���̕ύX
	enum Option {
		optNothing = 0,					//!< �I�v�V�����Ȃ�
		optCaseSensitive = 1,			//!< �啶����������ʃI�v�V����(/i�����Ȃ�)
		optGlobal = 2					//!< �S��I�v�V����(/g)
	};
	//! �����p�^�[����`
	enum Pattern {
		PAT_UNKNOWN = 0,		//!< �s���i�����l)
		PAT_NORMAL = 1,			//!< �ʏ�
		PAT_TOP = 2,			//!< �s��"^"
		PAT_BOTTOM = 4,			//!< �s��"$"
		PAT_TAB = 8,			//!< �s���s��"^$"
		PAT_LOOKAHEAD = 16		//!< ��ǂ�"(?[=]"
	};

	const char* GetVersion(){		//!< DLL�̃o�[�W���������擾
		return IsAvailable() ? BRegexpVersion() : "";
	}

	//	CJre�G�~�����[�V�����֐�
	//!	�����p�^�[���̃R���p�C��
	// 2002/01/19 novice ���K�\���ɂ�镶����u��
	// 2002.01.26 hor    �u���㕶�����ʈ�����
	// 2002.02.01 hor    �啶���������𖳎�����I�v�V�����ǉ�
	//>> 2002/03/27 Azumaiya ���K�\���u���ɃR���p�C���֐����g���`����ǉ�
	bool Compile(const char *szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const char *szPattern0, const char *szPattern1, int nOption = 0);	//!< Replace�p
	bool Match(const char *szTarget, int nLen, int nStart = 0);						//!< ���������s����
	int Replace(const char *szTarget, int nLen, int nStart = 0);					//!< �u�������s����	// 2007.01.16 ryoji �߂�l��u�����ɕύX

	//-----------------------------------------
	// 2005.03.19 ����� �N���X�������B��
	/*! @{
		@name ���ʂ𓾂郁�\�b�h��ǉ����ABREGEXP���O������B��
	*/
	/*!
	    �����Ɉ�v����������̐擪�ʒu��Ԃ�(������擪�Ȃ�0)
		@retval �����Ɉ�v����������̐擪�ʒu
	*/
	int GetIndex(void) {
		return m_pRegExp->startp[0] - m_szTarget;
	}
	/*!
	    �����Ɉ�v����������̎��̈ʒu��Ԃ�
		@retval �����Ɉ�v����������̎��̈ʒu
	*/
	int GetLastIndex(void) {
		return m_pRegExp->endp[0] - m_szTarget;
	}
	/*!
		�����Ɉ�v����������̒�����Ԃ�
		@retval �����Ɉ�v����������̒���
	*/
	int GetMatchLen(void) {
		return m_pRegExp->endp[0] - m_pRegExp->startp[0];
	}
	/*!
		�u�����ꂽ������̒�����Ԃ�
		@retval �u�����ꂽ������̒���
	*/
	int GetStringLen(void) {
		// �u���㕶���񂪂O���Ȃ� outp�Aoutendp��NULL�ɂȂ�
		// NULL�|�C���^�̈����Z�͖��Ȃ��O�ɂȂ�B
		// outendp�� '\0'�Ȃ̂ŁA�����񒷂� +1�s�v

		// Jun. 03, 2005 Karoto
		//	�u���㕶����0���̏ꍇ��outp��NULL�ł�outendp��NULL�łȂ��ꍇ������̂ŁC
		//	outp��NULL�`�F�b�N���K�v
		if (m_pRegExp->outp == NULL) {
			return 0;
		} else {
			return m_pRegExp->outendp - m_pRegExp->outp;
		}
	}
	/*!
		�u�����ꂽ�������Ԃ�
		@retval �u�����ꂽ������ւ̃|�C���^
	*/
	const char *GetString(void) {
		return m_pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXP���b�Z�[�W���擾����
		@retval ���b�Z�[�W�ւ̃|�C���^
	*/
	const char* GetLastMessage(void) const { return m_szMsg; }
	/*!	��ǂ݃p�^�[�������݂��邩��Ԃ�
		���̊֐��́A�R���p�C����ł��邱�Ƃ��O��Ȃ̂ŁA�R���p�C���O��false
		@retval true ��ǂ݂�����
		@retval false ��ǂ݂��Ȃ� ���� �R���p�C���O
	*/
	bool IsLookAhead(void) {
		return m_ePatType & PAT_LOOKAHEAD ? true : false;
	}
	/*!	�����p�^�[���ɐ�ǂ݂��܂܂�邩�H�i�R���p�C���O�ł����ʉ\�j
		@param[in] pattern �����p�^�[��
		@retval true ��ǂ݂�����
		@retval false ��ǂ݂��Ȃ�
	*/
	bool IsLookAhead(const char *pattern) {
		CheckPattern(pattern);
		return IsLookAhead();
	}

protected:
	//	Jul. 5, 2001 genta �C���^�[�t�F�[�X�ύX�ɔ��������ǉ�
	virtual char* GetDllName(char*);
	virtual int InitDll(void);
	virtual int DeinitDll(void);

	//	DLL Interface�̎󂯎M
	//	Aug. 20, 2005 Aroka : �œK���I�v�V�����Ńf�t�H���g��__fastcall�ɕύX���Ă�
	//	�e�����󂯂Ȃ��悤�ɂ���D
	typedef int (__cdecl *BREGEXP_BMatch)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BSubst)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BTrans)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (__cdecl *BREGEXP_BSplit)(const char*,char *,char *,int,BREGEXP **,char *);
	typedef void (__cdecl *BREGEXP_BRegfree)(BREGEXP*);
	typedef const char* (__cdecl *BREGEXP_BRegexpVersion)(void);
	// 2005.03.19 ����� �O����v�p�V�C���^�[�t�F�[�X
	typedef int (*BREGEXP_BMatchEx)(const char*,const char*, const char *,const char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSubstEx)(const char*,const char*, const char *,const char *,BREGEXP **,char *);

	BREGEXP_BMatch BMatch;
	BREGEXP_BSubst BSubst;
	BREGEXP_BTrans BTrans;
	BREGEXP_BSplit BSplit;
	BREGEXP_BRegfree BRegfree;
	BREGEXP_BRegexpVersion BRegexpVersion;
	BREGEXP_BMatchEx BMatchEx;
	BREGEXP_BSubstEx BSubstEx;

	//!	�R���p�C���o�b�t�@���������
	/*!
		m_pcReg��BRegfree()�ɓn���ĉ������D������NULL�ɃZ�b�g����D
		���XNULL�Ȃ牽�����Ȃ�
	*/
	void ReleaseCompileBuffer(void){
		if( m_pRegExp ){
			BRegfree( m_pRegExp );
			m_pRegExp = NULL;
		}
		m_ePatType = PAT_UNKNOWN;
	}

private:
	//	�����֐�

	//! �����p�^�[���쐬
	int CheckPattern( const char *szPattern );
	char* MakePatternSub( const char* szPattern, const char* szPattern2, const char* szAdd2, int nOption );
	char* MakePattern( const char* szPattern, const char* szPattern2, int nOption );
	//	�����o�ϐ�
	BREGEXP*			m_pRegExp;			//!< �R���p�C���\����
	int					m_ePatType;			//!< ����������p�^�[�����
	const char			*m_szTarget;		//!< �Ώە�����ւ̃|�C���^
	char				m_szMsg[80];		//!< BREGEXP����̃��b�Z�[�W��ێ�����
	// �ÓI�����o�ϐ�
	static const char	m_tmpBuf[2];		//!< �_�~�[������
};

//	�ȉ��͊֐��|�C���^�ɓǂݍ��܂��֐��̉��
/*!	@fn int CBregexp::BMatch(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option �`����Perl�݊��p�^�[���}�b�`���O���s���B

	@param str [in] ��������p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W

	target <= p < targetendp �͈̔͂������ΏۂɂȂ�B
*/
/*!	@fn int CBregexp::BMatchEx(char* str,char *targetbeg, char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option �`����Perl�݊��p�^�[���}�b�`���O���s���B

	@param str [in] ��������p�^�[��(�R���p�C���ς݂Ȃ�NULL)
	@param targetbeg [in] �����Ώە�����(�s������)
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W

	target <= p < targetendp �͈̔͂������ΏۂɂȂ�B
*/

/*!	@fn int CBregexp::BSubst(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option �`����Perl�݊�������u�����s��

	@param str [in] �����E�u���p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W

	@return �u������������̐�
	
	rxp->outp����rxp->outendp�ɒu����̕����񂪊i�[�����B

*/
/*!	@fn int CBregexp::BSubstEx(char* str,char *targetbeg char *target, ,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option �`����Perl�݊�������u�����s��

	@param str [in] �����E�u���p�^�[���i�R���p�C���ς݂Ȃ�NULL)
	@param target [in] �����Ώە�����i�s������j
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W

	@return �u������������̐�
	
	rxp->outp����rxp->outendp�ɒu����̕����񂪊i�[�����B

*/

/*!	@fn	int CBregexp::BTrans(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	tr/pattern/replace/option �`����Perl�݊������u�����s��

	@param str [in] �����E�u���p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W

	@return �ϊ�����������

	rxp->outp����rxp->outendp�ɕϊ���̕����񂪊i�[�����B

*/

/*!	@fn int CBregexp::BSplit(char* str,char *target,char *targetendp, int limit,BREGEXP **rxp,char *msg)

	split( /patttern/, string ) �����̕����񕪊����s��

	@param str [in] �����p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@param msg [out] �G���[���b�Z�[�W
	@param limit [in] �ő啪�����B������z�������ɂ��Ă͕����͍s��ꂸ�ɍŏI�v�f�ɓ���B

	@return ������

*/

/*!	@fn void CBregexp::BRegfree(BREGEXP* rx)

	�����֐��ɂ���ēn���ꂽBREGEXP�\���̂̉��

	@param rx [in] �������\����
*/


/*!	@fn const char* CBregexp::BRegexpVersion(void)

	BREGEXP.DLL�̃o�[�W�����ԍ���Ԃ��B
	@return �o�[�W����������ւ̃|�C���^�B

	@par Sample
	Version: Bregexp.dll V1.1 Build 22 Apr 29 2000 21:13:19
*/

#endif
/*[EOF]*/
