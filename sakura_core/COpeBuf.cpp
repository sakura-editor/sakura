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
#include "COpeBuf.h"
#include "debug.h"

/* COpeBuf�N���X�\�z */
COpeBuf::COpeBuf()
{
	m_nCOpeBlkArrNum = 0;	/* ����u���b�N�̐� */
	m_ppCOpeBlkArr = NULL;	/* ����u���b�N�̔z�� */
	m_nCurrentPointer = 0;	/* ���݈ʒu */
	m_nNoModifiedIndex = 0;	/* ���ύX�ȏ�ԂɂȂ����ʒu */

	return;

}




/* COpeBuf�N���X���� */
COpeBuf::~COpeBuf()
{
	int		i;
	/* ����u���b�N�̔z����폜���� */
	if( 0 >= m_nCOpeBlkArrNum && NULL != m_ppCOpeBlkArr ){
		return;
	}
	for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
		if( NULL != m_ppCOpeBlkArr[i] ){
			delete m_ppCOpeBlkArr[i];
			m_ppCOpeBlkArr[i] = NULL;
		}
	}
	free( m_ppCOpeBlkArr );
	m_ppCOpeBlkArr = NULL;
	m_nCOpeBlkArrNum = 0;
	return;
}





/* ����̒ǉ� */
int COpeBuf::AppendOpeBlk( COpeBlk* pcOpeBlk )
{
	int i;
//	pcOpeBlk->DUMP();
	/* ���݈ʒu�����i�A���h�D�Ώہj������ꍇ�́A���� */
	if( m_nCurrentPointer < m_nCOpeBlkArrNum ){
		for( i = m_nCurrentPointer; i < m_nCOpeBlkArrNum; ++i ){
			if( NULL != m_ppCOpeBlkArr[i] ){
				delete m_ppCOpeBlkArr[i];
				m_ppCOpeBlkArr[i] = NULL;
			}
		}
		m_nCOpeBlkArrNum = m_nCurrentPointer;
		/* �z��̗v�f�����[���ɂȂ����ꍇ�͔z��̃���������� */
		if( 0 == m_nCOpeBlkArrNum ){
			free( m_ppCOpeBlkArr );
			m_ppCOpeBlkArr = NULL;
		}
	}
	/* �z��̃������T�C�Y�𒲐� */
	if( 0 == m_nCOpeBlkArrNum ){
		m_ppCOpeBlkArr = (COpeBlk**)malloc( sizeof( COpeBlk* ) );
	}else{
		m_ppCOpeBlkArr = (COpeBlk**)realloc( (void*)m_ppCOpeBlkArr,  sizeof( COpeBlk* ) * (m_nCOpeBlkArrNum + 1 ) );
	}
	if( NULL == m_ppCOpeBlkArr ){
		::MessageBox( 0, "COpeBlk::AppendOpe() error", "�������m�ۂɎ��s���܂����B\n���Ɋ댯�ȏ�Ԃł��B", MB_OK );
		return FALSE;
	}
	m_ppCOpeBlkArr[m_nCOpeBlkArrNum] = pcOpeBlk;
	m_nCOpeBlkArrNum++;
	m_nCurrentPointer++;
	return TRUE;
}





/* Undo�\�ȏ�Ԃ� */
int COpeBuf::IsEnableUndo( void )
{
	if( 0 < m_nCOpeBlkArrNum && 0 < m_nCurrentPointer ){
		return TRUE;
	}else{
		return FALSE;
	}
}





/* Redo�\�ȏ�Ԃ� */
int	COpeBuf::IsEnableRedo( void )
{
	if( 0 < m_nCOpeBlkArrNum && m_nCurrentPointer < m_nCOpeBlkArrNum ){
		return TRUE;
	}else{
		return FALSE;
	}
}





/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
COpeBlk* COpeBuf::DoUndo( int* pbModified )
{
	/* Undo�\�ȏ�Ԃ� */
	if( !IsEnableUndo() ){
		return FALSE;
	}
	m_nCurrentPointer--;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* ���ύX�ȏ�ԂɂȂ����ʒu */
		*pbModified = FALSE;
	}else{
		*pbModified = TRUE;
	}
	return m_ppCOpeBlkArr[m_nCurrentPointer];
}





/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
COpeBlk* COpeBuf::DoRedo( int* pbModified )
{
	COpeBlk*	pcOpeBlk;
	/* Redo�\�ȏ�Ԃ� */
	if( !IsEnableRedo() ){
		return FALSE;
	}
	pcOpeBlk = m_ppCOpeBlkArr[m_nCurrentPointer];
	m_nCurrentPointer++;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* ���ύX�ȏ�ԂɂȂ����ʒu */
		*pbModified = FALSE;
	}else{
		*pbModified = TRUE;
	}
	return pcOpeBlk;
}



/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
void COpeBuf::SetNoModified( void )
{
	m_nNoModifiedIndex = m_nCurrentPointer;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
	return;
}


/* �A���h�D�E���h�D�o�b�t�@�̃_���v */
void COpeBuf::DUMP( void )
{
#ifdef _DEBUG
	int i;
	MYTRACE( "COpeBuf.m_nCurrentPointer=[%d]----\n", m_nCurrentPointer );
	for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
		MYTRACE( "COpeBuf.m_ppCOpeBlkArr[%d]----\n", i );
		m_ppCOpeBlkArr[i]->DUMP();
	}
	MYTRACE( "COpeBuf.m_nCurrentPointer=[%d]----\n", m_nCurrentPointer );
#endif
	return;
}


/* �S�v�f�̃N���A */
void COpeBuf::ClearAll( void )
{
	int		i;
	/* ����u���b�N�̔z����폜���� */
	if( 0 < m_nCOpeBlkArrNum && NULL != m_ppCOpeBlkArr ){
		for( i = 0; i < m_nCOpeBlkArrNum; ++i ){
			if( NULL != m_ppCOpeBlkArr[i] ){
				delete m_ppCOpeBlkArr[i];
				m_ppCOpeBlkArr[i] = NULL;
			}
		}
		free( m_ppCOpeBlkArr );
		m_ppCOpeBlkArr = NULL;
		m_nCOpeBlkArrNum = 0;
	}
	m_nCurrentPointer = 0;	/* ���݈ʒu */
	m_nNoModifiedIndex = 0;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
	return;
}

/*[EOF]*/
