//	$Id$
/************************************************************************
*
*	CEditApp.h
*
*	MRU�A�L�[���蓖�āA���ʐݒ�A�ҏW�E�B���h�E�̊Ǘ�
*	Copyright (C) 1998-2000, Norio Nakatani
*
*
*    CREATE: 1998/5/13
*
************************************************************************/

class CEditApp;

#ifndef _CEDITAPP_H_
#define _CEDITAPP_H_





#include <windows.h>
#include "CEditWnd.h"
#include "CKeyBind.h"
#include "CShareData.h"
#include "CMenuDrawer.h"







/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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
	int	CreatePopUpMenu_L( void );	/* �|�b�v�A�b�v���j���[(�g���C���{�^��) */
	int	CreatePopUpMenu_R( void );	/* �|�b�v�A�b�v���j���[(�g���C�E�{�^��) */

	static bool OpenNewEditor( HINSTANCE, HWND, char*, int, BOOL, bool sync = false );		/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 0 */
	static bool OpenNewEditor2( HINSTANCE, HWND , FileInfo*, BOOL, bool sync = false );	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 1 */
//�V���O���v���Z�X�ŗp
//	static HWND OpenNewEditor3( HINSTANCE, HWND , const char*, BOOL );	/* �V�K�ҏW�E�B���h�E�̒ǉ� ver 2 */

	static BOOL CloseAllEditor( void );	/* ���ׂẴE�B���h�E����� */	//Oct. 7, 2000 jepro �u�ҏW�E�B���h�E�̑S�I���v�Ƃ������������L�̂悤�ɕύX
	static void TerminateApplication( void );	/* �e�L�X�g�G�f�B�^�̑S�I�� */
	/* �R�}���h���C���̉�� */
	static void CEditApp::ParseCommandLine( 
		const char*	pszCmdLineSrc,
		BOOL*		pbGrepMode,
		CMemory*	pcmGrepKey,
		CMemory*	pcmGrepFile,
		CMemory*	pcmGrepFolder,
		BOOL*		pbGrepSubFolder,
		BOOL*		pbGrepLoHiCase,
		BOOL*		pbGrepRegularExp,
		BOOL*		pbGrepKanjiCode_AutoDetect,
		BOOL*		pbGrepOutputLine,
		int	*		pnGrepOutputStyle,
		BOOL*		pbDebugMode,
		BOOL*		pbNoWindow,
		FileInfo*	pfi,
		BOOL*		pbReadOnly
	);

	/*
	|| �����o�ϐ�
	*/
//	CKeyBind		m_CKeyBind;
//	HACCEL			m_hAccel;

private:
	CMenuDrawer		m_CMenuDrawer;
	HINSTANCE		m_hInstance;
	HWND			m_hWnd;
	char*			m_pszAppName;
	BOOL			m_bCreatedTrayIcon;	/* �g���C�ɃA�C�R��������� */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	int				m_nSettingType;

	CImageListMgr	m_hIcons;

    void    DoGrep();   //Stonee, 2001/03/21
	//	Apr. 6, 2001 genta �R�}���h���C���I�v�V�����̉��
	static int CheckCommandLine( char *str, char** arg );
	//	Apr. 24, 2001 genta
	//	RegisterMessage�œ�����Message ID�̕ۊǏꏊ
	UINT	m_uCreateTaskBarMsg;

	/*
	|| �����w���p�n
	*/
protected:
	BOOL TrayMessage(HWND , DWORD , UINT , HICON , const char* );	/* �^�X�N�g���C�̃A�C�R���Ɋւ��鏈�� */
	void OnCommand( WORD , WORD  , HWND );	/* WM_COMMAND���b�Z�[�W���� */


};


///////////////////////////////////////////////////////////////////////
#endif /* _CEDITAPP_H_ */

/*[EOF]*/

