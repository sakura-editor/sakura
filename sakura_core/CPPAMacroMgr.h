/*!	@file
	@brief PPA.DLL�}�N��

	@author YAZAKI
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPPAMACROMGR_H_
#define _CPPAMACROMGR_H_

#include <windows.h>
#include "CKeyMacroMgr.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//! �L�[�{�[�h�}�N��
class CPPAMacroMgr : public CKeyMacroMgr
{
public:
	/*
	||  Constructors
	*/
	CPPAMacroMgr();
	~CPPAMacroMgr();

	/*
	||	PPA.DLL�ɈϏ����镔��
	*/
	virtual void ExecKeyMacro( class CEditView* pcEditView ) const;				/* �L�[�{�[�h�}�N���̎��s */
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const char* pszPath);	/* �L�[�{�[�h�}�N����ǂݍ��݁ACMacro�̗�ɕϊ� */

	static class CPPA m_cPPA;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPPAMACROMGR_H_ */


/*[EOF]*/
	