//	$Id$
/*!	@file
	@brief BREGEXP Library Handler

	Perl5�݊����K�\��������DLL�ł���BREGEXP.DLL�𗘗p���邽�߂̃C���^�[�t�F�[�X

	@author genta
	@date Jun. 10, 2001
	@date Jan. 05, 2002 genta �R�����g�ǉ�
	@date 2002/2/1 hor
	@date Jul. 25, 2002 genta �s�������̍l����ǉ�

	$Revision$
*/
/*
	Copyright (C) 2001-2002, genta
	Copyright (C) 2001, novice, hor

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
*/
class SAKURA_CORE_API CBregexp : public CDllHandler {
public:
	CBregexp();
	virtual ~CBregexp();

	const char* GetVersion(){		//!< DLL�̃o�[�W���������擾
		return IsAvailable() ? BRegexpVersion() : "";
	}

	//	CJre�G�~�����[�V�����֐�
	//!	�����p�^�[���̃R���p�C��
	bool Compile(const char *szPattern, int bOption );
	bool GetMatchInfo(const char*target, int len, int nStart, BREGEXP**rep);
	// 2002/01/19 novice ���K�\���ɂ�镶����u��
	// 2002.01.26 hor    �u���㕶�����ʈ�����
	// 2002.02.01 hor    �啶���������𖳎�����I�v�V�����ǉ�
	bool Replace(const char* szPattern0, const char* szPattern1, char *target, int len, char **out, int bOption );
	//>> 2002/03/27 Azumaiya ���K�\���u���ɃR���p�C���֐����g���`����ǉ�
	bool CompileReplace(const char *szPattern0, const char *szPattern1, int bOption );
	bool GetReplaceInfo(char *szTarget, int nLen, char **pszOut, int *pnOutLen);
	//<< 2002/03/27 Azumaiya

	//! BREGEXP���b�Z�[�W���擾����
	const char* GetLastMessage(void) const { return m_szMsg; }

protected:
	//	Jul. 5, 2001 genta �C���^�[�t�F�[�X�ύX�ɔ��������ǉ�
	virtual char* GetDllName(char*);
	virtual int InitDll(void);
	virtual int DeinitDll(void);

	//	DLL Interface�̎󂯎M
	typedef int (*BREGEXP_BMatch)(const char*,const char *,const char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSubst)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BTrans)(const char*,char *,char *,BREGEXP **,char *);
	typedef int (*BREGEXP_BSplit)(const char*,char *,char *,int,BREGEXP **,char *);
	typedef void (*BREGEXP_BRegfree)(BREGEXP*);
	typedef const char* (*BREGEXP_BRegexpVersion)(void);

	BREGEXP_BMatch BMatch;
	BREGEXP_BSubst BSubst;
	BREGEXP_BTrans BTrans;
	BREGEXP_BSplit BSplit;
	BREGEXP_BRegfree BRegfree;
	BREGEXP_BRegexpVersion BRegexpVersion;

	//!	�R���p�C���o�b�t�@���������
	/*!
		m_sRep��BRegfree()�ɓn���ĉ������D������NULL�ɃZ�b�g����D
		���XNULL�Ȃ牽�����Ȃ�
	*/
	void ReleaseCompileBuffer(void){
		if( m_sRep ){
			BRegfree( m_sRep );
			m_sRep = NULL;
		}
		m_bTop = false;	//	Jul, 25, 2002 genta
	}

private:
	//	�����֐�

	//!	���E�I��
	//int ChooseBoundary(const char* str1, const char* str2 = NULL );

	//	�����o�ϐ�
	BREGEXP*	m_sRep;	//!< �R���p�C���\����
	/*! true: �s�����񂠂�, false: �s������Ȃ�
		@date 2002.07.25 genta
	*/
	bool		m_bTop;
	char		m_szMsg[80];		//!< BREGEXP����̃��b�Z�[�W��ێ�����
};

//	�ȉ��͊֐��|�C���^�ɓǂݍ��܂��֐��̉��
/*!	@fn int CBregexp::BMatch(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	m/pattern/option �`����Perl�݊��p�^�[���}�b�`���O���s���B

	@param str [in] ��������p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@pararm msg [out] �G���[���b�Z�[�W

	target <= p < targetendp �͈̔͂������ΏۂɂȂ�B
*/

/*!	@fn int CBregexp::BSubst(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg);

	s/pattern/replace/option �`����Perl�݊�������u�����s��

	@param str [in] �����E�u���p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@pararm msg [out] �G���[���b�Z�[�W

	@return �u������������̐�
	
	rxp->outp����rxp->outendp�ɒu����̕����񂪊i�[�����B

*/

/*!	@fn	int CBregexp::BTrans(char* str,char *target,char *targetendp, BREGEXP **rxp,char *msg)

	tr/pattern/replace/option �`����Perl�݊������u�����s��

	@param str [in] �����E�u���p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@pararm msg [out] �G���[���b�Z�[�W

	@return �ϊ�����������

	rxp->outp����rxp->outendp�ɕϊ���̕����񂪊i�[�����B

*/

/*!	@fn int CBregexp::BSplit(char* str,char *target,char *targetendp, int limit,BREGEXP **rxp,char *msg)

	split( /patttern/, string ) �����̕����񕪊����s��

	@param str [in] �����p�^�[��
	@param target [in] �����Ώۗ̈�擪
	@param targetendp [in] �����Ώۗ̈斖��
	@param rxp [out] BREGEXP�\���́B���ʂ͂�������擾����B
	@pararm msg [out] �G���[���b�Z�[�W
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
