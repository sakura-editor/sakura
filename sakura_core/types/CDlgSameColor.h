/*!	@file
	@brief �����F�^�w�i�F����_�C�A���O

	@author ryoji
	@date 2006/04/26 �쐬
*/
/*
	Copyright (C) 2006, ryoji

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

#ifndef SC_CDLGSAMECOLOR_H__
#define SC_CDLGSAMECOLOR_H__

#include "dlg/CDialog.h"
#include "CShareData.h"

/*!	@brief �����F�^�w�i�F����_�C�A���O

	�^�C�v�ʐݒ�̃J���[�ݒ�ŁC�����F�^�w�i�F����̑ΏېF���w�肷�邽�߂ɕ⏕�I��
	�g�p�����_�C�A���O�{�b�N�X
*/
class CDlgSameColor : public CDialog
{
public:
	CDlgSameColor();
	~CDlgSameColor();
	int DoModal( HINSTANCE, HWND, WORD, STypeConfig*, COLORREF );		//!< ���[�_���_�C�A���O�̕\��

protected:

	virtual LPVOID GetHelpIdTable( void );
	virtual INT_PTR DispatchEvent( HWND, UINT, WPARAM, LPARAM );	//! �_�C�A���O�̃��b�Z�[�W����
	virtual BOOL OnInitDialog( HWND, WPARAM, LPARAM );			//!< WM_INITDIALOG ����
	virtual BOOL OnBnClicked( int );							//!< BN_CLICKED ����
	virtual BOOL OnDrawItem( WPARAM wParam, LPARAM lParam );	//!< WM_DRAWITEM ����
	BOOL OnSelChangeListColors( HWND hwndCtl );					//!< �F�I�����X�g�� LBN_SELCHANGE ����

	static LRESULT CALLBACK ColorStatic_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< �T�u�N���X�����ꂽ�w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W��
	static LRESULT CALLBACK ColorList_SubclassProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );	//!< �T�u�N���X�����ꂽ�F�I�����X�g�̃E�B���h�E�v���V�[�W��

	WNDPROC m_wpColorStaticProc;	//!< �T�u�N���X���ȑO�̎w��F�X�^�e�B�b�N�̃E�B���h�E�v���V�[�W��
	WNDPROC m_wpColorListProc;		//!< �T�u�N���X���ȑO�̐F�I�����X�g�̃E�B���h�E�v���V�[�W��

	WORD m_wID;			//!< �^�C�v�ʐݒ�_�C�A���O�i�e�_�C�A���O�j�ŉ����ꂽ�{�^��ID
	STypeConfig* m_pTypes;	//!< �^�C�v�ʐݒ�f�[�^
	COLORREF m_cr;		//!< �w��F
};

#endif


