//	$Id$
/************************************************************************

	COpe.cpp

	�ҏW����v�f
	Copyright (C) 1998-2000, Norio Nakatani

    CREATE: 1998/6/9  �V�K�쐬

************************************************************************/
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
