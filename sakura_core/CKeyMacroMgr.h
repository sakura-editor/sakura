//	$Id$
/*!	@file
	@brief �L�[�{�[�h�}�N��

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CKEYMACROMGR_H_
#define _CKEYMACROMGR_H_

#include <windows.h>

/*
struct KeyMacroData {
	int		m_nFuncID;
	LPARAM	m_lParam1;
};
*/

//#define MAX_STRLEN			70
//#define MAX_KEYMACRONUM		10000
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//! �L�[�{�[�h�}�N��
class CKeyMacroMgr
{
public:
	/*
	||  Constructors
	*/
	CKeyMacroMgr();
	~CKeyMacroMgr();

	/*
	||  Attributes & Operations
	*/
	void ClearAll( void );				/* �L�[�}�N���̃o�b�t�@���N���A���� */
	void Append( int , LPARAM, class CEditView* pcEditView );		/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
	void Append( class CMacro* macro );		/* �L�[�}�N���̃o�b�t�@�Ƀf�[�^�ǉ� */
//	int GetMacroNum() { return m_nKeyMacroDataArrNum; };
	
	/* �L�[�{�[�h�}�N�����܂Ƃ߂Ď�舵�� */
	BOOL SaveKeyMacro( HINSTANCE hInstance, const char* pszPath) const;	/* CMacro�̗���A�L�[�{�[�h�}�N���ɕۑ� */
	//@@@2002.2.2 YAZAKI PPA.DLL�A��/�i�V�����̂���virtual�ɁB
	virtual void ExecKeyMacro( class CEditView* pcEditView ) const;				/* �L�[�{�[�h�}�N���̎��s */
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const char* pszPath);		/* �L�[�{�[�h�}�N����ǂݍ��݁ACMacro�̗�ɕϊ� */
	
	/* �L�[�{�[�h�}�N�����ǂݍ��ݍς݂��m�F���� */
	BOOL IsReady(){ return m_nReady; }

protected:
//	int				m_nKeyMacroDataArrNum;
	BOOL			m_nReady;	//	Load�ς݂��ǂ�����\���t���O TRUE...Load�ς݁AFALSE...��Load�B

	class CMacro*	m_pTop;	//	�擪�ƏI�[��ێ�
	class CMacro*	m_pBot;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CKEYMACROMGR_H_ */


/*[EOF]*/
