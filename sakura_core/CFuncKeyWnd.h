//	$Id$
/*!	@file
	�t�@���N�V�����L�[�E�B���h�E

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
class CFuncKeyWnd;

#ifndef _CFUNCKEYWND_H_
#define _CFUNCKEYWND_H_

#include "CWnd.h"
//#include <windows.h>
#include "CShareData.h"
#include "CEditDoc.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
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
	HWND Open( HINSTANCE, HWND, CEditDoc*, BOOL );	/* �E�B���h�E �I�[�v�� */
	void Close( void );	/* �E�B���h�E �N���[�Y */
	void SizeBox_ONOFF( BOOL );	/* �T�C�Y�{�b�N�X�̕\���^��\���؂�ւ� */
	/*
	|| �����o�ϐ�
	*/
	const char*		m_pszClassName;	/* �N���X�� */
	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
	CEditDoc*		m_pCEditDoc;
	HFONT			m_hFont;	/* �\���p�t�H���g */
	int				m_nCurrentKeyState;
	char			m_szFuncNameArr[12][256];
	HWND			m_hwndButtonArr[12];
	int				m_nFuncCodeArr[12];
	int				m_nButtonGroupNum;
	BOOL			m_bSizeBox;
	HWND			m_hwndSizeBox;
	int				m_nTimerCount;
protected:
	/*
	|| �����w���p�n
	*/
	void CreateButtons( void );	/* �{�^���̐��� */
	int CalcButtonSize( void );	/* �{�^���̃T�C�Y���v�Z */

	/* ���z�֐� */

	/* ���z�֐� ���b�Z�[�W���� �ڂ����͎������Q�� */
	virtual LRESULT OnTimer( HWND, UINT, WPARAM, LPARAM );	// WM_TIMER�^�C�}�[�̏���
	virtual LRESULT OnCommand( HWND, UINT, WPARAM, LPARAM );	// WM_COMMAND����
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );// WM_SIZE����
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );// WM_DSESTROY����
};


///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCKEYWND_H_ */


/*[EOF]*/
