//	$Id$
/*!	@file
	@brief �v���Z�X���N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
	$Revision$
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESS_H_
#define _CPROCESS_H_

#include "global.h"

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �v���Z�X���N���X
*/
class SAKURA_CORE_API CProcess {
public:
	CProcess( HINSTANCE hInstance, LPSTR lpCmdLine );
	bool Run();
	virtual ~CProcess(){};
protected:
	CProcess();
	virtual bool Initialize() = 0;
	virtual bool MainLoop() = 0;
	virtual void Terminate() = 0;
	
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	LPSTR m_CommandLine;

private:
};


///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESS_H_ */

/*[EOF]*/
