/*!	@file
	@brief �E�B���h�E�̈ʒu�Ƒ傫���_�C�A���O

	@author Moca
	@date 2004/05/13 �쐬
*/
/*
	Copyright (C) 2004, Moca

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

#ifndef SC_CDLGWINPOSSIZE_H__
#define SC_CDLGWINPOSSIZE_H__

#include "dlg/CDialog.h"

/*!	@brief �ʒu�Ƒ傫���̐ݒ�_�C�A���O

	���ʐݒ�̃E�B���h�E�ݒ�ŁC�E�B���h�E�ʒu���w�肷�邽�߂ɕ⏕�I��
	�g�p�����_�C�A���O�{�b�N�X
*/
class CDlgWinSize : public CDialog
{
public:
	CDlgWinSize();
	~CDlgWinSize();
	int DoModal( HINSTANCE, HWND, EWinSizeMode&, EWinSizeMode&, int&, RECT& );	//!< ���[�_���_�C�A���O�̕\��

protected:

	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
	int  GetData( void );
	void SetData( void );
	LPVOID GetHelpIdTable( void );

	void RenewItemState( void );

private:
	EWinSizeMode	m_eSaveWinSize;	//!< �E�B���h�E�T�C�Y�̕ۑ�: 0/�f�t�H���g�C1/�p���C2/�w��
	EWinSizeMode	m_eSaveWinPos;	//!< �E�B���h�E�ʒu�̕ۑ�: 0/�f�t�H���g�C1/�p���C2/�w��
	int				m_nWinSizeType;	//!< �E�B���h�E�\�����@: 0/�W���C1/�ő剻�C2/�ŏ���
	RECT			m_rc;
};

#endif


