//	$Id$
/*!	@file
	@brief �E�B���h�E�̊�{�N���X

	@author Norio Nakatani
	@date 2000/01/11 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 2000-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
class CWnd;

#ifndef _CWND_H_
#define _CWND_H_

#include <windows.h>
#include "global.h"
extern CWnd* gm_pCWnd;

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
//!	�E�B���h�E�̊�{�N���X
/*!
	@par CWnd�N���X�̊�{�I�ȋ@�\
	@li �E�B���h�E�쐬
	@li �E�B���h�E���b�Z�[�W�z��

	@par ����?�̃E�B���h�E�̎g�p���@�͈ȉ��̎菇
	@li Init()		������
	@li RegisterWC()	�E�B���h�E�N���X�o�^
	@li Create()		�E�B���h�E�쐬
*/
class SAKURA_CORE_API CWnd
{
public:
	/* Constructors */
	CWnd();
	virtual ~CWnd();
	/*
	||  Attributes & Operations
	*/
protected: // 2002/2/10 aroka �A�N�Z�X���ύX
	char		m_szClassInheritances[1024];
	HINSTANCE	m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND		m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
public:
	HWND		m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	void Init( HINSTANCE, HWND );/* ������ */
	ATOM RegisterWC( HICON, HICON, HCURSOR, HBRUSH, LPCTSTR, LPCTSTR );/* �E�B���h�E�N���X�쐬 */
	HWND Create( DWORD, LPCTSTR, LPCTSTR, DWORD, int,  int, int, int, HMENU );/* �쐬 */
	virtual LRESULT DispatchEvent( HWND, UINT, WPARAM, LPARAM );/* ���b�Z�[�W�z�� */
protected:
	/* ���z�֐� */
	virtual LRESULT DispatchEvent_WM_APP( HWND, UINT, WPARAM, LPARAM );/* �A�v���P�[�V������`�̃��b�Z�[�W(WM_APP <= msg <= 0xBFFF) */
	virtual void PreviCreateWindow( void ){return;};/* �E�B���h�E�쐬�O�̏���(�N���X�o�^�O) ( virtual )*/
	virtual void AfterCreateWindow( void ){::ShowWindow( m_hWnd, SW_SHOW );}/* �E�B���h�E�쐬��̏��� ( virtual )*/

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	virtual LRESULT OnNcDestroy( HWND, UINT, WPARAM, LPARAM );// WM_NCDESTROY

	/* ���z�֐� ���b�Z�[�W����(�f�t�H���g����) */
	#define DECLH(method) LRESULT method( HWND hwnd, UINT msg, WPARAM wp, LPARAM lp ){return CallDefWndProc( hwnd, msg, wp, lp );}
	virtual DECLH( OnCreate			);	// WM_CREATE
	virtual DECLH( OnCommand		);	// WM_COMMAND
	virtual DECLH( OnPaint			);	// WM_PAINT
	virtual DECLH( OnLButtonDown	);	// WM_LBUTTONDOWN
	virtual DECLH( OnLButtonUp		);	// WM_LBUTTONUP
	virtual DECLH( OnLButtonDblClk	);	// WM_LBUTTONDBLCLK
	virtual DECLH( OnRButtonDown	);	// WM_RBUTTONDOWN
	virtual DECLH( OnRButtonUp		);	// WM_RBUTTONUP
	virtual DECLH( OnRButtonDblClk	);	// WM_RBUTTONDBLCLK
	virtual DECLH( OnMButtonDown	);	// WM_MBUTTONDOWN
	virtual DECLH( OnMButtonUp		);	// WM_MBUTTONUP
	virtual DECLH( OnMButtonDblClk	);	// WM_MBUTTONDBLCLK
	virtual DECLH( OnMouseMove		);	// WM_MOUSEMOVE
	virtual DECLH( OnMouseWheel		);	// WM_MOUSEWHEEL
	virtual DECLH( OnTimer			);	// WM_TIMER
	virtual DECLH( OnSize			);	// WM_SIZE
	virtual DECLH( OnMove			);	// WM_MOVE
	virtual DECLH( OnClose			);	// WM_CLOSE
	virtual DECLH( OnDestroy		);	// WM_DSESTROY
	virtual DECLH( OnQueryEndSession);	// WM_QUERYENDSESSION

	virtual DECLH( OnMeasureItem	);	// WM_MEASUREITEM
	virtual DECLH( OnMenuChar		);	// WM_MENUCHAR
	virtual DECLH( OnNotify			);	// WM_NOTIFY	//@@@ 2003.05.31 MIK
	virtual DECLH( OnDrawItem		);	// WM_DRAWITEM	// 2006.02.01 ryoji

//	virtual DECLH( OnNcCreate			);	// WM_NCCREATE
//	virtual DECLH( OnNcCalcSize			);	// WM_NCCALCSIZE
//	virtual DECLH( OnNcHitTest			);	// WM_NCHITTEST
//	virtual DECLH( OnNcPaint			);	// WM_NCPAINT
//	virtual DECLH( OnNcActivate			);	// WM_NCACTIVATE
//	virtual DECLH( OnNcMouseMove		);	// WM_NCMOUSEMOVE
//	virtual DECLH( OnNcLButtonDown		);	// WM_NCLBUTTONDOWN
//	virtual DECLH( OnNcLButtonUp		);	// WM_NCLBUTTONUP
//	virtual DECLH( OnNcLButtonDblClk	);	// WM_NCLBUTTONDBLCLK
//	virtual DECLH( OnNcRButtonDown		);	// WM_NCRBUTTONDOWN
//	virtual DECLH( OnNcRButtonUp		);	// WM_NCRBUTTONUP
//	virtual DECLH( OnNcRButtonDblClk	);	// WM_NCRBUTTONDBLCLK
//	virtual DECLH( OnNcMButtonDown		);	// WM_NCMBUTTONDOWN
//	virtual DECLH( OnNcMButtonUp		);	// WM_NCMBUTTONUP
//	virtual DECLH( OnNcMButtonDblClk	);	// WM_NCMBUTTONDBLCLK


	/* MDI�p */
	virtual DECLH( OnMDIActivate	);	// WM_MDIACTIVATE
//	virtual DECLH( OnMDICascade		);	// WM_MDICASCADE
//	virtual DECLH( OnMDICreate		);	// WM_MDICREATE
//	virtual DECLH( OnMDIDestroy		);	// WM_MDIDESTROY
//	virtual DECLH( OnMDIGetActive	);	// WM_MDIGETACTIVE
//	virtual DECLH( OnMDIIconArrange	);	// WM_MDIICONARRANGE
//	virtual DECLH( OnMDIMaximize	);	// WM_MDIMAXIMIZE
//	virtual DECLH( OnMDINext		);	// WM_MDINEXT
//	virtual DECLH( OnMDIRefreshMenu	);	// WM_MDIREFRESHMENU
//	virtual DECLH( OnMDIRestore		);	// WM_MDIRESTORE
//	virtual DECLH( OnMDISetMenu		);	// WM_MDISETMENU
//	virtual DECLH( OnMDITile		);	// WM_MDITILE


	/* �f�t�H���g���b�Z�[�W���� */
	virtual LRESULT CallDefWndProc( HWND, UINT, WPARAM, LPARAM );

};

///////////////////////////////////////////////////////////////////////
#endif /* _CWND_H_ */


/*[EOF]*/
