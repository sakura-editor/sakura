/*!	@file
	@brief PPA.DLL�}�N��

	@author YAZAKI
	@date 2002�N1��26��
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, aroka
	Copyright (C) 2002, YAZAKI, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPPAMACROMGR_H_
#define _CPPAMACROMGR_H_

#include <windows.h>
#include "CKeyMacroMgr.h"
#include "mem/CMemory.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//! PPA�}�N��
class CPPAMacroMgr: public CMacroManagerBase
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
	virtual BOOL LoadKeyMacro( HINSTANCE hInstance, const TCHAR* pszPath);	/* �L�[�{�[�h�}�N����ǂݍ��݁ACMacro�̗�ɕϊ� */

	static class CPPA m_cPPA;

	// Apr. 29, 2002 genta
	static CMacroManagerBase* Creator(const TCHAR* ext);
	static void declare(void);

protected:
	CNativeW m_cBuffer;
};



///////////////////////////////////////////////////////////////////////
#endif /* _CPPAMACROMGR_H_ */



	
