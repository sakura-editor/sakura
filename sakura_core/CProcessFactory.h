//	$Id$
/*!	@file
	@brief �v���Z�X�����N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
	$Revision$
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CPROCESSFACTORY_H_
#define _CPROCESSFACTORY_H_

#include "global.h"

class CProcess;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �v���Z�X�����N���X
*/
class SAKURA_CORE_API CProcessFactory {
public:
	CProcess* Create( HINSTANCE hInstance, LPSTR lpCmdLine );
protected:
private:
	bool IsValidVersion();
	bool IsStartingControlProcess();
	bool IsExistControlProcess();
	bool StartControlProcess();
};

///////////////////////////////////////////////////////////////////////
#endif /* _CPROCESSFACTORY_H_ */

/*[EOF]*/
