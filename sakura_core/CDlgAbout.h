//	$Id$
/*!	@file
	�o�[�W�������_�C�A���O
	
	@author Norio Nakatani
	@date 1998/05/22 �쐬
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
/************************************************************************

	CDlgAbout.h


    UPDATE: 1999.12/05 �č쐬
    CREATE: 1998.05/22 �V�K�쐬
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

class CDlgAbout;

#ifndef _CDLGABOUT_H_
#define _CDLGABOUT_H_

#include "CDialog.h"
/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
class CDlgAbout : public CDialog
{
public:
	int DoModal( HINSTANCE, HWND );	/* ���[�_���_�C�A���O�̕\�� */
	//	Nov. 7, 2000 genta	�W���ȊO�̃��b�Z�[�W��ߑ�����
	BOOL DispatchEvent( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam );
protected:
	BOOL OnInitDialog( HWND, WPARAM, LPARAM );
	BOOL OnBnClicked( int );
private:
	int	 nCursorState;
};

///////////////////////////////////////////////////////////////////////
#endif /* _CDLGABOUT_H_ */

/*[EOF]*/
