//	$Id$
/*!	@file
	�t�@�C���I�[�v���_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/08/10 �쐬
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

class CDlgOpenFile;

#ifndef _CDLGOPENFILE_H_
#define _CDLGOPENFILE_H_

#include <windows.h>
#include "CShareData.h"
#include "CEol.h"


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgOpenFile
{
public:
	/*
	||  Constructors
	*/
	CDlgOpenFile();
	~CDlgOpenFile();
	/*
	||  Attributes & Operations
	*/
	void Create( HINSTANCE, HWND, const char*, const char*, const char**,const char** );
	BOOL DoModal_GetOpenFileName( char* );	/* �J���_�C�A���O ���[�_���_�C�A���O�̕\�� */
	BOOL DoModal_GetSaveFileName( char* );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */
	BOOL DoModalOpenDlg( char* , int*, BOOL* );	/* �J���_�C�A�O ���[�_���_�C�A���O�̕\�� */
	//	Feb. 9, 2001 genta	�����ǉ�
	BOOL DoModalSaveDlg( char* , int*, CEOL* );	/* �ۑ��_�C�A���O ���[�_���_�C�A���O�̕\�� */

	BOOL DispatchEvent(	HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	HINSTANCE		m_hInstance;	/* �A�v���P�[�V�����C���X�^���X�̃n���h�� */
	HWND			m_hwndParent;	/* �I�[�i�[�E�B���h�E�̃n���h�� */
	HWND			m_hWnd;			/* ���̃_�C�A���O�̃n���h�� */

	CShareData		m_cShareData;
	DLLSHAREDATA*	m_pShareData;
//	int				m_nSettingType;

	char			m_szDefaultWildCard[_MAX_PATH + 1];	/* �u�J���v�ł̍ŏ��̃��C���h�J�[�h */
	char			m_szInitialDir[_MAX_PATH + 1];		/* �u�J���v�ł̏����f�B���N�g�� */
	OPENFILENAME	m_ofn;							/* �u�t�@�C�����J���v�_�C�A���O�p�\���� */
	int				m_nCharCode;					/* �����R�[�h */
//	char			m_szHelpFile[_MAX_PATH + 1];
//	int				m_nHelpTopicID;
	CEOL			m_cEol;	//	Feb. 9, 2001 genta
	bool			m_bUseEol;	//	Feb. 9, 2001 genta

protected:
	/*
	||  �����w���p�֐�
	*/
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGOPENFILE_H_ */


/*[EOF]*/
