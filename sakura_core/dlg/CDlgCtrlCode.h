/*!	@file
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X

	@author MIK
	@date 2002.6.2
*/
/*
	Copyright (C) 2002, MIK

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

class CDlgCtrlCode;

#ifndef _CDLGCTRLCODE_H_
#define _CDLGCTRLCODE_H_

#include "dlg/CDialog.h"
/*!
	@brief �R���g���[���R�[�h���̓_�C�A���O�{�b�N�X
*/
//2007.10.18 kobake GetCharCode()���쐬�B
class CDlgCtrlCode : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgCtrlCode();

	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */

	wchar_t GetCharCode() const{ return m_nCode; } //!< �I�����ꂽ�����R�[�h���擾

private:
	/*
	||  �����w���p�֐�
	*/
	BOOL	OnInitDialog( HWND, WPARAM wParam, LPARAM lParam );
	BOOL	OnBnClicked( int );
	BOOL	OnNotify( WPARAM wParam, LPARAM lParam );
	LPVOID	GetHelpIdTable( void );

	void	SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int		GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */

private:
	/*
	|| �����o�ϐ�
	*/
	wchar_t		m_nCode;	// �R�[�h
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGCTRLCODE_H_ */


