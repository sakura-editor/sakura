//	$Id$
/*!	@file
	�A�E�g���C�����  �f�[�^�v�f
	
	@author Norio Nakatani
	@date	1998/06/23 �쐬
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

class CFuncInfo;

#ifndef _CFUNCINFO_H_
#define _CFUNCINFO_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �A���h�D�E���h�D�o�b�t�@ */
class CFuncInfo {
	public:
		CFuncInfo( int, int, char*, int );	/* CFuncInfo�N���X�\�z */
		~CFuncInfo();	/* CFuncInfo�N���X���� */

//	private:
		int			m_nFuncLineCRLF;	/* �֐��̂���s(CRLF�P��) */
		int			m_nFuncLineLAYOUT;	/* �֐��̂���s(�܂�Ԃ��P��) */
		CMemory		m_cmemFuncName;	/* �֐��� */
		int			m_nInfo;		/* �t����� */
};



///////////////////////////////////////////////////////////////////////
#endif /* _CFUNCINFO_H_ */

/*[EOF]*/
