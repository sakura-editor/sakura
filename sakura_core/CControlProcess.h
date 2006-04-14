/*!	@file
	@brief �R���g���[���v���Z�X�N���X�w�b�_�t�@�C��

	@author aroka
	@date	2002/01/08 �쐬
*/
/*
	Copyright (C) 2002, aroka �V�K�쐬, YAZAKI
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CCONTROLPROCESS_H_
#define _CCONTROLPROCESS_H_

#include "global.h"
#include "CShareData.h"
#include "CProcess.h"

class CEditApp;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �R���g���[���v���Z�X�N���X
	
	�R���g���[���v���Z�X��CEditApp�N���X�̃C���X�^���X�����B
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CControlProcess : public CProcess {
public:
	CControlProcess( HINSTANCE hInstance, LPSTR lpCmdLine ) : 
		CProcess( hInstance, lpCmdLine ),
		m_pcEditApp( 0 ),
		// 2006.04.10 ryoji �����I�u�W�F�N�g�̃n���h����������
		m_hMutex( NULL ),
		m_hMutexCP( NULL ),
		m_hEventCPInitialized( NULL )
	{}

	static INT_PTR CALLBACK ExitingDlgProc(
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
	virtual ~CControlProcess();
protected:
	CControlProcess();
	virtual bool Initialize();
	virtual bool MainLoop();
	virtual void Terminate();

private:
	HANDLE			m_hMutex;
	HANDLE			m_hMutexCP;
	HANDLE			m_hEventCPInitialized;	// �R���g���[���v���Z�X�����������C�x���g 2006.04.10 ryoji
	CEditApp*		m_pcEditApp;
};


///////////////////////////////////////////////////////////////////////
#endif /* _CCONTROLPROCESS_H_ */

/*[EOF]*/
