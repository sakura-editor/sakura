//	$Id$
/*!	@file
	�����f�[�^1�s

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

#include "CDocLine.h"

CDocLine::CDocLine()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pLine = NULL;
//	m_nType = 0;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	m_bModify = TRUE;	/* �ύX�t���O */
//	m_enumEOLType = EOL_NONE;	/* ���s�R�[�h�̎�� */
//	m_nEOLLen = gm_pnEolLenArr[EOL_NONE];
//	m_nModifyCount = 32000;	/* �ύX�� */
	return;
}


CDocLine::~CDocLine()
{
	if( NULL != m_pLine ){
		delete m_pLine;
		m_pLine = NULL;
	}
	return;
}


/*[EOF]*/
