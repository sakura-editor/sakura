/*!	@file
	@brief �풓��

	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�

	@author Norio Nakatani
	@date 1998/05/13 �V�K�쐬
	@date 2001/06/03 N.Nakatani grep�P��P�ʂŌ�������������Ƃ��̂��߂ɃR�}���h���C���I�v�V�����̏����ǉ�
	@date 2007/10/23 kobake     �N���X���A�t�@�C�����ύX: CControlTray��CControlTray
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, Stonee, aroka, genta
	Copyright (C) 2002, MIK, YAZAKI, aroka
	Copyright (C) 2003, genta
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_

#include <Windows.h>
#include "CMenuDrawer.h"
#include "dlg/CDlgGrep.h" // 2002/2/10 aroka
#include "CImageListMgr.h" // 2002/2/10 aroka

struct SLoadInfo;
struct EditInfo;
struct DLLSHAREDATA;

//!	�풓���̊Ǘ�
/*!
	�^�X�N�g���C�A�C�R���̊Ǘ��C�^�X�N�g���C���j���[�̃A�N�V�����C
	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ��Ȃ�
	
	@date 2002.2.17 YAZAKI CShareData�̃C���X�^���X�́ACProcess�ɂЂƂ���̂݁B
*/
class SAKURA_CORE_API CControlTray
{
public:
	/*
	||  Constructors
	*/
	CControlTray();
	~CControlTray();

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

	//�E�B���h�E�Ǘ�
	static bool OpenNewEditor(							//!< �V�K�ҏW�E�B���h�E�̒ǉ� ver 0
		HINSTANCE			hInstance,					//!< [in] �C���X�^���XID (���͖��g�p)
		HWND				hWndParent,					//!< [in] �e�E�B���h�E�n���h���D�G���[���b�Z�[�W�\���p
		const SLoadInfo&	sLoadInfo,					//!< [in]
		const TCHAR*		szCmdLineOption	= NULL,		//!< [in] �ǉ��̃R�}���h���C���I�v�V����
		bool				sync			= false,	//!< [in] true�Ȃ�V�K�G�f�B�^�̋N���܂őҋ@����
		const TCHAR*		szCurDir		= NULL,		//!< [in] �V�K�G�f�B�^�̃J�����g�f�B���N�g��
		bool				bNewWindow		= false		//!< [in] �V�K�G�f�B�^���E�C���h�E�ŊJ��
	);
	static bool OpenNewEditor2(						//!< �V�K�ҏW�E�B���h�E�̒ǉ� ver 1
		HINSTANCE		hInstance,
		HWND			hWndParent,
		const EditInfo*	pfi,
		bool			bViewMode,
		bool			sync		= false,
		bool			bNewWindow	= false
	);
	static void ActiveNextWindow();
	static void ActivePrevWindow();

	static BOOL CloseAllEditor( BOOL bCheckConfirm, HWND hWndFrom, BOOL bExit, int nGroup );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX	// 2006.12.25, 2007.02.13 ryoji �����ǉ�
	static void TerminateApplication( HWND hWndFrom );	/* �T�N���G�f�B�^�̑S�I�� */	// 2006.12.25 ryoji �����ǉ�

public:
	HWND GetTrayHwnd() const{ return m_hWnd; }

	/*
	|| �����w���p�n
	*/
	static void DoGrepCreateWindow(HINSTANCE hinst, HWND, CDlgGrep& cDlgGrep);
protected:
	void	DoGrep();	//Stonee, 2001/03/21
	BOOL TrayMessage(HWND , DWORD , UINT , HICON , const TCHAR* );	/*!< �^�X�N�g���C�̃A�C�R���Ɋւ��鏈�� */
	void OnCommand( WORD , WORD  , HWND );	/*!< WM_COMMAND���b�Z�[�W���� */
	void OnNewEditor( bool ); //!< 2003.05.30 genta �V�K�E�B���h�E�쐬������؂�o��

	static INT_PTR CALLBACK ExitingDlgProc(	/*!< �I���_�C�A���O�p�v���V�[�W�� */	// 2006.07.02 ryoji CControlProcess ����ړ�
		HWND	hwndDlg,	// handle to dialog box
		UINT	uMsg,		// message
		WPARAM	wParam,		// first message parameter
		LPARAM	lParam		// second message parameter
	);


	/*
	|| �����o�ϐ�
	*/
private:
	CMenuDrawer		m_CMenuDrawer;
	bool			m_bUseTrayMenu;			//�g���C���j���[�\����
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	BOOL			m_bCreatedTrayIcon;		//!< �g���C�ɃA�C�R���������

	DLLSHAREDATA*	m_pShareData;
	CDlgGrep		m_cDlgGrep;				// Jul. 2, 2001 genta
	int				m_nCurSearchKeySequence;

	CImageListMgr	m_hIcons;

	UINT			m_uCreateTaskBarMsg;	//!< RegisterMessage�œ�����Message ID�̕ۊǏꏊ�BApr. 24, 2001 genta
};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */



