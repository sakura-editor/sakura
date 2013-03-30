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

class COpeBuf;

#ifndef _COPEBUF_H_
#define _COPEBUF_H_


#include <vector>
#include "_main/global.h"
class COpeBlk;/// 2002/2/10 aroka




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �A���h�D�E���h�D�o�b�t�@
*/
class COpeBuf {
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	COpeBuf();
	~COpeBuf();

	//���
	bool IsEnableUndo();						//!< Undo�\�ȏ�Ԃ�
	bool IsEnableRedo();						//!< Redo�\�ȏ�Ԃ�
	int GetCurrentPointer( void ) const { return m_nCurrentPointer; }	/* ���݈ʒu��Ԃ� */	// 2007.12.09 ryoji

	//����
	void ClearAll();							//!< �S�v�f�̃N���A
	bool AppendOpeBlk( COpeBlk* pcOpeBlk );		//!< ����u���b�N�̒ǉ�
	void SetNoModified();						//!< ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm

	//�g�p
	COpeBlk* DoUndo( bool* pbModified );		//!< ���݂�Undo�Ώۂ̑���u���b�N��Ԃ�
	COpeBlk* DoRedo( bool* pbModified );		//!< ���݂�Redo�Ώۂ̑���u���b�N��Ԃ�

	//�f�o�b�O
	void DUMP();								//!< �ҏW����v�f�u���b�N�̃_���v

private:
	std::vector<COpeBlk*>	m_vCOpeBlkArr;		//!< ����u���b�N�̔z��
	int						m_nCurrentPointer;	//!< ���݈ʒu
	int						m_nNoModifiedIndex;	//!< ���ύX�ȏ�ԂɂȂ����ʒu
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPEBUF_H_ */



