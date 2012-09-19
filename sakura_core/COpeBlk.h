/*!	@file
	@brief �ҏW����v�f�u���b�N

	@author Norio Nakatani
	@date 1998/06/09 �V�K�쐬
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

class COpeBlk;

#ifndef _COPEBLK_H_
#define _COPEBLK_H_

#include "COpe.h"
#include <vector>



/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �ҏW����v�f�u���b�N
	
	COpe �𕡐����˂邽�߂̂��́BUndo, Redo�͂��̃u���b�N�P�ʂōs����B
*/
class COpeBlk {
public:
	//�R���X�g���N�^�E�f�X�g���N�^
	COpeBlk();
	~COpeBlk();

	//�C���^�[�t�F�[�X
	int GetNum() const{ return (int)m_ppCOpeArr.size(); }	//!< ����̐���Ԃ�
	bool AppendOpe( COpe* pcOpe );							//!< ����̒ǉ�
	COpe* GetOpe( int nIndex );								//!< �����Ԃ�
	void AddRef() { m_refCount++; }	//!< �Q�ƃJ�E���^����
	int Release() { return m_refCount > 0 ? --m_refCount : 0; }	//!< �Q�ƃJ�E���^����

	//�f�o�b�O
	void DUMP();									//!< �ҏW����v�f�u���b�N�̃_���v

private:
	//�����o�ϐ�
	std::vector<COpe*>	m_ppCOpeArr;	//!< ����̔z��

	//�Q�ƃJ�E���^
	//�@HandleCommand������ċA�I��HandleCommand���Ă΂��ꍇ�A
	//  ������HandleCommand�I������COpeBlk���j������Č㑱�̏����ɉe�����o��̂�h�����߁A
	//�@�Q�ƃJ�E���^��p���Ĉ�ԊO����HandleCommand�I�����̂�COpeBlk��j������B
	//�@COpeBlk��new�����Ƃ���AddRef()����̂���@�����A���Ȃ��Ă��g����B
	int m_refCount;
};



//////////////////////////////////////////////////////////////////////12
#endif /* _COPEBLK_H_ */



