//	$Id$
/*!	@file
	@brief �^�u�E�B���h�E

	@author MIK
	$Revision$
*/
/*
	Copyright (C) 2003, MIK

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

class CTabWnd;

#ifndef _CTABWND_H_
#define _CTABWND_H_

#include "CWnd.h"
#include "CEditDoc.h"
#include "CShareData.h"

//! �t�@���N�V�����L�[�E�B���h�E
class SAKURA_CORE_API CTabWnd : public CWnd
{
public:
	/*
	||  Constructors
	*/
	CTabWnd();
	virtual ~CTabWnd();

	/*
	|| �����o�֐�
	*/
	HWND Open( HINSTANCE, HWND );		/*!< �E�B���h�E �I�[�v�� */
	void Close( void );					/*!< �E�B���h�E �N���[�Y */
	void TabWindowNotify( WPARAM wParam, LPARAM lParam );
	void ForceActiveWindow( HWND hwnd );
	void TabWnd_ActivateFrameWindow( HWND hwnd );

	LRESULT TabWndDispatchEvent( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	/*
	|| �����o�ϐ�
	*/
	LPCTSTR			m_pszClassName;	/*!< �N���X�� */
	DLLSHAREDATA*	m_pShareData;	/*!< ���L�f�[�^ */
	HFONT			m_hFont;		/*!< �\���p�t�H���g */
	HWND			m_hwndTab;		/*!< �^�u�R���g���[�� */
	HWND			m_hwndToolTip;	/*!< �c�[���`�b�v */
	wchar_t			m_szTextWide[1024];
	char			m_szTextAnsi[1024];

protected:
	/*
	|| �����w���p�n
	*/
	void CreateTabButtons( void );	/*!< �{�^���̐��� */

	void Refresh( void );
	int FindTabIndexByHWND( HWND hWnd );
	void ShowHideWindow( HWND hwnd, BOOL bDisp );
	int GetFirstOpenedWindow( void );

	/* ���z�֐� ���b�Z�[�W���� */
	virtual LRESULT OnSize( HWND, UINT, WPARAM, LPARAM );		/*!< WM_SIZE���� */
	virtual LRESULT OnDestroy( HWND, UINT, WPARAM, LPARAM );	/*!< WM_DSESTROY���� */
	virtual LRESULT OnNotify( HWND, UINT, WPARAM, LPARAM );		/*!< WM_NOTIFY���� */
};

#endif /* _CTABWND_H_ */

/*[EOF]*/
