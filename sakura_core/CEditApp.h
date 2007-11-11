//2007.10.23 kobake �쐬

#pragma once

#include "util/design_template.h"
class CEditWnd;
enum EFunctionCode;
#include "CEditDoc.h"
#include "CImageListMgr.h"

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

	CEditDoc&		GetDocument(){ return m_cEditDoc; }
	CImageListMgr&	GetIcons(){ return m_cIcons; }


public:
	HINSTANCE		m_hInst;

	//�E�B���h�E
	CEditWnd*		m_pcEditWnd;

	//�h�L�������g
	CEditDoc		m_cEditDoc;

	//�w���p
	CImageListMgr	m_cIcons;					//!< Image List
};
