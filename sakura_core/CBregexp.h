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

#include "CBregexpDll2.h"

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
class SAKURA_CORE_API CBregexp : public CBregexpDll2{
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

	//! DLL�̃o�[�W���������擾
	const TCHAR* GetVersionT(){ return IsAvailable() ? to_tchar(BRegexpVersion()) : _T(""); }

	//	CJre�G�~�����[�V�����֐�
	//!	�����p�^�[���̃R���p�C��
	// 2002/01/19 novice ���K�\���ɂ�镶����u��
	// 2002.01.26 hor    �u���㕶�����ʈ�����
	// 2002.02.01 hor    �啶���������𖳎�����I�v�V�����ǉ�
	//>> 2002/03/27 Azumaiya ���K�\���u���ɃR���p�C���֐����g���`����ǉ�
	bool Compile(const wchar_t *szPattern, int nOption = 0) {
		return Compile(szPattern, NULL, nOption);
	}
	bool Compile(const wchar_t *szPattern0, const wchar_t *szPattern1, int nOption = 0);	//!< Replace�p
	bool Match(const wchar_t *szTarget, int nLen, int nStart = 0);						//!< ���������s����
	int Replace(const wchar_t *szTarget, int nLen, int nStart = 0);					//!< �u�������s����	// 2007.01.16 ryoji �߂�l��u�����ɕύX

	//-----------------------------------------
	// 2005.03.19 ����� �N���X�������B��
	/*! @{
		@name ���ʂ𓾂郁�\�b�h��ǉ����ABREGEXP���O������B��
	*/
	/*!
	    �����Ɉ�v����������̐擪�ʒu��Ԃ�(������擪�Ȃ�0)
		@retval �����Ɉ�v����������̐擪�ʒu
	*/
	CLogicInt GetIndex(void)
	{
		return CLogicInt(m_pRegExp->startp[0] - m_szTarget);
	}
	/*!
	    �����Ɉ�v����������̎��̈ʒu��Ԃ�
		@retval �����Ɉ�v����������̎��̈ʒu
	*/
	CLogicInt GetLastIndex(void)
	{
		return CLogicInt(m_pRegExp->endp[0] - m_szTarget);
	}
	/*!
		�����Ɉ�v����������̒�����Ԃ�
		@retval �����Ɉ�v����������̒���
	*/
	CLogicInt GetMatchLen(void)
	{
		return CLogicInt(m_pRegExp->endp[0] - m_pRegExp->startp[0]);
	}
	/*!
		�u�����ꂽ������̒�����Ԃ�
		@retval �u�����ꂽ������̒���
	*/
	CLogicInt GetStringLen(void) {
		// �u���㕶���񂪂O���Ȃ� outp�Aoutendp��NULL�ɂȂ�
		// NULL�|�C���^�̈����Z�͖��Ȃ��O�ɂȂ�B
		// outendp�� '\0'�Ȃ̂ŁA�����񒷂� +1�s�v

		// Jun. 03, 2005 Karoto
		//	�u���㕶����0���̏ꍇ��outp��NULL�ł�outendp��NULL�łȂ��ꍇ������̂ŁC
		//	outp��NULL�`�F�b�N���K�v

		if (m_pRegExp->outp == NULL) {
			return CLogicInt(0);
		} else {
			return CLogicInt(m_pRegExp->outendp - m_pRegExp->outp);
		}
	}
	/*!
		�u�����ꂽ�������Ԃ�
		@retval �u�����ꂽ������ւ̃|�C���^
	*/
	const wchar_t *GetString(void)
	{
		return m_pRegExp->outp;
	}
	/*! @} */
	//-----------------------------------------

	/*! BREGEXP���b�Z�[�W���擾����
		@retval ���b�Z�[�W�ւ̃|�C���^
	*/
	const TCHAR* GetLastMessage() const;// { return m_szMsg; }

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
	bool IsLookAhead(const wchar_t *pattern) {
		CheckPattern(pattern);
		return IsLookAhead();
	}

protected:


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
	int CheckPattern( const wchar_t* szPattern );
	wchar_t* MakePatternSub( const wchar_t* szPattern, const wchar_t* szPattern2, const wchar_t* szAdd2, int nOption );
	wchar_t* MakePattern( const wchar_t* szPattern, const wchar_t* szPattern2, int nOption );

	//	�����o�ϐ�
	BREGEXP_W*			m_pRegExp;			//!< �R���p�C���\����
	int					m_ePatType;			//!< ����������p�^�[�����
	const wchar_t*		m_szTarget;			//!< �Ώە�����ւ̃|�C���^
	wchar_t				m_szMsg[80];		//!< BREGEXP_W����̃��b�Z�[�W��ێ�����

	// �ÓI�����o�ϐ�
	static const wchar_t	m_tmpBuf[2];	//!< �_�~�[������
};


//	Jun. 26, 2001 genta
//!	���K�\�����C�u�����̃o�[�W�����擾
SAKURA_CORE_API bool CheckRegexpVersion( HWND hWnd, int nCmpId, bool bShowMsg = false );
SAKURA_CORE_API bool CheckRegexpSyntax( const wchar_t* szPattern, HWND hWnd, bool bShowMessage, int nOption = -1 );// 2002/2/1 hor�ǉ�
SAKURA_CORE_API bool InitRegexp( HWND hWnd, CBregexp& rRegexp, bool bShowMessage );


#endif



