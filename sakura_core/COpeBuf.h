//	$Id$
/*!	@file
	�A���h�D�E���h�D�o�b�t�@
	
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

class COpeBuf;

#ifndef _COPEBUF_H_
#define _COPEBUF_H_

/* �A���h�D�o�b�t�@�p�@����R�[�h */
enum enumOPECODE {
	OPE_INSERT		= 1,
	OPE_DELETE		= 2,
	OPE_MOVECARET	= 3,
};


#include "COpeBlk.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �A���h�D�E���h�D�o�b�t�@ */
class SAKURA_CORE_API COpeBuf {
	public:
		COpeBuf();	/* COpeBuf�N���X�\�z */
		~COpeBuf();	/* COpeBuf�N���X���� */
		void ClearAll( void );	/* �S�v�f�̃N���A */
		int AppendOpeBlk( COpeBlk* );	/* ����u���b�N�̒ǉ� */
		int	IsEnableUndo( void );	/* Undo�\�ȏ�Ԃ� */
		int	IsEnableRedo( void );	/* Redo�\�ȏ�Ԃ� */
		COpeBlk* DoUndo( int* );	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
		COpeBlk* DoRedo( int* );	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
		void SetNoModified( void );	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */

		void DUMP( void );	/* �ҏW����v�f�u���b�N�̃_���v */
	private:
		int			 m_nCOpeBlkArrNum;	/* ����u���b�N�̐� */
		COpeBlk**	m_ppCOpeBlkArr;	/* ����u���b�N�̔z�� */
		int			m_nCurrentPointer;	/* ���݈ʒu */
		int			m_nNoModifiedIndex;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPEBUF_H_ */

/*[EOF]*/
