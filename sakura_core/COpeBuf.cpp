/*!	@file
	@brief �A���h�D�E���h�D�o�b�t�@

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/
#include "StdAfx.h"
#include "COpeBuf.h"
#include "COpeBlk.h"// 2002/2/10 aroka


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               �R���X�g���N�^�E�f�X�g���N�^                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* COpeBuf�N���X�\�z */
COpeBuf::COpeBuf()
{
	m_nCurrentPointer = 0;	/* ���݈ʒu */
	m_nNoModifiedIndex = 0;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
}

/* COpeBuf�N���X���� */
COpeBuf::~COpeBuf()
{
	/* ����u���b�N�̔z����폜���� */
	int size = (int)m_vCOpeBlkArr.size();
	for( int i = 0; i < size; ++i ){
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ���                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* Undo�\�ȏ�Ԃ� */
bool COpeBuf::IsEnableUndo()
{
	return 0 < m_vCOpeBlkArr.size() && 0 < m_nCurrentPointer;
}

/* Redo�\�ȏ�Ԃ� */
bool COpeBuf::IsEnableRedo()
{
	return 0 < m_vCOpeBlkArr.size() && m_nCurrentPointer < (int)m_vCOpeBlkArr.size();
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ����̒ǉ� */
bool COpeBuf::AppendOpeBlk( COpeBlk* pcOpeBlk )
{
	/* ���݈ʒu�����i�A���h�D�Ώہj������ꍇ�́A���� */
	int size = (int)m_vCOpeBlkArr.size();
	if( m_nCurrentPointer < size ){
		for( int i = m_nCurrentPointer; i < size; ++i ){
			SAFE_DELETE(m_vCOpeBlkArr[i]);
		}
		m_vCOpeBlkArr.resize(m_nCurrentPointer);
	}
	/* �z��̃������T�C�Y�𒲐� */
	m_vCOpeBlkArr.push_back(pcOpeBlk);
	m_nCurrentPointer++;
	return true;
}

/* �S�v�f�̃N���A */
void COpeBuf::ClearAll()
{
	/* ����u���b�N�̔z����폜���� */
	int size = (int)m_vCOpeBlkArr.size();
	for( int i = 0; i < size; ++i ){
		SAFE_DELETE(m_vCOpeBlkArr[i]);
	}
	m_vCOpeBlkArr.clear();
	m_nCurrentPointer = 0;	/* ���݈ʒu */
	m_nNoModifiedIndex = 0;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
}

/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */
void COpeBuf::SetNoModified()
{
	m_nNoModifiedIndex = m_nCurrentPointer;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           �g�p                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
COpeBlk* COpeBuf::DoUndo( bool* pbModified )
{
	/* Undo�\�ȏ�Ԃ� */
	if( !IsEnableUndo() ){
		return NULL;
	}
	m_nCurrentPointer--;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* ���ύX�ȏ�ԂɂȂ����ʒu */
		*pbModified = false;
	}else{
		*pbModified = true;
	}
	return m_vCOpeBlkArr[m_nCurrentPointer];
}

/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
COpeBlk* COpeBuf::DoRedo( bool* pbModified )
{
	COpeBlk*	pcOpeBlk;
	/* Redo�\�ȏ�Ԃ� */
	if( !IsEnableRedo() ){
		return NULL;
	}
	pcOpeBlk = m_vCOpeBlkArr[m_nCurrentPointer];
	m_nCurrentPointer++;
	if( m_nCurrentPointer == m_nNoModifiedIndex ){		/* ���ύX�ȏ�ԂɂȂ����ʒu */
		*pbModified = false;
	}else{
		*pbModified = true;
	}
	return pcOpeBlk;
}



// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         �f�o�b�O                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/* �A���h�D�E���h�D�o�b�t�@�̃_���v */
void COpeBuf::DUMP()
{
#ifdef _DEBUG
	int i;
	MYTRACE( _T("COpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer );
	int size = (int)m_vCOpeBlkArr.size();
	for( i = 0; i < size; ++i ){
		MYTRACE( _T("COpeBuf.m_vCOpeBlkArr[%d]----\n"), i );
		m_vCOpeBlkArr[i]->DUMP();
	}
	MYTRACE( _T("COpeBuf.m_nCurrentPointer=[%d]----\n"), m_nCurrentPointer );
#endif
}



