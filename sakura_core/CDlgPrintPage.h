//	$Id$
/*!	@file
	����_�C�A���O�{�b�N�X

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
#include "CDialog.h"

#ifndef _CDLGPRINTPAGE_H_
#define _CDLGPRINTPAGE_H_

/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgPrintPage : public CDialog
{
public:
	/*
	||  Constructors
	*/
	CDlgPrintPage();
	/*
	||  Attributes & Operations
	*/
	int DoModal( HINSTANCE, HWND, LPARAM );	/* ���[�_���_�C�A���O�̕\�� */


	int	m_nPageMin;
	int	m_nPageMax;

	BOOL m_bAllPage;
	int	m_nPageFrom;
	int	m_nPageTo;



protected:
	/* �I�[�o�[���C�h? */
	int GetData( void );	/* �_�C�A���O�f�[�^�̎擾 */
	void SetData( void );	/* �_�C�A���O�f�[�^�̐ݒ� */
	BOOL OnBnClicked( int );


};



///////////////////////////////////////////////////////////////////////
#endif /* _CDLGPRINTPAGE_H_ */

/*[EOF]*/
