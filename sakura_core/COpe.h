//	$Id$
/************************************************************************

	COpe.h

	�ҏW����v�f
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  �V�K�쐬

************************************************************************/

class COpe;

#ifndef _COPE_H_
#define _COPE_H_

#include "CMemory.h"




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/* �ҏW����v�f�@COpe */
class COpe {
	public:
	   COpe();	/* COpe�N���X�\�z */
	   ~COpe();	/* COpe�N���X���� */

		void DUMP( void );	/* �ҏW����v�f�̃_���v */

		int		m_nOpe;			/* ������ */

//- 1999.12.22 �������H��		
//-		int		m_nCaretPosX_Before;	/* ����O�̃L�����b�g�ʒu�w */
//-		int		m_nCaretPosY_Before;	/* ����O�̃L�����b�g�ʒu�x */
//-		int		m_nCaretPosX_To;	/* ����O�̃L�����b�g�ʒu�w To */
//-		int		m_nCaretPosY_To;	/* ����O�̃L�����b�g�ʒu�x To */
//-		int		m_nCaretPosX_After; 	/* �����̃L�����b�g�ʒu�w */
//-		int		m_nCaretPosY_After; 	/* �����̃L�����b�g�ʒu�x */

		int		m_nCaretPosX_PHY_Before;	/* �J�[�\���ʒu�@���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
		int		m_nCaretPosY_PHY_Before;	/* �J�[�\���ʒu�@���s�P�ʍs�̍s�ԍ��i�O�J�n�j */
		int		m_nCaretPosX_PHY_To;	/* ����O�̃L�����b�g�ʒu�w To ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
		int		m_nCaretPosY_PHY_To;	/* ����O�̃L�����b�g�ʒu�x To ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
		int		m_nCaretPosX_PHY_After;		/* �J�[�\���ʒu�@���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j */
		int		m_nCaretPosY_PHY_After;		/* �J�[�\���ʒu�@���s�P�ʍs�̍s�ԍ��i�O�J�n�j */
	   

		
		int		m_nDataLen;				/* ����Ɋ֘A����f�[�^�̃T�C�Y */
		CMemory*	m_pcmemData;			/* ����Ɋ֘A����f�[�^ */

	public:
	private:
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPE_H_ */

/*[EOF]*/
