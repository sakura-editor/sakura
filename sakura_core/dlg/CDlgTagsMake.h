/*!	@file
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X

	@author MIK
	@date 2003.5.12
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

class CDlgTagsMake;

#ifndef _CDLGTAGSMAKE_H_
#define _CDLGTAGSMAKE_H_

#include "dlg/CDialog.h"
/*!
	@brief �^�O�t�@�C���쐬�_�C�A���O�{�b�N�X
*/
class CDlgTagsMake : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgTagsMake();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, const TCHAR* );	/* ���[�_���_�C�A���O�̕\�� */

	TCHAR	m_szPath[_MAX_PATH+1];	/* �t�H���_ */
	TCHAR	m_szTagsCmdLine[_MAX_PATH];	/* �R�}���h���C���I�v�V����(��) */
	int		m_nTagsOpt;				/* CTAGS�I�v�V����(�`�F�b�N) */

protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnBnClicked( int );
	LPVOID	GetHelpIdTable(void);

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

private:
	void SelectFolder( HWND hwndDlg );

};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGTAGSMAKE_H_ */


