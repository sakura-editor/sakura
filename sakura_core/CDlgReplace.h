//	$Id$
/*!	@file
	�u���_�C�A���O

	@author Norio Nakatani
	@date 1998/10/02  �V�K�쐬
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

class CDlgReplace;

#ifndef _CDLGREPLACE_H_
#define _CDLGREPLACE_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgReplace : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgReplace();
	/*
	||  Attributes & Operations
	*/
	HWND DoModeless( HINSTANCE, HWND, LPARAM, BOOL );	/* ���[�_���_�C�A���O�̕\�� */
	void ChangeView( LPARAM );	/* ���[�h���X���F�u���E�����ΏۂƂȂ�r���[�̕ύX */

	int				m_bLoHiCase;	/* �p�啶���Ɖp����������ʂ��� */
	int				m_bWordOnly;	/* ��v����P��̂݌������� */
	int				m_bRegularExp;	/* ���K�\�� */
	char			m_szText[_MAX_PATH + 1];	/* ���������� */
	char			m_szText2[_MAX_PATH + 1];	/* �u���㕶���� */
	BOOL			m_bSelectedArea;	/* �I��͈͓��u�� */
	int				m_bNOTIFYNOTFOUND;				/* �����^�u��  ������Ȃ��Ƃ����b�Z�[�W��\�� */
	int				m_nSettingType;
	BOOL			m_bSelected;	/* �e�L�X�g�I�𒆂� */
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );

	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGREPLACE_H_ */


/*[EOF]*/
