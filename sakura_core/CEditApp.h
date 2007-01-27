/*!	@file
	@brief �풓��

	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2001/06/03 N.Nakatani grep�P��P�ʂŌ�������������Ƃ��̂��߂ɃR�}���h���C���I�v�V�����̏����ǉ�
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_





#include <windows.h>
#include "CShareData.h"
#include "CMenuDrawer.h"
#include "CDlgGrep.h" // 2002/2/10 aroka
#include "CImageListMgr.h" // 2002/2/10 aroka

//!	�풓���̊Ǘ�
/*!
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CEditApp
{
public:
	/*
	||  Constructors
	*/
	CEditApp();
	~CEditApp();

	/*
	|| �����o�֐�
	*/
	HWND Create( HINSTANCE );	/* �쐬 */
	bool CreateTrayIcon( HWND );	// 20010412 by aroka
	LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* ���b�Z�[�W���� */
	void MessageLoop( void );	/* ���b�Z�[�W���[�v */
	void OnDestroy( void );		/* WM_DESTROY ���� */	// 2006.07.09 ryoji
	int	CreatePopUpMenu_L( void );	/* �|�b�v�A�b�v���j���[(�g���C���{�^��) */
	int	CreatePopUpMenu_R( void );	/* �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */

	static bool OpenNewEditor( HINSTANCE, HWND, const char*, int, BOOL, bool sync = false, const char* szCurDir = NULL );		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
	static bool OpenNewEditor2( HINSTANCE, HWND , const FileInfo*, BOOL, bool sync = false );	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 1 */

	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX	// 2006.12.25 ryoji �����ǉ�
	static void TerminateApplication( HWND hWndFrom );	/* �T�N���G�f�B�^�̑S�I�� */	// 2006.12.25 ryoji �����ǉ�

	/*
	|| �����o�ϐ�
	*/

private:
	CMenuDrawer		m_CMenuDrawer;
	bool			m_bUseTrayMenu;	//�g���C���j���[�\����
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	BOOL			m_bCreatedTrayIcon;	/*!< �g���C�ɃA�C�R��������� */

	DLLSHAREDATA*	m_pShareData;
	CDlgGrep		m_cDlgGrep; // Jul. 2, 2001 genta

	CImageListMgr	m_hIcons;

	void	DoGrep();	//Stonee, 2001/03/21
#if 0
	//	Apr. 6, 2001 genta �R�}���h���C���I�v�V�����̉��
	static int CheckCommandLine( char *str, char** arg );
#endif
	//	Apr. 24, 2001 genta
	/*!	RegisterMessage�œ�����Message ID�̕ۊǏꏊ */
	UINT	m_uCreateTaskBarMsg;

	/*
	|| �����w���p�n
	*/
protected:
	BOOL TrayMessage(HWND , DWORD , UINT , HICON , const char* );	/*!< �^�X�N�g���C�̃A�C�R���Ɋւ��鏈�� */
	void OnCommand( WORD , WORD  , HWND );	/*!< WM_COMMAND���b�Z�[�W���� */
	void OnNewEditor( void ); //!< 2003.05.30 genta �V�K�E�B���h�E�쐬������؂�o��

	static INT_PTR CALLBACK ExitingDlgProc(	/*!< �I���_�C�A���O�p�v���V�[�W�� */	// 2006.07.02 ryoji CControlProcess ����ړ�
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */


/*[EOF]*/
