//	$Id$
/*!	@file
	�ҏW����v�f

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

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �ҏW����v�f COpe */
class COpe {
	public:
		COpe();		/* COpe�N���X�\�z */
		~COpe();	/* COpe�N���X���� */

		void DUMP( void );	/* �ҏW����v�f�̃_���v */

		int		m_nOpe;			/* ������ */

//- 1999.12.22 �������H��
//-		int		m_nCaretPosX_Before;	/* ����O�̃L�����b�g�ʒu�w */
//-		int		m_nCaretPosY_Before;	/* ����O�̃L�����b�g�ʒu�x */
//-		int		m_nCaretPosX_To;		/* ����O�̃L�����b�g�ʒu�w To */
//-		int		m_nCaretPosY_To;		/* ����O�̃L�����b�g�ʒu�x To */
//-		int		m_nCaretPosX_After; 	/* �����̃L�����b�g�ʒu�w */
//-		int		m_nCaretPosY_After; 	/* �����̃L�����b�g�ʒu�x */

		int		m_nCaretPosX_PHY_Before;	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
		int		m_nCaretPosY_PHY_Before;	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j */
		int		m_nCaretPosX_PHY_To;		/* ����O�̃L�����b�g�ʒu�w To ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
		int		m_nCaretPosY_PHY_To;		/* ����O�̃L�����b�g�ʒu�x To ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
		int		m_nCaretPosX_PHY_After;		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
		int		m_nCaretPosY_PHY_After;		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j */



		int		m_nDataLen;					/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		CMemory*	m_pcmemData;			/* ����Ɋ֘A����f�[�^ */

	public:
	private:
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */


/*[EOF]*/
