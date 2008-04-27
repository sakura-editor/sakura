/*!	@file
	@brief �t�@���N�V�����L�[�E�B���h�E

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, YAZAKI, aroka
	Copyright (C) 2006, aroka
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CFuncKeyWnd;

#ifndef _CFUNCKEYWND_H_
#define _CFUNCKEYWND_H_

#include "CWnd.h"
#include "CShareData.h"
class CEditDoc; // 2002/2/10 aroka

//! �t�@���N�V�����L�[�E�B���h�E
//	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
class SAKURA_CORE_API CFuncKeyWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CFuncKeyWnd();
	virtual ~CFuncKeyWnd();
	/*
	|| �����o�֐�
	*/
	HWND Open( HINSTANCE, HWND, CEditDoc*, bool );	/* �E�B���h�E �I�[�v�� */
	void Close( void );	/* �E�B���h�E �N���[�Y */
	void SizeBox_ONOFF( bool );	/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
	void Timer_ONOFF( bool ); /* �X�V�̊J�n�^��~ 20060126 aroka */
	/*
	|| �����o�ϐ�
	*/
private:
	// 20060126 aroka ���ׂ�Private�ɂ��āA�����������ɍ��킹�ĕ��בւ�
	const TCHAR*	m_pszClassName;	/*!< �N���X�� */
	CEditDoc*		m_pCEditDoc;
	DLLSHAREDATA*	m_pShareData;
	int				m_nCurrentKeyState;
	WCHAR			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	HFONT			m_hFont;	/*!< �\���p�t�H���g */
	bool			m_bSizeBox;
	HWND			m_hwndSizeBox;
	int				m_nTimerCount;
	int				m_nButtonGroupNum; // Open�ŏ�����
	EFunctionCode	m_nFuncCodeArr[12]; // Open->CreateButtons�ŏ�����
protected:
	/*
	|| �����w���p�n
	*/
	void CreateButtons( void );	/* �{�^���̐��� */
	int CalcButtonSize( void );	/* �{�^���̃T�C�Y���v�Z */

	/* ���z�֐� */
	virtual void AfterCreateWindow( void ){}	// �E�B���h�E�쐬��̏���	// 2007.03.13 ryoji �������Ȃ�

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );	// WM_TIMER�^�C�}�[�̏���
	virtual LRESULT OnCommand( HWND, UINT, WPARAM, LPARAM );	// WM_COMMAND����
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );// WM_SIZE����
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );// WM_DESTROY����
};


///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCKEYWND_H_ */



