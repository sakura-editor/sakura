/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_
#define SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_

//2007.10.23 kobake �쐬

#include "util/design_template.h"
class CEditDoc;
class CEditWnd;
class CLoadAgent;
class CSaveAgent;
class CVisualProgress;
class CMruListener;
class CSMacroMgr;
class CPropertyManager;
class CGrepAgent;
enum EFunctionCode;
#include "CSoundSet.h"
#include "CImageListMgr.h"

//!�G�f�B�^�����A�v���P�[�V�����N���X�BCNormalProcess1�ɂ��A1���݁B
// 2007.10.30 kobake OnHelp_MenuItem��CEditWnd���玝���Ă���
// 2007.10.30 kobake �֐����ύX: OnHelp_MenuItem��ShowFuncHelp
class CEditApp : public TSingleInstance<CEditApp>{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CEditApp(HINSTANCE hInst, int);
	virtual ~CEditApp();

	//���W���[�����
	HINSTANCE GetAppInstance() const{ return m_hInst; }	//!< �C���X�^���X�n���h���擾

	//�E�B���h�E���
	CEditWnd* GetWindow(){ return m_pcEditWnd; }		//!< �E�B���h�E�擾

	//�p�X���
	LPCTSTR GetHelpFilePath() const;					//!< �w���v�t�@�C���̃t���p�X��Ԃ�

	//�w���v�֘A
	void ShowFuncHelp( HWND, EFunctionCode ) const;					//!< ���j���[�A�C�e���ɑΉ�����w���v��\��

	CEditDoc&		GetDocument(){ return *m_pcEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }


public:
	HINSTANCE			m_hInst;

	//�h�L�������g
	CEditDoc*			m_pcEditDoc;

	//�E�B���h�E
	CEditWnd*			m_pcEditWnd;

	//IO�Ǘ�
	CLoadAgent*			m_pcLoadAgent;
	CSaveAgent*			m_pcSaveAgent;
	CVisualProgress*	m_pcVisualProgress;

	//���̑��w���p
	CMruListener*		m_pcMruListener;		//MRU�Ǘ�
	CSMacroMgr*			m_pcSMacroMgr;			//�}�N���Ǘ�
	CPropertyManager*	m_pcPropertyManager;	//�v���p�e�B�Ǘ�
	CGrepAgent*			m_pcGrepAgent;			//GREP���[�h
	CSoundSet			m_cSoundSet;			//�T�E���h�Ǘ�

	//GUI�I�u�W�F�N�g
	CImageListMgr		m_cIcons;					//!< Image List
};


//WM_QUIT���o��O
class CAppExitException : public std::exception{
public:
	const char* what() const throw(){ return "CAppExitException"; }
};


#endif /* SAKURA_CEDITAPP_935DB250_AEB5_40A5_BCFF_3B72F8E3D8339_H_ */
/*[EOF]*/
