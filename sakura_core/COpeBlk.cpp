//	$Id$
/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include "COpeBlk.h"
#include "debug.h"



/* COpeBlk�N���X�\�z */
COpeBlk::COpeBlk()
{
	Init();
	return;
}

void COpeBlk::Init( void )
{
	m_nCOpeArrNum = 0;	/* ����̐� */
	m_ppCOpeArr = NULL;	/* ����̔z�� */
}

void COpeBlk::Empty( void )
{
	int		i;
	/* ����̔z����폜���� */
	if( 0 < m_nCOpeArrNum && NULL != m_ppCOpeArr ){
		for( i = 0; i < m_nCOpeArrNum; ++i ){
			if( NULL != m_ppCOpeArr[i] ){
				delete m_ppCOpeArr[i];
				m_ppCOpeArr[i] = NULL;
			}
		}
		free( m_ppCOpeArr );
		m_ppCOpeArr = NULL;
		m_nCOpeArrNum = 0;
	}
	return;
}



/* COpeBlk�N���X���� */
COpeBlk::~COpeBlk()
{
	Empty();

}





/* ����̒ǉ� */
int COpeBlk::AppendOpe( COpe* pcOpe/*, CLayoutMgr* pCLayoutMgr*/ )
{
	if( -1 == pcOpe->m_nCaretPosX_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_Before	/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosX_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�擪����̃o�C�g���i�O�J�n�j*/
	 || -1 == pcOpe->m_nCaretPosY_PHY_After		/* �J�[�\���ʒu ���s�P�ʍs�̍s�ԍ��i�O�J�n�j*/
	){
//		MessageBox( 0, "COpeBlk::AppendOpe() error", "�o�O", MB_OK );
		MYMESSAGEBOX( NULL, MB_OK | MB_ICONSTOP | MB_TOPMOST, GSTR_APPNAME,
			"COpeBlk::AppendOpe() error.\n �o�O\n pcOpe->m_nCaretPosX_PHY_Before = %d\npcOpe->m_nCaretPosY_PHY_Before = %d\npcOpe->m_nCaretPosX_PHY_After = %d\npcOpe->m_nCaretPosY_PHY_After = %d\n",
			pcOpe->m_nCaretPosX_PHY_Before,
			pcOpe->m_nCaretPosY_PHY_Before,
			pcOpe->m_nCaretPosX_PHY_After,
			pcOpe->m_nCaretPosY_PHY_After
		);
	}
//	/*
//	  �J�[�\���ʒu�ϊ�
//	  ���C�A�E�g�ʒu(�s������̕\�����ʒu�A�܂�Ԃ�����s�ʒu)
//	  ��
//	  �����ʒu(�s������̃o�C�g���A�܂�Ԃ������s�ʒu)
//	*/
//	int		nX;
//	int		nY;
//	pCLayoutMgr->CaretPos_Log2Phys(
//		pcOpe->m_nCaretPosX_Before,
//		pcOpe->m_nCaretPosY_Before,
//		&pcOpe->m_nCaretPosX_PHY_Before,
//		&pcOpe->m_nCaretPosY_PHY_Before
//	);
//	pCLayoutMgr->CaretPos_Log2Phys(
//		pcOpe->m_nCaretPosX_After,
//		pcOpe->m_nCaretPosY_After,
//		&pcOpe->m_nCaretPosX_PHY_After,
//		&pcOpe->m_nCaretPosY_PHY_After
//	);


	/* �z��̃������T�C�Y�𒲐� */
	if( 0 == m_nCOpeArrNum ){
		m_ppCOpeArr = (COpe**)malloc( sizeof( COpe* ) );
	}else{
		m_ppCOpeArr = (COpe**)realloc( (void*)m_ppCOpeArr,  sizeof( COpe* ) * (m_nCOpeArrNum + 1 ) );
	}
	if( NULL == m_ppCOpeArr ){
		MessageBox( 0, "COpeBlk::AppendOpe() error", "�������m�ۂɎ��s���܂����B\n���Ɋ댯�ȏ�Ԃł��B", MB_OK );
		return FALSE;
	}
	m_ppCOpeArr[m_nCOpeArrNum] = pcOpe;
	m_nCOpeArrNum++;
	return TRUE;
}





/* �����Ԃ� */
COpe* COpeBlk::GetOpe( int nIndex )
{
	if( m_nCOpeArrNum <= nIndex ){
		return NULL;
	}
	return m_ppCOpeArr[nIndex];
}


/* �ҏW����v�f�u���b�N�̃_���v */
void COpeBlk::DUMP( void )
{
#ifdef _DEBUG
	int i;
	for( i = 0; i < m_nCOpeArrNum; ++i ){
		MYTRACE( "\tCOpeBlk.m_ppCOpeArr[%d]----\n", i );
		m_ppCOpeArr[i]->DUMP();
	}
#endif
	return;
}


/*[EOF]*/
