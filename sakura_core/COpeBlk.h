//	$Id$
/*!	@file
	�ҏW����v�f�u���b�N
	
	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
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

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

#include <windows.h>
#include "COpe.h"
#include "CLayoutMgr.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �ҏW����v�f�u���b�N�@COpeBlk */
class COpeBlk {
	public:
		COpeBlk();	/* COpeBlk�N���X�\�z */
		~COpeBlk();	/* COpeBlk�N���X���� */

		int GetNum( void ){ return m_nCOpeArrNum; };	/* ����̐���Ԃ� */
//		int AppendOpe( COpe*, CLayoutMgr* );	/* ����̒ǉ� */
		int AppendOpe( COpe* );	/* ����̒ǉ� */
		COpe* GetOpe( int );	/* �����Ԃ� */

		void DUMP( void );	/* �ҏW����v�f�u���b�N�̃_���v */
	private:
		void Init( void );
		void Empty( void );

		/* �f�[�^ */
		int		m_nCOpeArrNum;	/* ����̐� */
		COpe**	m_ppCOpeArr;	/* ����̔z�� */
};



//////////////////////////////////////////////////////////////////////12
#endif /* _COPEBLK_H_ */

/*[EOF]*/
