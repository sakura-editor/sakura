/*!	@file
	@brief �G�f�B�^�v���Z�X�N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CNORMALPROCESS_H_
#define _CNORMALPROCESS_H_

#include "global.h"
#include "CProcess.h"
#include "CMigemo.h"
#include "CEditApp.h"
#include "util/design_template.h"
class CEditWnd;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �G�f�B�^�v���Z�X�N���X
	
	�G�f�B�^�v���Z�X��CEditWnd�N���X�̃C���X�^���X�����B
*/
class SAKURA_CORE_API CNormalProcess : public CProcess {
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CNormalProcess( HINSTANCE hInstance, LPCTSTR lpCmdLine );
	virtual ~CNormalProcess();

protected:
	//�v���Z�X�n���h��
	virtual bool InitializeProcess();
	virtual bool MainLoop();
	virtual void OnExitProcess();

protected:
	//�����⏕
	HANDLE _GetInitializeMutex() const; // 2002/2/8 aroka

private:
	CEditApp*	m_pcEditApp;	//2007.10.23 kobake
};


///////////////////////////////////////////////////////////////////////
#endif /* _CNORMALPROCESS_H_ */


