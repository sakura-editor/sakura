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

#include "COpe.h"
#include "debug.h"


/* COpe�N���X�\�z */
COpe::COpe()
{
	m_nOpe = 0;					/* ������ */
//	m_nCaretPosX_Before = 0;	/* ����O�̃L�����b�g�ʒu�w */
//	m_nCaretPosY_Before = 0;	/* ����O�̃L�����b�g�ʒu�x */
//	m_nCaretPosX_To = 0;		/* ����O�̃L�����b�g�ʒu�w To */
//	m_nCaretPosY_To = 0;		/* ����O�̃L�����b�g�ʒu�x To */
//	m_nCaretPosX_After = 0; 	/* �����̃L�����b�g�ʒu�w */
//	m_nCaretPosY_After = 0; 	/* �����̃L�����b�g�ʒu�x */

	m_nCaretPosX_PHY_To = 0;	/* ����O�̃L�����b�g�ʒu�w To ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	m_nCaretPosY_PHY_To = 0;	/* ����O�̃L�����b�g�ʒu�x To ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	m_nCaretPosX_PHY_Before = -1;	/* �J�[�\���ʒu�@���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
	m_nCaretPosY_PHY_Before = -1;	/* �J�[�\���ʒu�@���s�P�ʍs�̍s�ԍ��i�O�J�n�j */
	m_nCaretPosX_PHY_After = -1;		/* �J�[�\���ʒu�@���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
	m_nCaretPosY_PHY_After = -1;		/* �J�[�\���ʒu�@���s�P�ʍs�̍s�ԍ��i�O�J�n�j */
//	m_nOpePosX = 0;				/* ����ʒu�w */
//	m_nOpePosY = 0;				/* ����ʒu�x */
	m_nDataLen = 0;				/* ����Ɋ֘A����f�[�^�̃T�C�Y */
	m_pcmemData = NULL;			/* ����Ɋ֘A����f�[�^ */
	return;
}




/* COpe�N���X���� */
COpe::~COpe()
{
	if( NULL != m_pcmemData ){	/* ����Ɋ֘A����f�[�^ */
		delete m_pcmemData;
		m_pcmemData = NULL;
	}
	return;
}

/* �ҏW����v�f�̃_���v */
void COpe::DUMP( void )
{
#ifdef _DEBUG
	MYTRACE( "\t\tm_nOpe              = [%d]\n", m_nOpe               );
	MYTRACE( "\t\tm_nCaretPosX_PHY_Before  = [%d]\n", m_nCaretPosX_PHY_Before   );
	MYTRACE( "\t\tm_nCaretPosY_PHY_Before  = [%d]\n", m_nCaretPosY_PHY_Before   );
	MYTRACE( "\t\tm_nCaretPosX_PHY_After;  = [%d]\n", m_nCaretPosX_PHY_After   );
	MYTRACE( "\t\tm_nCaretPosY_PHY_After;  = [%d]\n", m_nCaretPosY_PHY_After   );
	MYTRACE( "\t\tm_nDataLen          = [%d]\n", m_nDataLen           );
	if( NULL == m_pcmemData ){
		MYTRACE( "\t\tm_pcmemData         = [NULL]\n" );
	}else{
		MYTRACE( "\t\tm_pcmemData         = [%s]\n", m_pcmemData->GetPtr( NULL ) );
	}
#endif
	return;
}




/*[EOF]*/
