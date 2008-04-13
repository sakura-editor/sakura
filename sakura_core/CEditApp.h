//2007.10.23 kobake �쐬

#pragma once

#include "util/design_template.h"
class CEditWnd;
class CPropertyManager;
class CMruListener;
enum EFunctionCode;
#include "doc/CEditDoc.h"
#include "CImageListMgr.h"
#include "CPropertyManager.h"
#include "macro/CSMacroMgr.h"
#include "CGrepAgent.h"
#include "CSoundSet.h"
#include "CSaveAgent.h"
#include "CLoadAgent.h"
#include "CVisualProgress.h"

//!�G�f�B�^�����A�v���P�[�V�����N���X�BCNormalProcess1�ɂ��A1���݁B
// 2007.10.30 kobake OnHelp_MenuItem��CEditWnd���玝���Ă���
// 2007.10.30 kobake �֐����ύX: OnHelp_MenuItem��ShowFuncHelp
class CEditApp : public TSingleInstance<CEditApp>{
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	CEditApp(HINSTANCE hInst);
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

