//	$Id$
/*!	@file
	�w��s�ւ̃W�����v�_�C�A���O�{�b�N�X

	@author Norio Nakatani
	@date 1998/05/31 �쐬
	@date 1999/12/05 �č쐬
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

class CDlgJump;

#ifndef _CDLGJUMP_H_
#define _CDLGJUMP_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class SAKURA_CORE_API CDlgJump : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgJump();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM, BOOL );	/* ���[�_���_�C�A���O�̕\�� */

	int			m_nLineNum;		/* �s�ԍ� */
	BOOL		m_bPLSQL;		/* PL/SQL�\�[�X�̗L���s�� */
	int			m_nPLSQL_E1;
	int			m_nPLSQL_E2;
	BOOL		m_bLineNumIsCRLF;	/* �s�ԍ��̕\�� FALSE=�܂�Ԃ��P�ʁ^TRUE=���s�P�� */
protected:
	/*
	||  �����w���p�֐�
	*/
	BOOL OnNotify( WPARAM,  LPARAM );	//	Oct. 6, 2000 JEPRO added for Spin control
	BOOL OnCbnSelChange( HWND, int );
	BOOL OnBnClicked( int );
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGJUMP_H_ */


/*[EOF]*/
