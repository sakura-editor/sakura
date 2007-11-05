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


#include "global.h"
class COpeBlk;/// 2002/2/10 aroka




/*-----------------------------------------------------------------------
�N���X�̐錾
-----------------------------------------------------------------------*/
/*!
	@brief �A���h�D�E���h�D�o�b�t�@
*/
class SAKURA_CORE_API COpeBuf {
	public:
		COpeBuf();	/* COpeBuf�N���X�\�z */
		~COpeBuf();	/* COpeBuf�N���X���� */
		void ClearAll( void );	/* �S�v�f�̃N���A */
		int AppendOpeBlk( COpeBlk* );	/* ����u���b�N�̒ǉ� */
		int	IsEnableUndo( void );	/* Undo�\�ȏ�Ԃ� */
		int	IsEnableRedo( void );	/* Redo�\�ȏ�Ԃ� */
		COpeBlk* DoUndo( bool* );	/* ���݂�Undo�Ώۂ̑���u���b�N��Ԃ� */
		COpeBlk* DoRedo( bool* );	/* ���݂�Redo�Ώۂ̑���u���b�N��Ԃ� */
		void SetNoModified( void );	/* ���݈ʒu�Ŗ��ύX�ȏ�ԂɂȂ������Ƃ�ʒm */

		void DUMP( void );	/* �ҏW����v�f�u���b�N�̃_���v */
	private:
		int			 m_nCOpeBlkArrNum;	/* ����u���b�N�̐� */
		COpeBlk**	m_ppCOpeBlkArr;	/* ����u���b�N�̔z�� */
		int			m_nCurrentPointer;	/* ���݈ʒu */
		int			m_nNoModifiedIndex;	/* ���ύX�ȏ�ԂɂȂ����ʒu */
};



///////////////////////////////////////////////////////////////////////
#endif /* _COPEBUF_H_ */


/*[EOF]*/
