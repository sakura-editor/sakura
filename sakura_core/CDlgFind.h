//	$Id$
/*!	@file
	�����_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date	1998/12/02 �č쐬
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
#include "CDialog.h"

#ifndef _CDLGFIND_H_
#define _CDLGFIND_H_


/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgFind : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgFind();
	/*
	||  Attributes & Operations
	*/
//	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */
	HWND DoModeless( HINSTANCE, HWND, LPARAM );	/* ���[�h���X�_�C�A���O�̕\�� */
//	BOOL DispatchEvent( HWND, UINT, WPARAM, LPARAM );	/* �_�C�A���O�̃��b�Z�[�W���� */

	void ChangeView( LPARAM );

	int		m_bLoHiCase;	/* �p�啶���Ɖp����������ʂ��� */
	int		m_bWordOnly;	/* ��v����P��̂݌������� */
	int		m_bRegularExp;	/* ���K�\�� */
	int		m_bNOTIFYNOTFOUND;	/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	char	m_szText[_MAX_PATH + 1];	/* ���������� */


protected:
	void AddToSearchKeyArr( const char* );
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );
	// virtual BOOL OnKeyDown( WPARAM wParam, LPARAM lParam );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGFIND_H_ */


/*[EOF]*/
