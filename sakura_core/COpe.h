//	$Id$
/*!	@file
	@brief �ҏW����v�f

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

//#include "CMemory.h"// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka

/*!
	@brief �ҏW����v�f
	
	Undo�̂��߂ɂɑ���菇���L�^���邽�߂ɗp����B
	1�I�u�W�F�N�g���P�̑����\���B
*/
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
